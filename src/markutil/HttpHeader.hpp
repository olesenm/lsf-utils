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

Class
    markutil::HttpHeader

Description
    HTTP response header

SourceFiles
    HttpHeader.cpp

SeeAlso
    http://en.wikipedia.org/wiki/List_of_HTTP_header_field

\*---------------------------------------------------------------------------*/
#ifndef MARK_HTTP_HEADER_H
#define MARK_HTTP_HEADER_H

#include "markutil/HttpCore.hpp"
#include "markutil/HttpRequest.hpp"

#include <iostream>
#include <string>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace markutil
{

// Forward declaration of classes
class HttpHeader;

// Forward declaration of friend functions and operators
std::ostream& operator<<(std::ostream&, const HttpHeader&);


/*---------------------------------------------------------------------------*\
                         Class HttpHeader Declaration
\*---------------------------------------------------------------------------*/

class HttpHeader
:
    public HttpCore
{
public:

    //- short-list of common response codes
    enum StatusCode
    {
        INVALID = 0,                       //!< invalid
        _200_OK = 200,                     //!< OK
        _301_MOVED_PERMANENTLY = 301,      //!< MOVED PERMANENTLY
        _302_FOUND = 302,                  //!< FOUND
        _304_NOT_MODIFIED = 304,           //!< NOT MODIFIED
        _400_BAD_REQUEST = 400,            //!< BAD REQUEST
        _401_UNAUTHORIZED = 401,           //!< UNAUTHORIZED
        _403_FORBIDDEN = 403,              //!< FORBIDDEN
        _404_NOT_FOUND = 404,              //!< NOT FOUND
        _405_METHOD_NOT_ALLOWED = 405,     //!< METHOD NOT ALLOWED
        _500_INTERNAL_SERVER_ERROR = 500,  //!< INTERNAL SERVER ERROR
        _501_NOT_IMPLEMENTED = 501,        //!< NOT IMPLEMENTED
        _503_SERVICE_UNAVAILABLE = 503,    //!< SERVICE UNAVAILABLE
    };


private:

    // Private data

        //- The response code to be sent back
        StatusCode status_;

        //- The originating request
        HttpRequest request_;


    // Private Member Functions

        //- Populate with header with commonly used information
        //  General Header Fields (HTTP 1.1 Section 4.5)
        void setDefaults();


public:


    // Static Member Functions


        //- Provide text representation of the status code
        static const char *statusAsText(StatusCode code);


    // Constructors

        //- Construct null
        HttpHeader();

        //- Construct with given status
        explicit HttpHeader(StatusCode code);


    //- Destructor
    ~HttpHeader();


    // Member Functions

        // Access

            //- Special treatment for commonly-used "Content-Type" header
            //  Avoids potential typing mistakes
            const std::string& contentType() const;

            //- Special treatment for commonly-used "Content-Length" header
            //  Avoids potential typing mistakes
            const std::string& contentLength() const;

            //- Read-only access to the originating request
            const HttpRequest& request() const;

            //- The status code as integer
            int status() const;

            //- Provide text representation of the status code
            const char *statusAsText() const;


        // Edit

            //- Sets commonly-used "Content-Type" header
            //  Avoids potential typing mistakes
            //  If the mime-type is missing a '/', lookup from extension
            HttpHeader& contentType(const std::string& val);

            //- Sets commonly-used "Content-Length" header
            //  Avoids potential typing mistakes
            HttpHeader& contentLength(const std::string& val);

            //- Sets commonly-used "Content-Length" header from an integer
            //  Avoids potential typing mistakes
            HttpHeader& contentLength(const unsigned val);

            //- Read/write access to the originating request
            HttpRequest& request();

            //- Alter the status code
            HttpHeader& status(StatusCode code);


        // Write

            //- Begin HTML header with H1 and resource information
            std::ostream& htmlBeg(std::ostream&) const;

            //- End HTML with server information
            std::ostream& htmlEnd(std::ostream&) const;

            //- Output information as XML/HTML content
            //  Useful for generated error messages
            //  Equivalent to htmlBeg/htmlEnd without intermediate content
            std::ostream& html(std::ostream&) const;

            //- Output header contents, optionally with html
            std::ostream& print(std::ostream&, bool withHtml=false) const;


    // Member Operators

        using HttpCore::operator();
        using HttpCore::operator[];

        //- Alter the status code
        HttpHeader& operator()(StatusCode);


    // IOstream Operators

        //- Output header contents
        friend std::ostream& operator<<(std::ostream&, const HttpHeader&);

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace markutil

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif  // MARK_HTTP_HEADER_H

// ************************************************************************* //
