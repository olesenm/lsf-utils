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

#ifndef Rusage_H
#define Rusage_H

#include <map>
#include <string>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace LsfUtil
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

    std::map<std::string, std::string> rusageMap(const std::string& resReq);


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace LsfUtil

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //


#endif

// ************************************************************************* //
