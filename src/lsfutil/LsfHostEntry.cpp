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

#include "lsfutil/LsfHostEntry.hpp"

#include <lsf/lsbatch.h>

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

lsfutil::LsfHostEntry::LsfHostEntry(const struct hostInfoEnt& host)
:
    name(host.host),
    load_15m(0),
    free_tmp(0),
    free_swp(0),
    free_mem(0),
    maxJobs(host.maxJobs),
    numJobs(host.numJobs),
    numRUN(host.numRUN)
{
    // paranoid: check nIdx to ensure we are within bounds on the load arrays
    if (host.nIdx > R15M) { load_15m = host.load[R15M]; }
    if (host.nIdx > TMP)  { free_tmp = host.load[R15M]; }
    if (host.nIdx > SWP)  { free_swp = host.load[SWP]; }
    if (host.nIdx > MEM)  { free_mem = host.load[MEM]; }
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

lsfutil::LsfHostEntry::~LsfHostEntry()
{}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

std::ostream& lsfutil::LsfHostEntry::dump(std::ostream& os) const
{
    os  << "host: " << name << "\n";
    os  << "load 15m: "  << load_15m << "\n";
    os  << "free tmp: "  << free_tmp << "\n";
    os  << "free swap: " << free_swp << "\n";
    os  << "free mem: "  << free_mem << "\n";
    os  << "maxJobs: " << maxJobs << "\n";
    os  << "numJobs: " << numJobs << "\n";
    os  << "numRUN: " << numRUN << "\n";

    for (unsigned queueI = 0; queueI < queues.size(); ++queueI)
    {
        os  << "    queue: " << queues[queueI] << "\n";
    }

    return os;
}


/* ************************************************************************* */
