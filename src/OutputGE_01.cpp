/*---------------------------------*- C++ -*---------------------------------*\
Copyright (c) 2011-2011 Mark Olesen
-------------------------------------------------------------------------------
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

#include "OutputGE_01.hpp"
#include "XmlUtils.hpp"


// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

// local scope

std::string xmlns;
const char* indent0 = "  ";
const char* indent = "    ";


// * * * * * * * * * * * * * Static Member Functions * * * * * * * * * * * * //


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void printXmlList
(
    std::ostream& os,
    const std::vector<std::string>& list,
    const char *listTag,
    const char *elemTag
)
{
    if (list.size())
    {
        os  << indent << "<" << xmlns << listTag
            <<" count='" << list.size() << "'>\n";

        for
        (
            std::vector<std::string>::const_iterator iter = list.begin();
            iter != list.end();
            ++iter
        )
        {
            os  << indent << indent0
                << LsfUtil::XmlTag(elemTag, *iter) << "\n";
        }

        os  << indent << "</" << xmlns << listTag << ">\n";
    }
}


// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

std::ostream&
LsfUtil::OutputGE_01::printXML
(
    std::ostream& os,
    const LsfUtil::JobList& list
)
{
    const char* jobListTag = "job-list";

    os  << "<?xml version='1.0'?>\n";

    if (list.hasError())
    {
        os  << "<lsf-error/>\n";

        return os;
    }

    os  << "<" << xmlns << jobListTag
        << " type='lsf' count='" << list.size() << "'>\n";


    for (unsigned jobI = 0; jobI < list.size(); ++jobI)
    {
        printXML(os, list[jobI]);
    }

    os
        << "</" << xmlns << jobListTag << ">\n";

    return os;
}


std::ostream&
LsfUtil::OutputGE_01::printXML
(
    std::ostream& os,
    const LsfUtil::JobInfoEntry& job
)
{
     os  << indent0
         << "<" << xmlns << "job type='lsf'"
         << " status='" << job.status << "'>\n";

     os  << indent << "<" << xmlns << "job-id>"
         << job.jobId << "</" << xmlns << "job-id>\n";

    if (job.taskId)
    {
         os  << indent << "<" << xmlns << "task-id>"
             << job.taskId << "</" << xmlns << "task-id>>\n";
     }

     // The name of the user who submitted the job
     os  << indent << XmlTag(xmlns + "owner", job.user) << "\n";

     // The current status of the job. Possible values are shown in job_states
     os  << indent << "<" << xmlns << "status>"
         << job.status
         << "</" << xmlns << "status>\n";


//     int     *reasonTb;     /**< Pending or suspending reasons of the job */
//     int     numReasons;    /**< Length of reasonTb[] */
//     int     reasons;       /**< The reason a job is pending or suspended. */
//     int     subreasons;    /**< The reason a job is pending or suspended. If status
//                             * is JOB_STAT_PEND, the values of reasons and subreasons
//                             * are explained by \ref lsb_pendreason. If status is
//                             * JOB_STAT_PSUSP, the values of reasons and subreasons
//                             * are explained by \ref lsb_suspreason.
//                             *
//                             * When reasons is PEND_HOST_LOAD or SUSP_LOAD_REASON,
//                             * subreasons indicates the load indices that are out
//                             * of bounds. If reasons is PEND_HOST_LOAD, subreasons
//                             * is the same as busySched in the hostInfoEnt structure;
//                             * if reasons is SUSP_LOAD_REASON, subreasons is the same
//                             * as busyStop in the hostInfoEnt structure.
//                             * (See \ref lsb_hostinfo) */
//     int     jobPid;        /**< The job process ID. */

    // The time the job was submitted, in seconds since 00:00:00 GMT, Jan. 1, 1970.
    if (job.submitTime)
    {
        os  << indent
            << XmlTimeTag(xmlns + "submit-time", job.submitTime) << "\n";
    }

    // Time when job slots are reserved
    if (job.reserveTime)
    {
        os  << indent
            << XmlTimeTag(xmlns + "reserve-time", job.reserveTime) << "\n";
    }

     // The time that the job started running, if it has been dispatched
     if (job.startTime)
     {
         os  << indent
             << XmlTimeTag(xmlns + "start-time", job.startTime) << "\n";
     }

    // Job's predicted start time
    if (job.predictedStartTime)
     {
         os  << indent
             << XmlTimeTag(xmlns + "predicted-start-time", job.predictedStartTime) << "\n";
     }

    // The termination time of the job, if it has completed.
    if (job.endTime)
    {
        os  << indent
            << XmlTimeTag(xmlns + "end-time", job.endTime) << "\n";
    }
