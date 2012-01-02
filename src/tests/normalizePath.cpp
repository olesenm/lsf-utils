/*---------------------------------*- C++ -*---------------------------------*\
Copyright (c) 2011-2012 Mark Olesen
-------------------------------------------------------------------------------
License
    This file is part of lsf-utils.

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

Application
    normalizePath

Description
    test that decoding paths, elimination of '..' etc works as expected

\*---------------------------------------------------------------------------*/

#include <cstdio>
#include <cstdlib>

#include <string>
#include <iostream>

#include "markutil/HttpCore.hpp"
using namespace markutil;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char **argv)
{
    std::cerr
        << "test HttpCore::httpNormalizePath()\n\n";

    if (argc == 1)
    {
        std::cerr
            << "no arguments given\n\n";
        return 1;
    }

    for (int argI = 1; argI < argc; ++argI)
    {
        std::string path(argv[argI]);

        std::cout
            << "raw>" << path << "<\n";

        path = HttpCore::httpDecodeUri(path);

        std::cout
            << "in >" << path << "<\n";

        HttpCore::httpNormalizePath(path);

        std::cout
            << "out>" << path << "<\n\n";
    }

    return 0;
}


// ************************************************************************* //
