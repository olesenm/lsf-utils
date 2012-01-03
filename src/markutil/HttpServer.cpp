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

#include "markutil/HttpServer.hpp"
#include "markutil/HttpHeader.hpp"
#include "markutil/HttpRequest.hpp"

#include <cstdlib>
#include <string>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>

#include "fdstream/fdstream.hpp"


// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

unsigned markutil::HttpServer::defaultPort = 8080;

std::string markutil::HttpServer::defaultRoot = "/usr/local/www/htdocs";

std::string markutil::HttpServer::defaultName = "HttpServer";

std::string markutil::HttpServer::defaultCgiPrefix = "/cgi-bin";

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


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

bool markutil::HttpServer::setPath
(
    std::string& target,
    const std::string& path
)
{
    if (!path.empty())
    {
        if (path[0] == '/')
        {
            if (path.size() == 1)
            {
                return false; // never serve from filesystem root!
            }
            target = path;
        }
        else
        {
            // resolve relative -> abs path
            if (!::getcwd(buffer, BufSize))
            {
                return false;
            }
            target = buffer;
            target += '/';
            target += path;
        }

        // this ensures we never have a trailing slash
        target += "/.";
        httpNormalizePath(target);

        return true;
    }

    return false;
}


void markutil::HttpServer::prepareCgiEnv(int fd, HttpHeader& head) const
{
    HttpRequest& req = head.request();
    std::string script_url = req.path();

    // we might need the requested "Host:" as a fallback value
    // HTTP_HOST="<host>[:<port>]"
    std::string http_host = req["Host"];
    if (http_host.size())
    {
        setenv("HTTP_HOST", http_host.c_str(), 1);
    }

    //
    // REMOTE_ADDR, REMOTE_PORT from socket information
    // and REMOTE_HOST from gethostbyaddr
    //
    setenv("REMOTE_ADDR", socketinfo_.peerAddr().c_str(), 1);
    setenv("REMOTE_HOST", socketinfo_.peerName().c_str(), 1);
    setenv("REMOTE_PORT", socketinfo_.peerPort().c_str(), 1);

    //
    // SERVER_ADDR, SERVER_PORT from socket information
    // and SERVER_NAME from gethostbyaddr
    //
    setenv("SERVER_ADDR", socketinfo_.hostAddr().c_str(), 1);
    setenv("SERVER_NAME", socketinfo_.hostName().c_str(), 1);
    setenv("SERVER_PORT", socketinfo_.hostPort().c_str(), 1);

    std::string server_url = "http://" + socketinfo_.hostName();
    if (socketinfo_.hostPort().size() && socketinfo_.hostPort() != "80")
    {
        server_url += ":" + socketinfo_.hostPort();
    }


    setenv("REQUEST_METHOD", req.method().c_str(), 1);

    // REQUEST_URI is "/cgi-bin/...?..."
    setenv("REQUEST_URI", req.requestURI().c_str(), 1);

    // QUERY_STRING is everything after the '?'
    setenv("QUERY_STRING", req.query().toString().c_str(), 1);


    // Apache may not export SERVER_URL
    // SERVER_URL="http://<host>[:<port>]/"
    setenv("SERVER_URL", (server_url + '/').c_str(), 1);

    setenv("SERVER_PROTOCOL", req.protocol().c_str(), 1);
    setenv("SERVER_SOFTWARE", this->name().c_str(), 1);

    setenv("DOCUMENT_ROOT", this->root().c_str(), 1);


    // SCRIPT_URI="http://<host>[:<port>]/cgi-bin/..."
    // without the query string
    setenv("SCRIPT_URI", (server_url + script_url).c_str(), 1);

    // SCRIPT_URL="/cgi-bin/..."
    // without the query string
    setenv("SCRIPT_URL", script_url.c_str(), 1);

    //
    // finally
    // split SCRIPT_URL into SCRIPT_NAME + PATH_INFO

    std::string script_name;
    size_t slash = script_url.find('/', cgiPrefix_.size() + 1);
    if (slash == std::string::npos)
    {
        script_name = script_url;
    }
    else
    {
        script_name = script_url.substr(0, slash);
    }

    // script name includes the leading "/cgi-bin/"
    setenv("SCRIPT_NAME", script_name.c_str(), 1);

    // remove leading "/cgi-bin" for SCRIPT_FILENAME
    script_url.erase(0, cgiPrefix_.size());
    slash = script_url.find('/', 1);
    if (slash == std::string::npos)
    {
        unsetenv("PATH_INFO");
        unsetenv("PATH_TRANSLATED");
    }
    else
    {
        std::string path_info = script_url.substr(slash);
        std::string path_translated = this->root() + path_info;

        setenv("PATH_INFO", path_info.c_str(), 1);
        setenv("PATH_TRANSLATED", path_translated.c_str(), 1);

        // change script_url -> script_name
        script_url.resize(slash);
    }

    std::string script_filename = this->cgibin() + script_url;

    setenv("SCRIPT_FILENAME", script_filename.c_str(), 1);


    //
    // pass through some request header information
    //
    if (req["User-Agent"].size())
    {
        setenv("HTTP_USER_AGENT", req["User-Agent"].c_str(), 1);
    }

    //
    // partially cleanse environment
    //
    setenv("PATH", "/usr/bin:/bin", 1);
    unsetenv("LD_LIBRARY_PATH");
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

markutil::HttpServer::HttpServer(unsigned short port, bool reuse)
:
    HttpCore(),
    SocketServer(reuse),
    port_(defaultPort),
    root_(defaultRoot),
    name_(defaultName),
    cgiPrefix_(defaultCgiPrefix),
    cgibin_()
{
    if (port)
    {
        port_ = port;
    }

    this->bind(port);
}


markutil::HttpServer::HttpServer(const std::string& port, bool reuse)
:
    HttpCore(),
    SocketServer(reuse),
    port_(defaultPort),
    root_(defaultRoot),
    name_(defaultName),
    cgiPrefix_(defaultCgiPrefix),
    cgibin_()
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
    HttpCore(),
    SocketServer(reuse),
    port_(defaultPort),
    root_(defaultRoot),
    name_(defaultName),
    cgiPrefix_(defaultCgiPrefix),
    cgibin_()
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


bool markutil::HttpServer::name(const std::string& name)
{
    if (!name.empty())
    {
        name_ = name;
        return true;
    }

    return false;
}


const std::string& markutil::HttpServer::root() const
{
    return root_;
}


bool markutil::HttpServer::root(const std::string& path)
{
    return this->setPath(root_, path);
}


const std::string& markutil::HttpServer::cgibin() const
{
    return cgibin_;
}


bool markutil::HttpServer::cgibin(const std::string& path)
{
    return this->setPath(cgibin_, path);
}


const std::string& markutil::HttpServer::cgiPrefix() const
{
    return cgiPrefix_;
}


bool markutil::HttpServer::cgiPrefix(const std::string& path)
{
    return false;
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

            // fill with request
            head.request().readHeader(is);

            // fill host/peer information
            socketinfo_.setInfo(sockfd);


            // check for cgi-bin
            const std::string& url = head.request().path();
            const std::string& prefix = this->cgiPrefix();
            const size_t len = prefix.size();

            if (url.size() >= len && url.substr(0, len) == prefix)
            {
                if (url.size() == len)
                {
                    head(head._404_NOT_FOUND);
                    head.print(os, true);

#ifdef LINUX
                    sleep(1);      // let socket to drain
#endif
                    return 1;
                }
                else if (url[len] == '/')
                {
                    HttpRequest& req = head.request();

                    int ret = 1;
                    if (url.size() == len+1)
                    {
                        head(head._403_FORBIDDEN);
                        head.print(os, true);
                    }
                    else if (this->cgibin().empty())
                    {
                        head(head._404_NOT_FOUND);
                        head.print(os, true);
                    }
                    else if
                    (
                        req.type() == req.HEAD
                     || req.type() == req.GET
                    )
                    {
                        this->prepareCgiEnv(sockfd, head);

                        // serve cgi
                        ret = this->cgi(sockfd, head);
                    }
                    else
                    {
                        // only support HEAD, GET for cgi
                        head(head._405_METHOD_NOT_ALLOWED);
                        head("Allow", "GET,HEAD");
                        head.print(os, true);
                    }
#ifdef LINUX
                    sleep(1);      // let socket to drain
#endif
                    return ret;
                }
            }

            int ret = this->reply(os, head);
#ifdef LINUX
            sleep(1);      // let socket to drain
#endif
            return ret;
        }
    }

    return 0;
}


