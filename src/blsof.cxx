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

#include <set>
#include <string>
#include <iostream>

#include <unistd.h>
#include <pwd.h>
#include <lsf/lsbatch.h>

#include "JobIdentifier.hxx"
#include "Rusage.hxx"

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

    std::cerr
        << "\n"
        << "copyright (c) 2011 Mark Olesen <Mark.Olesen@faurecia.com>\n";
}


std::string Script;

static void die(const std::string& message = "")
{
    if (!message.empty())
    {
        std::cerr << Script << ": " <<message << "\n";
    }
    std::cerr << "Try '" << Script << " -help' for more information.\n";

    exit(1);
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
    Script = argv[0];
    {
        std::string::size_type slash = Script.find_last_of('/');
        if (slash != std::string::npos)
        {
            Script.erase(0, slash+1);
        }
    }

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
                    die("missing arg for " + arg);
                    return 1;
                }
                addToFilter(jobFilter, argv[argI]);
            }
            else if (arg == "-resources" || arg == "-r" || arg == "-l")
            {
                ++argI;
                if (argI >= argc)
                {
                    die("missing arg for " + arg);
                    return 1;
                }
                addToFilter(rusageFilter, argv[argI]);
            }
            else if (arg == "-user" || arg == "-u")
            {
                ++argI;
                if (argI >= argc)
                {
                    die("missing arg for " + arg);
                    return 1;
                }
                addToFilter(userFilter, argv[argI]);
            }
            else if (arg == "-done")
            {
                options |= DONE_JOB;    // include done jobs
            }
            else if (arg == "-wait" || arg == "-w")
            {
                options |= PEND_JOB;    // include pending jobs
            }
            else
            {
                die("unknown option: " + arg);
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
            die("unknown argument: " + arg);
            return 1;
        }
    }

    if (jobFilter.empty() && rusageFilter.empty())
    {
        // no job or resource filters - must select by users

        if (userFilter.erase("*") || userFilter.erase("all"))
        {
            // any/all users - don't need a filter
            userFilter.clear();
        }
        else if (userFilter.empty())
        {
            // no other filters specified
            // - restrict to current user
            struct passwd* pw = ::getpwuid(::getuid());

            if (pw != NULL)
            {
                addToFilter(userFilter, pw->pw_name);
            }
        }
    }


    // initialize LSBLIB and get the configuration environment
    if (lsb_init(argv[0]) < 0)
    {
        std::cerr << "Error in lsb_init\n";
        return 1;
    }

    // gets the total number of pending job. Exits if failure
    int nJobs = lsb_openjobinfo(0, NULL, "all", NULL, NULL, options);

    if (nJobs < 0)
    {
        std::cerr << "Error in lsb_openjobinfo\n";
        return 1;
    }

    while (nJobs > 0)
    {
        const struct jobInfoEnt *job = lsb_readjobinfo(&nJobs);

        if (!job)
        {
            continue;
        }

        const struct submit& sub = job->submit;

        LsfUtil::JobIdentifier jobIdent(*job);

        // filter based on owner criterion
        if
        (
            !userFilter.empty()
         && userFilter.find(jobIdent.user) == userFilter.end()
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
                std::map<std::string, std::string> resReq
                    = LsfUtil::rusageMap(sub.resReq);

                for
                (
                    std::map<std::string, std::string>::const_iterator iter = resReq.begin();
                    iter != resReq.end();
                    ++iter
                )
                {
                    if ((rusageFilter.find(iter->first) != rusageFilter.end()))
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

        std::cout
            << jobIdent.cwd
            << " " << jobIdent.outfile
            << " " << jobIdent.jobIdString << "\n";
    }

    // close the connection
    lsb_closejobinfo();

    return 0;
}

/* ************************************************************************* */
