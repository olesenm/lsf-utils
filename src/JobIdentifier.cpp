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

#include <string>
#include <sstream>
#include "JobIdentifier.hpp"


// misc utils
inline std::string toString(const char* str)
{
    return std::string(str ? str : "");
}


inline std::string toString(int i)
{
    if (i)
    {
        std::ostringstream os;
        os  << i;
        return os.str();
    }
    else
    {
        return "0";
    }
}


bool fixDirName(std::string& name)
{
    bool changed = false;
    while (name.size() > 1 && name[name.size()-1] == '/')
    {
        name.resize(name.size()-1);
        changed = true;
    }

    return changed;
}

bool fixFileName(std::string& name)
{
    if (name.size() > 2 && name[0] == '.' && name[1] == '/')
    {
        name.erase(0, 2);
        return true;
    }
    else
    {
        return false;
    }
}


std::string& replaceAll
(
    std::string& context,
    const std::string& from,
    const std::string& to
)
{
    std::string::size_type lookHere = 0;
    std::string::size_type foundHere;

    while ((foundHere = context.find(from, lookHere)) != std::string::npos)
    {
        context.replace(foundHere, from.size(), to);
        lookHere = foundHere + to.size();
    }
    return context;
}




LsfUtil::JobIdentifier::JobIdentifier(const struct jobInfoEnt& job)
:
    jobId(LSB_ARRAY_JOBID(job.jobId)),
    taskId(LSB_ARRAY_IDX(job.jobId)),
    user(toString(job.user)),
    cwd(toString(job.cwd)),
    outfile(toString(job.submit.outFile)),
    jobIdString()
{
    fixDirName(cwd);
    fixFileName(outfile);

    {
        std::ostringstream os;
        os  << jobId;
        if (taskId)
        {
            os  << "." << taskId;
        }
        jobIdString = os.str();
    }

    // filename relative to cwd whenever possible
    if
    (
        outfile.size() > cwd.size()+1
     && outfile[cwd.size()] == '/'
     && outfile.substr(0, cwd.size()) == cwd
    )
    {
        outfile.erase(0, cwd.size()+1);
    }


    // replace %J with jobId and %I with taskId
    replaceAll(outfile, "%J", toString(jobId));
    replaceAll(outfile, "%I", toString(taskId));
}


std::string LsfUtil::JobIdentifier::tokenJ() const
{
    return toString(jobId);
}


/* ************************************************************************* */
