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
#include <sstream>
#include <iostream>

#include <unistd.h>
#include <pwd.h>
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


// misc utils
inline std::string makeString(const char* str)
{
    return std::string(str ? str : "");
}

bool fixDirName(std::string& name)
{
    bool changed = false;
    while (name.size() > 1 && name[name.size()-1] == '/')
    {
        name.resize(name.size()-1);
        changed = true;
    }

    return changed;
}

bool fixFileName(std::string& name)
{
    if (name.size() > 2 && name[0] == '.' && name[1] == '/')
    {
        name.erase(0, 2);
        return true;
    }
    else
    {
        return false;
    }
}


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



/*---------------------------------------------------------------------------*\
                        Class JobIdentifier Declaration
\*---------------------------------------------------------------------------*/

class JobIdentifier
{
public:

    // Public data

        //- The job ID
        int jobId;

        //- The task ID. Is 0 for non-array job
        int taskId;

        //- The current working directory when the job was submitted
        std::string cwd;

        //- The output filename (absolute or relative to cwd)
        std::string outfile;

    // Constructors

        //- Construct from jobInfoEnt
        JobIdentifier(const struct jobInfoEnt& jobInfo)
        :
            jobId(LSB_ARRAY_JOBID(jobInfo.jobId)),
            taskId(LSB_ARRAY_IDX(jobInfo.jobId)),
            cwd(makeString(jobInfo.cwd)),
            outfile(makeString(jobInfo.submit.outFile))
        {
            fixDirName(cwd);
            fixFileName(outfile);

            // filename relative to cwd whenever possible
            if
            (
                outfile.size() > cwd.size()+1
             && outfile[cwd.size()] == '/'
             && outfile.substr(0, cwd.size()) == cwd
            )
            {
                outfile.erase(0, cwd.size()+1);
            }

            // replace %J with jobId and %I with taskId
            replaceAll(outfile, "%J", this->tokenJ());
            replaceAll(outfile, "%I", this->tokenI());
        }


    // Member Functions

        inline bool hasTasks() const
        {
            return taskId > 0;
        }

        inline std::string jobIdString() const
        {
            std::ostringstream os;
            os  << jobId;
            if (taskId)
            {
                os  << "." << taskId;
            }
            return os.str();
        }

        inline std::string tokenI() const
        {
            if (taskId)
            {
                std::ostringstream os;
                os  << taskId;
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
            os  << jobId;
            return os.str();
        }

};



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


std::set<std::string> getRusage(const std::string& resReq)
{
    std::set<std::string> filter;

    const std::string begMark = "rusage[";
    std::string::size_type beg = resReq.find(begMark);
    if (beg != std::string::npos)
    {
        beg += begMark.size();
        std::string::size_type end;
        std::string::size_type equals;

        while
        (
            (end = resReq.find_first_of(",:]", beg)) != std::string::npos
         && (equals = resReq.find('=', beg)) != std::string::npos
         && (equals < end)
        )
        {
            std::string item(resReq.substr(beg, equals-beg));
            if (!item.empty())
            {
                filter.insert(item);
            }

            ++equals;

            if (resReq[end] == ':')
            {
                end = resReq.find_first_of(",]", end);
            }

            if (end == std::string::npos || resReq[end] == ']')
            {
                break;
            }


            beg = end + 1;
        }
    }

    return filter;
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
                options |= DONE_JOB; // include done jobs
            }
            else if (arg == "-wait" || arg == "-w")
            {
                options |= PEND_JOB; // include pending jobs
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

    // sort out filters a bit
    if (!jobFilter.empty())
    {
        // sort by job only - for all users
        userFilter.clear();
    }
    else
    {
        if (userFilter.erase("*") || userFilter.erase("all"))
        {
            // any/all users - don't need a filter
            userFilter.clear();
        }
        else if (userFilter.empty())
        {
            // no user specified - restrict to current user
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
        lsb_perror(NULL);
        return 1;
    }


    // gets the total number of pending job. Exits if failure
    int nJobs = 0;

    nJobs = lsb_openjobinfo(0, NULL, "all", NULL, NULL, options);

    if (nJobs < 0)
    {
        std::cerr << "Error initializing lsb_openjobinfo\n";
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

        JobIdentifier jobIdent(*job);

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

        std::cout
            << jobIdent.cwd
            << " " << jobIdent.outfile
            << " " << jobIdent.jobIdString() << "\n";
    }

    // close the connection
    lsb_closejobinfo();

    return 0;
}

/* ************************************************************************* */
