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

#include "lsfutil/OutputQstatJ.hpp"
#include "lsfutil/XmlUtils.hpp"


// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //


// * * * * * * * * * * * * * Static Member Functions * * * * * * * * * * * * //


// * * * * * * * * * * * * * * * Local Functions * * * * * * * * * * * * * * //


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

std::ostream&
lsfutil::OutputQstatJ::print
(
    std::ostream& os,
    const lsfutil::LsfJobEntry& job
)
{
    return os;
}


std::ostream&
lsfutil::OutputQstatJ::print
(
    std::ostream& os,
    const lsfutil::LsfJobSubEntry& sub
)
{
    return os;
}



// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

std::ostream&
lsfutil::OutputQstatJ::print
(
    std::ostream& os,
    const lsfutil::LsfJobList& list
)
{
    os  << "<?xml version='1.0'?>\n";

    if (list.hasError())
    {
        os  << "<lsf-error/>\n";

        return os;
    }

    os  << "<detailed_job_info"
        << " xmlns:xsd='http://www.w3.org/2001/XMLSchema'"
        << " type='lsf' count='" << list.size() << "'>\n";

    os
        << "</detailed_job_info>\n";

    return os;
}



// * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * Friend Functions  * * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * Friend Operators * * * * * * * * * * * * * * //


/* ************************************************************************* */
