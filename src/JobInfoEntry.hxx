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

#ifndef JobInfoEntry_H
#define JobInfoEntry_H

#include <ctime>
#include <string>
#include <vector>

#include "JobSubmitEntry.hxx"

// Forward declaration of classes
struct jobInfoEnt;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace LsfUtil
{

// Forward declaration of friend functions and operators
class JobInfoEntry;


/*---------------------------------------------------------------------------*\
                    Class JobInfoEntry Declaration
\*---------------------------------------------------------------------------*/

class JobInfoEntry
{
    // Private Member Functions

        static std::string jobStatusToString(const struct jobInfoEnt&);

public:

    // Static data members


    // Public data
    //

        JobSubmitEntry submit;

        //- The job ID that the LSF system assigned to the job
        //  split into job-id + task-id
        int jobId;
        int taskId;

        std::string status;

        //- The name of the user who submitted the job
        std::string user;

        //- The time the job was submitted, in seconds since 00:00:00 GMT, Jan. 1, 1970.
        time_t submitTime;

        //- Time when job slots are reserved
        time_t reserveTime;

        //- The time that the job started running, if it has been dispatched
        time_t startTime;

        //- Job's predicted start time
        time_t predictedStartTime;

        //- The termination time of the job, if it has completed.
        time_t endTime;

        //- Duration time (minutes)
        int duration;

        //- The CPU time (float) consumed by the job
        float cpuTime;

        //- The file creation mask when the job was submitted.
        int umask;

        //- The current working directory when the job was submitted.
        std::string cwd;

        //- Home directory on submission host.
        std::string subHomeDir;

        //- The name of the host from which the job was submitted.
        std::string fromHost;

        //- Job exit status
        int exitStatus;

        //- Home directory for the job on the execution host
        std::string execHome;

        //- The rusage satisfied at job runtime
        std::string execRusage;

        //- Host list when job starts
        std::vector<std::string> execHosts;



    // Constructors

        //- Construct from jobInfoEnt
        JobInfoEntry(const jobInfoEnt&);


    //- Destructor
    ~JobInfoEntry();


    // Member Functions

        // Access

        // Check

        // Edit

        // Write

    // Member Operators


    // Friend Functions

    // Friend Operators

    // IOstream Operators

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace LsfUtil

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// #include "JobInfoEntryI.hxx"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
