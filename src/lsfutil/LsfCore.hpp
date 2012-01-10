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

Class
    lsfutil::LsfCore

Description
    Core utilities required by various LSF-related elements

SourceFiles
    LsfCore.cpp

Namespace
    lsfutil

Description
    Namespace for various pieces related to LSF routines

\*---------------------------------------------------------------------------*/

#ifndef LSF_CORE_H
#define LSF_CORE_H

#include <map>
#include <vector>
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

    //- The map type for rusage
    typedef std::map<std::string, std::string> rusage_map;


    // Constructors

        //- Construct null
        LsfCore();


    //- Destructor
    ~LsfCore();


        // Member Functions

            //- Create a string, even from a NULL pointer
            static inline std::string makeString(const char* str)
            {
                return std::string(str ? str : "");
            }

            //- Create a string from an integer
            static std::string makeString(int i);


            //- Remove trailing '/' from dir name
            static bool fixDirName(std::string& name);

            //- Remove leading './' from file name
            static bool fixFileName(std::string& name);

            //- Replace all occurrences of a given string
            static std::string& replaceAll
            (
                std::string& context,
                const std::string& from,
                const std::string& to
            );

            //- Parse rusage information
            //  This includes stuff that looks like this:
            //  \verbatim
            //  rusage[starcdLic=1:duration=5,starccmpLic=5:duration=5,starcdJob=6]
            //  \endverbatim
            static rusage_map parseRusage(const std::string& resReq);

            //- Parse space-delimited string into a vector of strings
            static std::vector<std::string>
            parseSpaceDelimited(const std::string&);

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace lsfutil

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif  // LSF_CORE_H

// ************************************************************************* //
