/*---------------------------------*- C++ -*---------------------------------*\
Copyright (c) 2011-2011 Mark Olesen
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


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
// misc utils
//

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
    while (name.size() > 1 && name[name.size()-1] == '/')
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



// parse stuff like this
// rusage[starcdLic=1:duration=5,starccmpLic=5:duration=5,starcdJob=6]
std::map<std::string, std::string>
lsfutil::LsfCore::rusageMap(const std::string& resReq)
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
            std::string val = resReq.substr(equals, end-equals);
            output[key] = val;

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

    return output;
}


lsfutil::LsfCore::LsfCore()
{}

lsfutil::LsfCore::~LsfCore()
{}



/* ************************************************************************* */