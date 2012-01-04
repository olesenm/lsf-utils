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
    lsfutil::LsfHostList

Description
    A list of lsfutil::LsfHostEntry elements.

\*---------------------------------------------------------------------------*/

#ifndef LSF_HOST_LIST_H
#define LSF_HOST_LIST_H

#include <string>
#include <vector>
#include <iostream>

#include "lsfutil/LsfHostEntry.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace lsfutil
{

// Forward declaration of friend functions and operators
class LsfHostList;


/*---------------------------------------------------------------------------*\
                         Class LsfHostList Declaration
\*---------------------------------------------------------------------------*/

class LsfHostList
:
    private std::vector<lsfutil::LsfHostEntry>
{
    // Private data

        //! The last update time
        time_t lastUpdate_;

        //! The update interval
        unsigned interval_;

        //! Error
        bool error_;

public:

    // Constructors

        //! Construct with a given update interval
        //  In the future, allow for internal caching
        LsfHostList(unsigned interval = 10);


    //! Destructor
    ~LsfHostList();


    // Member Functions

        // Access

            using std::vector<lsfutil::LsfHostEntry>::empty;
            using std::vector<lsfutil::LsfHostEntry>::size;
            using std::vector<lsfutil::LsfHostEntry>::operator[];


        // Check

            //- Any errors encountered?
            inline bool hasError() const
            {
                return error_;
            }


        // Edit

            //- Populate the list with contents
            bool update();


        // Write

            //- Raw dump of information in text format
            std::ostream& dump(std::ostream&) const;

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace lsfutil

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif  // LSF_HOST_LIST_H

// ************************************************************************* //
