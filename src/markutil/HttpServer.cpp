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
#include <netdb.h>
#include <signal.h>
#include <unistd.h>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

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


std::string markutil::HttpServer::hostName()
{
    char buf[128];
    ::gethostname(buf, sizeof(buf));

    // implementation as per hostname from net-tools
    struct hostent *hp = ::gethostbyname(buf);
    if (hp)
    {
        return hp->h_name;
    }

    return buf;
}


// local scope
static bool getAddrAndPort
(
    int fd,
    const std::string& prefix,
    std::string& sock_addr,
    std::string& sock_port,
    std::string& sock_host
)
{
    sock_addr.clear();
    sock_port.clear();
    sock_host.clear();

    // get socket information
    struct sockaddr_in sockAddr;
    socklen_t sin_len = sizeof(sockAddr);
    if (prefix == "REMOTE")
    {
        if
        (
            ::getpeername
            (
                fd,
                reinterpret_cast<struct sockaddr *>(&sockAddr),
                &sin_len
            )
         || !sin_len
        )
        {
            return false;
        }
    }
    else if (prefix == "SERVER")
    {
        if
        (
            ::getsockname
            (
                fd,
                reinterpret_cast<struct sockaddr *>(&sockAddr),
                &sin_len
            )
         || !sin_len
        )
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    // convert IP and PORT
    // PORT is always smaller that any IP-Address in string form
    char buf[INET_ADDRSTRLEN];

    sprintf(buf, "%d", ntohs(sockAddr.sin_port));
    sock_port = buf;

    if
    (
        inet_ntop
        (
            AF_INET,
            &(sockAddr.sin_addr.s_addr),
            buf,
            sizeof(buf)
        )
    )
    {
        sock_addr = buf;

        struct hostent *hp = ::gethostbyaddr
        (
            &(sockAddr.sin_addr.s_addr),
            sizeof(sockAddr.sin_addr.s_addr),
            AF_INET
        );
        if (hp)
        {
            sock_host = hp->h_name;
        }
    }
    else
    {
        return false;
    }

    return true;
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
            char buf[255];
            if (!::getcwd(buf, 255))
            {
                return false;
            }
            target = buf;
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
    else
    {
        unsetenv("HTTP_HOST");
    }

    //
    // get REMOTE_ADDR, REMOTE_PORT from socket information
    // and REMOTE_HOST from gethostbyaddr
    //
    std::string remote_addr, remote_port, remote_host;
    getAddrAndPort(fd, "REMOTE", remote_addr, remote_port, remote_host);
    if (remote_addr.size() && remote_port.size())
    {
        setenv("REMOTE_ADDR", remote_addr.c_str(), 1);
        setenv("REMOTE_PORT", remote_port.c_str(), 1);
        if (remote_host.size())
        {
            setenv("REMOTE_HOST", remote_host.c_str(), 1);
        }
        else
        {
            unsetenv("REMOTE_HOST");
        }
    }
    else
    {
        unsetenv("REMOTE_ADDR");
        unsetenv("REMOTE_PORT");
        unsetenv("REMOTE_HOST");
    }

    //
    // get SERVER_ADDR, SERVER_PORT from socket information
    // and SERVER_NAME from gethostbyaddr
    //
    std::string server_addr, server_port, server_name;
    getAddrAndPort(fd, "SERVER", server_addr, server_port, server_name);
    if (server_addr.size() && server_port.size())
    {
        setenv("SERVER_ADDR", server_addr.c_str(), 1);
    }
    else
    {
        unsetenv("SERVER_ADDR");
    }

    if (server_port.empty())
    {
        server_port = "80";    // default HTTP port
    }

    //
    // if reverse lookup of SERVER_ADDR -> SERVER_NAME failed ...
    //
    if (server_name.empty())
    {
        if (http_host.size())
        {
            // 1. From the "Host:" - this represents the externally seen name
            server_name = http_host;

            // split into <host>:<port>
            size_t colon = server_name.find(':');
            if (colon != std::string::npos)
            {
                server_port = server_name.substr(colon + 1);
                server_name.resize(colon);

                // should not happen
                if (server_port.empty())
                {
                    server_port = "80";    // default HTTP port
                }
            }
        }
        else
        {
            // 2. Just use the host-name directly and hope it can also be
            //    seen externally
            server_name = this->hostName();

            std::ostringstream oss;
            oss << port_;
            server_port = oss.str();
        }
    }

    setenv("SERVER_NAME", server_name.c_str(), 1);
    setenv("SERVER_PORT", server_port.c_str(), 1);

    std::string server_url = "http://" + server_name;
    if (server_port != "80")
    {
        server_url += ":" + server_port;
    }


    // Apache may not export this
    // SERVER_URL="http://<host>[:<port>]/"
    setenv("SERVER_URL", (server_url + '/').c_str(), 1);


    std::string script_uri = server_url + script_url;
    server_url += "/";

    setenv("REQUEST_METHOD", req.method().c_str(), 1);

    setenv("SERVER_NAME", server_name.c_str(), 1);
    setenv("SERVER_PORT", server_port.c_str(), 1);
    setenv("SERVER_PROTOCOL", req.protocol().c_str(), 1);
    setenv("SERVER_SOFTWARE", this->name().c_str(), 1);

    setenv("DOCUMENT_ROOT", this->root().c_str(), 1);

    // QUERY_STRING is everything after the '?'
    setenv("QUERY_STRING", req.query().toString().c_str(), 1);

    // REQUEST_URI is "/cgi-bin/...?..."
    setenv("REQUEST_URI", req.requestURI().c_str(), 1);

    // SCRIPT_URI="http://<host>[:<port>]/cgi-bin/..."
    // without the query string
    setenv("SCRIPT_URI", script_uri.c_str(), 1);

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

    // remove leading "/cgi-bin"
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
    // partially cleanse the environment
    //
    setenv("PATH", "/usr/bin:/bin", 1);
    unsetenv("LD_LIBRARY_PATH");

    unsetenv("SSH_CLIENT");
    unsetenv("SSH_TTY");
    unsetenv("OLDPWD");
    unsetenv("USER");
    unsetenv("MAIL");
    unsetenv("HOME");
    unsetenv("LOGNAME");
    unsetenv("USER");
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


            // check for cgi-bin
            const std::string& url = head.request().path();
            const std::string& prefix = this->cgiPrefix();
            const size_t len = prefix.size();

            if (url.size() >= len && url.substr(0, len) == prefix)
            {
                if
                (
                    url.size() == len
                 || (url.size() == len+1 && (url[len] == '/'))
                )
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
                    // only support HEAD, GET for cgi
                    HttpRequest& req = head.request();

                    int ret = 1;
                    if (this->cgibin().empty())
                    {
                        head(head._404_NOT_FOUND);
                        head.print(os, true);
                    }
                    else if
                    (
                        req.type() != req.HEAD
                     && req.type() != req.GET
                    )
                    {
                        head(head._405_METHOD_NOT_ALLOWED);
                        head("Allow", "GET,HEAD");
                        head.print(os, true);
                    }
                    else
                    {
                        this->prepareCgiEnv(sockfd, head);

                        // serve cgi
                        ret = this->cgi(sockfd, head);
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
    HttpRequest& req = head.request();
    std::string script_filename = req.path();

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
        head.print(os, true);
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
        os  << "<html><head>"
            << "<title>server-info</title>"
            << "</head><body>"
            << "<hr /><h3>Server Info</h3><blockquote>"
            << "Date: " << head["Date"] << "<br />"
            << "Server-Software: " << this->name() << "<br />"
            << "Server-Name: " << this->hostName() << "<br />"
            << "Server-Port: " << this->port_ << "<br />"
            << "Document-Root: " << this->root() << "<br />"
            << "cgi-bin: " << this->cgibin() << "<br />"
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
