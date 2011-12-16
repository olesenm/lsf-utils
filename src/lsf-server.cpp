/*---------------------------------*- C++ -*---------------------------------*\
Copyright 2011 Mark Olesen
-------------------------------------------------------------------------------
License
    This file is part of lsf-utils.

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

Class
    LsfServer

Description

SourceFiles
    LsfServer.cpp

\*---------------------------------------------------------------------------*/

#include <cstdio>
#include <cstdlib>

#include <set>
#include <string>
#include <iostream>
#include <sstream>

#include "markutil/HttpServer.hpp"
#include "lsfutil/JobList.hpp"
#include "lsfutil/JobInfoEntry.hpp"
#include "lsfutil/OutputGE_02.hpp"
#include "lsfutil/Rusage.hpp"
#include <lsf/lsbatch.h>


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// specialization of HttpServer
class LsfServer
:
    public markutil::HttpServer
{

    template<class T>
    static void addToFilter
    (
        std::set<T>& filter,
        const std::string& s
    )
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


    template<class T>
    static void addToFilter
    (
        std::set<T>& filter,
        const QueryType& query,
        const std::string& name
    )
    {
       const QueryType::string_list& args = query.param(name);

       for
       (
           QueryType::string_list::const_iterator iter = args.begin();
           iter != args.end();
           ++iter
       )
       {
           addToFilter(filter, *iter);
       }
    }


    template<class T, class AnyT>
    static bool intersectsFilter
    (
        std::set<T>& filter,
        std::map<T, AnyT>& source
    )
    {
        bool matched = false;

        for
        (
            typename std::map<T, AnyT>::const_iterator iter = source.begin();
            !matched && iter != source.end();
            ++iter
        )
        {
            matched = filter.count(iter->first);
        }

        return matched;
    }



    virtual int serve_blsof
    (
        std::ostream& os,
        HeaderType& head
    ) const
    {
        LsfUtil::JobList jobs;

        if (jobs.hasError())
        {
            head(head._500_INTERNAL_SERVER_ERROR);
            head.print(os, true);

            return 1;
        }

        head.contentType("txt");
        os  << head(head._200_OK);

        if (head.request().type() == head.request().GET)
        {
            const QueryType& query = head.request().query();

            std::set<std::string> jobFilter;
            std::set<std::string> userFilter;
            std::set<std::string> rusageFilter;

            addToFilter(jobFilter, query, "jobid");
            addToFilter(rusageFilter, query, "resources");
            addToFilter(userFilter, query, "owner");
            addToFilter(userFilter, query, "user");

            // if userFilter has 'all' this is the same as no filter
            // also respect '*' as per GridEngine
            if (userFilter.count("all") || userFilter.count("*"))
            {
                userFilter.clear();
            }

            // filter job-list based on query parameters
            std::vector<int> displayJob;
            displayJob.reserve(jobs.size());


            for (unsigned jobI = 0; jobI < jobs.size(); ++jobI)
            {
                const LsfUtil::JobInfoEntry& job = jobs[jobI];

                // filter based on owner criterion
                if (!userFilter.empty() && !userFilter.count(job.user))
                {
                    continue;
                }


                // filter based on job id criterion
                if (!jobFilter.empty() && !jobFilter.count(job.tokenJ()))
                {
                    continue;
                }


                // filter based on resource requests
                if (!rusageFilter.empty())
                {
                    std::map<std::string, std::string> resReq
                        = LsfUtil::rusageMap(job.submit.resReq);

                    if (!intersectsFilter(rusageFilter, resReq))
                    {
                        continue;
                    }
                }

                // we got this far, keep it
                displayJob.push_back(jobI);
            }

            for (unsigned displayI = 0; displayI < displayJob.size(); ++displayI)
            {
                const LsfUtil::JobInfoEntry& job = jobs[displayJob[displayI]];

                os  << job.cwd << " "
                    << job.relativeFilePath(job.submit.outFile) << " "
                    << job.jobId << "\n";
            }

        }

        return 0;
    }


    virtual int serve_qstat_xml
    (
        std::ostream& os,
        HeaderType& head
    ) const
    {
        LsfUtil::JobList jobs;

        if (jobs.hasError())
        {
            head(head._500_INTERNAL_SERVER_ERROR);
            head.print(os, true);

            return 1;
        }

        head.contentType("xml");
        os  << head(head._200_OK);

        if (head.request().type() == head.request().GET)
        {
            LsfUtil::OutputGE_02::printXML(os, jobs);
        }

        return 0;
    }


public:

    typedef markutil::HttpServer ParentClass;

    // Constructors

        //- Create a server on specified port
        LsfServer(unsigned short port, const std::string& root)
        :
            ParentClass(port)
        {
            this->name("lsf-utils");
            this->root(root);
        }

        virtual int reply(std::ostream& os, HeaderType& head) const
        {
            RequestType& req = head.request();
            const std::string& url = req.path();

            if
            (
                req.type() != req.HEAD
             && req.type() != req.GET
            )
            {
                head(head._405_METHOD_NOT_ALLOWED);
                head("Allow", "GET,HEAD");
                head.print(os, true);

                return 1;
            }

            if (url == "/blsof")
            {
                return serve_blsof(os, head);
            }

            if (url == "/qstat.xml")
            {
                return serve_qstat_xml(os, head);
            }


            // catch balance with standard document serving
            return this->ParentClass::reply(os, head);
        }

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //


int main(int argc, char **argv)
{
    const std::string name("sample-server");

    if (argc != 3)
    {
        std::cerr
            << "incorrect number of arguments\n\n";

        std::cerr
            << "usage: "<< name << " Port DocRoot\n\n"
            << "A small sample web server in C++\n\n"
            << "Eg,\n"
            << name << " " << markutil::HttpServer::defaultPort
            << " " << markutil::HttpServer::defaultRoot << "\n\n";

        return 1;
    }

    int port = atoi(argv[1]);
    std::string docRoot(argv[2]);

    // check port-number
    if (port < 1 || port > 65535)
    {
        std::cerr
            << "Invalid port number: (try 1 .. 65535)\n";
        return 1;
    }

    // check if doc-root exists
    if (!markutil::HttpServer::isDir(docRoot))
    {
        std::cerr
            << "Directory does not exist: "<< docRoot << "\n";
        return 1;
    }

    markutil::HttpServer::daemonize();

    LsfServer server(port, docRoot);
    server.listen(64);

    return server.run();
}


// ************************************************************************* //
