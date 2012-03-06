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
    markutil::HttpServer

Description
    Short Summary

    If a port < 1 is specified, the default port is used

SourceFiles
    HttpServer.cpp

Notes
    The inheritance from markutil::HttpCore is purely a convenience to
    pass-through its static methods.

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

/*---------------------------------------------------------------------------*\
                         Class HttpServer Declaration
\*---------------------------------------------------------------------------*/

class HttpServer
:
    public HttpCore,
    public SocketServer
{
    // Private data

        //- The port number
        unsigned short port_;

        //- The document root
        std::string root_;

        //- The server-name
        std::string name_;

        //- The prefix for serving cgi-bin
        std::string cgiPrefix_;

        //- The cgi-bin
        std::string cgibin_;


    // Private Member Functions

        //! set absolute or relative path
        bool setPath(std::string& target, const std::string& path);

        //! dispatch to cgi or normal document serving
        int dispatch(int sockfd);

protected:

    // Protected Member Functions

        //- Return true if it appears to be CGI
        bool isCgi(const HttpHeader& head) const;

        //- Return true if the CGI look okay, or send appropriate response
        bool cgiOkay(std::ostream& os, HttpHeader& head) const;

        //- Set CGI environment variables prior to calling cgi() itself
        //  Returns the SCRIPT_NAME for valid requests and an empty
        //  string if there are problems
        std::string setCgiEnv(HttpHeader& head) const;

        //- Check for GET or HEAD, emitting error 405 if it doesn't match
        bool notGetOrHead(std::ostream& os, HttpHeader& head) const;


public:

    //!  The run-type to use
    enum RunType
    {
        FORKING,    //!< traditional forking server
        SELECT      //!< select-based server
    };


    // Convenience Typedefs
    //- The header type
    typedef markutil::HttpHeader  HeaderType;

    //- The query type
    typedef markutil::HttpQuery   QueryType;

    //- The request type
    typedef markutil::HttpRequest RequestType;


    // Static data members

        //- The default port to use
        static unsigned defaultPort;

        //- The default document root to use
        static std::string defaultRoot;

        //- The default name for the 'Server' header
        static std::string defaultName;

        //- The default prefix for cgi-bin, no trailing slash
        static std::string defaultCgiPrefix;


    // Static Functions

        //- Convenience routine for daemonizing a process
        //  optionally suppress exit and handle in caller instead
        static int daemonize(const bool doNotExit=false);


    // Constructors

        //- Create a server on specified port
        HttpServer(unsigned short port = 0, bool reuse = true);

        //- Create a server on specified port
        HttpServer(const std::string& port, bool reuse = true);

        //- Create a server on specified port
        HttpServer(const char* port, bool reuse = true);


    //- Destructor
    virtual ~HttpServer();


    // Member Functions

        // Access

            //- The value to be used for the 'Server' header
            const std::string& name() const;

            //- The location used for the document root
            const std::string& root() const;

            //- The location used for serving the cgi-bin
            const std::string& cgibin() const;

            //- The CGI prefix
            const std::string& cgiPrefix() const;


        // Edit

            //- Set the value to be used for the 'Server' header
            bool name(const std::string& name);

            //- Set the location used for the document root
            //  Resolved automatically to an absolute path
            //  Serving the "/" directory is not permitted.
            bool root(const std::string& root);

            //- Set the location used for the cgi-bin
            //  Resolved automatically to an absolute path
            //  Serving the "/" directory is not permitted.
            bool cgibin(const std::string& bindir);

            //- Set the CGI prefix
            bool cgiPrefix(const std::string& prefix);


        // General Operation

            //- Enter infinite loop, replying to incoming requests
            //  Use a fork-based server
            int run_fork();

            //- Enter infinite loop, replying to incoming requests
            //  Use a select-based server
            int run_select();

            //- Enter infinite loop, replying to incoming requests
            //  Use the specified server type
            int run(RunType how = FORKING);

            //- Invoke cgi for incoming request, which is already embedded in the reply header
            //  We are especially lazy and only support Non-Parsed-Headers for now.
            virtual int cgi(int fd, HeaderType&) const;

            //- Reply to the incoming request, which is already embedded in the reply header
            virtual int reply(std::ostream&, HeaderType&) const;

            //- Reply with basic server information as html
            virtual int server_about(std::ostream&, HttpHeader&) const;

            //- Reply with server information as html
            virtual int server_info(std::ostream&, HttpHeader&) const;

            //- Extra content to add into server_about
            virtual void content_about(std::ostream&, const HttpHeader&) const
            {}

            //- Extra content to add into server_info
            virtual void content_info(std::ostream&, const HttpHeader&) const
            {}

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace markutil

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif  // MARK_HTTP_SERVER_H

// ************************************************************************* //
