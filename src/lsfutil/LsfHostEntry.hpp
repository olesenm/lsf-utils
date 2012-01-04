/*---------------------------------*- C++ -*---------------------------------*\
Copyright (c) 2012 Mark Olesen
-------------------------------------------------------------------------------
License
    This file is part of lsf-utils

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
    lsfutil::LsfHostEntry

Description
    Encapsulation of LSF \c hostInfoEnt structure into a C++ class.

    Since the primary purpose of the class it to avoid memory
    allocation/de-allocation issues and provide other C++ conveniences,
    almost all information is available directly as public members.

\*---------------------------------------------------------------------------*/

#ifndef LSF_HOST_ENTRY_H
#define LSF_HOST_ENTRY_H

#include <ctime>
#include <string>
#include <vector>
#include <iostream>

#include "lsfutil/LsfCore.hpp"

// Forward declaration of classes
struct hostInfoEnt;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace lsfutil
{

// Forward declaration of friend functions and operators
class LsfHostEntry;


/*---------------------------------------------------------------------------*\
                    Class LsfHostEntry Declaration
\*---------------------------------------------------------------------------*/

class LsfHostEntry
:
    public LsfCore
{
    // Private Data


    // Private Member Functions

public:
    // Static data members


    // Public data

        //- The host name
        std::string name;

        //- The load over the last 15 minutes
        float  load_15m;

        //- The amount of free disk space in the file system containing /tmp, in MB
        float  free_tmp;

        //- The amount of swap space available, in MB
        float  free_swp;

        //- The amount of available user memory on this host, in MB
        float  free_mem;

        //- Maximum job slots
        int maxJobs;

        //- The number of job slots running or suspended
        int numJobs;

        //- The number of job slots running
        int numRUN;

        //- The names of the associated queues
        std::vector<std::string> queues;


    // Constructors

        //! Construct from hostInfoEnt
        LsfHostEntry(const hostInfoEnt&);


    //! Destructor
    ~LsfHostEntry();


    // Member Functions

        // Access

        // Check


        // Edit

        // Write

            //- Raw dump of information in text format
            std::ostream& dump(std::ostream&) const;


    // Member Operators


    // Friend Functions

    // Friend Operators

    // IOstream Operators

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace lsfutil

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif  // LSF_HOST_ENTRY_H

// ************************************************************************* //
