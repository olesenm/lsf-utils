/*---------------------------------*- C++ -*---------------------------------*\
                  ___                                _
                 / __)                              (_)
               _| |__ _____ _   _  ____ _____  ____ |_| _____
              (_   __|____ | | | |/ ___) ___ |/ ___)| |(____ |
                | |  / ___ | |_| | |   | ____( (___ | |/ ___ |
                |_|  \_____|____/|_|   |_____)\____)|_|\_____|

    Copyright (C) 2011-2011 Faurecia Emissions Control Technologies
-------------------------------------------------------------------------------
License
    This file contains proprietary and confidential information and
    is subject to a non-disclosure-agreement for use outside of FECT

Description
    --

Author
    --
\*---------------------------------------------------------------------------*/

#include "JobList.hxx"

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
