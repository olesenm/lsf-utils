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

Class
    markutil::SocketInfo

Description
    Utilities for getting information from a socket

SourceFiles
    SocketInfo.cpp

\*---------------------------------------------------------------------------*/

#ifndef MARK_SOCKETINFO_H
#define MARK_SOCKETINFO_H

#include <string>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace markutil
{

/*---------------------------------------------------------------------------*\
                         Class SocketInfo Declaration
\*---------------------------------------------------------------------------*/

class SocketInfo
{
    // Private data

        //! \brief The host address
        std::string hostAddr_;

        //! \brief The host name
        std::string hostName_;

        //! \brief The host port
        std::string hostPort_;

        //! \brief The peer address
        std::string peerAddr_;

        //! \brief The peer name
        std::string peerName_;

        //! \brief The peer port
        std::string peerPort_;


    // Private Member Functions

        //! \brief Get address and port
        bool getAddrAndPort
        (
            int fd,
            bool isPeer,
            std::string& sock_addr,
            std::string& sock_host,
            std::string& sock_port
        );


public:

    // Constructors

        //! \brief Construct null
        SocketInfo();

        //! \brief Construct with information from socket file-descriptor
        explicit SocketInfo(int sockfd);


    //! Destructor
    ~SocketInfo();


    // Member Functions

        // Access

            //! \brief The host address
            const std::string& hostAddr() const;

            //! \brief The host name
            const std::string& hostName() const;

            //! \brief The host port
            const std::string& hostPort() const;

            //! \brief The peer address
            const std::string& peerAddr() const;

            //! \brief The peer name
            const std::string& peerName() const;

            //! \brief The peer port
            const std::string& peerPort() const;


        // Edit

            //! \brief Set (host/peer) information from socket file-descriptor
            void setInfo(int sockfd);

            //! \brief Set host information from socket file-descriptor
            void setHostInfo(int sockfd);

            //! \brief Set peer information from socket file-descriptor
            void setPeerInfo(int sockfd);

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace markutil

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif  // MARK_SOCKETINFO_H

// ************************************************************************* //
