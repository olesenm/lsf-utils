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

\*---------------------------------------------------------------------------*/

#include "markutil/HttpQuery.hpp"
#include "markutil/HttpCore.hpp"

// #include <iostream>
#include <cctype>

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

const std::string markutil::HttpQuery::nullString;
const std::vector<std::string> markutil::HttpQuery::nullList;


// * * * * * * * * * * * * * Static Member Functions * * * * * * * * * * * * //


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //


// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

markutil::HttpQuery& markutil::HttpQuery::parseUrl
(
    const std::string& str,
    size_t pos,
    size_t n
)
{
    if (n == std::string::npos)
    {
        n = str.size() - pos;
    }

    unnamed_.clear();
    named_.clear();

    enum parseState
    {
        stateKEY,
        stateVAL,
        stateKEY_ESC,
        stateVAL_ESC,
        stateDONE
    };

    // current parsing state
    parseState state = stateKEY;

    std::string key, val;  // key/value pairs

    int nhex  = 0;         // count of hex digits
    int dehex = 0;         // decoded hex value

    for (; n && pos < str.size(); ++pos, --n)
    {
        char ch = str[pos];

        switch (state)
        {
            case stateKEY: // parsing key
                switch (ch)
                {
                    case '%':
                        dehex = nhex  = 0;
                        state = stateKEY_ESC;
                        break;

                    case '&':  // change state
                    case ';':  // - done with this key=val
                        unnamed_.push_back(key);
                        key.clear();
                        val.clear();
                        state = stateKEY;
                        break;

                    case '+':  // space encoding
                        key.push_back(' ');
                        break;

                    case '=':  // change state - now parsing value
                        state = stateVAL;
                        break;

                    default:
                        key.push_back(ch);
                        break;
                }
                break;

            case stateVAL: // parsing key
                switch (ch)
                {
                    case '%':
                        dehex = nhex = 0;
                        state = stateVAL_ESC;
                        break;

                    case '&':  // change state
                    case ';':  // - done with this key=val
                        named_[key].push_back(val);
                        key.clear();
                        val.clear();
                        state = stateKEY;
                        break;

                    case '+':  // space encoding
                        val.push_back(' ');
                        break;

                    default:
                        val.push_back(ch);
                        break;
                }
                break;

            case stateKEY_ESC:  // %<hex><hex> within key
            case stateVAL_ESC:  // %<hex><hex> within val
                dehex =
                (
                    (dehex * 16)
                  + (isdigit(ch) ? (ch - '0') : (toupper(ch) - 'A' + 10))
                );

                if (++nhex == 2)
                {
                    switch (state)
                    {
                        case stateKEY_ESC:
                            key.push_back(static_cast<char>(dehex));
                            state = stateKEY;
                            break;

                        case stateVAL_ESC:
                            val.push_back(static_cast<char>(dehex));
                            state = stateVAL;
                            break;

                        default:
                            break;
                    }

                    dehex = nhex = 0;
                }
                break;

            case stateDONE:
                break;
        }
    }

    // finalize cleanup
    switch (state)
    {
        case stateKEY:
        case stateKEY_ESC:
            if (!key.empty())
            {
                unnamed_.push_back(key);
            }
            break;

        case stateVAL:
        case stateVAL_ESC:
            named_[key].push_back(val);
            break;

        default:
            break;
    }

    return *this;
}



// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

markutil::HttpQuery::HttpQuery()
:
    unnamed_(),
    named_()
{}


markutil::HttpQuery::HttpQuery
(
    const std::string& str,
    size_t pos,
    size_t n
)
:
    unnamed_(),
    named_()
{
    parseUrl(str, pos, n);
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

markutil::HttpQuery::~HttpQuery()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

const markutil::HttpQuery::string_list& markutil::HttpQuery::unnamed() const
{
    return unnamed_;
}


const std::string& markutil::HttpQuery::unnamed(size_t n) const
{
    if (n >= 0 && n < unnamed_.size())
    {
        return unnamed_[n];
    }
    else
    {
        return nullString;
    }
}


markutil::HttpQuery::string_list markutil::HttpQuery::param() const
{
    string_list names;
    names.reserve(named_.size());

    for
    (
        param_map::const_iterator iter = named_.begin();
        iter != named_.end();
        ++iter
    )
    {
        names.push_back(iter->first);
    }

    return names;
}


const markutil::HttpQuery::string_list& markutil::HttpQuery::param
(
    const std::string& name
) const
{
    param_map::const_iterator iter = named_.find(name);
    if (iter != named_.end())
    {
        return iter->second;
    }
    else
    {
        return nullList;
    }
}


const std::string& markutil::HttpQuery::param
(
    const std::string& name,
    size_t n
) const
{
    param_map::const_iterator iter = named_.find(name);
    if (iter != named_.end() && n < iter->second.size())
    {
        return iter->second[n];
    }
    else
    {
        return nullString;
    }
}


bool markutil::HttpQuery::empty() const
{
    return unnamed_.empty() && named_.empty();
}


bool markutil::HttpQuery::foundUnnamed(const std::string& name) const
{
    for
    (
        string_list::const_iterator iter = unnamed_.begin();
        iter != unnamed_.end();
        ++iter
    )
    {
        if (name == *iter)
        {
            return true;
        }
    }

    return false;
}


bool markutil::HttpQuery::found(const std::string& name) const
{
    return named_.find(name) != named_.end();
}


void markutil::HttpQuery::clear()
{
    unnamed_.clear();
    named_.clear();
}


std::string markutil::HttpQuery::toString() const
{
    std::string out;

    for
    (
        string_list::const_iterator iter = unnamed_.begin();
        iter != unnamed_.end();
        ++iter
    )
    {
        HttpCore::httpAppendUrl(out, *iter) += ';';
    }

    for
    (
        param_map::const_iterator outerIter = named_.begin();
        outerIter != named_.end();
        ++outerIter
    )
    {
        const std::string& name = outerIter->first;
        const string_list& list = outerIter->second;

        for
        (
            string_list::const_iterator iter = list.begin();
            iter != list.end();
            ++iter
        )
        {
            HttpCore::httpAppendUrl(out, name) += '=';
            HttpCore::httpAppendUrl(out, *iter) += ';';
        }
    }

    // truncate superfluous trailing ';'
    if (!out.empty())
    {
        out.erase(out.end() - 1);
    }

    return out;
}


// * * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * * Friend Functions  * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * * Friend Operators  * * * * * * * * * * * * * //


