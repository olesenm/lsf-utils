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
#include <sys/select.h>
#include <sys/wait.h>

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
    ::chdir("/");                 // detach from original directory
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


// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

bool markutil::HttpServer::notGetOrHead
(
    std::ostream& os,
    HttpHeader& head
)
const
{
    const RequestType& req = head.request();

    if (req.type() == req.GET || req.type() == req.HEAD)
    {
        return false;
    }

    head(head._405_METHOD_NOT_ALLOWED);
    head("Allow", "GET,HEAD");
    head.print(os, true);

    return true;
}



bool markutil::HttpServer::isCgi(const HttpHeader& head) const
{
    // check for cgi-bin
    const std::string& requestPath = head.request().path();
    const size_t beg = cgiPrefix_.size();

    if
    (
        requestPath.size() > beg
     && requestPath.substr(0, beg) == cgiPrefix_
     && requestPath[beg] == '/'
    )
    {
        return true;
    }

    return false;
}


bool markutil::HttpServer::cgiOkay
(
    std::ostream& os,
    HttpHeader& head
)
const
{
    if (this->cgibin().empty())
    {
        head(head._501_NOT_IMPLEMENTED);
        head.print(os);

        head.htmlBeg(os)
            << "<p>no CGI-bin defined for this server</p>";
        head.htmlEnd(os);

        return false;
    }

    const std::string& requestPath = head.request().path();
    const size_t beg   = cgiPrefix_.size();
    const size_t slash = requestPath.find('/', beg + 1);

    // attempt to serve cgi-bin
    if (requestPath.size() == beg+1)
    {
        head(head._403_FORBIDDEN);
        head.print(os, true);
        return false;
    }

    // convert SCRIPT_URL -> SCRIPT_NAME
    // remove leading "/cgi-bin"

    const std::string cgiProg =
    (
        this->cgibin()
      +
        (
            slash == std::string::npos
          ? requestPath.substr(beg)
          : requestPath.substr(beg, slash - beg)
        )
    );

    struct stat sb;
    if (::stat(cgiProg.c_str(), &sb) != 0 || !S_ISREG(sb.st_mode))
    {
        head(head._503_SERVICE_UNAVAILABLE);
        head.print(os);
        head.htmlBeg(os) << "<p>CGI does not exist or is not executable</p>";
        head.htmlEnd(os);
        return false;
    }

    return true;
}


std::string markutil::HttpServer::setCgiEnv(HttpHeader& head) const
{
    const HttpRequest& req = head.request();
    std::string script_url = req.path();

    // we might need the requested "Host:" as a fallback value
    // HTTP_HOST="<host>[:<port>]"
    std::string http_host = req["Host"];
    if (http_host.size())
    {
        setenv("HTTP_HOST", http_host.c_str(), 1);
    }
    else
    {
        unsetenv("HTTP_HOST");
    }

    //
    // REMOTE_ADDR, REMOTE_PORT from socket information
    // and REMOTE_HOST from gethostbyaddr
    //
    setenv("REMOTE_ADDR", req.socketInfo().peerAddr().c_str(), 1);
    setenv("REMOTE_HOST", req.socketInfo().peerName().c_str(), 1);
    setenv("REMOTE_PORT", req.socketInfo().peerPort().c_str(), 1);

    //
    // SERVER_ADDR, SERVER_PORT from socket information
    // and SERVER_NAME from gethostbyaddr
    //
    setenv("SERVER_ADDR", req.socketInfo().hostAddr().c_str(), 1);
    setenv("SERVER_NAME", req.socketInfo().hostName().c_str(), 1);
    setenv("SERVER_PORT", req.socketInfo().hostPort().c_str(), 1);

    std::string server_url = "http://" + req.socketInfo().hostName();
    if (req.socketInfo().hostPort().size() && req.socketInfo().hostPort() != "80")
    {
        server_url += ":" + req.socketInfo().hostPort();
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
    else
    {
        unsetenv("HTTP_USER_AGENT");
    }

    //
    // partially cleanse environment
    //
    setenv("PATH", "/usr/bin:/bin", 1);
    unsetenv("LD_LIBRARY_PATH");

    return script_filename;
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


int markutil::HttpServer::run_fork()
{
    this->bind(port_);   // only if not already bound
    this->listen();      // only if not already listening

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
            head.request().socketInfo().setInfo(sockfd);

            int ret = 1;    // assume failure

            // check for cgi-bin
            if (this->isCgi(head))
            {
                if (this->cgiOkay(os, head))
                {
                    // serve cgi
                    ret = this->cgi(sockfd, head);
                }
            }
            else
            {
                // serve normal document
                ret = this->reply(os, head);
            }

            return ret;
        }
    }

    return 0;
}


