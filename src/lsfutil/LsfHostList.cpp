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

#include "lsfutil/LsfHostList.hpp"

#include <lsf/lsbatch.h>


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

lsfutil::LsfHostList::LsfHostList(unsigned interval)
:
    std::vector<lsfutil::LsfHostEntry>(),
    lastUpdate_(0),
    interval_(interval),
    error_(false)
{
    this->update();
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

lsfutil::LsfHostList::~LsfHostList()
{
    this->clear();
}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

bool lsfutil::LsfHostList::update()
{
    const time_t now = time(0);
    const bool updated = (now >= lastUpdate_ + interval_);

    if (updated)
    {
        lastUpdate_ = now;

        this->clear();

        if (lsb_init("lsfutil::LsfHostList::update()") < 0)
        {
            error_ = true;
        }
        else
        {
            int numHosts = 0;   // get all hosts

            // gets the total number of hosts, return NULL on failure
            struct hostInfoEnt *hostArray = lsb_hostinfo(NULL, &numHosts);

            if (hostArray && numHosts >= 0)
            {
                this->reserve(numHosts);
                for (int hostI = 0; hostI < numHosts; ++hostI)
                {
                    this->push_back(lsfutil::LsfHostEntry(hostArray[hostI]));
                }
            }


            int numQueues = 0;   // get all queues

            // gets the total number of hosts, return NULL on failure
            struct queueInfoEnt *queueArray = lsb_queueinfo
            (
                NULL,
                &numQueues,
                NULL,
                NULL,
                0
            );

            if (queueArray && numQueues >= 0)
            {
                for (int queueI = 0; queueI < numQueues; ++queueI)
                {
                    const struct queueInfoEnt* queueInfo = &(queueArray[queueI]);

                    std::vector<std::string> qhosts =
                        LsfCore::parseSpaceDelimited(queueInfo->hostList);

                    // add as appropriate
                    for
                    (
                        unsigned qhostI = 0;
                        qhostI < qhosts.size();
                        ++qhostI
                    )
                    {
                        const std::string& hname = qhosts[qhostI];

                        for
                        (
                            iterator iter = this->begin();
                            iter != this->end();
                            ++iter
                        )
                        {
                            if (iter->name == hname)
                            {
                                iter->queues.push_back(queueInfo->queue);
                            }
                        }
                    }

//                    std::cerr
//                        <<"queue: " << queueInfo->queue << "\n"
//                        <<"  hostList: " << queueInfo->hostList << "\n";
//                    if (job)
//                    {
//                        this->push_back(lsfutil::LsfHostEntry(*job));
//                    }
                }
            }

        }
    }

    return updated;
}


std::ostream& lsfutil::LsfHostList::dump(std::ostream& os) const
{
    for (unsigned hostI = 0; hostI < this->size(); ++hostI)
    {
        if (!hostI)
        {
            os  << "==================================================\n";
        }
        this->operator[](hostI).dump(os);
        os  << "==================================================\n";
    }

    return os;
}


/* ************************************************************************* */
