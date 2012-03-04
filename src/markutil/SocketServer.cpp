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

#include "markutil/SocketServer.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <netdb.h>
// #include <sys/ioctl.h>


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

bool markutil::SocketServer::create(bool reuse)
{
    state_ = UNKNOWN;

    fd_ = ::socket(PF_INET, SOCK_STREAM, 0);
    if (fd_ == -1)
    {
        error_ = "SocketServer: failed to create a socket";
        return false;
    }

    // re-use socket address
    if (reuse)
    {
        int sockopt = 1;
        int retval = ::setsockopt
        (
            fd_,
            SOL_SOCKET,
            SO_REUSEADDR,
            &sockopt,
            sizeof(sockopt)
        );

        if (retval == -1)
        {
            error_ = "SocketServer: setsockopt error";
            this->close();
            return false;
        }
    }

    // Change to non-blocking socket
//    char arg;
//    if (::ioctl(fd_, FIONBIO, &arg) == -1)
//    {
//        error_ = "failed to set socket to non-blocking mode";
//        this->close();
//        return false;
//    }

    return true;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

markutil::SocketServer::SocketServer(bool reuse)
:
    fd_(-1),
    state_(UNKNOWN),
    error_()
{
    this->create(reuse);
}


markutil::SocketServer::SocketServer(unsigned short port, bool reuse)
:
    fd_(-1),
    state_(UNKNOWN),
    error_()
{
    this->create(reuse);
    this->bind(port);
}


markutil::SocketServer::SocketServer(const std::string& port, bool reuse)
:
    fd_(-1),
    state_(UNKNOWN),
    error_()
{
    this->create(reuse);
    this->bind(port);
}


markutil::SocketServer::SocketServer(const char* port, bool reuse)
:
    fd_(-1),
    state_(UNKNOWN),
    error_()
{
    this->create(reuse);
    if (port && *port)
    {
        this->bind(port);
    }
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

markutil::SocketServer::~SocketServer()
{
    this->close();
}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

unsigned short markutil::SocketServer::bind(unsigned short port)
{
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    // port == 0 is not permitted
    if (!port)
    {
        return 0;
    }

    // already bound - assume that the port hadn't changed
    if (this->bound())
    {
        return port;
    }

    const bool ok =
    (
        -1 != ::bind
        (
            fd_,
            reinterpret_cast<struct sockaddr *>(&serverAddr),
            sizeof(serverAddr)
        )
    );

    if (ok)
    {
        state_ = BOUND;
    }
    else
    {
        error_ = "bind to port failed";
        return 0;
    }

    return port;
}


unsigned short markutil::SocketServer::bind(const std::string& port)
{
    long val = strtol(port.c_str(), NULL, 10);

    if (val > 0 && val <= 65535)
    {
        unsigned short portNo(val);

        return this->bind(portNo);
    }
    else
    {
        return 0;
    }
}


bool markutil::SocketServer::listen(int backlog)
{
    // already listening
    if (this->listening())
    {
        return true;
    }
    else if (!this->bound())
    {
        error_ = "listen failed - socket not bound to a port";
        return false;
    }


    const bool ok =
    (
        -1 != ::listen(fd_, backlog)
    );

    if (ok)
    {
        state_ = LISTENING;
    }
    else
    {
        error_ = "listen failed";
    }

    return ok;
}


int markutil::SocketServer::accept()
{
    this->listen();      // only if not already listening

    struct sockaddr_in serverAddr;
    socklen_t s = sizeof(serverAddr);
    return ::accept(fd_, (struct sockaddr *)&serverAddr, &s);
}


void markutil::SocketServer::close()
{
    if (fd_ >= 0)
    {
        ::close(fd_);
        fd_ = -1;
    }
    state_ = UNKNOWN;
}


bool markutil::SocketServer::bound() const
{
    return state_ >= BOUND;
}


bool markutil::SocketServer::listening() const
{
    return state_ >= LISTENING;
}


// ************************************************************************* //
