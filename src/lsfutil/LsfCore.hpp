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

Class
    lsfutil::LsfCore

Description
    Core utilities required by various LSF-related elements

SourceFiles
    LsfCore.cpp

\*---------------------------------------------------------------------------*/

#ifndef LSF_CORE_H
#define LSF_CORE_H

#include <map>
#include <string>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace lsfutil
{

/*---------------------------------------------------------------------------*\
                           Class LsfCore Declaration
\*---------------------------------------------------------------------------*/

class LsfCore
{
public:

    // Constructors

        //! Construct null
        LsfCore();

        //! Destructor
        ~LsfCore();


    static inline std::string makeString(const char* str)
    {
        return std::string(str ? str : "");
    }

    static std::string makeString(int i);

    //- Remove trailing '/' from dir names
    static bool fixDirName(std::string& name);

    //- Remove leading './' from file names
    static bool fixFileName(std::string& name);


    // parse stuff like this
    // rusage[starcdLic=1:duration=5,starccmpLic=5:duration=5,starcdJob=6]
    static std::map<std::string, std::string>
    rusageMap(const std::string& resReq);

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace lsfutil

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif  // LSF_CORE_H

// ************************************************************************* //
