// util.h
//
// Copyright (C) 2001, Chris Laurel <claurel@shatters.net>
//
// Miscellaneous useful functions.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

#ifndef _CELUTIL_UTIL_H_
#define _CELUTIL_UTIL_H_

#include <string>
#include <iostream>
#include <functional>

#ifndef _ /* unless somebody already took care of this */
#define _(string) string
#endif
#define dgettext(domain, text) (text)

template<typename T>
std::string concatenate(const T& begin, const T& end, const std::string& divider = "") {
    std::string result;
    if (begin == end) {
        return result;
    }

    auto itr = begin;
    bool first{ true };
    while (itr != end) {
        if (first) {
            first = false;
        } else {
            result += divider;
        }
        result += *itr;
        ++itr;
    }
    return result;
}

extern std::string toUpperStr(const std::string& s);
extern int compareIgnoringCase(const std::string& s1, const std::string& s2);
extern int compareIgnoringCase(const std::string& s1, const std::string& s2, size_t n);
extern std::string LocaleFilename(const std::string & filename);

class CompareIgnoringCasePredicate {
 public:
    bool operator()(const std::string&, const std::string&) const;
};

template <class T> struct printlineFunc 
{
    printlineFunc(std::ostream& o) : out(o) {};
    void operator() (T x) { out << x << '\n'; };
    std::ostream& out;
};

template <class T> struct deleteFunc 
{
    deleteFunc() {};
    void operator() (T x) { delete x; };
};

#endif // _CELUTIL_UTIL_H_
