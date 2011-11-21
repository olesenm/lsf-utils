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

#include <lsf/lsbatch.h>

static void usage(const std::string& message = "")
{
    if (!message.empty())
    {
        std::cerr << "\nError: " << message << "\n\n";
    }

    std::cerr
        << "usage: blsof [OPTION] [spec1 .. specN]\n\n"
        << "A list open files (lsof) for LSF\n"
        << "  - prints directory, outputfile, jobID\n\n"
        << "options:\n";

    std::cerr
        << "  -job job1[,job2..]          filter based on jobids\n"
        << "  -user user1[,user2..]       filter based on job owner\n"
        << "  -l, -resources rc1[,rc2..]  filter based on resource requests\n"
        << "  -wait                       include waiting jobs\n"
        << "  -done                       include completed jobs\n"
        << "  -help                       usage\n"
        << std::endl;

    std::cerr
        << "equivalent key/value specs:\n"
        << "  -job        =>  jobid=job1[,job2..]\n"
        << "  -resources  =>  resources=rc1[,rc2..]\n"
        << "  -user       =>  owner=user1[,user2..]\n"
        << "  -users      =>  user=user1[,user2..]\n"
        << std::endl;
}


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


// parse something like this
//     "accesDb=1 rusage[abaqus=10,abaqusJob=6] span[..]"
// or
//     "accesDb=1 rusage[abaqus=10:duration=1,abaqusJob=6] span[..]"
//  ->


std::set<std::string> getRusage(const std::string& s)
{
    std::set<std::string> filter;
    std::string::size_type pos = s.find("rusage[");
    if (pos != std::string::npos)
    {
        std::string::size_type endpos = pos;

        while ((pos = s.find_first_of(",[", endpos)) != std::string::npos)
        {
            ++pos;
            endpos = s.find_first_of(",:=]", pos);

            if (endpos == std::string::npos)
            {
                break;
            }

            std::string item(s.substr(pos, endpos-pos));
            if (!item.empty())
            {
                filter.insert(item);
            }
        }
    }

    return filter;
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

    std::set<std::string> jobFilter;
    std::set<std::string> userFilter;
    std::set<std::string> rusageFilter;

    // pass 2: process other options
    for (int argI = 1; argI < argc; ++argI)
    {
        std::string arg(argv[argI]);
        if (arg[0] == '-')
        {
            if (arg == "-job" || arg == "-j")
            {
                ++argI;
                if (argI >= argc)
                {
                    usage("missing arg for " + arg);
                    return 1;
                }
                addToFilter(jobFilter, argv[argI]);
            }
            else if (arg == "-resources" || arg == "-r" || arg == "-l")
            {
                ++argI;
                if (argI >= argc)
                {
                    usage("missing arg for " + arg);
                    return 1;
                }
                addToFilter(rusageFilter, argv[argI]);
            }
            else if (arg == "-user" || arg == "-u")
            {
                ++argI;
                if (argI >= argc)
                {
                    usage("missing arg for " + arg);
                    return 1;
                }
                addToFilter(userFilter, argv[argI]);
            }
            else if (arg == "-done")
            {
                options |= DONE_JOB; // include done jobs
            }
            else if (arg == "-wait" || arg == "-w")
            {
                options |= PEND_JOB; // include pending jobs
            }
            else
            {
                usage("unknown option: " + arg);
                return 1;
            }
        }
        else if (arg.compare(0,6,"jobid=") == 0)
        {
            addToFilter(jobFilter, arg.substr(6));
        }
        else if (arg.compare(0,5,"user=") == 0)
        {
            addToFilter(userFilter, arg.substr(5));
        }
        else if (arg.compare(0,6,"owner=") == 0)
        {
            addToFilter(userFilter, arg.substr(6));
        }
        else if (arg.compare(0,10,"resources=") == 0)
        {
            addToFilter(rusageFilter, arg.substr(10));
        }
        else
        {
            usage("unknown argument: " + arg);
            return 1;
        }
    }


    // initialize LSBLIB and get the configuration environment
    if (lsb_init(argv[0]) < 0) {
        lsb_perror(NULL);
        return 1;
    }


    // gets the total number of pending job. Exits if failure
    int nJobs = 0;

    if (!jobFilter.empty())
    {
        userFilter.clear();
        nJobs = lsb_openjobinfo(0, NULL, "all", NULL, NULL, options);
    }
    else if (userFilter.empty())
    {
        // current user
        nJobs = lsb_openjobinfo(0, NULL, NULL, NULL, NULL, options);
    }
    else
    {
        // any/all users
        if (userFilter.erase("*") || userFilter.erase("all"))
        {
            userFilter.clear();
        }
        nJobs = lsb_openjobinfo(0, NULL, "all", NULL, NULL, options);
    }

    if (nJobs < 0)
    {
        lsb_perror("lsb_openjobinfo");
        return 1;
    }

    while (nJobs > 0)
    {
        const struct jobInfoEnt *job = lsb_readjobinfo(&nJobs);

        if
        (
            !job
         || !(job->cwd)
         || !((job->submit).outFile)
        )
        {
            // break on errors
            errorCode = 1;
            break;
        }

        const struct submit& sub = job->submit;

        jobIdentifier jobIdent(job);

        // filter based on owner criterion
        if
        (
            !userFilter.empty()
         && userFilter.find(job->user) == userFilter.end()
        )
        {
            continue;
        }

        // filter based on job id criterion
        if
        (
            !jobFilter.empty()
         && jobFilter.find(jobIdent.tokenJ()) == jobFilter.end()
        )
        {
            continue;
        }

        // filter based on resource requests
        if (!rusageFilter.empty())
        {
            bool matched = false;
            if (sub.resReq && strlen(sub.resReq))
            {
                std::set<std::string> resReq(getRusage(sub.resReq));

                for
                (
                    std::set<std::string>::const_iterator iter = resReq.begin();
                    iter != resReq.end();
                    ++iter
                )
                {
                    if ((rusageFilter.find(*iter) != rusageFilter.end()))
                    {
                        matched = true;
                        break;
                    }
                }
            }
            if (!matched)
            {
                continue;
            }
        }

        std::string outFile((job->submit).outFile);
        replaceAll(outFile, "%J", jobIdent.tokenJ());
        replaceAll(outFile, "%I", jobIdent.tokenI());

        std::cout << job->cwd << " "
            << outFile << " "
            << jobIdent.toString() << std::endl;
    }

    /* close the connection */
    lsb_closejobinfo();

    return errorCode;
}

/* ************************************************************************* */
