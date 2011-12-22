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


void markutil::HttpServer::prepareCgiEnv(HttpHeader& head) const
{
    HttpRequest& req = head.request();

    std::string server_name = this->hostName();
    std::string script_url = req.path();
    std::string server_url = "http://" + server_name;

    std::string server_port("80");
    {
        std::ostringstream oss;
        oss << port_;
        server_port = oss.str();
    }
    if (port_ != 80)
    {
        server_url += ":" + server_port;
    }

    std::string script_uri = server_url + script_url;
    server_url += "/";


    // safer environment
    setenv("PATH", "/usr/bin:/bin", 1);
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


    if (req["User-Agent"].size())
    {
        setenv("HTTP_USER_AGENT", req["User-Agent"].c_str(), 1);
    }
    else
    {
        unsetenv("HTTP_USER_AGENT");
    }

    if (req["Referer"].size())
    {
        setenv("HTTP_REFERER", req["Referer"].c_str(), 1);
    }
    else
    {
        unsetenv("HTTP_REFERER");
    }

    if (req["Host"].size())
    {
        setenv("HTTP_HOST", req["Host"].c_str(), 1);
    }
    else
    {
        unsetenv("HTTP_HOST");
    }

    setenv("REQUEST_METHOD", req.method().c_str(), 1);

    setenv("SERVER_NAME", server_name.c_str(), 1);
    setenv("SERVER_PORT", server_port.c_str(), 1);
    setenv("SERVER_URL",  server_url.c_str(), 1);
    setenv("SERVER_PROTOCOL", req.protocol().c_str(), 1);
    setenv("SERVER_SOFTWARE", this->name().c_str(), 1);

    setenv("DOCUMENT_ROOT", this->root().c_str(), 1);

    // REMOTE_HOST
    // REMOTE_ADDR
    // REMOTE_PORT

    setenv("QUERY_STRING", req.query().toString().c_str(), 1);
    setenv("REQUEST_URI", req.requestURI().c_str(), 1);

    setenv("SCRIPT_URI", script_uri.c_str(), 1);
    setenv("SCRIPT_URL", script_url.c_str(), 1);

    // split SCRIPT_URL into SCRIPT_NAME + PATH_INFO

    // remove leading "/cgi-bin"
    script_url.erase(0, cgiPrefix_.size());
    size_t slash = script_url.find('/', 1);
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

    setenv("SCRIPT_NAME", script_url.c_str(), 1);
    setenv("SCRIPT_FILENAME", script_filename.c_str(), 1);
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
                        this->prepareCgiEnv(head);

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
    const unsigned canExec = (S_IRUSR | S_IXUSR);


    // assume error
    int ret = 1;
    FILE *pipe;

    // for execution
    if
    (
        ::stat(script_filename.c_str(), &sb) == 0
     && (sb.st_mode & canExec) == canExec
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
        ) << "<br />";
        os  << "<hr />";
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
