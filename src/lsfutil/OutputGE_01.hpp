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

Description
    --

\*---------------------------------------------------------------------------*/

#ifndef OutputGE_01_H
#define OutputGE_01_H

#include <iostream>

#include "lsfutil/JobList.hpp"
#include "lsfutil/JobInfoEntry.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace LsfUtil
{

namespace OutputGE_01
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

    std::ostream& printXML(std::ostream&, const JobList&);
    std::ostream& printXML(std::ostream&, const JobInfoEntry&);
    std::ostream& printXML(std::ostream&, const JobSubmitEntry&);


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace OutputGE_01

} // End namespace LsfUtil

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
