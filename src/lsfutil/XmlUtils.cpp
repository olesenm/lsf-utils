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

#include "lsfutil/XmlUtils.hpp"


// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //


const char* const lsfutil::xml::indent0 = "  ";
const char* const lsfutil::xml::indent  = "    ";


// * * * * * * * * * * * * * Static Member Functions * * * * * * * * * * * * //

std::ostream& lsfutil::xml::printList
(
    std::ostream& os,
    const std::vector<std::string>& list,
    const char* listTag,
    const char* elemTag
)
{
    if (list.size())
    {
        os  << indent << "<" << listTag
            <<" count='" << list.size() << "'>\n";

        for
        (
            std::vector<std::string>::const_iterator iter = list.begin();
            iter != list.end();
            ++iter
        )
        {
            os  << indent << lsfutil::xml::indent0
                << Tag(elemTag, *iter) << "\n";
        }

        os  <<  indent << "</" << listTag << ">\n";
    }

    return os;
}


// * * * * * * * * * * * * * * * Local Functions * * * * * * * * * * * * * * //


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * Friend Functions  * * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * Friend Operators * * * * * * * * * * * * * * //


/* ************************************************************************* */
