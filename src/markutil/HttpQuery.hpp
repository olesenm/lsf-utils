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

Class
    markutil::HttpQuery

Description
    Uses both ampersand '&' and semicolon ';' for separating fields,
    as recommended in
    http://www.w3.org/TR/1999/REC-html401-19991224/appendix/notes.html#h-B.2.2

    The parsing behaviour regarding named and unnamed parameters differs from
    what the Perl CGI normally implements.  For the testing,
    CGI.pm 3.49 and CGI::Fast.pm 1.08 were used.

Query consisting entirely of a single string, without field separators:
@verbatim
Query: ?field

    keywords() == [ "field" ]
    params()   == keywords => [ "field" ]

@endverbatim

Special treatment for splitting on space. This follows from
http://tools.ietf.org/html/rfc3875 recommendation.
@verbatim
Query: ?foo+bar+baz
Query: ?foo%20bar%20baz

    keywords() == [ "foo", "bar", "baz" ]
    params()   == [ keywords => [ "foo", "bar", "baz" ] ]
@endverbatim

This special behavior changes when a '&' or ';' field separator is present.
@verbatim
Query: ?field;field1=value1;field2=value2;field1=value3;field2;field3=;

    keywords() == []
    params()   ==
    [
        field  => [ "" ],
        field1 => [ "value1", "value3" ],
        field2 => [ "value2", "" ],
        field3 => [ "" ],
    ]
@endverbatim

Similarly the splitting on space works much differently, although
http://tools.ietf.org/html/rfc3875 only mentions changing behavior if a
'=' is present
@verbatim
Query: ?foo+bar+baz;field1
Query: ?foo%20bar%20baz;field1

    keywords() == []
    params()   == 
    [
        "foo bar baz" => [ "" ],
        field1 => [ "" ],
    ]
@endverbatim

This HttpQuery module disposes of the splitting on space "magic" entirely.
All fields with an '=' are treated as named parameters. All fields without
are treated as unnamed parameters.

Query consisting entirely of a single string, without field separators:
@verbatim
Query: ?field

    unnamed() == [ "field" ]
    params()  == []
@endverbatim

No special treatment for splitting on space.
@verbatim
Query: ?foo+bar+baz
Query: ?foo%20bar%20baz

    unnamed() == [ "foo bar baz" ]
    params()  == []
@endverbatim

No behavior change when a '&' or ';' field separator is present
@verbatim
Query: ?field;field1=value1;field2=value2;field1=value3;field2;field3=;

    unnamed() == [ "field", "field2" ]
    params()  ==
    [
        field1 => [ "value1", "value3" ],
        field2 => [ "value2" ],
        field3 => [ "" ],
    ]
@endverbatim

Similarly the splitting on space works much differently
@verbatim
Query: ?foo+bar+baz;field1=value1
Query: ?foo%20bar%20baz;field1=value1

    unnamed() == [ "foo bar baz" ]
    params()  ==
    [
        field1 => [ "value1" ],
    ]
@endverbatim


To help bridge the gap somewhat of mixed unnamed and named parameters,
we provide a foundUnnamed() method as well.


SourceFiles
    HttpQuery.cpp

\*---------------------------------------------------------------------------*/
#ifndef MARK_HTTP_QUERY_H
#define MARK_HTTP_QUERY_H

#include <string>
#include <vector>
#include <map>


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace markutil
{

/*---------------------------------------------------------------------------*\
                          Class HttpQuery Declaration
\*---------------------------------------------------------------------------*/


class HttpQuery
{
public:
    //! Storage type for unnamed parameters
    typedef std::vector<std::string> string_list;

    //! Storage type for named parameters
    typedef std::map<std::string, string_list> param_map;


private:

    // Private data

        //! Unnamed query parameters
        string_list  unnamed_;

        //! Named query parameters
        param_map    named_;


        //! Null-string for empty responses
        static const std::string nullString;

        //! Null-list for empty responses
        static const string_list nullList;


public:

    // Constructors

        //! Construct null
        HttpQuery();

        //! Construct with query from URL
        HttpQuery
        (
            const std::string& str,
            size_t pos = 0,
            size_t n = std::string::npos
        );


        //! Destructor
        ~HttpQuery();


    // Member Functions

        // Access

            //! all unnamed parameters
            const string_list& unnamed() const;

            //! get unnamed parameter by number (no range-check!)
            const std::string& unnamed(size_t n) const;

            //! Get names of all named parameters
            string_list param() const;

            //! Fetch all values of a named parameter
            const string_list& param(const std::string& name) const;

            //! Fetch a single value of a named parameter
            const std::string& param(const std::string& name, size_t n) const;


        // Check

            //! true when no parameters exist (named or unnamed)
            bool empty() const;

            //! true if unnamed parameter exists
            bool foundUnnamed(const std::string& name) const;

            //! true if named parameter exists
            bool found(const std::string& name) const;


        // Edit

            //! Remove all data
            void clear();

            //! define query by parsing URL (pos points after the '?')
            HttpQuery& parseUrl
            (
                const std::string& str,
                size_t pos = 0,
                size_t n = std::string::npos
            );


        // Write

            //! get parameters in url-syntax
            std::string toString() const;

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace markutil

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif  // MARK_HTTP_QUERY_H

// ************************************************************************* //
