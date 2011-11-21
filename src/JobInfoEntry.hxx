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
