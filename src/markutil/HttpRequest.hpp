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

Class
    markutil::HttpRequest

Description

SourceFiles
    HttpRequest.cpp

SeeAlso
    http://en.wikipedia.org/wiki/List_of_HTTP_header_field

\*---------------------------------------------------------------------------*/
#ifndef MARK_HTTP_REQUEST_H
#define MARK_HTTP_REQUEST_H

#include "markutil/HttpCore.hpp"
#include <iostream>


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace markutil
{

/*---------------------------------------------------------------------------*\
                         Class HttpRequest Declaration
\*---------------------------------------------------------------------------*/


class HttpRequest
:
    public HttpCore
{
public:

    //! known request methods
    enum MethodType
    {
        UNKNOWN = 0,  // unknown/invalid
        OPTIONS,
        GET,
        HEAD,
        POST,
        PUT,
        DELETE,
        TRACE,
        CONNECT
    };

private:

    // Private data

        // The method as an enumeration
        MethodType type_;

        // The method as a string
        std::string method_;

        // The raw URI
        std::string uri_;

        // Everything up to the '?' query - decoded
        std::string path_;

        // Everything after the '?' query - decoded
        std::string query_;

        unsigned short httpMajor_;

        unsigned short httpMinor_;


    // Private Member Functions

        //- Lookup enumerated method-type from string content
        static MethodType lookupMethod(const std::string& method);


public:

    // Constructors

        //! Construct null, the request will be invalid
        HttpRequest();

        //! Construct request by reading from an input stream
        //  reads until a blank line
        HttpRequest(std::istream&);

        //! Construct request by reading from an input file descriptor
        //  reads until a blank line
        explicit HttpRequest(int fd);


    // Destructor

        ~HttpRequest();


       // Member Functions

        //! Construct new request by reading from an input stream
        //  reads until a blank line
        void readHeader(std::istream&);

        //! Clear request, making it invalid
        void clear();


        // Access

            //! \brief Return the Method as an enumeration
            const MethodType& type() const;

            //! \brief Return the Method as a string
            const std::string& method() const;

            //! \brief Return the Request-URI
            const std::string& requestURI() const;

            //! \brief Rewrite the Request-URI, updatind path and query too
            std::string& requestURI(const std::string& newURI);

            //! \brief Return the Request Path, which is everything up to the '?'
            const std::string& path() const;

            //! \brief Return the Request Query string, which is everything after the '?'
            const std::string& query() const;

            //! \brief Return the extension of the path
            std::string ext() const;


        // Write

            //! \brief Write request output stream
            std::ostream& print(std::ostream&) const;


    // Member Operators

        using HttpCore::operator[];

};


} // End namespace markutil

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif  // MARK_HTTP_REQUEST_H

// ************************************************************************* //