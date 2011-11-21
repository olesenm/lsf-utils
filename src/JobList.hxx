/*---------------------------------*- C++ -*---------------------------------*\
                  ___                                _
                 / __)                              (_)
               _| |__ _____ _   _  ____ _____  ____ |_| _____
              (_   __|____ | | | |/ ___) ___ |/ ___)| |(____ |
                | |  / ___ | |_| | |   | ____( (___ | |/ ___ |
                |_|  \_____|____/|_|   |_____)\____)|_|\_____|

    Copyright (C) 2011-2011 Faurecia Emissions Control Technologies
-------------------------------------------------------------------------------
License
    This file contains proprietary and confidential information and
    is subject to a non-disclosure-agreement for use outside of FECT

Description
    --

Author
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
