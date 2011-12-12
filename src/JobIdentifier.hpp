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
