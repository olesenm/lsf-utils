/*---------------------------------*- C++ -*---------------------------------*\
Copyright 2011 Mark Olesen
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
    markutil::HttpCore

Description
    Core utilities required by various HTTP-related elements

SourceFiles
    HttpCore.cpp

\*---------------------------------------------------------------------------*/
#ifndef MARK_HTTP_CORE_H
#define MARK_HTTP_CORE_H

#include <cstring>
#include <string>
#include <map>
#include <iostream>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace markutil
{

/*---------------------------------------------------------------------------*\
                         Class HttpCore Declaration
\*---------------------------------------------------------------------------*/

class HttpCore
{
public:

    //! case-insensitive compare
    //
    //  return value for "string1 < string2"
    //  may not work for non-ASCII, but HTTP headers are ASCII anynow
    struct noCaseCmp
    {
        bool operator()(const std::string& s1, const std::string& s2) const
        {
            return strcasecmp(s1.c_str(), s2.c_str()) < 0;
        }
    };

    //! string -> string map with case-insensitive search
    typedef std::map
    <
       std::string,
       std::string,
       noCaseCmp
    > RawHeaderType;

private:

    // Private data

        //! An empty string
        static const std::string nullString;

        //! Raw Request or Response headers
        RawHeaderType headers_;


public:

    // Constructors

        //! Construct null
        HttpCore();


    //! Destructor
    ~HttpCore();


      // Member Functions

        //! Return timestamp as RFC1123-compliant string
        static std::string timestring(const time_t&);

        //! Return current time as RFC1123-compliant string
        static std::string timestring();

        //! Return mime-type corresponding to the extension name
        static const std::string& lookupMime(const std::string& ext);

        //! String append while encoding special characters
        //
        //  Reserved characters '%', '&', '+', ';', '=', '?'
        //
        // http://tools.ietf.org/html/rfc3986#section-2.2
        // but only handle some
        //
        static std::string& httpAppendUrl
        (
            std::string& url,
            char ch
        );

        //! String append while encoding special characters
        static std::string& httpAppendUrl
        (
            std::string& url,
            const std::string& str
        );

        //! Decode %<hex><hex> and '+' characters
        static std::string httpDecodeUrl
        (
            const std::string& str,
            size_t pos = 0,
            size_t n = std::string::npos
        );


        //! normalize abs-path
        //  http://tools.ietf.org/html/rfc3986#section-5.2.4
        static std::string& httpNormalizePath(std::string&);


        //! Escape '&', '<', '>' characters
        static std::ostream& xmlEscapeChars
        (
            std::ostream&,
            const std::string&
        );


        //! Convenience routine to check for existence of a directory
        static bool isDir(const std::string& name);

        //! Convenience routine to check for existence of a file
        static bool isFile(const std::string& name);


        //! Read header lines of form "Key: Value ..."
        //  Return the number read
        int readHeader(std::istream&);

        //! Clear headers
        void clear();


        // Access

            //! Raw Request or Response headers
            const RawHeaderType& rawHeaders() const;

            //! Lookup existing header entry or use default provided
            const std::string& lookupOrDefault
            (
                const std::string& key,
                const std::string& defValue
            ) const;


        // Write

            //! Write request output stream
            std::ostream& print(std::ostream&) const;


    // Member Operators

        //- Sets an HTTP header to the value
        //  This definition allows headers to be 'chained'
        HttpCore& operator()
        (
            const std::string& name,
            const std::string& value
        );

        //! Return named header or null-string if it does not exist
        const std::string& operator[](const std::string& name) const;

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace markutil

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif  // MARK_HTTP_CORE_H

// ************************************************************************* //
