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
    lsfutil::OutputQstat

Description
    Output job list in GridEngine qstat xml format

SourceFiles
    OutputQstat.cpp

\*---------------------------------------------------------------------------*/

#ifndef LSF_OUTPUT_QSTAT_H
#define LSF_OUTPUT_QSTAT_H

#include <iostream>

#include "lsfutil/LsfJobList.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace lsfutil
{

/*---------------------------------------------------------------------------*\
                         Class OutputQstat Declaration
\*---------------------------------------------------------------------------*/

class OutputQstat
{
    // Private Member Functions

        static std::ostream& print(std::ostream&, const LsfJobEntry&);
        static std::ostream& print(std::ostream&, const LsfJobSubEntry&);

public:

        static std::ostream& print(std::ostream&, const LsfJobList&);


};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //


} // End namespace lsfutil


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif  // LSF_OUTPUT_QSTAT_H

// ************************************************************************* //
