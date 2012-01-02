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

Namespace
    lsfutil::xml

Description
    Namespace for XML-related output routines

\*---------------------------------------------------------------------------*/

#ifndef LSF_XMLUTILS_H
#define LSF_XMLUTILS_H

#include <ctime>
#include <string>
#include <vector>
#include <iostream>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace lsfutil
{

namespace xml
{
    //- half indentation
    extern const char* const indent0;

    //- Full indentation
    extern const char* const indent;

    //- Simple output of a list
    std::ostream& printList
    (
        std::ostream& os,
        const std::vector<std::string>& list,
        const char* listTag,
        const char* elemTag
    );


/*---------------------------------------------------------------------------*\
                          Class xml::String Declaration
\*---------------------------------------------------------------------------*/

//! Helper to transliterate reserved XML characters on output
class String
{
    const std::string& str_;

public:

    //- Construct from a const reference
    String(const std::string& s)
    :
        str_(s)
    {}


    //- The size of the referenced string
    inline bool size() const
    {
        return str_.size();
    }


    //- Output with reserved XML characters transliterated
    std::ostream& print(std::ostream& os) const
    {
        for
        (
            std::string::const_iterator iter = str_.begin();
            iter != str_.end();
            ++iter
        )
        {
            switch (*iter)
            {
                case '&':
                    os  << "&amp;";
                    break;
                case '<':
                    os  << "&lt;";
                    break;
                case '>':
                    os  << "&gt;";
                    break;
                case '"':
                    os  << "&quot;";
                    break;
                case '\'':
                    os  << "&apos;";
                    break;

                default:
                    if (*iter >= 32 && *iter < 127)
                    {
                        os  << *iter;
                    }
                    else
                    {
                        os  << "&#" << static_cast<unsigned int>(*iter) << ';';
                    }
                    break;
            }
        }

        return os;
    }


    //- Output with reserved XML characters transliterated
    friend std::ostream& operator<<(std::ostream& os, const String& s)
    {
        return s.print(os);
    }

};


/*---------------------------------------------------------------------------*\
                          Class xml::Tag Declaration
\*---------------------------------------------------------------------------*/

//! Helper to output a tag with string content
class Tag
{
    const std::string tag_;
    const xml::String val_;

public:

    //- Construct from a tag name and the value
    Tag(const std::string& tag, const std::string& val)
    :
        tag_(tag),
        val_(val)
    {}


    //- Output with reserved XML characters transliterated
    std::ostream& print(std::ostream& os) const
    {
        os  << '<' << tag_ << '>'
            << val_
            << "</" << tag_ << '>';

        return os;
    }


    //- Output with reserved XML characters transliterated
    friend std::ostream& operator<<(std::ostream& os, const Tag& t)
    {
        return t.print(os);
    }
};


/*---------------------------------------------------------------------------*\
                        Class xml::TimeTag Declaration
\*---------------------------------------------------------------------------*/

//! Helper to output a time tag in ISO-8601 format
class TimeTag
{
    const std::string tag_;
    const time_t epoch_;

public:

    //- Construct from a tag name and the time value
    TimeTag(const std::string& tag, const time_t& epoch)
    :
        tag_(tag),
        epoch_(epoch)
    {}

    //- Express epoch in iso-8601 form
    std::string iso8601() const;


    //- Output time in iso-8601 format with the epoch as an attribute
    std::ostream& print(std::ostream& os) const
    {
        os  << '<' << tag_ << " epoch='"
            << epoch_ << "'>"
            << this->iso8601()
            << "</" << tag_ << '>';

        return os;
    }


    //- Output time in iso-8601 format with epoch as an attribute
    friend std::ostream& operator<<(std::ostream& os, const TimeTag& tt)
    {
        return tt.print(os);
    }

};



// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace xml

} // End namespace lsfutil

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif  // LSF_XMLUTILS_H

// ************************************************************************* //
