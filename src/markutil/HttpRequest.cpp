/*---------------------------------*- C++ -*---------------------------------*\
Copyright (c) 2011-2012 Mark Olesen
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


// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //


markutil::HttpRequest::MethodLookupType markutil::HttpRequest::methodLookup_;


// * * * * * * * * * * * * * * * Local Functions * * * * * * * * * * * * * * //

// local scope
static std::string getToken(const std::string& buf, size_t& beg)
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

void markutil::HttpRequest::populateLookup()
{
    if (methodLookup_.empty())
    {
        methodLookup_["OPTIONS"] = OPTIONS;
        methodLookup_["GET"] = GET;
        methodLookup_["HEAD"] =  HEAD;
        methodLookup_["POST"] = POST;
        methodLookup_["PUT"] = PUT;
        methodLookup_["DELETE"] = DELETE;
        methodLookup_["TRACE"] = TRACE;
        methodLookup_["CONNECT"] = CONNECT;
    }
}


markutil::HttpRequest::MethodType
markutil::HttpRequest::lookupMethod(const std::string& method)
{
    populateLookup();

    if (method.empty())
    {
        return UNKNOWN;
    }

    MethodLookupType::const_iterator iter = methodLookup_.find(method);
    if (iter == methodLookup_.end())
    {
        return UNKNOWN;
    }
    else
    {
        return iter->second;
    }
}


std::string markutil::HttpRequest::lookupMethod(MethodType method)
{
    populateLookup();

    for
    (
        MethodLookupType::const_iterator iter = methodLookup_.begin();
        iter != methodLookup_.end();
        ++iter
    )
    {
        if (iter->second == method)
        {
            return iter->first;
        }
    }

    return "";
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

markutil::HttpRequest::HttpRequest()
:
    type_(UNKNOWN),
    method_(),
    path_(),
    query_(),
    httpver_()
{}


markutil::HttpRequest::HttpRequest
(
    MethodType method,
    const std::string& url
)
:
    type_(method),
    method_(lookupMethod(method)),
    path_(),
    query_(),
    httpver_()
{
    this->requestURI(url);
}


markutil::HttpRequest::HttpRequest(std::istream& is)
:
    type_(UNKNOWN),
    method_(),
    path_(),
    query_(),
    httpver_()
{
    readHeader(is);
}


markutil::HttpRequest::HttpRequest(int fd)
:
    type_(UNKNOWN),
    method_(),
    path_(),
    query_(),
    httpver_()
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
    path_.clear();
    query_.clear();
    httpver_.clear();

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
        httpver_ = getToken(buf, beg);
    }
    else
    {
        httpver_.clear();
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


const std::string& markutil::HttpRequest::protocol() const
{
    return httpver_;
}


std::string markutil::HttpRequest::requestURI() const
{
    std::string uri;
    std::string q = query_.toString();

    uri.reserve(path_.size() + q.size());

    httpAppendUrl(uri, path_);
    if (q.size())
    {
        uri += '?';
        uri += q;
    }

    return uri;
}


void markutil::HttpRequest::requestURI(const std::string& uri)
{
    std::string::size_type frag = uri.find_first_of("#?");

    path_ = httpDecodeUrl(uri, 0, frag);
    httpNormalizePath(path_);

    if
    (
        frag != std::string::npos
     && uri[frag] == '?'
    )
    {
        // a question (not a fragment) ... keep looking
        std::string::size_type question = frag;
        frag = uri.find('#', frag);

        if (frag != std::string::npos)
        {
            // convert to parameter length
            frag = frag - question - 1;
        }
        query_.parseUrl(uri, question + 1, frag);
    }
    else
    {
        query_.clear();
    }
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


const markutil::HttpQuery& markutil::HttpRequest::query() const
{
    return query_;
}


const markutil::SocketInfo& markutil::HttpRequest::socketInfo() const
{
    return socketinfo_;
}


markutil::SocketInfo& markutil::HttpRequest::socketInfo()
{
    return socketinfo_;
}


std::ostream& markutil::HttpRequest::print(std::ostream& os) const
{
    os  << method_ << " " << requestURI() << "\r\n";
    this->HttpCore::print(os);
    os  << "\r\n";

    return os;
}


// ************************************************************************* //
