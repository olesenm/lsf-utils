/*---------------------------------*- C++ -*---------------------------------*\
Copyright 2011 Mark Olesen
-------------------------------------------------------------------------------
License
    This file is part of lsf-utils.

    lsf-utils is free software: you can redistribute it and/or modify it under
    the terms of the GNU Affero General Public License as published by the
    Free Software Foundation, either version 3 of the License,
    or (at your option) any later version.

    lsf-utils is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
    or FITNESS FOR A PARTICULAR PURPOSE.
    See the GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with lsf-utils. If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "markutil/HttpHeader.hpp"

#include <ctime>
#include <clocale>
#include <cstring>
#include <iostream>
#include <sstream>


// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //


// * * * * * * * * * * * * * Static Member Functions * * * * * * * * * * * * //

const char *markutil::HttpHeader::statusAsText(StatusCode code)
{
    // populate lookup table on the first call
    static std::map<int, const char*> lookup;
    static const char* notFound = "INVALID";

    if (lookup.empty())
    {
        lookup[_200_OK] = "OK";
        lookup[_301_MOVED_PERMANENTLY] = "Moved Permanently";
        lookup[_302_FOUND] = "FOUND";
        lookup[_400_BAD_REQUEST] = "Bad request";
        lookup[_401_UNAUTHORIZED] = "Unauthorized";
        lookup[_403_FORBIDDEN] = "Forbidden";
        lookup[_404_NOT_FOUND] = "Not Found";
        lookup[_405_METHOD_NOT_ALLOWED] = "Method Not Allowed";
        lookup[_500_INTERNAL_SERVER_ERROR] = "Internal Server Error";
        lookup[_501_NOT_IMPLEMENTED] = "Not Implemented";
    }

    std::map<int, const char*>::const_iterator iter = lookup.find(code);
    if (iter == lookup.end())
    {
        return notFound;
    }
    else
    {
        return iter->second;
    }
}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void markutil::HttpHeader::setDefaults()
{
    const std::string now = timestring();

    headers_["Date"] = now;
    headers_["Last-Modified"] = now;

    headers_["Cache-Control"] = "no-cache";
    headers_["Content-Type"]  = "text/html, charset=UFT-8";
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

markutil::HttpHeader::HttpHeader()
:
    HttpCore(),
    status_(INVALID),
    request_()
{
    setDefaults();
}


markutil::HttpHeader::HttpHeader(StatusCode code)
:
    HttpCore(),
    status_(code),
    request_()
{
    setDefaults();
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

markutil::HttpHeader::~HttpHeader()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

// special treatment for Content-Type and Content-Length headers
const std::string& markutil::HttpHeader::contentType() const
{
    return this->lookupOrDefault("Content-Type", "text/html");
}


const std::string& markutil::HttpHeader::contentLength() const
{
    return this->lookupOrDefault("Content-Length", "0");
}


const markutil::HttpRequest& markutil::HttpHeader::request() const
{
    return request_;
}


const char *markutil::HttpHeader::statusAsText() const
{
    return statusAsText(this->status_);
}


markutil::HttpHeader& markutil::HttpHeader::header
(
    const std::string& name,
    const std::string& value
)
{
    headers_[name] = value;
    return *this;
}


markutil::HttpHeader&
markutil::HttpHeader::contentType(const std::string& val)
{
    headers_["Content-Type"] = val;
    return *this;
}


markutil::HttpHeader&
markutil::HttpHeader::contentLength(const std::string& val)
{
    headers_["Content-Length"] = val;
    return *this;
}


markutil::HttpHeader&
markutil::HttpHeader::contentLength(const unsigned val)
{
    std::ostringstream oss;
    oss << val;
    headers_["Content-Length"] = oss.str();
    return *this;
}


markutil::HttpRequest&
markutil::HttpHeader::request()
{
    return request_;
}


markutil::HttpHeader&
markutil::HttpHeader::status(StatusCode code)
{
    status_ = code;
    return *this;
}


std::ostream& markutil::HttpHeader::html(std::ostream& os) const
{
    os  << "<html><head><title>"
        << status_ << " " << statusAsText()
        << "</title></head><body>"
        << status_ << " " << statusAsText()
        << "<br />";

    const std::string& uri = request_.requestURI();
    if (!uri.empty())
    {
        os  << "uri: ";
        escapeHtmlCharacters(os, uri);
    }

    os  << "</body></html>\n";

    return os;
}


std::ostream& markutil::HttpHeader::print(std::ostream& os) const
{
    os  << "HTTP/1.0 " << status_ << " " << statusAsText() << "\r\n";
    this->HttpCore::print(os);
    os  << "\r\n";

    return os;
}


// * * * * * * * * * * * * * * * Friend Operators  * * * * * * * * * * * * * //

std::ostream& markutil::operator<<
(
    std::ostream& os,
    const markutil::HttpHeader& head
)
{
    return head.print(os);
}


// ************************************************************************* //
