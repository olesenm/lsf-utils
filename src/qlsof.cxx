/*---------------------------------*- C++ -*---------------------------------*\
Copyright 2011-2011 Mark Olesen

License
    This file is part of lsf-utils

    lsf-utils is free software: you can redistribute it and/or modify it under
    the terms of the GNU Affero General Public License as published by the
    Free Software Foundation, either version 3 of the License,
    or (at your option) any later version.

    lsf-utils is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
    or FITNESS FOR A PARTICULAR PURPOSE.
    See the GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with lsf-utils. If not, see <http://www.gnu.org/licenses/>.

Description
    --

\*---------------------------------------------------------------------------*/

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <string>
#include <iostream>
#include <sstream>

#include <lsf/lsbatch.h>

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


void usage()
{
    std::cerr <<
        "usage: blsof [OPTION] [spec1 .. specN]\n\n";

    fprintf
    (
        stderr,
        "A list open files (lsof) for LSF\n"
        "  - prints directory, outputfile, jobID\n\n"
    );
    fprintf
    (
        stderr,
        "options:\n"
    );
    fprintf
    (
        stderr,
        "  -j jobspec   filter based on jobids              job1[,job2..]\n"
        "                            key/value:       jobid=job1[,job2..]\n\n"
    );
    fprintf
    (
        stderr,
        "  -l request   filter based on resource requests   rc1[,rc2..]\n"
        "                            key/value:   resources=rc1[,rc2..]\n\n"
    );
    fprintf
    (
        stderr,
        "  -u userspec  filter based on job owner           user1[,user2..]\n"
        "                            key/value:       owner=user1[,user2..]\n"
        "                            key/value:        user=user1[,user2..]\n\n"
    );
    fprintf
    (
        stderr,
        "  -w           include waiting jobs\n"
    );
    fprintf
    (
        stderr,
        "  -h           usage\n"
    );
}


/*---------------------------------------------------------------------------*\
                    Class JobInfoEntry Declaration
\*---------------------------------------------------------------------------*/

class BlsofEntry
{
public:

    // Static data members


    // Public data
    //

        //- The job ID that the LSF system assigned to the job
        //  split into job-id + task-id
        int jobId;
        int taskId;

        //- The current working directory when the job was submitted.
        std::string cwd;

        //- The name of the user who submitted the job
        std::string outfile;


        std::string jobIdStr() const
        {
            std::ostringstream os;
            os  << jobId;
            if (taskId)
            {
                os  << "." << taskId;
            }
            return os.str();
        }

        std::string tokenI() const
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

        std::string tokenJ() const
        {
            std::ostringstream os;
            os  << jobId;
            return os.str();
        }

    // Constructors

        //- Construct from jobInfoEnt
        BlsofEntry(const jobInfoEnt& job)
        :
            jobId(LSB_ARRAY_JOBID(job.jobId)),
            taskId(LSB_ARRAY_IDX(job.jobId)),
            cwd(makeString(job.cwd)),
            outfile(makeString(job.submit.outFile))
        {
            fixDirName(cwd);
            fixFileName(outfile);

            // filename relative to cwd if possible
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


    //- Destructor
    ~BlsofEntry()
    {}


    // Member Functions

        // Access

        // Check

        // Edit

        // Write

    // Member Operators


    // Friend Functions

    // Friend Operators

    // IOstream Operators

};


int main(int argc, char **argv)
{
//    usage();
    int nJobs = 0;
    int errorCode = 0;

    struct submit req;            /* job specifications */
    memset(&req, 0, sizeof(req)); /* initializes req */

    /* variables for simulating bjobs command */

    int  options = CUR_JOB;  // the status of the jobs whose info is returned
    char *user = "all";         /* match jobs for all users */

    // initialize LSBLIB  and  get the configuration environment
    if (lsb_init(argv[0]) < 0) {
        lsb_perror("simbjobs: lsb_init() failed");
        return 1;
    }

    /* check if input is in the right format:
     * "./simbjobs COMMAND ARGUMENTS" */

    /* if (argc < 2) {
     fprintf(stderr, "Usage: simbjobs command\n");
     exit(-1);
     } */

    // gets the total number of pending job. Exits if failure/
    if (lsb_openjobinfo(0, NULL, user, NULL, NULL, options) < 0)
    {
        lsb_perror("lsb_openjobinfo");
        return 1;
    }

    /* display all pending jobs */

/*    printf("All pending jobs submitted by all users:\n"); */

    do
    {
        struct jobInfoEnt *job = lsb_readjobinfo(&nJobs);

        if (job)
        {
            BlsofEntry lsof(*job);

            std::cout
                << lsof.cwd
                << " " << lsof.outfile
                << " " << lsof.jobIdStr()
                << "\n";
        }
    }
    while (nJobs);

    /* close the connection */
    lsb_closejobinfo();

    return errorCode;
}


