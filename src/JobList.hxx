/*---------------------------------*- C++ -*---------------------------------*\
Copyright 2011-2011 Mark Olesen

License
    This file is part of lsf-utils

    lsf-utils is free software: you can redistribute it and/or modify it under
    the terms of the GNU Affero General Public License as published by the
    Free Software Foundation, either version 3 of the License,
    or (at your option) any later version.

    lsf-utils is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
    or FITNESS FOR A PARTICULAR PURPOSE.
    See the GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with lsf-utils. If not, see <http://www.gnu.org/licenses/>.

Description
    --

\*---------------------------------------------------------------------------*/

#ifndef JobList_H
#define JobList_H

#include <ctime>
#include <string>
#include <vector>
#include <iostream>

#include "JobInfoEntry.hxx"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace LsfUtil
{

// Forward declaration of friend functions and operators
class JobList;


/*---------------------------------------------------------------------------*\
                    Class JobList Declaration
\*---------------------------------------------------------------------------*/

class JobList
:
    private std::vector<LsfUtil::JobInfoEntry>
{
    // Private data

        //- The last update time
        time_t lastUpdate_;

        //- The update interval
        unsigned interval_;

        //- Error
        bool error_;

        //- variables for simulating bjobs command
        int options_;

public:

    // Static data members


    // Constructors

        //- Construct
        JobList(unsigned interval = 10, bool withPending = true);


    //- Destructor
    ~JobList();


    // Member Functions

        // Access

        bool hasError() const
        {
            return error_;
        }

        using std::vector<LsfUtil::JobInfoEntry>::empty;
        using std::vector<LsfUtil::JobInfoEntry>::size;
        using std::vector<LsfUtil::JobInfoEntry>::operator[];

        // Check

        // Edit
        bool update();

        // Write

        std::ostream& printXML(std::ostream&) const;


    // Member Operators

    // Friend Functions

    // Friend Operators

    // IOstream Operators

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace LsfUtil

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// #include "JobListI.hxx"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
