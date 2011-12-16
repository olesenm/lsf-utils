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

#include "lsfutil/JobList.hpp"

#include <ctime>
#include <lsf/lsbatch.h>

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //


// * * * * * * * * * * * * * Static Member Functions * * * * * * * * * * * * //


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //


// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

LsfUtil::JobList::JobList(unsigned interval, bool withPending)
:
    std::vector<LsfUtil::JobInfoEntry>(),
    lastUpdate_(0),
    interval_(interval),
    error_(false),
    options_(CUR_JOB)
{
    if (withPending)
    {
        options_ |= PEND_JOB;   // include pending jobs
    }

    this->update();

}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

LsfUtil::JobList::~JobList()
{
    this->clear();
}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

bool LsfUtil::JobList::update()
{
    const time_t now = time(0);
    const bool updated = (now >= lastUpdate_ + interval_);

    if (updated)
    {
        lastUpdate_ = now;

        this->clear();

        if (lsb_init("LsfUtil::JobList::update()") < 0)
        {
            error_ = true;
        }
        else
        {
            int nJobs = lsb_openjobinfo(0, NULL, "all", NULL, NULL, options_);
            // gets the total number of jobs, -1 on failure

            if (nJobs >= 0)
            {
                this->reserve(nJobs);
                while (nJobs > 0)
                {
                    const struct jobInfoEnt *job = lsb_readjobinfo(&nJobs);
                    if (job)
                    {
                        this->push_back(LsfUtil::JobInfoEntry(*job));
                    }
                }

                // close the connection
                lsb_closejobinfo();
            }
        }
    }

    return updated;
}


// * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * Friend Functions  * * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * Friend Operators * * * * * * * * * * * * * * //


/* ************************************************************************* */