//     time_t  lastEvent;     /**< Last time event */
//     time_t  nextEvent;     /**< Next time event */

    // Duration time (minutes)
    if (job.duration)
    {
        os  << indent
            << "<" << xmlns << "duration units='min'>"
            << job.duration
            << "</" << xmlns << "duration>\n";
    }

    // (float) CPU time consumed by the job
    if (job.cpuTime)
    {
        os  << indent
            << "<" << xmlns << "cpu-time units='sec'>"
            << job.cpuTime
            << "</" << xmlns << "cpu-time>\n";
    }

    // The file creation mask when the job was submitted.
    os  << indent
        << "<" << xmlns << "umask>"
        << std::oct << job.umask << std::dec
        << "</" << xmlns << "umask>\n";

    // The current working directory when the job was submitted.
    os  << indent
        << XmlTag(xmlns + "cwd", job.cwd) << "\n";

    // Home directory on submission host.
    os  << indent
        << XmlTag(xmlns + "submission-home", job.subHomeDir) << "\n";

    // The name of the host from which the job was submitted.
    os  << indent
        << XmlTag(xmlns + "submission-host", job.fromHost) << "\n";

//     char    **exHosts;     /**< The array of names of hosts on which the job
//                             * executes. */
//     int     numExHosts;    /**< The number of hosts on which the job executes. */
//     float   cpuFactor;     /**< The CPU factor for normalizing CPU and wall clock
//                             * time limits.*/
//     int     nIdx;          /**< The number of load indices in the loadSched and
//                             * loadStop arrays.*/
//     float   *loadSched;    /**< The values in the loadSched array specify
//                             * the thresholds for the corresponding load indices.
//                             * Only if the current values of all specified load
//                             * indices of a host are within (below or above,
//                             * depending on the meaning of the load index) their
//                             * corresponding thresholds may the suspended job be
//                             * resumed on this host.
//                             *
//                             * For an explanation of the entries in the loadSched,
//                             * see \ref lsb_hostinfo.
//                             */
//     float   *loadStop;     /**< The values in the loadStop array specify the
//                             * thresholds for job suspension; if any of the current
//                             * load index values of the host crosses its threshold,
//                             * the job will be suspended.
//                             *
//                             * For an explanation of the entries in the loadStop,
//                             * see \ref lsb_hostinfo.
//                             */

    printXML(os, job.submit);

    // Job exit status
    os  << indent
        << "<" << xmlns << "exit-code>"
        << job.exitStatus
        << "<" << xmlns << "exit-code>\n";

//     int     execUid;        /**< Mapped UNIX user ID on the execution host.*/

    // Home directory for the job on the execution host
    os  << indent
        << XmlTag(xmlns + "exec-home", job.execHome) << "\n";

