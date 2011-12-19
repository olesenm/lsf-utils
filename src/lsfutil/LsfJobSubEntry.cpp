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

#include "lsfutil/LsfJobSubEntry.hpp"

#include <iostream>
#include <lsf/lsbatch.h>

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //


// * * * * * * * * * * * * * Static Member Functions * * * * * * * * * * * * //


// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

lsfutil::LsfJobSubEntry::LsfJobSubEntry(const struct submit& sub)
:
    jobName(sub.jobName),
    queue(sub.queue),
    askedHosts(),
    numProcessors(sub.numProcessors),
    dependCond(sub.dependCond),
    beginTime(sub.beginTime),
    termTime(sub.termTime),
    inFile(makeString(sub.inFile)),
    outFile(makeString(sub.outFile)),
    errFile(makeString(sub.errFile)),
    command(makeString(sub.command)),
    chkpntDir(makeString(sub.chkpntDir)),
    preExecCmd(makeString(sub.preExecCmd)),
    mailUser(makeString(sub.mailUser)),
    projectName(makeString(sub.projectName)),
    loginShell(makeString(sub.loginShell)),
    userGroup(makeString(sub.userGroup)),
    jobGroup(makeString(sub.jobGroup)),
    licenseProject(makeString(sub.licenseProject)),
    app(makeString(sub.app)),
    postExecCmd(makeString(sub.postExecCmd)),
    cwd(makeString(sub.cwd)),
    notifyCmd(makeString(sub.notifyCmd)),
    jobDescription(makeString(sub.jobDescription))
{

    std::string::size_type eolp = command.find('\n');
    if (eolp != std::string::npos)
    {
        command.resize(eolp);
    }
    else if (command.size() > 256)
    {
        // truncate for really long commands (eg, shell files)
        command.resize(256);
    }

    // The number of invoker specified candidate hosts for running
    // the job. If numAskedHosts is 0, all qualified hosts will be
    // considered
    if (sub.numAskedHosts)
    {
        askedHosts.reserve(sub.numAskedHosts);

        // The array of names of invoker specified candidate hosts.
        // The number of hosts is given by numAskedHosts.
        for (int i=0; i < sub.numAskedHosts; ++i)
        {
            askedHosts.push_back((sub.askedHosts)[i]);
        }
    }

    resReq = sub.resReq;

//    timeEvent_ = sub.timeEvent;

    fixDirName(cwd);
    fixFileName(inFile);
    fixFileName(outFile);
    fixFileName(errFile);

    if (inFile == "/dev/null")
    {
        inFile.clear();
    }

    if (errFile == outFile)
    {
        errFile.clear();
    }
}



// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

lsfutil::LsfJobSubEntry::~LsfJobSubEntry()
{}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * Friend Functions  * * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * Friend Operators * * * * * * * * * * * * * * //


/* ************************************************************************* */