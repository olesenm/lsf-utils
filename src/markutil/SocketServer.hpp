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
    markutil::SocketServer

Description
    A simple socket server.

SourceFiles
    SocketServer.cpp

\*---------------------------------------------------------------------------*/

#ifndef MARK_SOCKETSERVER_H
#define MARK_SOCKETSERVER_H

#include <string>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace markutil
{

/*---------------------------------------------------------------------------*\
                        Class SocketServer Declaration
\*---------------------------------------------------------------------------*/


class SocketServer
{

    // Private Member Functions

        //! Create a socket
        bool create(bool reuse);


protected:

    // Protected data

        //! known request methods
        enum SocketState
        {
            UNKNOWN = 0,  //!< unknown/invalid
            BOUND,        //!< file-descriptor bound to a port
            LISTENING,    //!< socket listening for connections
        };


        //! File-descriptor for socket
        int fd_;

        //! The current socket state
        SocketState state_;

        //! Simple error-tracking mechanism
        std::string error_;

public:

    // Constructors

        //! Create a socket
        explicit SocketServer(bool reuse = true);

        //! Create a socket and bind to specified port
        SocketServer(unsigned short port, bool reuse = true);

        //! Create a socket and bind to specified port
        SocketServer(const std::string& port, bool reuse = true);

        //! Create a socket and bind to specified port
        SocketServer(const char* port, bool reuse = true);


    //! Destructor
    ~SocketServer();


    // Member Functions

        //! \brief The socket file descriptor
        int sock() const;

        //! \brief Set the specified socket to be blocking
        //  \param sockfd The socket file descriptor
        //  \return true on success
        static bool setBlocking(int sockfd);

        //! \brief Set the specified socket to be non-blocking
        //  \param sockfd The socket file descriptor
        //  \return true on success
        static bool setNonBlocking(int sockfd);

        //! \brief Set the port for receiving data
        //  Only if not already bound
        //  \param port The port number
        //  \return port number on success, 0 on failure
        unsigned short bind(unsigned short port);

        //! \brief Set the port for receiving data
        //  Only if not already bound
        //  \param port The port number
        //  \return port number on success, 0 on failure
        unsigned short bind(const std::string& port);

        //! \brief Listen for incoming connections
        //  Only if not already listening
        //  \return true on success
        bool listen(int backlog = 64);

        //! \brief Accept an incoming connection
        //  \return File-descriptor of accepted socket or 0 on failure
        int accept();

        //! \brief Close the listening socket
        void close();

        //! \brief Is file-descriptor bound to a port?
        bool bound() const;

        //! \brief Is socket listening for connections?
        bool listening() const;


        //! \brief Simple error-tracking mechanism
        const std::string& error() const
        {
            return error_;
        }

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace markutil

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif  // MARK_SOCKETSERVER_H

// ************************************************************************* //