//     char    *execCwd;       /**< Current working directory for the job on the
//                              * execution host.*/
//     char    *execUsername;  /**< Mapped user name on the execution host.*/
//     time_t  jRusageUpdateTime; /**< Time of the last job resource usage update.*/
//     struct  jRusage runRusage; /**< Contains resource usage information for the job.*/
//
//     int     jType;          /**< Job type.N_JOB, N_GROUP, N_HEAD */
//     char    *parentGroup;   /**< The parent job group of a job or job group. */
//     char    *jName;         /**< If jType is JGRP_NODE_GROUP, then it is the job
//                              * group name. Otherwise, it is the
//                              *job name. */
//     int     counter[NUM_JGRP_COUNTERS];  /**< Index into the counter array, only
//                                           * used for job arrays. Possible index values are
//                                           * shown in \ref jobgroup_counterIndex*/
//     u_short port;           /**< Service port of the job. */
//     int     jobPriority;    /**< Job dynamic priority */
//     int numExternalMsg;     /**< The number of external messages in the job. */
//     struct jobExternalMsgReply **externalMsg; /**< This structure contains the
//                                                * information required to define
//                                                * an external message reply.*/
//     int     clusterId;      /**< MultiCluster cluster ID. If clusterId is greater
//                              * than or equal to 0, the job is a pending remote job,
//                              * and \ref lsb_readjobinfo checks for host_name\@cluster_name.
//                              * If host name is needed, it should be found in
//                              * jInfoH->remoteHosts. If the remote host name is not
//                              * available, the constant string remoteHost is used.*/
//     char   *detailReason;   /**<  Detail reason field */
//     float   idleFactor;     /**< Idle factor for job exception handling. If the job
//                              * idle factor is less than the specified threshold, LSF
//                              * invokes LSF_SERVERDIR/eadmin to trigger the action for
//                              * a job idle exception.*/
//     int     exceptMask;     /**< Job exception handling mask */
//
//     char   *additionalInfo; /**< Placement information of LSF HPC jobs.Placement
//                              * information of LSF HPC jobs.Arbitrary information of
//                              * a job stored as a string currently used by rms_rid
//                              * and rms_alloc */
//     int     exitInfo;       /**< Job termination reason. See lsbatch.h.*/
//     int    warningTimePeriod; /**< Job warning time period in seconds; -1 if unspecified. */
//     char   *warningAction;  /**< Warning action, SIGNAL | CHKPNT |
//                              *command, NULL if unspecified */
//     char   *chargedSAAP;    /**< SAAP charged for job */

    // The rusage satisfied at job runtime
    if (job.execRusage.size())
    {
        os  << indent
            << XmlTag(xmlns + "exec-rusage", job.execRusage) << "\n";
    }

#if 0

//     time_t rsvInActive;     /**< The time when advance reservation expired or was deleted. */

    // The licenses reported from License Scheduler.
    printXmlList
    (
        os,
        job.licenseNames,
        "license-list",
        "license"
    );
#endif

//     float  aps;             /**< Absolute priority scheduling (APS) priority value.*/
//     float  adminAps;        /**< Absolute priority scheduling (APS) string set by
//                              * administrators to denote static system APS value */
//     int    runTime;         /**< The real runtime on the execution host. */
//     int reserveCnt;         /**< How many kinds of resource are reserved by this job*/
//     struct reserveItem *items; /**< Detail reservation information for each
//                                 * kind of resource*/
//     float  adminFactorVal;  /**< Absolute priority scheduling (APS) string set by
//                              * administrators to denote ADMIN
//                              * factor APS value. */
//     int    resizeMin;       /**< Pending resize min. 0, if no resize pending. */
//     int    resizeMax;       /**< Pending resize max. 0, if no resize pending */
//     time_t resizeReqTime;   /**< Time when pending request was issued */

    // hosts when job starts
    printXmlList
    (
        os,
        job.execHosts,
        "exec-host-list",
        "exec-host"
    );

//     time_t lastResizeTime;   /**< Last time when job allocation changed */

//     <grid:account>default</grid:account>
//     <grid:status>DONE</grid:status>
//     <grid:queue>priority</grid:queue>
//     <grid:command>/usr/bin/spamc</grid:command>
//     <grid:submission-time month="Jan" day="21" hour="23" minute="06" year="2005"/>
//     <grid:submission-host>mail</grid:submission-host>
//     <grid:submission-directory>$HOME</grid:submission-directory>
//     <grid:execution-time month="Jan" day="21" hour="23" minute="06" year="2005"/>
//     <grid:execution-host>mail</grid:execution-host>
//     <grid:termination-time month="Jan" day="21" hour="23" minute="06" year="2005"/>
//     <grid:total-cpu-usage>0.0</grid:total-cpu-usage>
//     <grid:exit-code>0</grid:exit-code>

     os  << indent0 << "</" << xmlns << "job>\n";

    return os;
}


