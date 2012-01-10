/*---------------------------------*- C++ -*---------------------------------*\
Copyright (c) 2011-2012 Mark Olesen
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

#include "lsfutil/OutputQstat.hpp"
#include "lsfutil/XmlUtils.hpp"


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

std::ostream&
lsfutil::OutputQstat::print
(
    std::ostream& os,
    const lsfutil::LsfJobEntry& job
)
{
    const lsfutil::LsfJobSubEntry& sub = job.submit;

    os  << xml::indent0
        << "<job_list type='lsf'"
        << " state='" << job.status << "'>\n";

    os  << xml::indent << "<JB_job_number>"
        << job.jobId << "</JB_job_number>\n";

    if (job.taskId)
    {
        os  << xml::indent << "<tasks>"
            << job.taskId << "</tasks>\n";
    }

    // The name of the user who submitted the job
    os  << xml::indent << xml::Tag("JB_owner", job.user) << "\n";

    os  << xml::indent;

    // this is a real hack
    if (job.isRunning())
    {
        os  << "<state>r</state>\n";
    }
    else if (job.isPending())
    {
        os  << "<state>qw</state>\n";
    }
    else
    {
        os  << "<state>" << job.status[0] << "</state>\n";
    }

    // <JAT_prio> ... </JAT_prio>

    // The time the job was submitted, in seconds since 00:00:00 GMT, Jan. 1, 1970.
    if (job.submitTime)
    {
        os  << xml::indent
            << xml::TimeTag("JB_submission_time", job.submitTime) << "\n";
    }

    // The time that the job started running, if it has been dispatched
    if (job.startTime)
    {
        os  << xml::indent
            << xml::TimeTag("JAT_start_time", job.startTime) << "\n";
    }

    // The current working directory when the job was submitted.
    os  << xml::indent
        << xml::Tag("JB_cwd", job.cwd) << "\n";

    print(os, sub);

    // print queue_name which corresponds to the master process
    // or to the requested queue
    if (sub.queue.size())
    {
        std::string qinstance = sub.queue;

        if (job.execHosts.size())
        {
            qinstance += "@" + job.execHosts[0];
        }
        os  << xml::indent
            << xml::Tag("queue_name", qinstance) << "\n";
    }

    os  << xml::indent0 << "</job_list>\n";

    return os;
}


std::ostream&
lsfutil::OutputQstat::print
(
    std::ostream& os,
    const lsfutil::LsfJobSubEntry& sub
)
{
    // The job name. If jobName is NULL, command is used as the job name.
    if (sub.jobName.size())
    {
        os  << xml::indent
            << xml::Tag("JB_name", sub.jobName) << "\n"
            << xml::indent
            << xml::Tag("full_job_name", sub.jobName) << "\n";
    }

    // The number of invoker specified candidate hosts for running
    // the job. If numAskedHosts is 0, all qualified hosts will be
    // considered
    xml::printList
    (
        os,
        sub.askedHosts,
        "asked-host-list",
        "asked-host"
    );

    // The resource requirements of the job.
    // If resReq is NULL, the batch system will try to obtain
    // resource requirements for command from the remote task
    // lists (see \ref ls_task ). If the task does not appear in the
    // remote task lists, then the default resource requirement
    // is to run on host() of the same type.
    LsfCore::rusage_map rusage = LsfCore::parseRusage(sub.resReq);
    for
    (
        LsfCore::rusage_map::const_iterator iter = rusage.begin();
        iter != rusage.end();
        ++iter
    )
    {
        os  << xml::indent
            << "<hard_request name='"
            << iter->first
            << "' resource_contribution='0.0'>"
            << iter->second
            << "</hard_request>" << "\n";
    }

    // The initial number of processors needed by a (parallel) job.
    // The default is 1.
    {
        os  << xml::indent << "<slots>"
            << sub.numProcessors << "</slots>\n";
    }

    // The job dependency condition.
    if (sub.dependCond.size())
    {
        os  << xml::indent
            << xml::Tag("depend-condition", sub.dependCond) << "\n";
    }

    // Dispatch the job on or after
    // beginTime, where beginTime is the number of seconds since
    // 00:00:00 GMT, Jan. 1, 1970 (See time(), ctime()). If
    // beginTime is 0, start the job as soon as possible.
    if (sub.beginTime)
    {
        os  << xml::indent
            << xml::TimeTag("begin-time", sub.beginTime) << "\n";
    }

    // The job termination deadline. If the job is still running at
    // termTime, it will be sent a USR2 signal. If the job does not
    // terminate within 10 minutes after being sent this signal, it
    // will be ended. termTime has the same representation as
    // termTime. If termTime is 0, allow the job to run until it
    // reaches a resource limit.
    if (sub.termTime)
    {
        os  << xml::indent
            << xml::TimeTag("term-time", sub.termTime) << "\n";
    }

    // The path name of the job's standard input file.
    // If inFile is NULL, use /dev/null as the default.
    if (sub.inFile.size())
    {
        os  << xml::indent
            << xml::Tag("input-file", sub.inFile) << "\n";
    }

    // The path name of the job's standard output file.
    // If outFile is NULL, the job's output will be mailed to the submitter
    if (sub.outFile.size())
    {
        os  << xml::indent
            << xml::Tag("output-file", sub.outFile) << "\n";
    }

    // The path name of the job's standard error output file.
    // If errFile is NULL, the standard error output will be merged with the standard output of the job.
    if (sub.errFile.size())
    {
        os  << xml::indent
            << xml::Tag("error-file", sub.errFile) << "\n";
    }

    // When submitting a job, the command line of the job.
    // When modifying a job, a mandatory parameter that should be set to jobId in string format.
    if (sub.command.size() && (sub.command)[0] == '#')
    {
        os  << xml::indent
            << xml::Tag("JB_script_file", "STDIN") << "\n";
    }
    else
    {
        os  << xml::indent
            << xml::Tag("JB_script_file", sub.command) << "\n";
    }

    // The directory where the chk directory for this job checkpoint
    // files will be created. When a job is checkpointed, its
    // checkpoint files are placed in chkpntDir/chk. chkpntDir can be
    // a relative or absolute path name.
    //
    if (sub.chkpntDir.size())
    {
        os  << xml::indent
            << xml::Tag("checkpoint-dir", sub.chkpntDir) << "\n";
    }

    // The job pre-execution command
    if (sub.preExecCmd.size())
    {
        os  << xml::indent
            << xml::Tag("pre-exec-cmd", sub.preExecCmd) << "\n";
    }

    // The user that results are mailed to
    if (sub.mailUser.size())
    {
        os  << xml::indent
            << xml::Tag("mail-user", sub.mailUser) << "\n";
    }

    // The name of the project the job will be charged to.
    if (sub.projectName.size())
    {
        os  << xml::indent
            << xml::Tag("project", sub.projectName) << "\n";
    }

    // Specified login shell used to initialize the execution
    // environment for the job (see the -L option of bsub).
    if (sub.loginShell.size())
    {
        os  << xml::indent
            << xml::Tag("login-shell", sub.loginShell) << "\n";
    }

    // The name of the LSF user group (see lsb.users) to which the
    // job will belong. (see the -G option of bsub)
    if (sub.userGroup.size())
    {
        os  << xml::indent
            << xml::Tag("user-group", sub.userGroup) << "\n";
    }

    // Application profile under which the job runs.
    if (sub.app.size())
    {
        os  << xml::indent
            << xml::Tag("application-profile-group", sub.app) << "\n";
    }

    // Post-execution commands specified by -Ep option of bsub and bmod.
    if (sub.postExecCmd.size())
    {
        os  << xml::indent
            << xml::Tag("post-exec-cmd", sub.postExecCmd) << "\n";
    }

    // Current working directory specified by -cwd option of bsub and bmod.
    if (sub.cwd.size())
    {
        os  << xml::indent
            << xml::Tag("current-working-directory", sub.cwd) << "\n";
    }

    // Job resize notification command to be invoked on the first
    // execution host when a resize request has been satisfied.
    if (sub.notifyCmd.size())
    {
        os  << xml::indent
            << xml::Tag("notify-cmd", sub.notifyCmd) << "\n";
    }

    // Job description.
    if (sub.jobDescription.size())
    {
        os  << xml::indent
            << xml::Tag("job-description", sub.jobDescription) << "\n";
    }

    return os;
}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

std::ostream&
lsfutil::OutputQstat::print
(
    std::ostream& os,
    const lsfutil::LsfJobList& list
)
{
    os  << "<?xml version='1.0'?>\n";

    if (list.hasError())
    {
        os  << "<lsf-error/>\n";

        return os;
    }

    os  << "<job_info"
        << " xmlns:xsd='http://www.w3.org/2001/XMLSchema'"
        << " type='lsf' count='" << list.size() << "'>\n";


    // active jobs:
    os  << "<queue_info>\n";
    for (unsigned jobI = 0; jobI < list.size(); ++jobI)
    {
        if (list[jobI].isRunning())
        {
            print(os, list[jobI]);
        }
    }
    os  << "</queue_info>\n";


    // pending jobs:
    os  << "<job_info>\n";
    for (unsigned jobI = 0; jobI < list.size(); ++jobI)
    {
        if (list[jobI].isPending())
        {
            print(os, list[jobI]);
        }
    }
    os  << "</job_info>\n";


    os  << "</job_info>\n";

    return os;
}


/* ************************************************************************* */
