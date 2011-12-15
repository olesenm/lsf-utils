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

#include <cstdio>
#include <cstdlib>

#include <string>
#include <iostream>

#include "markutil/HttpServer.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// specialization of HttpServer
class MyServer
:
    public markutil::HttpServer
{
public:

    typedef markutil::HttpServer ParentClass;

    // Constructors

        //- Create a server on specified port
        MyServer(unsigned short port, const std::string& root)
        :
            ParentClass(port)
        {
            this->name("lsf-utils");
            this->root(root);
        }


        virtual int reply(std::ostream& os, HeaderType& head) const
        {
            RequestType& req = head.request();

            if
            (
                req.type() != req.HEAD
             && req.type() != req.GET
            )
            {
                head(head._405_METHOD_NOT_ALLOWED);
                head("Allow", "GET,HEAD");
                head.print(os, true);

                return 1;
            }


            // rewrite rules
            const std::string& url = req.path();

            // convert no filename to index file
            if (url == "/")
            {
                req.requestURI("/index.html");
            }

            if (url == "/server-info")
            {
                os  << head(head._200_OK);

                if (req.type() == req.GET)
                {
                    const QueryType& query = req.query();

                    os  << "<html><head>"
                        << "<title>server-info</title>"
                        << "</head><body>"
                        << "Date: " << head["Date"] << "<br />"
                        << "Server: " << this->name() << "<br />"
                        << "Request: ";

                    xmlEscapeChars
                    (
                        os,
                        req.requestURI()
                    ) << "<br />";
                    os  << "<hr />";

                    if (!query.empty())
                    {
                        const QueryType::string_list& unnamed = query.unnamed();
                        if (!unnamed.empty())
                        {
                            os  << unnamed.size()
                                << " unnamed<br /><blockquote>";

                            for
                            (
                                unsigned nameI = 0;
                                nameI < unnamed.size();
                                ++nameI
                            )
                            {
                                xmlEscapeChars
                                (
                                    os,
                                    unnamed[nameI]
                                )  << "<br />";
                            }
                            os  << "</blockquote><hr />";
                        }


                        QueryType::string_list param = query.param();
                        if (!param.empty())
                        {
                            os  << param.size()
                                << " param<br /><blockquote>";
                            for
                            (
                                unsigned nameI = 0;
                                nameI < param.size();
                                ++nameI
                            )
                            {
                                const std::string& name = param[nameI];
                                const QueryType::string_list& vals
                                    = query.param(name);

                                for
                                (
                                    unsigned valI = 0;
                                    valI < vals.size();
                                    ++valI
                                )
                                {
                                    xmlEscapeChars
                                    (
                                        os,
                                        name
                                    ) << "=";
                                    xmlEscapeChars
                                    (
                                        os,
                                        vals[valI]
                                    ) << "<br />";
                                }
                            }
                            os  << "</blockquote><hr />";
                        }
                    }
                    os  << "</body></html>\n";
                }

                return 0;
            }


            // catch balance with standard document serving
            return this->ParentClass::reply(os, head);
        }

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //


int main(int argc, char **argv)
{
    const std::string name("sample-server");

    if (argc != 3)
    {
        std::cerr
            << "incorrect number of arguments\n\n";

        std::cerr
            << "usage: "<< name << " Port DocRoot\n\n"
            << "A small sample web server in C++\n\n"
            << "Eg,\n"
            << name << " " << markutil::HttpServer::defaultPort
            << " " << markutil::HttpServer::defaultRoot << "\n\n";

        return 1;
    }

    int port = atoi(argv[1]);
    std::string docRoot(argv[2]);

    // check port-number
    if (port < 1 || port > 65535)
    {
        std::cerr
            << "Invalid port number: (try 1 .. 65535)\n";
        return 1;
    }

    // check if doc-root exists
    if (!markutil::HttpServer::isDir(docRoot))
    {
        std::cerr
            << "Directory does not exist: "<< docRoot << "\n";
        return 1;
    }

    markutil::HttpServer::daemonize();

    MyServer server(port, docRoot);
    server.listen(64);

    return server.run();
}


// ************************************************************************* //
