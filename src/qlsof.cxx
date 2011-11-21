/******************************************************

* LSBLIB -- Examples
*
* simple bjobs
* Submit command as an lsbatch job with no options set
* and retrieve the job info
* It is similar to the "bjobs" command with no options.

******************************************************/
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <string>
#include <iostream>

#include <lsf/lsbatch.h>

/* #include "submit_cmd.h"*/

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


int main(int argc, char **argv)
{
     usage();
    int nJobs = 0;
    int errorCode = 0;

    struct submit req;            /* job specifications */
    memset(&req, 0, sizeof(req)); /* initializes req */

    int  jobId;                /* job ID of submitted job */

    /* variables for simulating bjobs command */

    int  options = CUR_JOB;    /* the status of the jobs
                                whose info is returned */

    char *user = "all";         /* match jobs for all users */
    char *outfile;
    LS_LONG_INT id=2136;

    int more;                   /* number of remaining jobs unread */

    /* initialize LSBLIB  and  get the configuration environment */

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

    /* gets the total number of pending job. Exits if failure */

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

        if
        (
            !job
         || !(job->cwd)
         || !((job->submit).outFile)
        )
        {
            /* break on errors */
            errorCode = 1;
            break;
        }

        printf("%s ", job->cwd);
        {
            char *p;
            char *outFileRaw = (job->submit).outFile;

            int nTokens = 0;
            int nExtra = 0;
            for (p = outFileRaw; *p; ++p)
            {
                switch (*p)
                {
                    case '%':
                        {
                            break;
                        }
                }
            }

            /* printf("%s ", outfile); */
            printf("%s ", outFileRaw);

        }
        printf("%s\n", lsb_jobid2str(job->jobId));
    }
    while (nJobs);

    /* close the connection */
    lsb_closejobinfo();

    return errorCode;
}


