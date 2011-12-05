/*---------------------------------*- C++ -*---------------------------------*\
Copyright (c) 2011-2011 Mark Olesen
-------------------------------------------------------------------------------
License
    This file is part of lsf-utils

    lsf-utils is free software: you can redistribute it and/or modify it under
    the terms of the GNU Affero General Public License as published by the
    Free Software Foundation, either version 3 of the License,
    or (at your option) any later version.

    lsf-utils is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
    or FITNESS FOR A PARTICULAR PURPOSE.
    See the GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with lsf-utils. If not, see <http://www.gnu.org/licenses/>.

Description
    --

\*---------------------------------------------------------------------------*/

#ifndef XmlUtils_H
#define XmlUtils_H

#include <string>
#include <iostream>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace LsfUtil
{

/*---------------------------------------------------------------------------*\
                          Class XmlString Declaration
\*---------------------------------------------------------------------------*/

class XmlString
{
    const std::string& str_;

public:
    XmlString(const std::string& s)
    :
        str_(s)
    {}


    inline bool size() const
    {
        return str_.size();
    }

    friend std::ostream& operator<<
    (
        std::ostream& os,
        const XmlString& xmlstr
    )
    {
        const std::string& s = xmlstr.str_;

        for
        (
            std::string::const_iterator iter = s.begin();
            iter != s.end();
            ++iter
        )
        {
            unsigned char c = (unsigned char)*iter;
            switch (c)
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
                    if (c < 32 || c > 127)
                    {
                        os  << "&#" << (unsigned int)c << ';';
                    }
                    else
                    {
                        os  << c;
                    }
            }
        }

        return os;
    }

};


/*---------------------------------------------------------------------------*\
                          Class XmlTag Declaration
\*---------------------------------------------------------------------------*/

class XmlTag
{
    const std::string tag_;
    const XmlString val_;

public:

    XmlTag(const std::string& tag, const std::string& val)
    :
        tag_(tag),
        val_(val)
    {}

    friend std::ostream& operator<<
    (
        std::ostream& os,
        const XmlTag& xmltag
    )
    {
        os  << "<" << xmltag.tag_ << ">"
            << xmltag.val_
            << "</" << xmltag.tag_ << ">";

        return os;
    }
};


/*---------------------------------------------------------------------------*\
                         Class XmlTimeTag Declaration
\*---------------------------------------------------------------------------*/

class XmlTimeTag
{
    const std::string tag_;
    const time_t epoch_;

public:

    XmlTimeTag(const std::string& tag, const time_t& epoch)
    :
        tag_(tag),
        epoch_(epoch)
    {}

    //! Express epoch in iso-8601 form
    std::string iso8601() const
    {
        char buf[32];
        ::strftime
        (
            buf,
            sizeof(buf),
            "%Y-%m-%dT%H:%M:%S",
            ::localtime(&epoch_)
        );

        return buf;
    }


    friend std::ostream& operator<<
    (
        std::ostream& os,
        const XmlTimeTag& xmltag
    )
    {
        os  << "<" << xmltag.tag_ << " epoch='"
            << xmltag.epoch_ << "'>"
            << xmltag.iso8601()
            << "</" << xmltag.tag_ << ">";

        return os;
    }
};



// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace LsfUtil

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// #include "XmlUtilsI.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
