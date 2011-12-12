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
    markutil::HttpServer

Description
    Short Summary

    If a port < 1 is specified, the default port is used

SourceFiles
    HttpServer.cpp

\*---------------------------------------------------------------------------*/

#ifndef MARK_HTTP_SERVER_H
#define MARK_HTTP_SERVER_H

#include "markutil/HttpHeader.hpp"
#include "markutil/SocketServer.hpp"

#include <iostream>
#include <string>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace markutil
{

// Forward declaration of classes

// Forward declaration of friend functions and operators
//class className;
//std::istream& operator>>(std::istream&, className&);
//std::ostream& operator<<(std::ostream&, const className&);


/*---------------------------------------------------------------------------*\
                         Class HttpServer Declaration
\*---------------------------------------------------------------------------*/

class HttpServer
:
    public SocketServer
{
    // Private data

        //! \brief The port number
        unsigned short port_;

        //! \brief The document root
        std::string root_;

        //! \brief The server-name
        std::string name_;


public:

    // Static data members

        //! \brief The default port to use
        static unsigned defaultPort;

        //! \brief The default document root to use
        static std::string defaultRoot;

        //! \brief The default name for the 'Server' header
        static std::string defaultName;

        //! \brief Convenience routine for daemonizing a process
        static int daemonize();

        //! \brief Convenience routine to check for existence of a directory
        static bool isDir(const std::string& dir);


    // Constructors

        //! \brief Create a server on specified port
        HttpServer(unsigned short port = 0, bool reuse = true);

        //! \brief Create a server on specified port
        HttpServer(const std::string& port, bool reuse = true);

        //! \brief Create a server on specified port
        HttpServer(const char* port, bool reuse = true);


    // Destructor

        virtual ~HttpServer();


    // Member Functions

        // Access

            //! \brief The value to be used for the 'Server' header
            const std::string& name() const;

            //! \brief The location used for the document root
            const std::string& root() const;

        // Edit

            //! \brief Set the value to be used for the 'Server' header
            void name(const std::string& name);

            //! \brief Set the location used for the document root
            void root(const std::string& root);

        // General Operation

            //! \brief Enter infinite loop, replying to incoming requests
            int run();

            //! \brief Reply to the incoming request which is already embedded in the reply header
            virtual int reply(std::ostream&, HttpHeader&) const;

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace markutil

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //