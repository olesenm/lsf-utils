/*---------------------------------*- C++ -*---------------------------------*\
Copyright 2011 Mark Olesen
-------------------------------------------------------------------------------
License
    This file is part of lsf-utils.

    lsf-utils is free software: you can redistribute it and/or modify it under
    the terms of the GNU General Public License as published by the
    Free Software Foundation, either version 3 of the License,
    or (at your option) any later version.

    lsf-utils is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
    or FITNESS FOR A PARTICULAR PURPOSE.
    See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with lsf-utils. If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "markutil/HttpCore.hpp"

#include <ctime>
#include <clocale>
#include <cstring>
#include <string>
#include <sstream>
#include <vector>

#include <sys/stat.h>


// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

const std::string markutil::HttpCore::nullString;


// * * * * * * * * * * * * * Static Member Functions * * * * * * * * * * * * //

inline const std::string& hexEncode(char ch)
{
    static char hex[] = "0123456789ABCDEF";
    static std::string buf('%', 3);

    buf[0] = '%';
    buf[1] = hex[((ch >> 4) & 0x0F)];
    buf[2] = hex[(ch & 0x0F)];

    return buf;
}


// date in RFC1123 format
// eg
//     Sun, 06 Nov 1994 08:49:37 GMT
std::string markutil::HttpCore::timestring(const time_t& timestamp)
{
    char buf[30];
    char *prev = setlocale(LC_TIME, "C"); // english locale
    strftime(buf, 30, "%a, %d %b %Y %H:%M:%S GMT", ::gmtime(&timestamp));
    setlocale(LC_TIME, prev);             // restore locale
    return buf;
}

std::string markutil::HttpCore::timestring()
{
    time_t now = ::time(NULL);
    return timestring(now);
}


const std::string& markutil::HttpCore::lookupMime(const std::string& ext)
{
    static RawHeaderType lookup;

    // populate lookup table on the first call
    if (lookup.empty())
    {
        // text
        lookup["css"]  = "text/css";
        lookup["htm"]  = "text/html";
        lookup["html"] = "text/html";
        lookup["txt"]  = "text/plain";
        lookup["xml"]  = "text/xml";
        lookup["xsl"]  = "text/xsl";
        lookup["xhtml"] = "application/xhtml+xml";

        // image
        lookup["gif"]  = "image/gif";
        lookup["ico"]  = "image/x-icon";
        lookup["jpeg"] = "image/jpeg";
        lookup["jpg"]  = "image/jpeg";
        lookup["png"]  = "image/png";

        // application
        lookup["gz"]   = "application/x-gzip";
        lookup["pdf"]  = "application/pdf";
        lookup["tar"]  = "application/x-tar";
        lookup["zip"]  = "application/x-zip-compressed";
    }

    if (ext.empty())
    {
        return nullString;
    }

    RawHeaderType::const_iterator iter = lookup.find(ext);
    if (iter == lookup.end())
    {
        return nullString;
    }
    else
    {
        return iter->second;
    }
}


std::string& markutil::HttpCore::httpAppendUrl(std::string& url, char ch)
{
    switch (ch)
    {
        case '\x20' :
            url += '+';
            break;

        case '#' :   // fragment
        case '%' :   // percent must be encoded
        case '&' :   // field separator
        case '+' :   // encoded space
        case ';' :   // field separator
        case '=' :   // field=value
        case '?' :   // query
            url += hexEncode(ch);
            break;

        default :
            if (ch > 32 && ch < 127)
            {
                url += ch;
            }
            else
            {
                url += hexEncode(ch);
            }
            break;
    }

    return url;
}


std::string& markutil::HttpCore::httpAppendUrl
(
    std::string& url,
    const std::string& str
)
{
    for
    (
        std::string::const_iterator iter = str.begin();
        iter != str.end();
        ++iter
    )
    {
        httpAppendUrl(url, *iter);
    }

    return url;
}


std::string markutil::HttpCore::httpDecodeUrl
(
    const std::string& str,
    size_t pos,
    size_t n
)
{
    std::string out;

    if (n == std::string::npos)
    {
        n = str.size() - pos;
    }

    // max this many characters, after hex-dcoding there may be few
    if (n > 0)
    {
        out.reserve(n);
    }

    int nhex = -1;         // count of hex digits
    int dehex = 0;         // decoded hex value

    for (; n && pos < str.size(); ++pos, --n)
    {
        char ch = str[pos];

        if (ch == '%')
        {
            dehex = nhex = 0;
        }
        else if (ch == '+')
        {
            out += ' ';
            nhex = -1;
        }
        else if (nhex >= 0)
        {
            dehex =
            (
                (dehex * 16)
              + (isdigit(ch) ? (ch - '0') : (toupper(ch) - 'A' + 10))
            );

            if (++nhex == 2)
            {
                out += static_cast<char>(dehex);
                dehex = 0;
                nhex = -1;
            }
        }
        else
        {
            out += ch;
            nhex = -1;
        }
    }

    return out;
}


std::string& markutil::HttpCore::httpNormalizePath(std::string& path)
{
    if (path.empty())
    {
        path = '/';
        return path;
    }

    const bool trailing = *(path.rbegin()) == '/';

    // gather segments
    std::vector<std::string> segments;
    std::string seg;

    for
    (
        std::string::const_iterator iter = path.begin();
        iter != path.end();
        ++iter
    )
    {
        if (*iter == '/')
        {
            if (!seg.empty())    // avoids duplicate slashes
            {
                if (seg == "..")
                {
                    if (!segments.empty())
                    {
                        segments.pop_back();
                    }
                }
                else if (seg != ".")
                {
                    segments.push_back(seg);
                }
                seg.clear();
            }
        }
        else
        {
            seg += *iter;
        }
    }

    if (!seg.empty())    // avoids duplicate slashes
    {
        if (seg == "..")
        {
            if (!segments.empty())
            {
                segments.pop_back();
            }
        }
        else if (seg != ".")
        {
            segments.push_back(seg);
        }
        seg.clear();
    }


    path = '/';
    for
    (
        std::vector<std::string>::const_iterator iter = segments.begin();
        iter != segments.end();
        ++iter
    )
    {
        path += *iter;
        path += '/';
    }

    // final trailing slash as required
    if (path.size() > 1 && !trailing)
    {
        path.resize(path.size() - 1);
    }

    return path;
}


std::ostream& markutil::HttpCore::xmlEscapeChars
(
    std::ostream& os,
    const std::string& str
)
{
    for (std::size_t pos = 0; pos < str.size(); ++pos)
    {
        switch (str[pos])
        {
            case '&':
                os  << "&amp;";
                break;
            case '<':
                os  << "&lt;";
                break;
            case '>':
                os  << "&gt;";
                break;
            default:
                os  << str[pos];
                break;
        }
    }

    return os;
}


bool markutil::HttpCore::isDir(const std::string& name)
{
    struct stat sb;
    return (::stat(name.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode));
}


bool markutil::HttpCore::isFile(const std::string& name)
{
    struct stat sb;
    return (::stat(name.c_str(), &sb) == 0 && S_ISREG(sb.st_mode));
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

markutil::HttpCore::HttpCore()
:
    headers_()
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

markutil::HttpCore::~HttpCore()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

int markutil::HttpCore::readHeader(std::istream& is)
{
    headers_.clear();
    int nHeaders = 0;

    std::string buf, key;

    // read header lines of form "Key: Value ..."
    while (true)
    {
        std::getline(is, buf, '\n');
        if (buf.empty() || buf[0] == '\r')
        {
            break;
        }

        if (buf[buf.size()-1] == '\r')
        {
            buf.resize(buf.size()-1);
        }

        // Key:
        size_t delim = buf.find_first_of(":\t\n\r ", 0);
        if
        (
            delim == std::string::npos
         || delim == 0
         || buf[delim] != ':'
        )
        {
            break;
        }

        size_t val = buf.find_first_not_of(":\t\n\r ", delim);
        if (val == std::string::npos)
        {
            break;
        }

        headers_[buf.substr(0, delim)] = buf.substr(val);
        ++nHeaders;
    }

    return nHeaders;
}


void markutil::HttpCore::clear()
{
    headers_.clear();
}


const markutil::HttpCore::RawHeaderType& markutil::HttpCore::rawHeaders() const
{
    return headers_;
}


const std::string& markutil::HttpCore::lookupOrDefault
(
    const std::string& key,
    const std::string& defValue
) const
{
    RawHeaderType::const_iterator iter = headers_.find(key);
    return iter != headers_.end() ? iter->second : defValue;
}


std::ostream& markutil::HttpCore::print(std::ostream& os) const
{
    for
    (
        RawHeaderType::const_iterator iter = headers_.begin();
        iter != headers_.end();
        ++iter
    )
    {
        os  << iter->first << ": " << iter->second << "\r\n";
    }

    return os;
}


// * * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * //

markutil::HttpCore& markutil::HttpCore::operator()
(
    const std::string& name,
    const std::string& value
)
{
    headers_[name] = value;
    return *this;
}


const std::string& markutil::HttpCore::operator[]
(
    const std::string& name
) const
{
    return this->lookupOrDefault(name, nullString);
}


// ************************************************************************* //
