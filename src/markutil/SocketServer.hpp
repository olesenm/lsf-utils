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
    markutil::SocketServer

Description

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

        //! File-descriptor for socket
        int fd_;

        //! Is file-descriptor bound to a port?
        bool bound_;

        //! Is socket listening for connections?
        bool listen_;

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


    // Destructor

        //! \brief Destructor
        ~SocketServer();

    // Member Functions

        //! \brief Set the port for receiving data
        //  \param port The port number
        //  \return port number on success, 0 on failure
        unsigned short bind(unsigned short port);

        //! \brief Set the port for receiving data
        //  \param port The port number
        //  \return port number on success, 0 on failure
        unsigned short bind(const std::string& port);

        //! \brief Listen for incoming connections
        //  \return true on success
        bool listen(int backlog = 64);

        //! \brief Accept an incoming connection
        //  \return File-descriptor of accepted socket or 0 on failure
        int accept();

        //! \brief Close the listening socket
        void close();

        //! \brief Is file-descriptor bound to a port?
        bool bound() const
        {
            return bound_;
        }

        //! \brief Is socket listening for connections?
        bool listening() const
        {
            return bound_;
        }


        //! \brief Simple error-tracking mechanism
        const std::string& error() const
        {
            return error_;
        }

};


} // End namespace markutil

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif  // MARK_SOCKETSERVER_H

// ************************************************************************* //
