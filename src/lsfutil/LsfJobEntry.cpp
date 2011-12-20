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

\*---------------------------------------------------------------------------*/

#include "lsfutil/LsfJobEntry.hpp"

#include <cstring>
#include <sstream>

#include <lsf/lsbatch.h>

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //


// * * * * * * * * * * * * * Static Member Functions * * * * * * * * * * * * //

std::string lsfutil::LsfJobEntry::jobStatusToString(int stat)
{
    if (IS_PEND(stat))
    {
        return "pending";
    }
    else if (IS_FINISH(stat))
    {
        return "done";
    }
    else if (IS_SUSP(stat))
    {
        return "suspended";
    }
    else if (IS_START(stat))
    {
        return "running";
    }
    else
    {
        return "unknown";
    }
}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //


// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

lsfutil::LsfJobEntry::LsfJobEntry(const struct jobInfoEnt& job)
:
    submit(job.submit),
    jobId(LSB_ARRAY_JOBID(job.jobId)),
    taskId(LSB_ARRAY_IDX(job.jobId)),
    status(jobStatusToString(job.status)),
    user(makeString(job.user)),
    submitTime(job.submitTime),
    reserveTime(job.reserveTime),
    startTime(job.startTime),
    predictedStartTime(job.predictedStartTime),
    endTime(job.endTime),
    duration(job.duration),
    cpuTime(job.cpuTime),
    umask(job.umask),
    cwd(makeString(job.cwd)),
    subHomeDir(makeString(job.subHomeDir)),
    fromHost(makeString(job.fromHost)),
    exitStatus(job.exitStatus),
    execHome(makeString(job.execHome)),
    execRusage(makeString(job.execRusage)),
    execHosts()
{
    fixDirName(cwd);

    if (job.jStartNumExHosts)
    {
        execHosts.reserve(job.jStartNumExHosts);

        // Host list when job starts
        for (int i=0; i < job.jStartNumExHosts; ++i)
        {
            execHosts.push_back((job.jStartExHosts)[i]);
        }
    }

    // replace %J with jobId and %I with taskId immediately
    replaceAll(submit.outFile, "%J", makeString(jobId));
    replaceAll(submit.outFile, "%I", makeString(taskId));

    replaceAll(submit.errFile, "%J", makeString(jobId));
    replaceAll(submit.errFile, "%I", makeString(taskId));
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

lsfutil::LsfJobEntry::~LsfJobEntry()
{}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

std::string lsfutil::LsfJobEntry::relativeFilePath
(
    const std::string& absName
) const
{
    std::string relName(absName);
    fixFileName(relName);

    // filename relative to cwd whenever possible
    if
    (
        relName.size() > cwd.size()+1
     && relName[cwd.size()] == '/'
     && relName.substr(0, cwd.size()) == cwd
    )
    {
        relName.erase(0, cwd.size()+1);
    }

    return relName;
}


std::string lsfutil::LsfJobEntry::tokenJ() const
{
    return makeString(jobId);
}


bool lsfutil::LsfJobEntry::isPending() const
{
    return status == "pending";
}


bool lsfutil::LsfJobEntry::isDone() const
{
    return status == "done";
}


bool lsfutil::LsfJobEntry::isSuspend() const
{
    return status == "suspended";
}


bool lsfutil::LsfJobEntry::isRunning() const
{
    return status == "running";
}


std::ostream& lsfutil::LsfJobEntry::dump(std::ostream& os) const
{
    os  << "jobId: " << jobId << "\n";
    os  << "taskId: " << taskId << "\n";
    os  << "status: " << status << "\n";
    submit.dump(os);

    os  << "user: " << user << "\n";
    os  << "submitTime: " << submitTime << "\n";
    os  << "reserveTime: " << reserveTime << "\n";
    os  << "startTime: " << startTime << "\n";
    os  << "predictedStartTime: " << predictedStartTime << "\n";
    os  << "endTime: " << endTime << "\n";
    os  << "duration: " << duration << "\n";
    os  << "cpuTime: " << cpuTime << "\n";
    os  << "umask: " << umask << "\n";
    os  << "job-cwd: " << cwd << "\n";
    os  << "subHomeDir: " << subHomeDir << "\n";
    os  << "fromHost: " << fromHost << "\n";
    os  << "execHome: " << execHome << "\n";
    os  << "execRusage: " << execRusage << "\n";

    os  << "execHosts: (";
    for (unsigned i=0; i << execHosts.size(); ++i)
    {
        if (i)
        {
            os  << ' ';
        }
        os  << execHosts[i];
    }
    os  << ")\n";

    os  << "exitStatus: " << exitStatus << "\n";
    return os;
}

// * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * Friend Functions  * * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * Friend Operators * * * * * * * * * * * * * * //


/* ************************************************************************* */
