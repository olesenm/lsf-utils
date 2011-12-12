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

Class
    markutil::HttpCore

Description

SourceFiles
    HttpCore.cpp

\*---------------------------------------------------------------------------*/

#include "markutil/HttpCore.hpp"

#include <ctime>
#include <clocale>
#include <cstring>
#include <string>
#include <sstream>


// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

const std::string markutil::HttpCore::nullString;


// * * * * * * * * * * * * * Static Member Functions * * * * * * * * * * * * //

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
    static StringMapNoCase lookup;

    // populate lookup table on the first call
    if (lookup.empty())
    {
        // text
        lookup["html"] = "text/html";
        lookup["htm"]  = "text/html";
        lookup["txt"]  = "text/plain";
        lookup["xml"]  = "text/xml";
        lookup["xsl"]  = "text/xsl";
        lookup["xhtml"] = "application/xhtml+xml";
        lookup["css"]  = "text/css";

        // image
        lookup["ico"]  = "image/x-icon";
        lookup["gif"]  = "image/gif";
        lookup["jpg"]  = "image/jpeg";
        lookup["jpeg"] = "image/jpeg";
        lookup["png"]  = "image/png";

        // application
        lookup["pdf"]  = "application/pdf";
        lookup["tar"]  = "application/x-tar";
        lookup["gz"]   = "application/x-gzip";
        lookup["zip"]  = "application/x-zip-compressed";
    }

    if (ext.empty())
    {
        return nullString;
    }

    StringMapNoCase::const_iterator iter = lookup.find(ext);
    if (iter == lookup.end())
    {
        return nullString;
    }
    else
    {
        return iter->second;
    }
}



bool markutil::HttpCore::httpDecodeUri(std::string& str)
{
    bool ok = true;
    for
    (
        std::size_t pos = 0, len = 0;
        pos < str.size();
        ++pos, ++len
    )
    {
        if (str[pos] == '%')
        {
            if (pos + 3 <= str.size())   // %<hex><hex> needs 2 hex chars
            {
                int val = 0;
                std::istringstream is(str.substr(pos + 1, 2));
                if (is >> std::hex >> val)
                {
                    str[len] = static_cast<char>(val);
                    pos += 2;
                }
                else
                {
                    str[len] = str[pos];
                    ok = false;
                }
            }
            else
            {
                str[len] = str[pos];
                ok = false;
            }
        }
        else if (str[pos] == '+')
        {
            str[len] = ' ';
        }
        else
        {
            str[len] = str[pos];
        }
    }

    return true;
}


std::ostream& markutil::HttpCore::escapeHtmlCharacters
(
    std::ostream& os,
    const std::string& str
)
{
    for
    (
        std::size_t pos = 0;
        pos < str.size();
        ++pos
    )
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


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //


// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //


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

        headers_[buf.substr(0, delim-1)] = buf.substr(val);
        ++nHeaders;
    }

    return nHeaders;
}


void markutil::HttpCore::clear()
{
    headers_.clear();
}


const std::string& markutil::HttpCore::lookupOrDefault
(
    const std::string& key,
    const std::string& defValue
) const
{
    StringMapNoCase::const_iterator iter = headers_.find(key);
    return iter != headers_.end() ? iter->second : defValue;
}


std::ostream& markutil::HttpCore::print(std::ostream& os) const
{
    for
    (
        StringMapNoCase::const_iterator iter = headers_.begin();
        iter != headers_.end();
        ++iter
    )
    {
        os  << iter->first << ": " << iter->second << "\r\n";
    }

    return os;
}


// * * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * //

const std::string& markutil::HttpCore::operator[]
(
    const std::string& name
) const
{
    return this->lookupOrDefault(name, nullString);
}


markutil::HttpCore& markutil::HttpCore::operator()
(
    const std::string& name,
    const std::string& value
)
{
    headers_[name] = value;
}


// ************************************************************************* //
