/*---------------------------------*- C++ -*---------------------------------*\
                  ___                                _
                 / __)                              (_)
               _| |__ _____ _   _  ____ _____  ____ |_| _____
              (_   __|____ | | | |/ ___) ___ |/ ___)| |(____ |
                | |  / ___ | |_| | |   | ____( (___ | |/ ___ |
                |_|  \_____|____/|_|   |_____)\____)|_|\_____|

    Copyright (C) 2011-2011 Faurecia Emissions Control Technologies
-------------------------------------------------------------------------------
License
    This file contains proprietary and confidential information and
    is subject to a non-disclosure-agreement for use outside of FECT

Description
    --

Author
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

// #include "XmlUtilsI.hxx"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