int markutil::HttpServer::cgi(int fd, HttpHeader& head) const
{
    std::string script_filename = head.request().path();

    // convert SCRIPT_URL -> SCRIPT_NAME
    // remove leading "/cgi-bin"
    script_filename.erase(0, cgiPrefix_.size());
    size_t slash = script_filename.find('/', 1);
    if (slash != std::string::npos)
    {
        script_filename.resize(slash);
    }

    script_filename = this->cgibin() + script_filename;

    struct stat sb;

    // assume error
    int ret = 1;
    FILE *pipe;

    // for execution
    if
    (
        ::stat(script_filename.c_str(), &sb) == 0
     && S_ISREG(sb.st_mode)
     && (pipe = ::popen(script_filename.c_str(), "r")) != NULL
    )
    {
        int pipeFd = ::fileno(pipe);

        // read and send in 8KB block - last block may be smaller
        size_t nread;
        while ( (nread = ::read(pipeFd, buffer, BufSize)) > 0 )
        {
            // okay, we did read something
            ret = 0;

            size_t noff = 0;
            while (nread)
            {
                size_t nwrite = ::write(fd, &buffer[noff], nread);
                nread -= nwrite;
                noff  += nwrite;
            }
        }

        pclose(pipe);
    }

    if (ret)
    {
        boost::fdostream os(fd);

        head(head._503_SERVICE_UNAVAILABLE);
        head.print(os);

        head.htmlBeg(os) << "<br />CGI-script: ";
        xmlEscapeChars(os, script_filename);
        head.htmlEnd(os);
    }

    return ret;
}


