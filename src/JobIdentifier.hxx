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

#ifndef JobIdentifier_H
#define JobIdentifier_H

#include <string>
#include <sstream>

#include <lsf/lsbatch.h>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace LsfUtil
{

/*---------------------------------------------------------------------------*\
                        Class JobIdentifier Declaration
\*---------------------------------------------------------------------------*/

class JobIdentifier
{
public:

    // Public data

        //- The job ID
        int jobId;

        //- The task ID. Is 0 for non-array job
        int taskId;

        //- The job owner
        std::string user;

        //- The current working directory when the job was submitted
        std::string cwd;

        //- The output filename (absolute or relative to cwd)
        std::string outfile;

        std::string jobIdString;

    // Constructors

        //- Construct from jobInfoEnt
        JobIdentifier(const struct jobInfoEnt&);


    // Member Functions

        inline bool hasTasks() const
        {
            return taskId > 0;
        }

        std::string tokenJ() const;

};



// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace LsfUtil

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //


#endif

// ************************************************************************* //
