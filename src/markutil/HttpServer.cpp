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

#include "markutil/HttpServer.hpp"
#include "markutil/HttpHeader.hpp"
#include "markutil/HttpRequest.hpp"

#include <cstdlib>
#include <string>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>

#include "fdstream/fdstream.hpp"


// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

unsigned markutil::HttpServer::defaultPort = 8080;

std::string markutil::HttpServer::defaultRoot = "/usr/local/www/htdocs";

std::string markutil::HttpServer::defaultName = "HttpServer";

// local scope
static const unsigned BufSize = 8096;
static char buffer[BufSize];



// * * * * * * * * * * * * * Static Member Functions * * * * * * * * * * * * //

int markutil::HttpServer::daemonize(const bool doNotExit)
{
    // daemonize and no zombies children
    const int pid = ::fork();
    if (pid)
    {
        if (!doNotExit)
        {
            if (pid < 0)
            {
                std::cerr << "Error: could not fork\n";  // could not fork?
                ::exit(1);
            }
            else
            {
                ::exit(0);    // parent returns OK to shell
            }
        }
    }

    // this is the child
    ::setsid();                   // break away from process group
    ::signal(SIGCLD, SIG_IGN);    // ignore child death
    ::signal(SIGHUP, SIG_IGN);    // ignore terminal hangup

    // close any open file descriptors
    for (int fd = 0; fd < 32; ++fd)
    {
        ::close(fd);
    }

    return pid;
}


bool markutil::HttpServer::isDir(const std::string& dir)
{
    struct stat sb;
    return (::stat(dir.c_str(), &sb) == 0 && (sb.st_mode & S_IFDIR));
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

markutil::HttpServer::HttpServer(unsigned short port, bool reuse)
:
    SocketServer(reuse),
    port_(defaultPort),
    root_(defaultRoot),
    name_(defaultName)
{
    if (port)
    {
        port_ = port;
    }

    this->bind(port);
}


markutil::HttpServer::HttpServer(const std::string& port, bool reuse)
:
    SocketServer(reuse),
    port_(defaultPort),
    root_(defaultRoot),
    name_(defaultName)
{
    if (port.empty() || port[0] == '0')
    {
        this->bind(port_);
    }
    else
    {
        port_ = this->bind(port);
    }
}


markutil::HttpServer::HttpServer(const char* port, bool reuse)
:
    SocketServer(reuse),
    port_(defaultPort),
    root_(defaultRoot),
    name_(defaultName)
{
    if (!port || !*port || port[0] == '0')
    {
        this->bind(port_);
    }
    else
    {
        port_ = this->bind(port);
    }
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

markutil::HttpServer::~HttpServer()
{}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

const std::string& markutil::HttpServer::name() const
{
    return name_;
}


void markutil::HttpServer::name(const std::string& name)
{
    if (!name.empty())
    {
        name_ = name;
    }
}


const std::string& markutil::HttpServer::root() const
{
    return root_;
}


void markutil::HttpServer::root(const std::string& root)
{
    if (!root.empty())
    {
        root_ = root;
    }
}


int markutil::HttpServer::run()
{
    if (!this->bound())
    {
        this->bind(port_);
    }
    if (!this->listening())
    {
        this->listen();
    }

    while (true)
    {
        const int sockfd = this->accept();
        if (sockfd < 0)
        {
//            errorlog("system call", "accept");
            continue;
        }

        int pid = ::fork();
        if (pid < 0)
        {
//            errorlog("system call" ,"fork");
            continue;
        }
        else if (pid)
        {
            // parent
            ::close(sockfd);
        }
        else
        {
            // child
            this->close();
            boost::fdistream is(sockfd);
            boost::fdostream os(sockfd);

            HttpHeader head;
            head("Server", this->name());
            head.request().readHeader(is);

            int ret = this->reply(os, head);
#ifdef LINUX
            sleep(1);      // let socket to drain
#endif
            return ret;
        }
    }

    return 0;
}


int markutil::HttpServer::reply(std::ostream& os, HttpHeader& head) const
{
    HttpRequest& req = head.request();

    // rewrite rules
    std::string url = req.path();

    // convert no filename to index file
    if (url == "/")
    {
        url = "/index.html";
        req.requestURI(url);
    }

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

    std::string mimeType = HttpCore::lookupMime(req.ext());

    if (mimeType.empty())
    {
        head(head._404_NOT_FOUND);
        head.print(os, true);

        return 1;
    }

    std::string file = root_ + url;

    // open the file for reading
    int fileFd = open(file.c_str(), O_RDONLY);
    if (fileFd == -1)
    {
        head(head._404_NOT_FOUND);
        head.print(os, true);

        return 1;
    }


    head.contentType(mimeType);
    os  << head(head._200_OK);
    if (req.type() == req.GET)
    {
        // send file in 8KB block - last block may be smaller
        size_t nbyte;
        while ( (nbyte = ::read(fileFd, buffer, BufSize)) > 0 )
        {
            os.write(buffer, nbyte);
        }
    }

    return 0;
}


// ************************************************************************* //
