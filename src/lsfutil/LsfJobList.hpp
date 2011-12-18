/*---------------------------------*- C++ -*---------------------------------*\
Copyright (c) 2011-2011 Mark Olesen
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

Description
    --

\*---------------------------------------------------------------------------*/

#ifndef LSF_JOB_LIST_H
#define LSF_JOB_LIST_H

#include <string>
#include <vector>

#include "lsfutil/LsfJobEntry.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace lsfutil
{

// Forward declaration of friend functions and operators
class LsfJobList;


/*---------------------------------------------------------------------------*\
                         Class LsfJobList Declaration
\*---------------------------------------------------------------------------*/

class LsfJobList
:
    private std::vector<lsfutil::LsfJobEntry>
{
    // Private data

        //! The last update time
        time_t lastUpdate_;

        //! The update interval
        unsigned interval_;

        //! Error
        bool error_;

        //! variables for simulating bjobs command
        int options_;

public:

    // Static data members


    // Constructors

        //! Construct
        LsfJobList(unsigned interval = 10, bool withPending = true);


    //! Destructor
    ~LsfJobList();


    // Member Functions

        // Access

        bool hasError() const
        {
            return error_;
        }

        using std::vector<lsfutil::LsfJobEntry>::empty;
        using std::vector<lsfutil::LsfJobEntry>::size;
        using std::vector<lsfutil::LsfJobEntry>::operator[];

        // Check

        // Edit
        bool update();

        // Write

    // Member Operators

    // Friend Functions

    // Friend Operators

    // IOstream Operators

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace lsfutil

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif  // LSF_JOB_LIST_H

// ************************************************************************* //