int markutil::HttpServer::server_info(std::ostream& os, HttpHeader& head) const
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

    os  << head(head._200_OK);

    if (req.type() == req.GET)
    {
        os  << "<html><head><title>server-info</title></head><body>"
            << "<hr /><h3>Server Info</h3><blockquote>"
            << "Date: " << head["Date"] << "<br />"
            << "Server-Software: " << this->name() << "<br />"
            << "Server-Address: " << socketinfo_.hostAddr() << "<br />"
            << "Server-Name: " << socketinfo_.hostName() << "<br />"
            << "Server-Port: " << this->port_ << "<br />"
            << "Remote-Address: " << socketinfo_.peerAddr() << "<br />"
            << "Remote-Name: " << socketinfo_.peerName() << "<br />"
            << "Remote-Port: " << socketinfo_.peerPort() << "<br />"
            << "Document-Root: " << this->root() << "<br />"
            << "CGI-bin: " << this->cgibin() << "<br />"
            << "Request: ";

        xmlEscapeChars
        (
            os,
            req.requestURI()
        ) << "</blockquote>";

        // raw request headers
        os  << "<hr /><h3>Request Headers</h3><blockquote>";
        const RequestType::RawHeaderType& rawHead = req.rawHeaders();
        for
        (
            RequestType::RawHeaderType::const_iterator iter = rawHead.begin();
            iter != rawHead.end();
            ++iter
        )
        {
            os  << iter->first << ": ";
            xmlEscapeChars(os, iter->second) << "<br />\n";
        }
        os  << "</blockquote><hr />";
        os  << "</body></html>\n";
    }

    return 0;
}


int markutil::HttpServer::reply(std::ostream& os, HttpHeader& head) const
{
    HttpRequest& req = head.request();

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

    // some of ours
    if (req.path() == "/server-info")
    {
        return this->server_info(os, head);
    }


    std::string mimeType = HttpCore::lookupMime(req.ext());
    if (mimeType.empty())
    {
        head(head._404_NOT_FOUND);
        head.print(os, true);

        return 1;
    }


    // open the file for reading
    const std::string file = root_ + req.path();
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
