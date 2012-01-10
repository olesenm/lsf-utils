/*---------------------------------*- C++ -*---------------------------------*\
Copyright (c) 2011-2012 Mark Olesen
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

#include "lsfutil/LsfCore.hpp"
#include <sstream>


// * * * * * * * * * * * * * Static Member Functions * * * * * * * * * * * * //

std::string lsfutil::LsfCore::makeString(int i)
{
    if (i)
    {
        std::ostringstream os;
        os  << i;
        return os.str();
    }
    else
    {
        return "0";
    }
}


bool lsfutil::LsfCore::fixDirName(std::string& name)
{
    bool changed = false;
    while (name.size() > 1 && *(name.rbegin()) == '/')
    {
        name.resize(name.size()-1);
        changed = true;
    }

    return changed;
}


bool lsfutil::LsfCore::fixFileName(std::string& name)
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


std::string& lsfutil::LsfCore::replaceAll
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


// parse stuff like this
// rusage[starcdLic=1:duration=5,starccmpLic=5:duration=5,starcdJob=6]
std::map<std::string, std::string>
lsfutil::LsfCore::parseRusage(const std::string& resReq)
{
    std::map<std::string, std::string> output;

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
            std::string key = resReq.substr(beg, equals-beg);
            ++equals;
            std::string val = resReq.substr(equals, end-equals);
            output[key] = val;

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

    return output;
}


std::vector<std::string>
lsfutil::LsfCore::parseSpaceDelimited(const std::string& str)
{
    std::vector<std::string> output;

    std::istringstream ss(str);
    std::string item;
    while (std::getline(ss, item, ' '))
    {
        if (!item.empty())
        {
            output.push_back(item);
        }
    }

    return output;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

lsfutil::LsfCore::LsfCore()
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

lsfutil::LsfCore::~LsfCore()
{}


/* ************************************************************************* */
