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

#include "lsfutil/OutputQstatJ.hpp"
#include "lsfutil/XmlUtils.hpp"

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

std::ostream&
lsfutil::OutputQstatJ::print
(
    std::ostream& os,
    const lsfutil::LsfJobEntry& job
)
{
    const lsfutil::LsfJobSubEntry& sub = job.submit;

    os  << xml::indent0 << "<element>\n";

    os  << xml::indent << "<JB_job_number>"
        << job.jobId << "</JB_job_number>\n";

    if (job.submitTime)
    {
        os  << xml::indent
            << xml::TimeTag("JB_submission_time", job.submitTime) << "\n";
    }

    // The name of the user who submitted the job
    os  << xml::indent << xml::Tag("JB_owner", job.user) << "\n";

    os  << xml::indent << xml::Tag("JB_job_name", job.submit.jobName) << "\n";

    os  << xml::indent << "<JB_stdout_path_list>\n"
        << xml::indent << xml::indent0 << "<path_list>\n";

    os  << xml::indent << xml::indent
        << xml::Tag("PN_path", sub.outFile) << "\n";

    os  << xml::indent << xml::indent0 << "</path_list>\n"
        << xml::indent << "</JB_stdout_path_list>\n";


    // rusage
    LsfCore::rusage_map rusage = LsfCore::parseRusage(sub.resReq);

    if (rusage.size())
    {
        os << xml::indent << "<JB_hard_resource_list>\n";

        for
        (
            LsfCore::rusage_map::const_iterator iter = rusage.begin();
            iter != rusage.end();
            ++iter
        )
        {
            os  << xml::indent << xml::indent0
                << "<qstat_l_requests>\n";

            os  << xml::indent << xml::indent
                << xml::Tag("CE_name", iter->first) << "\n";
            os  << xml::indent << xml::indent
                << xml::Tag("CE_stringval", iter->second) << "\n";

            os  << xml::indent << xml::indent0
                << "</qstat_l_requests>\n";
        }

        os << xml::indent << "</JB_hard_resource_list>\n";
    }

    os  << xml::indent
        << xml::Tag("JB_cwd", job.cwd) << "\n";


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

    os  << xml::indent
        << "<JB_ja_tasks>\n";

    os  << xml::indent << xml::indent0
        << "<ulong_sublist>\n";

    // this is a real hack
    // return the raw SGE status codes
    os  << xml::indent << xml::indent << "<JAT_status>";
    if (job.isRunning())
    {
        // state = 'r'
        os  << 128;
    }
    else if (job.isPending())
    {
        // state = 'qw'
        os  << 2112;
    }
    else
    {
        os  << 0;
    }

    os  << "</JAT_status>\n";


    os  << xml::indent << xml::indent0
        << "</ulong_sublist>\n";
    os  << xml::indent
        << "</JB_ja_tasks>\n";


    os  << xml::indent0 << "</element>\n";

    return os;
}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

std::ostream&
lsfutil::OutputQstatJ::print
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

    os  << "<detailed_job_info"
        << " xmlns:xsd='http://www.w3.org/2001/XMLSchema'"
        << " type='lsf' count='" << list.size() << "'>\n";

    os  << "<djob_info>\n";

    // active jobs:
    for (unsigned jobI = 0; jobI < list.size(); ++jobI)
    {
        if (list[jobI].isRunning())
        {
            print(os, list[jobI]);
        }
    }

    // pending jobs:
    for (unsigned jobI = 0; jobI < list.size(); ++jobI)
    {
        if (list[jobI].isPending())
        {
            print(os, list[jobI]);
        }
    }

    os  << "</djob_info>\n";
    os  << "</detailed_job_info>\n";

    return os;
}


/* ************************************************************************* */
