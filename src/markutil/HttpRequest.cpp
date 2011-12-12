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

#include "markutil/HttpRequest.hpp"
#include "fdstream/fdstream.hpp"

#include <list>
#include <string>
#include <cstdlib>
#include <cstring>
#include <iostream>


// * * * * * * * * * * * * * * * Local Functions * * * * * * * * * * * * * * //

// local scope
std::string getToken(const std::string& buf, size_t& beg)
{
    if (beg == std::string::npos)
    {
        return std::string();
    }
    else
    {
        size_t oldPos = beg;
        size_t space = buf.find_first_of("\t\n\r ", oldPos);

        if (space == std::string::npos)
        {
            beg = std::string::npos;
            return buf.substr(oldPos);
        }
        else
        {
            beg = buf.find_first_not_of("\t\n\r ", space);
            return buf.substr(oldPos, space - oldPos);
        }
    }
}


// * * * * * * * * * * * * * Static Member Functions * * * * * * * * * * * * //

markutil::HttpRequest::MethodType
markutil::HttpRequest::lookupMethod(const std::string& method)
{
    typedef std::map<std::string, MethodType> LookupType;
    static LookupType lookup;

    if (lookup.empty())
    {
        lookup["OPTIONS"] = OPTIONS;
        lookup["GET"] = GET;
        lookup["HEAD"] =  HEAD;
        lookup["POST"] = POST;
        lookup["PUT"] = PUT;
        lookup["DELETE"] = DELETE;
        lookup["TRACE"] = TRACE;
        lookup["CONNECT"] = CONNECT;
    }

    if (method.empty())
    {
        return UNKNOWN;
    }

    LookupType::const_iterator iter = lookup.find(method);
    if (iter == lookup.end())
    {
        return UNKNOWN;
    }
    else
    {
        return iter->second;
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

markutil::HttpRequest::HttpRequest()
:
    type_(UNKNOWN),
    method_(),
    uri_(),
    path_(),
    query_(),
    httpMajor_(0),
    httpMinor_(9)
{}


markutil::HttpRequest::HttpRequest(std::istream& is)
:
    type_(UNKNOWN),
    method_(),
    uri_(),
    path_(),
    query_(),
    httpMajor_(0),
    httpMinor_(9)
{
    readHeader(is);
}


markutil::HttpRequest::HttpRequest(int fd)
:
    type_(UNKNOWN),
    method_(),
    uri_(),
    path_(),
    query_(),
    httpMajor_(0),
    httpMinor_(9)
{
    boost::fdistream is(fd);
    readHeader(is);
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

markutil::HttpRequest::~HttpRequest()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void markutil::HttpRequest::clear()
{
    type_ = UNKNOWN;
    method_.clear();
    uri_.clear(),
    path_.clear();
    query_.clear();
    httpMajor_ = 0;
    httpMinor_ = 9;

    this->HttpCore::clear();
}



void markutil::HttpRequest::readHeader(std::istream& is)
{
    this->clear();

    size_t beg;
    std::string buf;

    // Request        = Simple-Request | Full-Request
    // Simple-Request =  "GET" SP Request-URI CRLF
    // Full-Request   = Method SP Request-URI SP HTTP-Version
    std::getline(is, buf, '\n');

    // Method:
    beg = 0;
    method_ = getToken(buf, beg);
    type_   = lookupMethod(method_);

    // Request-URI:
    //   also define path/query
    this->requestURI(getToken(buf, beg));

    // a 3rd token?
    // HTTP-Version = "HTTP" "/" 1*DIGIT "." 1*DIGIT
    if (beg != std::string::npos)
    {
        std::string httpver = getToken(buf, beg);
        size_t slash = httpver.find('/');
        if
        (
            slash != std::string::npos
         && httpver.substr(0, slash) == "HTTP"
        )
        {
            ++slash;
            size_t dot = httpver.find('.', slash);

            if (dot != std::string::npos)
            {
                httpMajor_ = atoi(httpver.substr(slash, dot-slash).c_str());
                ++dot;
                httpMinor_ = atoi(httpver.substr(dot).c_str());
            }
        }
    }

    this->HttpCore::readHeader(is);
}


const markutil::HttpRequest::MethodType& markutil::HttpRequest::type() const
{
    return type_;
}


const std::string& markutil::HttpRequest::method() const
{
    return method_;
}


const std::string& markutil::HttpRequest::requestURI() const
{
    return uri_;
}


std::string& markutil::HttpRequest::requestURI(const std::string& newURI)
{
    uri_ = newURI;

    // started with incorrect http://host../
    if (uri_.substr(0, 7) == "http://")
    {
        size_t slash = uri_.find('/', 7);
        if (slash == std::string::npos)
        {
            uri_ = "/";
        }
        else
        {
            uri_.erase(0, slash);
        }
    }

    std::string::size_type question = uri_.find('?');
    if (question == std::string::npos)
    {
        path_ = uri_;
        query_.clear();
    }
    else
    {
        path_  = uri_.substr(0, question);
        query_ = uri_.substr(question+1);
        httpDecodeUri(query_);
    }
    httpDecodeUri(path_);

    return uri_;
}


const std::string& markutil::HttpRequest::path() const
{
    return path_;
}


std::string markutil::HttpRequest::ext() const
{
    std::string::size_type dot = path_.find_last_of("./");

    if (dot != std::string::npos && path_[dot] == '.')
    {
        return path_.substr(dot+1);
    }
    else
    {
        return std::string();
    }
}


const std::string& markutil::HttpRequest::query() const
{
    return query_;
}


std::ostream& markutil::HttpRequest::print(std::ostream& os) const
{
    os  << method_ << " " << uri_ << "\r\n";
    this->HttpCore::print(os);
    os  << "\r\n";

    return os;
}


// ************************************************************************* //
