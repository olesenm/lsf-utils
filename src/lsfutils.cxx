/*---------------------------------------------------------------------------*\
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
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <set>
#include <string>
#include <sstream>
#include <iostream>
#include <ctime>

#include "JobList.hxx"
#include "JobInfoEntry.hxx"
#include <lsf/lsbatch.h>

#include "OutputGE_02.hxx"

static void usage(const std::string& message = "")
{
    if (!message.empty())
    {
        std::cerr << "\nError: " << message << "\n\n";
    }

    std::cerr
        << "usage: bjobs-xml [OPTION] [spec1 .. specN]\n\n"
//        << "A list open files (lsof) for LSF\n"
//        << "  - prints directory, outputfile, jobID\n\n"
//        << "options:\n"
//        << "  -j jobspec   filter based on jobids              job1[,job2..]\n"
//        << "                            key/value:       jobid=job1[,job2..]\n\n"
//        << "  -l request   filter based on resource requests   rc1[,rc2..]\n"
//        << "                            key/value:   resources=rc1[,rc2..]\n\n"
//        << "  -u userspec  filter based on job owner           user1[,user2..]\n"
//        << "                            key/value:       owner=user1[,user2..]\n"
//        << "                            key/value:        user=user1[,user2..]\n\n"
        << "  -done        include done jobs\n"
        << "  -wait        include waiting jobs\n"
        << "  -help        usage\n"
        << std::endl;
}



// define xmlNamespace for linkage
std::string xmlNamespace;

class jobIdentifier
{
    int jobId_;
    int taskId_;

public:
    jobIdentifier(const struct jobInfoEnt* jobInfo)
    :
        jobId_(LSB_ARRAY_JOBID(jobInfo->jobId)),
        taskId_(LSB_ARRAY_IDX(jobInfo->jobId))
    {}

    jobIdentifier(const struct jobInfoEnt& jobInfo)
    :
        jobId_(LSB_ARRAY_JOBID(jobInfo.jobId)),
        taskId_(LSB_ARRAY_IDX(jobInfo.jobId))
    {}


    inline bool hasTasks() const
    {
        return taskId_ > 0;
    }


    inline int jobId() const
    {
        return jobId_;
    }

    inline int taskId() const
    {
        return taskId_;
    }

    inline std::string toString() const
    {
        std::ostringstream os;
        os  << jobId_;
        if (taskId_)
        {
            os  << "." << taskId_;
        }
        return os.str();
    }

    inline std::string tokenI() const
    {
        if (taskId_)
        {
            std::ostringstream os;
            os  << taskId_;
            return os.str();
        }
        else
        {
            return "0";
        }
    }

    inline std::string tokenJ() const
    {
        std::ostringstream os;
        os  << jobId_;
        return os.str();
    }

};


std::string& replaceAll
(
    std::string& context,
    const std::string& from,
    const std::string& to
)
{
    std::string::size_type lookHere = 0;
    std::string::size_type foundHere;

    while ((foundHere = context.find(from, lookHere)) != std::string::npos)
    {
          context.replace(foundHere, from.size(), to);
          lookHere = foundHere + to.size();
    }
    return context;
}


template<class T>
void addToFilter(std::set<T>& filter, const std::string& s)
{
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, ','))
    {
        if (!item.empty())
        {
            filter.insert(item);
        }
    }
}


int main(int argc, char **argv)
{
    // usage();
    int errorCode = 0;

    // variables for simulating bjobs command
    int options = CUR_JOB; // | JGRP_ARRAY_INFO;

    // pass 1: find -help option
    for (int argI = 1; argI < argc; ++argI)
    {
        const std::string arg(argv[argI]);
        if (arg == "-help" || arg == "-h")
        {
            usage();
            return 0;
        }
    }

    // pass 2: process other options
    for (int argI = 1; argI < argc; ++argI)
    {
        std::string arg(argv[argI]);
        if (arg[0] == '-')
        {
            if (arg == "-done" || arg == "-d")
            {
                options |= DONE_JOB; // include done jobs
            }
            else if (arg == "-wait" || arg == "-w")
            {
                options |= PEND_JOB; // include pending jobs
            }
        }
    }

    LsfUtil::JobList jobList;

    LsfUtil::OutputGE_02::printXML(std::cout, jobList);

    return errorCode;
}

/* ************************************************************************* */
