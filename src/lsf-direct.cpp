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

#include <iostream>

#include "lsfutil/LsfJobList.hpp"
#include "lsfutil/OutputQstat.hpp"
#include "lsfutil/OutputQstatJ.hpp"
#include "markutil/HttpRequest.hpp"

int main(int argc, char **argv)
{
    if (argc == 1)
    {
        std::cerr
            << "no resource specified\n";
        return 1;
    }

    markutil::HttpRequest req("GET", argv[1]);

    std::string url = req.path();

    if (url == "/dump")
    {
        lsfutil::LsfJobList jobs;
        jobs.dump(std::cout);
        return 0;
    }

    if (url == "/blsof")
    {
        lsfutil::LsfJobList jobs;

        for (unsigned jobI = 0; jobI < jobs.size(); ++jobI)
        {
            const lsfutil::LsfJobEntry& job = jobs[jobI];

            std::cout
                << job.cwd << " "
                << job.relativeFilePath(job.submit.outFile) << " "
                << job.jobId << "\n";
        }
        return 0;
    }

    if (url == "/qstat.xml")
    {
        lsfutil::LsfJobList jobs;
        lsfutil::OutputQstat::print(std::cout, jobs);
        return 0;
    }

    if (url == "/qstatj.xml")
    {
        lsfutil::LsfJobList jobs;
        lsfutil::OutputQstatJ::print(std::cout, jobs);
        return 0;
    }

    std::cerr
        << "no handler for " << url << "\n";

    return 1;
}

/* ************************************************************************* */
