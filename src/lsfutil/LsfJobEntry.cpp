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

std::string lsfutil::LsfJobEntry::jobStatusToString
(
    const struct jobInfoEnt& job
)
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


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //


// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

lsfutil::LsfJobEntry::LsfJobEntry(const struct jobInfoEnt& job)
:
    submit(job.submit),
    jobId(LSB_ARRAY_JOBID(job.jobId)),
    taskId(LSB_ARRAY_IDX(job.jobId)),
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
    status = jobStatusToString(job);
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


// * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * Friend Functions  * * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * Friend Operators * * * * * * * * * * * * * * //


/* ************************************************************************* */
