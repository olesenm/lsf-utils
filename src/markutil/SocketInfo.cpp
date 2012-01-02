/*---------------------------------*- C++ -*---------------------------------*\
Copyright (c) 2012 Mark Olesen
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

#include "markutil/SocketInfo.hpp"

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <netdb.h>
#include <arpa/inet.h>


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

bool markutil::SocketInfo::getAddrAndPort
(
    int fd,
    bool isPeer,
    std::string& sock_addr,
    std::string& sock_host,
    std::string& sock_port
)
{
    sock_addr.clear();
    sock_host.clear();
    sock_port.clear();

    // get socket information
    struct sockaddr_in sockAddr;
    socklen_t sin_len = sizeof(sockAddr);
    if (isPeer)
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
    else
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
        else
        {
            // reverse lookup failed ...
            sock_host = sock_addr;
        }
    }
    else
    {
        return false;
    }

    return true;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

markutil::SocketInfo::SocketInfo()
:
    hostAddr_(),
    hostName_(),
    hostPort_(),
    peerAddr_(),
    peerName_(),
    peerPort_()
{}


markutil::SocketInfo::SocketInfo(int sockfd)
:
    hostAddr_(),
    hostName_(),
    hostPort_(),
    peerAddr_(),
    peerName_(),
    peerPort_()
{
    this->setInfo(sockfd);
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

markutil::SocketInfo::~SocketInfo()
{}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

const std::string& markutil::SocketInfo::hostAddr() const
{
    return hostAddr_;
}


const std::string& markutil::SocketInfo::hostName() const
{
    return hostName_;
}


const std::string& markutil::SocketInfo::hostPort() const
{
    return hostPort_;
}


const std::string& markutil::SocketInfo::peerAddr() const
{
    return peerAddr_;
}


const std::string& markutil::SocketInfo::peerName() const
{
    return peerName_;
}


const std::string& markutil::SocketInfo::peerPort() const
{
    return peerPort_;
}


void markutil::SocketInfo::setInfo(int sockfd)
{
    setHostInfo(sockfd);
    setPeerInfo(sockfd);
}


void markutil::SocketInfo::setHostInfo(int sockfd)
{
    getAddrAndPort(sockfd, false, hostAddr_, hostName_, hostPort_);
}


void markutil::SocketInfo::setPeerInfo(int sockfd)
{
    getAddrAndPort(sockfd, true, peerAddr_, peerName_, peerPort_);
}


// ************************************************************************* //
