/*---------------------------------*- C++ -*---------------------------------*\
Copyright (c) 2012 Mark Olesen
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

#include "lsfutil/OutputQhost.hpp"
#include "lsfutil/XmlUtils.hpp"
#include <cstdio>

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

std::ostream&
lsfutil::OutputQhost::print
(
    std::ostream& os,
    const lsfutil::LsfHostEntry& host,
    const lsfutil::LsfJobList& jlist
)
{
    char buffer[32];

    os  << xml::indent0
        << "<host name='"
        << host.name << "'>\n";


    // write host values
    os  << xml::indent << "<hostvalue name='arch_string'>"
        << "lx26-amd64"
        << "</hostvalue>\n";

    // take max slots as being identical to num_proc
    os  << xml::indent << "<hostvalue name='num_proc'>"
        << host.maxJobs
        << "</hostvalue>\n";

    // total jobs slots
    os  << xml::indent << "<hostvalue name='slots'>"
        << host.maxJobs
        << "</hostvalue>\n";

    // num of jobs slots used (running or suspended)
    os  << xml::indent << "<hostvalue name='slots_used'>"
        << host.numJobs
        << "</hostvalue>\n";

    // num of jobs slots (running)
    //    os  << xml::indent << "<hostvalue name='slots_running'>"
    //        << host.numRUN
    //        << "</hostvalue>\n";


    // max of 3 decimal places
    snprintf(buffer, sizeof(buffer)-1, "%.3f", host.load_15m);
    os  << xml::indent << "<hostvalue name='load_avg'>"
        << buffer
        << "</hostvalue>\n";

    os  << xml::indent << "<hostvalue name='mem_free'>"
        << host.free_mem <<  "M</hostvalue>\n";

    os  << xml::indent << "<hostvalue name='swap_free'>"
        << host.free_swp <<  "M</hostvalue>\n";


    // count slots used per queue instance
    std::map<std::string, int> slots_used;
    for (unsigned queueI = 0; queueI < host.queues.size(); ++queueI)
    {
        const std::string& queueName = host.queues[queueI];
        slots_used[queueName] = 0;
    }


    // write job information and count slots
    for (unsigned jobI = 0; jobI < jlist.size(); ++jobI)
    {
        const LsfJobEntry& job = jlist[jobI];
        const std::string& queueName = job.submit.queue;

        std::string commonAttr;
        for (unsigned hostI = 0; hostI < job.execHosts.size(); ++hostI)
        {
            const std::string& hostName  = job.execHosts[hostI];

            if (hostName != host.name)
            {
                continue;
            }
            else if (commonAttr.empty())
            {
                commonAttr = "<jobvalue jobid='" + job.fqJobId() + "' name='";
            }

            slots_used[queueName] = slots_used[queueName] + 1;

            os  << xml::indent0 << "<job name='" << job.fqJobId() << "'>\n";


            // queue instance
            os  << xml::indent << commonAttr << "qinstance_name" << "'>"
                << job.submit.queue << "@" << hostName
                << "</jobvalue>\n";

            os  << xml::indent << commonAttr << "job_name" << "'>"
                << job.submit.jobName
                << "</jobvalue>\n";

            os  << xml::indent << commonAttr << "job_owner" << "'>"
                << job.user
                << "</jobvalue>\n";

            os  << xml::indent << commonAttr << "job_state" << "'>";
            if (job.isRunning())
            {
                os  << "r";
            }
            else if (job.isSuspend())
            {
                os  << "s";
            }

            os  << "</jobvalue>\n";

            os  << xml::indent << commonAttr << "start_time" << "'>"
                << job.startTime << "</jobvalue>\n";

            os  << xml::indent << commonAttr << "pe_master" << "'>";
            os  << (hostI ? "SLAVE" : "MASTER");
            os  << "</jobvalue>\n";
            os  << xml::indent0 << "</job>\n";
        }
    }


    // write queue information
    for (unsigned queueI = 0; queueI < host.queues.size(); ++queueI)
    {
        const std::string& queueName = host.queues[queueI];

        os  << xml::indent0 << "<queue name='" << queueName << "'>\n";

        std::string commonAttr = "<queuevalue qname='" + queueName + "' name='";

        // assume everything is BATCH
        os  << xml::indent << commonAttr << "qtype_string" << "'>"
            << "BP"
            << "</queuevalue>\n";

        // slots used
        os  << xml::indent << commonAttr << "slots_used" << "'>"
            << slots_used[queueName]
            << "</queuevalue>\n";

        // total jobs slots
        os  << xml::indent << commonAttr << "slots" << "'>"
            << host.maxJobs
            << "</queuevalue>\n";

        // 'S' for suspend etc
        os  << xml::indent << commonAttr << "state_string" << "'>"
            << "</queuevalue>\n";

        os  << xml::indent0 << "</queue>\n";
    }

    os  << xml::indent0 << "</host>\n";

    return os;
}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

std::ostream&
lsfutil::OutputQhost::print
(
    std::ostream& os,
    const lsfutil::LsfHostList& list,
    const lsfutil::LsfJobList& jlist
)
{
    os  << "<?xml version='1.0'?>\n";

    if (list.hasError())
    {
        os  << "<lsf-error/>\n";

        return os;
    }

    os  << "<qhost"
        << " xmlns:xsd='http://gridengine.sunsource.net/61/qhost'"
        << " type='lsf' count='" << list.size() << "'>\n";

    for (unsigned hostI = 0; hostI < list.size(); ++hostI)
    {
        print(os, list[hostI], jlist);
    }

    os  << "</qhost>\n";

    return os;
}


/* ************************************************************************* */
