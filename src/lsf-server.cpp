/*---------------------------------*- C++ -*---------------------------------*\
Copyright (c) 2011-2012 Mark Olesen
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

Application
    lsf-server

Description
    Basic functions for producing text and xml output that is
    somewhat compatible with GridEngine output.

    The main purpose is to provide an LSF interface to existing
    xml-qstat code.

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
#include "lsfutil/LsfJobList.hpp"
#include "lsfutil/OutputQstat.hpp"
#include "lsfutil/OutputQstatJ.hpp"


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

//! \class SampleServer
//! \brief specialization of markutil::HttpServer
//
//! Serves LSF information
class LsfServer
:
    public markutil::HttpServer
{
    // Private Member Functions

    template<class T>
    static std::set<T>& addToFilter
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
        return filter;
    }


    template<class T>
    static std::set<T>& addToFilter
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
        return filter;
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


    int serve_blsof(std::ostream& os, HeaderType& head) const
    {
        lsfutil::LsfJobList jobs;

        if (jobs.hasError())
        {
            head(head._503_SERVICE_UNAVAILABLE);
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

            // display pending jobs too?
            bool withPending = false;
            if (query.foundUnnamed("wait"))
            {
                withPending = true;
            }
            else if (query.found("wait"))
            {
                const QueryType::string_list& list = query.param("wait");
                for
                (
                    QueryType::string_list::const_iterator iter = list.begin();
                    iter != list.end();
                    ++iter
                )
                {
                    if (*iter == "true")
                    {
                        withPending = true;
                        break;
                    }
                }
            }

            // filter job-list based on query parameters
            std::vector<int> displayJob;
            displayJob.reserve(jobs.size());

            for
            (
                unsigned jobI = 0;
                jobI < jobs.size();
                ++jobI
            )
            {
                const lsfutil::LsfJobEntry& job = jobs[jobI];

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
                    lsfutil::LsfCore::rusage_map resReq
                        = lsfutil::LsfCore::parseRusage(job.submit.resReq);

                    if (!intersectsFilter(rusageFilter, resReq))
                    {
                        continue;
                    }
                }

                // we got this far, keep it
                if (job.isRunning() || (withPending && job.isPending()))
                {
                    displayJob.push_back(jobI);
                }
            }

            for
            (
                unsigned displayI = 0;
                displayI < displayJob.size();
                ++displayI
            )
            {
                const lsfutil::LsfJobEntry& job = jobs[displayJob[displayI]];

                os  << job.cwd << " "
                    << job.submit.outFile << " "
                    << job.jobId << "\n";
            }

        }

        return 0;
    }


    int serve_dump(std::ostream& os, HeaderType& head) const
    {
        lsfutil::LsfJobList jobs;

        if (jobs.hasError())
        {
            head(head._503_SERVICE_UNAVAILABLE);
            head.print(os, true);

            return 1;
        }

        head.contentType("txt");
        os  << head(head._200_OK);

        if (head.request().type() == head.request().GET)
        {
            lsfutil::LsfJobList jobs;
            jobs.dump(os);
        }

        return 0;
    }


    int serve_qstat_xml(std::ostream& os, HeaderType& head) const
    {
        lsfutil::LsfJobList jobs;

        if (jobs.hasError())
        {
            head(head._503_SERVICE_UNAVAILABLE);
            head.print(os, true);

            return 1;
        }

        head.contentType("xml");
        os  << head(head._200_OK);

        if (head.request().type() == head.request().GET)
        {
            lsfutil::OutputQstat::print(os, jobs);
        }

        return 0;
    }


    int serve_qstatj_xml(std::ostream& os, HeaderType& head) const
    {
        lsfutil::LsfJobList jobs;

        if (jobs.hasError())
        {
            head(head._503_SERVICE_UNAVAILABLE);
            head.print(os, true);

            return 1;
        }

        head.contentType("xml");
        os  << head(head._200_OK);

        if (head.request().type() == head.request().GET)
        {
            lsfutil::OutputQstatJ::print(os, jobs);
        }

        return 0;
    }


public:

    typedef markutil::HttpServer ParentClass;

    // Constructors

        //! Create a server on specified port
        LsfServer(unsigned short port, const std::string& root)
        :
            ParentClass(port)
        {
            this->name("lsf-utils");
            this->root(root);
        }


    // General Operation

        //! Specialized reply
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

            if (url == "/dump")
            {
                return serve_dump(os, head);
            }

            if (url == "/blsof")
            {
                return serve_blsof(os, head);
            }

            if (url == "/qstat.xml")
            {
                return serve_qstat_xml(os, head);
            }

            if (url == "/qstatj.xml")
            {
                return serve_qstatj_xml(os, head);
            }


            // catch balance with standard document serving
            return this->ParentClass::reply(os, head);
        }

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //


int main(int argc, char **argv)
{
    const std::string name("sample-server");

    if (argc < 3 || argc > 4)
    {
        std::cerr
            << "incorrect number of arguments\n\n";

        std::cerr
            << "usage: "<< name << " Port DocRoot [cgi-bin]\n\n"
            << "Serve LSF information as text or xml, as well as providing a basic web server.\n\n"
            << "Eg,\n"
            << name << " " << markutil::HttpServer::defaultPort
            << " " << markutil::HttpServer::defaultRoot << "\n\n";

        return 1;
    }

    const int port = atoi(argv[1]);
    const std::string docRoot(argv[2]);
    const std::string cgiBin = (argc > 3 ? argv[3] : "");

    // verify port-number
    if (port < 1 || port > 65535)
    {
        std::cerr
            << "Invalid port number: (try 1 .. 65535)\n";
        return 1;
    }

    // verify doc-root
    if (!markutil::HttpCore::isDir(docRoot))
    {
        std::cerr
            << "Directory does not exist: " << docRoot << "\n";
        return 1;
    }

    // verify cgi-bin
    if (cgiBin.size() && !markutil::HttpCore::isDir(cgiBin))
    {
        std::cerr
            << "Directory does not exist: " << cgiBin << "\n";
        return 1;
    }

    markutil::HttpServer::daemonize();

    LsfServer server(port, docRoot);
    server.cgibin(cgiBin);

    server.listen(64);

    return server.run();
}


// ************************************************************************* //
