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

#include "JobInfoEntry.hpp"
#include "XmlUtils.hpp"

#include <cstring>
#include <sstream>

#include <lsf/lsbatch.h>

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //


// * * * * * * * * * * * * * Static Member Functions * * * * * * * * * * * * //

inline std::string makeString(const char* str)
{
    return std::string(str ? str : "");
}


std::string LsfUtil::JobInfoEntry::jobStatusToString
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

LsfUtil::JobInfoEntry::JobInfoEntry(const struct jobInfoEnt& job)
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

    if (job.jStartNumExHosts)
    {
        execHosts.reserve(job.jStartNumExHosts);

        // Host list when job starts
        for (int i=0; i < job.jStartNumExHosts; ++i)
        {
            execHosts.push_back
            (
                (job.jStartExHosts)[i]
            );
        }
    }

}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

LsfUtil::JobInfoEntry::~JobInfoEntry()
{}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

#if 0
// print some of job info structure
//
std::ostream&
LsfUtil::JobInfoEntry::printXML(std::ostream& os) const
{
    std::string xmlNamespace;

    // const char* xmlns = "grid:";
    const char* indent0 = "  ";
    const char* indent = "    ";

    os  << indent0
        << "<" << xmlNamespace << "job type='lsf'"
        << " status='" << status_ << "'>\n";

    os  << indent << "<" << xmlNamespace << "job-id>"
        << jobId_ << "</" << xmlNamespace << "job-id>\n";

    if (taskId_)
    {
        os  << indent << "<" << xmlNamespace << "task-id>"
            << taskId_ << "</" << xmlNamespace << "task-id>>\n";
    }

    // The name of the user who submitted the job
    os  << indent << XmlTag(xmlNamespace + "owner", user_) << "\n";

    // The current status of the job. Possible values are shown in job_states
    os  << indent << "<" << xmlNamespace << "status>"
        << status_
        << "</" << xmlNamespace << "status>\n";


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
    if (submitTime_)
    {
        os  << indent
            << XmlTimeTag(xmlNamespace + "submit-time", submitTime_) << "\n";
    }

    // Time when job slots are reserved
    if (reserveTime_)
    {
        os  << indent
            << XmlTimeTag(xmlNamespace + "reserve-time", reserveTime_) << "\n";
    }

    // The time that the job started running, if it has been dispatched
    if (startTime_)
    {
        os  << indent
            << XmlTimeTag(xmlNamespace + "start-time", startTime_) << "\n";
    }

    // Job's predicted start time
    if (predictedStartTime_)
    {
        os  << indent
            << XmlTimeTag(xmlNamespace + "predicted-start-time", predictedStartTime_) << "\n";
    }

    // The termination time of the job, if it has completed.
    if (endTime_)
    {
        os  << indent
            << XmlTimeTag(xmlNamespace + "end-time", endTime_) << "\n";
    }
//     time_t  lastEvent;     /**< Last time event */
//     time_t  nextEvent;     /**< Next time event */

    // Duration time (minutes)
    if (duration_)
    {
        os  << indent
            << "<" << xmlNamespace << "duration units='min'>"
            << duration_
            << "</" << xmlNamespace << "duration>\n";
    }

    // (float) CPU time consumed by the job
    if (cpuTime_)
    {
        os  << indent
            << "<" << xmlNamespace << "cpu-time units='sec'>"
            << cpuTime_
            << "</" << xmlNamespace << "cpu-time>\n";
    }

    // The file creation mask when the job was submitted.
    os  << indent
        << "<" << xmlNamespace << "umask>"
        << std::oct << umask_ << std::dec
        << "</" << xmlNamespace << "umask>\n";

    // The current working directory when the job was submitted.
    os  << indent
        << XmlTag(xmlNamespace + "cwd", cwd_) << "\n";

    // Home directory on submission host.
    os  << indent
        << XmlTag(xmlNamespace + "submission-home", subHomeDir_) << "\n";

    // The name of the host from which the job was submitted.
    os  << indent
        << XmlTag(xmlNamespace + "submission-host", fromHost_) << "\n";

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

    submit_.printXML(os);

    // Job exit status
    os  << indent
        << "<" << xmlNamespace << "exit-code>"
        << exitStatus_
        << "<" << xmlNamespace << "exit-code>\n";

//     int     execUid;        /**< Mapped UNIX user ID on the execution host.*/

    // Home directory for the job on the execution host
    os  << indent
        << XmlTag(xmlNamespace + "exec-home", execHome_) << "\n";

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
    if (execRusage_.size())
    {
        os  << indent
            << XmlTag(xmlNamespace + "exec-rusage", execRusage_) << "\n";
    }

#if 0

//     time_t rsvInActive;     /**< The time when advance reservation expired or was deleted. */

    // The number of licenses reported from License Scheduler.
    if (licenseNames_.size())
    {
        const char *listTag = "license-list";
        const char *tag = "license";

        os  << indent << "<" << xmlNamespace << listTag
            <<" count='" << licenseNames_.size() << "'>\n";

        for
        (
            std::vector<std::string>::const_iterator iter = licenseNames_.begin();
            iter != execHosts_.end();
            ++iter
        )
        {
            os  << indent << indent0
                << XmlTag(tag, (*iter)) << "\n";
        }

        os  << indent << "</" << xmlNamespace << listTag << ">\n";
    }
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

    // Number of hosts when job starts
    if (execHosts_.size())
    {
        const char *listTag = "exec-host-list";
        const char *tag = "exec-host";

        os  << indent << "<" << xmlNamespace << listTag
            <<" count='" << execHosts_.size() << "'>\n";

        // Host list when job starts
        for
        (
            std::vector<std::string>::const_iterator iter = execHosts_.begin();
            iter != execHosts_.end();
            ++iter
        )
        {
            os  << indent << indent0
                << XmlTag(tag, (*iter)) << "\n";
        }

        os  << indent << "</" << xmlNamespace << listTag << ">\n";
    }

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

    os  << indent0 << "</" << xmlNamespace << "job>\n";

    return os;
}
#endif 

// * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * Friend Functions  * * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * Friend Operators * * * * * * * * * * * * * * //

#if 0
//
// print some of job info structure
//
std::ostream& LsfUtil::operator<<
(
    std::ostream& os,
    const LsfUtil::JobInfoEntry& job
)
{
    return job.printXML(os);
}
#endif

/* ************************************************************************* */
