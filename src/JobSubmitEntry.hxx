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

#ifndef JobSubmitEntry_H
#define JobSubmitEntry_H

#include <ctime>
#include <string>
#include <vector>
#include <iostream>

// Forward declaration of classes
struct submit;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace LsfUtil
{

// Forward declaration of friend functions and operators
class JobSubmitEntry;


/*---------------------------------------------------------------------------*\
                    Class JobSubmitEntry Declaration
\*---------------------------------------------------------------------------*/

class JobSubmitEntry
{

    // Private Member Functions

    //- Remove trailing '/' from dir names
    bool fixDirName(std::string& name);

    //- Remove leading './' from file names
    bool fixFileName(std::string& name);


public:

    // Static data members

    // Public data

        //- The job name. If jobName is empty, command is used as the job name.
        std::string jobName;

        //- Submit the job to this queue. If queue is NULL, submit the job
        // to a system default queue.
        std::string queue;

        //- List of specified candidate hosts.
        std::vector<std::string> askedHosts;

        //- The initial number of processors needed by a (parallel) job.
        //  The default is 1.
        int numProcessors;

        //- The job dependency condition.
        std::string dependCond;


        //- Time when job slots are reserved
        //- Dispatch the job on or after beginTime,
        // where beginTime is the number of seconds since
        // 00:00:00 GMT, Jan. 1, 1970 (See time(), ctime()). If
        // beginTime is 0, start the job as soon as possible.
        time_t beginTime;

        //- The job termination deadline.
        //
        //  If the job is still running at
        //  termTime, it will be sent a USR2 signal. If the job does not
        //  terminate within 10 minutes after being sent this signal, it
        //  will be ended. termTime has the same representation as
        //  termTime. If termTime is 0, allow the job to run until it
        //  reaches a resource limit.
        time_t termTime;

        //- The path name of the job's standard input file.
        //  If inFile is NULL, use /dev/null as the default.
        std::string inFile;

        //- The path name of the job's standard output file.
        //  If outFile is NULL, the job's output will be mailed to the submitter
        std::string outFile;

        //- The path name of the job's standard error output file.
        //  If errFile is NULL, the standard error output will be merged with the standard output of the job.
        std::string errFile;

        //- When submitting a job, the command line of the job.
        //  When modifying a job, a mandatory parameter that should be set to jobId in string format.
        std::string command;


        // The directory where the chk directory for this job checkpoint
        // files will be created. When a job is checkpointed, its
        // checkpoint files are placed in chkpntDir/chk. chkpntDir can be
        // a relative or absolute path name.
        //
        std::string chkpntDir;

        //- The job pre-execution command
        std::string preExecCmd;

        //- The user that results are mailed to
        std::string mailUser;

        //- The name of the project the job will be charged to.
        std::string projectName;

        //- Specified login shell used to initialize the execution
        //  environment for the job (see the -L option of bsub).
        std::string loginShell;

        //- The name of the LSF user group (see lsb.users) to which the
        //  job will belong. (see the -G option of bsub)
        std::string userGroup;

        //- Job group under which the job runs.
        std::string jobGroup;

        //- License Scheduler project name
        std::string licenseProject;

        //- Application profile under which the job runs.
        std::string app;

        //- Post-execution commands specified by -Ep option of bsub and bmod.
        std::string postExecCmd;

        // Current working directory specified by -cwd option of bsub and bmod.
        std::string cwd;

        //- Job resize notification command to be invoked on the first
        //  execution host when a resize request has been satisfied.
        std::string notifyCmd;

        //- Job description.
        std::string jobDescription;

        //- Resource Request.
        std::string resReq;


    // Constructors

        //- Construct from submit
        JobSubmitEntry(const submit&);


    //- Destructor
    ~JobSubmitEntry();


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

// #include "JobSubmitEntryI.hxx"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //

#if 0
//
//
std::string jobStatusToString(const struct jobInfoEnt& job)
{
    if (IS_PEND(job.status))
    {
        return "pending";
    }
    else if (IS_FINISH(job.status))
    {
        return "done";
    }
    else if (IS_SUSP(job.status))
    {
        return "suspended";
    }
    else if (IS_START(job.status))
    {
        return "running";
    }
    else
    {
        return "unknown";
    }
}


#endif
/* ************************************************************************* */