int markutil::HttpServer::run_select()
{
    this->setNonBlocking(this->sock());  // make socket non-blocking
    this->bind(port_);       // only if not already bound
    this->listen();          // only if not already listening

    const int listenFd = this->sock();

    fd_set monitorFds;

    // add listener to the monitored file descriptors
    FD_ZERO(&monitorFds);
    FD_SET(listenFd, &monitorFds);

    // track the largest file descriptor
    int maxFd = listenFd;  // currently it is the listener

    while (true)
    {
        // copy in monitored descriptors
        fd_set readFds = monitorFds;

        int nRead = ::select
        (
            maxFd+1,
            &readFds,    // readfds
            NULL,        // writefds
            NULL,        // exceptfds
            NULL         // timeout - block until something is there
        );

        if (nRead <= 0)
        {
            // select error or nothing ready to read, which would
            // be strange since we blocked
            continue;
        }

        // run through the existing connections looking for data to read
        for (int fdI = 0; fdI <= maxFd; ++fdI)
        {
            if (!FD_ISSET(fdI, &readFds))
            {
                // skip - not this descriptor
                continue;
            }

            if (fdI == listenFd)
            {
                // handle new connection
                const int sockfd = this->accept();

                if (sockfd >= 0)
                {
                    this->setNonBlocking(sockfd);
                    FD_SET(sockfd, &monitorFds);

                    if (maxFd < sockfd)
                    {
                        maxFd = sockfd;
                    }
                }
            }
            else
            {
                const int sockfd = fdI;

                boost::fdistream is(sockfd);
                boost::fdostream os(sockfd);

                HttpHeader head;
                head("Server", this->name());

                // fill with request
                head.request().readHeader(is);

                // fill host/peer information
                head.request().socketInfo().setInfo(sockfd);

                int ret = 1;    // assume failure

                // check for cgi-bin
                if (this->isCgi(head))
                {
                    if (this->cgiOkay(os, head))
                    {
                        // serve cgi
                        ret = this->cgi(sockfd, head);
                    }
                }
                else
                {
                    // serve normal document
                    ret = this->reply(os, head);
                }

                ::close(sockfd);
                FD_CLR(sockfd, &monitorFds); // remove from monitored descriptors

                // maxFd may need adjustment
                if (maxFd == sockfd)
                {
                    for (int i = maxFd; i >= listenFd; --i)
                    {
                        if (FD_ISSET(i, &monitorFds))
                        {
                            maxFd = i;
                            break;
                        }
                    }
                }

            }
        }
    }

    return 0;
}


int markutil::HttpServer::run(RunType how)
{
    switch (how)
    {
        case FORKING:
            return run_fork();
            break;
        case SELECT:
            return run_select();
            break;
    }

    return 0;
}


int markutil::HttpServer::server_about(std::ostream& os, HttpHeader& head) const
{
    const RequestType& req = head.request();

    const char* const br = "<br />\n";

    if (notGetOrHead(os, head))
    {
        return 1;
    }

    os  << head(head._200_OK);

    if (req.type() == req.GET)
    {
        os  << "<html><head><title>about:"
            << this->name() << "</title></head>";

        os  << "<body>" << head["Date"] << "\n";
        os  << "<blockquote><p>\n"
            << "Server-Address: "  << req.socketInfo().hostAddr() << br
            << "Server-Name: "     << req.socketInfo().hostName() << br
            << "Server-Port: "     << this->port_ << br
            << "Remote-Address: "  << req.socketInfo().peerAddr() << br
            << "Remote-Name: "     << req.socketInfo().peerName() << "</p>\n";

        os  << "<p>\n"
            << "Document-Root: "   << this->root() << br;

        // The CGI-bin may not be enabled
        os  << "CGI-bin: ";
        if (this->cgibin().empty())
        {
            os  << "not defined for this server";
        }
        else
        {
            os  << this->cgibin();
        }
        os  << "</p></blockquote>\n";

        this->content_about(os, head);

        os << "<hr />"
            << this->name() << "</body></html>\n";
    }

    return 0;
}