std::ostream&
LsfUtil::OutputGE_01::printXML
(
    std::ostream& os,
    const LsfUtil::JobSubmitEntry& sub
)
{
#if 1
//     int     options;        /**<  <lsf/lsbatch.h> defines the flags in \ref lsb_submit_options constructed from bits.
//                              * These flags correspond to some of the options of the bsub command line.
//                              * Use the bitwise OR to set more than one flag.*/
//
//     int     options2;       /**< Extended bitwise inclusive OR of some of the flags in \ref lsb_submit_options2. */

    // The job name. If jobName is NULL, command is used as the job name.
    if (sub.jobName.size())
    {
        os  << indent
            << XmlTag("job-name", sub.jobName) << "\n";
    }

    // Submit the job to this queue. If queue is NULL, submit the job
    // to a system default queue.
    if (sub.queue.size())
    {
        os  << indent
            << XmlTag("queue", sub.queue) << "\n";
    }

    // The number of invoker specified candidate hosts for running
    // the job. If numAskedHosts is 0, all qualified hosts will be
    // considered
    printXmlList
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
    if (sub.resReq.size())
    {
        os  << indent
            << XmlTag(xmlns + "resource-request", sub.resReq) << "\n";
    }

//     int     rLimits[LSF_RLIM_NLIMITS];
//                   /**< Limits on the consumption of
//                    * system resources by all processes belonging to this job.
//                    * See getrlimit() for details. If an element of the array
//                    * is -1, there is no limit for that resource. For the
//                    * constants used to index the array, see \ref lsb_queueinfo . */
//     char    *hostSpec;
//                   /**< Specify the host model to use
//                    * for scaling rLimits[LSF_RLIMIT_CPU] and
//                    * rLimits[LSF_RLIMIT_RUN]. (See \ref lsb_queueinfo).
//                    * If hostSpec is NULL, the local host is assumed. */

    // The initial number of processors needed by a (parallel) job.
    // The default is 1.
    {
        os  << indent << "<" << xmlns << "slots>"
            << sub.numProcessors << "</" << xmlns << "slots>\n";
    }

    // The job dependency condition.
    if (sub.dependCond.size())
    {
        os  << indent
            << XmlTag(xmlns + "depend-condition", sub.dependCond) << "\n";
    }

    // Time event string
#if 0
    if (sub.timeEvent.size())
    {
        os  << indent
            << XmlTag(xmlns + "time-event", sub.timeEvent) << "\n";
    }
#endif

    // Dispatch the job on or after
    // beginTime, where beginTime is the number of seconds since
    // 00:00:00 GMT, Jan. 1, 1970 (See time(), ctime()). If
    // beginTime is 0, start the job as soon as possible.
    if (sub.beginTime)
    {
        os  << indent
            << XmlTimeTag(xmlns + "begin-time", sub.beginTime) << "\n";
    }

    // The job termination deadline. If the job is still running at
    // termTime, it will be sent a USR2 signal. If the job does not
    // terminate within 10 minutes after being sent this signal, it
    // will be ended. termTime has the same representation as
    // termTime. If termTime is 0, allow the job to run until it
    // reaches a resource limit.
    if (sub.termTime)
    {
        os  << indent
            << XmlTimeTag(xmlns + "term-time", sub.termTime) << "\n";
    }

//     int     sigValue;
//                   /**< Applies to jobs submitted to a queue that has a run
//                    * window (See \ref lsb_queueinfo). Send signal sigValue to
//                    * the job 10 minutes before the run window is going to
//                    * close. This allows the job to clean up or checkpoint
//                    * itself, if desired. If the job does not terminate 10
//                    * minutes after being sent this signal, it will be
//                    * suspended. */

    // The path name of the job's standard input file.
    // If inFile is NULL, use /dev/null as the default.
    if (sub.inFile.size())
    {
        os  << indent
            << XmlTag(xmlns + "input-file", sub.inFile) << "\n";
    }

    // The path name of the job's standard output file.
    // If outFile is NULL, the job's output will be mailed to the submitter
    if (sub.outFile.size())
    {
        os  << indent
            << XmlTag(xmlns + "output-file", sub.outFile) << "\n";
    }

    // The path name of the job's standard error output file.
    // If errFile is NULL, the standard error output will be merged with the standard output of the job.
    if (sub.errFile.size())
    {
        os  << indent
            << XmlTag(xmlns + "error-file", sub.errFile) << "\n";
    }

    // When submitting a job, the command line of the job.
    // When modifying a job, a mandatory parameter that should be set to jobId in string format.
    os  << indent
        << XmlTag(xmlns + "command", sub.command) << "\n";

//     char    *newCommand;    /**< New command line for bmod. */
//     time_t  chkpntPeriod;   /**< The job is checkpointable with a period of
//                              * chkpntPeriod seconds. The value 0 disables
//                              * periodic checkpointing.  */

    // The directory where the chk directory for this job checkpoint
    // files will be created. When a job is checkpointed, its
    // checkpoint files are placed in chkpntDir/chk. chkpntDir can be
    // a relative or absolute path name.
    //
    if (sub.chkpntDir.size())
    {
        os  << indent
            << XmlTag(xmlns + "checkpoint-dir", sub.chkpntDir) << "\n";
    }

//     int     nxf;            /**< The number of files to transfer. */
//     struct xFile *xf;       /**< The array of file transfer
//                              * specifications. (The xFile structure is defined
//                              * in <lsf/lsbatch.h>.) */

    // The job pre-execution command
    if (sub.preExecCmd.size())
    {
        os  << indent
            << XmlTag(xmlns + "pre-exec-cmd", sub.preExecCmd) << "\n";
    }

    // The user that results are mailed to
    if (sub.mailUser.size())
    {
        os  << indent
            << XmlTag(xmlns + "mail-user", sub.mailUser) << "\n";
    }

//     int    delOptions;      /**< Delete options in options field. */
//     int    delOptions2;     /**< Extended delete options in options2 field. */

    // The name of the project the job will be charged to.
    if (sub.projectName.size())
    {
        os  << indent
            << XmlTag(xmlns + "project", sub.projectName) << "\n";
    }

//     int    maxNumProcessors; /**< Maximum number of processors required
//                               * to run the job. */

    // Specified login shell used to initialize the execution
    // environment for the job (see the -L option of bsub).
    if (sub.loginShell.size())
    {
        os  << indent
            << XmlTag(xmlns + "login-shell", sub.loginShell) << "\n";
    }

    // The name of the LSF user group (see lsb.users) to which the
    // job will belong. (see the -G option of bsub)
    if (sub.userGroup.size())
    {
        os  << indent
            << XmlTag(xmlns + "user-group", sub.userGroup) << "\n";
    }

//     char   *exceptList;
//             /**< Passes the exception handlers to
//              * mbatchd during a job. (see the -X option of bsub).
//              * Specifies execption handlers that tell the system
//              * how to respond to an exceptional condition for a job.
//              * An action is performed when any one of the following
//              * exceptions is detected:
//              *
//              * - \b missched - A job has not been scheduled within the time
//              * event specified in the -T option.
//              * - \b overrun - A job did not finish in its maximum time (maxtime).
//              * - \b underrun - A job finished before it reaches its minimum
//              * running time (mintime).
//              * - \b abend - A job terminated abnormally. Test an exit code that
//              * is one value, two or more comma separated values, or a
//              * range of values (two values separated by a `-' to indivate
//              * a range). If the job exits with one of the tested values,
//              * the abend condition is detected.
//              * - \b startfail - A job did not start due to insufficient system
//              * resources.
//              * - \b cantrun - A job did not start because a dependency condition
//              * (see the -w option of bsub) is invalid, or a startfail
//              * exception occurs 20 times in a row and the job is suspended.
//              * For jobs submitted with a time event (see the -T option of bsub),
//              * the cantrun exception condition can be detected once in each
//              * time event.
//              * - \b hostfail - The host running a job becomes unavailable.
//              *
//              * When one or more of the above exceptions is detected, you
//              * can specify one of the following actions to be taken:
//              * - \b alarm - Triggers an alarm incident (see balarms(1)).
//              * The alarm can be viewed, acknowledged and resolved.
//              * - \b setexcept - Causes the exception event event_name to be set.
//              * Other jobs waiting on the exception event event_name
//              * specified through the -w option can be triggered. event_name
//              * is an arbitrary string.
//              * - \b rerun - Causes the job to be rescheduled for execution.
//              * Any dependencies associated with the job must be satisfied
//              * before re-execution takes place. The rerun action can only
//              * be specified for the abend and hostfail exception conditions.
//              * The startfail exception condition automatically triggers
//              * the rerun action.
//              * - \b kill - Causes the current execution of the job to be
//              * terminated. This action can only be specified for the
//              * overrun exception condition.
//              */
//
//     int    userPriority; /**< User priority for fairshare scheduling. */
//     char   *rsvId;          /**< Reservation ID for advance reservation. */

    // Job group under which the job runs.
    if (sub.jobGroup.size())
    {
        os  << indent
            << XmlTag(xmlns + "job-group", sub.jobGroup) << "\n";
    }

//     char   *sla;            /**< SLA under which the job runs. */
//     char   *extsched;       /**< External scheduler options. */
//     int    warningTimePeriod;  /**< Warning time period in seconds,
//                                 * -1 if unspecified. */
//     char   *warningAction;  /**< Warning action, SIGNAL | CHKPNT |
//                              * command, NULL if unspecified. */

    // License Scheduler project name
    if (sub.licenseProject.size())
    {
        os  << indent
            << XmlTag(xmlns + "license-project", sub.licenseProject) << "\n";
    }

//     int    options3;        /**< Extended bitwise inclusive OR of options flags in \ref lsb_submit_options3. */
//     int    delOptions3;     /**< Extended delete options in options3 field. */

    // Application profile under which the job runs.
    if (sub.app.size())
    {
        os  << indent
            << XmlTag(xmlns + "application-profile-group", sub.app) << "\n";
    }

//     int  jsdlFlag;          /**< -1 if no -jsdl and -jsdl_strict options.
//                              * - 0 -jsdl_strict option
//                              * - 1 -jsdl option */
//     char *jsdlDoc;          /**< JSDL filename*/
//     void   *correlator;     /**< ARM correlator */
//     char *apsString;        /**<  Absolute priority scheduling string set by
//                              * administrators to denote static system APS
//                              * value or ADMIN factor APS value. This field
//                              * is ignored by \ref lsb_submit. */

    // Post-execution commands specified by -Ep option of bsub and bmod.
    if (sub.postExecCmd.size())
    {
        os  << indent
            << XmlTag(xmlns + "post-exec-cmd", sub.postExecCmd) << "\n";
    }

    // Current working directory specified by -cwd option of bsub and bmod.
    if (sub.cwd.size())
    {
        os  << indent
            << XmlTag(xmlns + "current-working-directory", sub.cwd) << "\n";
    }
//     int      runtimeEstimation;   /**< Runtime estimate specified by -We
//                                    * option of bsub and bmod.  */
//     char *requeueEValues;   /**< Job-level requeue exit values specified
//                              * by -Q option of bsub and bmod. */
//     int     initChkpntPeriod;  /**< Initial checkpoint period specified
//                                 * by -k option of bsub and bmod. */
//     int     migThreshold;      /**< Job migration threshold specified
//                                 * by -mig option of bsub and bmod. */

    // Job resize notification command to be invoked on the first
    // execution host when a resize request has been satisfied.
    if (sub.notifyCmd.size())
    {
        os  << indent
            << XmlTag(xmlns + "notify-cmd", sub.notifyCmd) << "\n";
    }

    // Job description.
    if (sub.jobDescription.size())
    {
        os  << indent
            << XmlTag(xmlns + "job-description", sub.jobDescription) << "\n";
    }

    // struct submit_ext *submitExt; /**< For new options in future  */
#endif
    return os;
}


// * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * Friend Functions  * * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * Friend Operators * * * * * * * * * * * * * * //


/* ************************************************************************* */
