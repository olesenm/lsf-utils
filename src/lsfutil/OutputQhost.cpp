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

#include <iomanip>

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

std::ostream&
lsfutil::OutputQhost::print
(
    std::ostream& os,
    const lsfutil::LsfHostEntry& host
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
        << buffer <<  "</hostvalue>\n";

    os  << xml::indent << "<hostvalue name='mem_free'>"
        << host.free_mem <<  "M</hostvalue>\n";

    os  << xml::indent << "<hostvalue name='swap_free'>"
        << host.free_swp <<  "M</hostvalue>\n";


    // write queue information
    for (unsigned queueI = 0; queueI < host.queues.size(); ++queueI)
    {
        os  << xml::indent0 << "<queue name='" << host.queues[queueI] << "'>\n";

        std::string commonAttr = "<queuevalue qname='" + host.queues[queueI] + "' name='";

        // write queue values
        os  << xml::indent << commonAttr << "qtype_string" << "'>"
            << "BP"
            << "</queuevalue>\n";

        // write queue values
        os  << xml::indent << commonAttr << "slots" << "'>"
            << 2
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
    const lsfutil::LsfHostList& list
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
        print(os, list[hostI]);
    }

    os  << "</qhost>\n";

    return os;
}


/* ************************************************************************* */