int markutil::HttpServer::server_info(std::ostream& os, HttpHeader& head) const
{
    const RequestType& req = head.request();

    const char* const br = "<br />\n";

    if (notGetOrHead(os, head))
    {
        return 1;
    }

    os  << head(head._200_OK);

    if (req.type() == req.GET)
    {
        os  << "<html><head><title>server-info</title></head><body>"
            << "<hr /><h3>Server Info</h3><blockquote>\n"
            << "Date: "            << head["Date"] << br
            << "Server-Software: " << this->name() << br
            << "Server-Address: "  << req.socketInfo().hostAddr() << br
            << "Server-Name: "     << req.socketInfo().hostName() << br
            << "Server-Port: "     << this->port_ << br
            << "Remote-Address: "  << req.socketInfo().peerAddr() << br
            << "Remote-Name: "     << req.socketInfo().peerName() << br
            << "Remote-Port: "     << req.socketInfo().peerPort() << br
            << "Document-Root: "   << this->root() << br;

        // The CGI-bin may not be enabled
        os  << "CGI-bin: ";
        if (this->cgibin().empty())
        {
            os  << "not defined for this server";
        }
        else
        {
            os  << this->cgibin();
        }
        os  << br;

        os  << "Request: ";
        xmlEscapeChars
        (
            os,
            req.requestURI()
        ) << "</blockquote>\n";

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
            xmlEscapeChars(os, iter->second) << br;
        }
        os  << "</blockquote>\n";

        this->content_info(os, head);

        os  << " <hr />";
        os  << "</body></html>\n";
    }

    return 0;
}


int markutil::HttpServer::cgi(int fd, HttpHeader& head) const
{
    boost::fdostream os(fd);
    if (notGetOrHead(os, head))
    {
        return 1;
    }

    int ret = 1;    // assume error
    std::string error;

    // this is really just popen(), but done by hand so that the
    // environment is only affected in the child process and to avoid
    // any possible shell effects
    int pipeFds[2];
    if (::pipe(pipeFds) == 0)
    {
        int readerFd = pipeFds[0];
        int writerFd = pipeFds[1];

        const int pid = ::fork();

        if (pid == 0)
        {
            // child - attach stdout to pipe
            ::close(readerFd);
            if (writerFd != 1)
            {
                ::close(1);
                ::dup2(writerFd, 1);
                ::close(writerFd);
            }

            const std::string cgiProg = setCgiEnv(head);

            ::execl(cgiProg.c_str(), cgiProg.c_str(), NULL);
            ::_exit(255);
        }
        else
        {
            ::close(writerFd);
            if (pid < 0)
            {
                // fork failed - no need to wait
                ::close(readerFd);
            }
            else
            {
                // parent

                // read and send blockwise - last block may be smaller
                size_t nread;
                while ( (nread = ::read(readerFd, buffer, BufSize)) > 0 )
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

                // this is really just pclose
                if (::close(readerFd) == 0)
                {
                    int waitstat;
                    ::wait(&waitstat);
                }
            }
        }
    }
    else
    {
        error = "could not open a pipe for reading";
    }

    if (ret)
    {
        boost::fdostream os(fd);

        head(head._503_SERVICE_UNAVAILABLE);
        head.print(os);

        head.htmlBeg(os)
            << "<p>" << (error.empty() ? "returned nothing" : error) << "</p>";
        head.htmlEnd(os);
    }

    return ret;
}


int markutil::HttpServer::reply(std::ostream& os, HttpHeader& head) const
{
    HttpRequest& req = head.request();

    if (notGetOrHead(os, head))
    {
        return 1;
    }

    // some of ours
    if (req.path() == "/about")
    {
        return this->server_about(os, head);
    }

    if (req.path() == "/server-info")
    {
        return this->server_info(os, head);
    }


    // the file name and file-descriptor
    std::string file = this->root() + req.path();
    std::string mimeType;
    int fileFd = -1;


    // rewrite rules:
    // - convert trailing slash to index.html file
    // - special treatment for "/" request:
    //   * return server-about if there is no index.html
    //
    if (*(file.rbegin()) == '/')
    {
        file += "index.html";
        fileFd = ::open(file.c_str(), O_RDONLY);

        if (fileFd == -1)
        {
            if (req.path() == "/")
            {
                return this->server_about(os, head);
            }
        }
        else
        {
            mimeType = HttpCore::lookupMime("html");
        }
    }
    else
    {
        mimeType = HttpCore::lookupMime(req.ext());
    }


    if (mimeType.empty())
    {
        head(head._404_NOT_FOUND);
        head.print(os, true);

        return 1;
    }


    // open the file for reading if not previously opened
    if (fileFd == -1)
    {
        fileFd = ::open(file.c_str(), O_RDONLY);
    }

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
        // send file blockwise - last block may be smaller
        size_t nbyte;
        while ( (nbyte = ::read(fileFd, buffer, BufSize)) > 0 )
        {
            os.write(buffer, nbyte);
        }
    }

    ::close(fileFd);

    return 0;
}


// ************************************************************************* //
