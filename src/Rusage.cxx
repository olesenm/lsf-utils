/*---------------------------------*- C++ -*---------------------------------*\
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

#include "Rusage.hxx"


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //


// parse stuff like this
// rusage[starcdLic=1:duration=5,starccmpLic=5:duration=5,starcdJob=6]
std::map<std::string, std::string>
LsfUtil::rusageMap(const std::string& resReq)
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


/* ************************************************************************* */
