/*
 * Copyright (c) 1989, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Guido van Rossum.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * From FreeBSD fnmatch.c 1.11
 * $Id: fnmatch.c,v 1.3 1997/08/19 02:34:30 jdp Exp $
 */

/*
 * Function fnmatch() as specified in POSIX 1003.2-1992, section B.6.
 * Compares a filename or pathname to a pattern.
 */

#pragma once
#include <cctype>
#include <cstdio>
#include <cstring>

#define FNM_NOMATCH 1 /* Match failed. */
#define FNM_NOESCAPE 0x01 /* Disable backslash escaping. */
#define FNM_PATHNAME 0x02 /* Slash must be matched by slash. */
#define FNM_PERIOD 0x04 /* Period must be matched by period. */
#define FNM_LEADING_DIR 0x08 /* Ignore /<tail> after Imatch. */
#define FNM_CASEFOLD 0x10 /* Case insensitive search. */
#define FNM_PREFIX_DIRS 0x20 /* Directory prefixes of pattern match too. */
#define EOS '\0'

#if defined(_MSC_VER)
#pragma warning(disable : 4706)
#endif
inline auto rangematch(const char* /*pattern*/, char /*test*/, int /*flags*/) -> const char*;
inline auto fnmatch(const char* pattern, const char* string, int flags) -> int
{
    const char* stringstart;
    char c;
    char test;

    for (stringstart = string;;)
    {
        switch (c = *pattern++)
        {
        case EOS:
            if (((flags & FNM_LEADING_DIR) != 0) && *string == '/')
            {
                return (0);
            }
            return (*string == EOS ? 0 : FNM_NOMATCH);
        case '?':
            if (*string == EOS)
            {
                return (FNM_NOMATCH);
            }
            if (*string == '/' && ((flags & FNM_PATHNAME) != 0))
            {
                return (FNM_NOMATCH);
            }
            if (*string == '.' && ((flags & FNM_PERIOD) != 0) && (string == stringstart || (((flags & FNM_PATHNAME) != 0) && *(string - 1) == '/')))
            {
                return (FNM_NOMATCH);
            }
            ++string;
            break;
        case '*':
            c = *pattern;
            /* Collapse multiple stars. */
            while (c == '*')
            {
                c = *++pattern;
            }

            if (*string == '.' && ((flags & FNM_PERIOD) != 0) && (string == stringstart || (((flags & FNM_PATHNAME) != 0) && *(string - 1) == '/')))
            {
                return (FNM_NOMATCH);
            }

            /* Optimize for pattern with * at end or before /. */
            if (c == EOS)
            {
                if ((flags & FNM_PATHNAME) != 0)
                {
                    return (((flags & FNM_LEADING_DIR) != 0) || strchr(string, '/') == nullptr ? 0 : FNM_NOMATCH);
                }
                return (0);
            }
            if (c == '/' && ((flags & FNM_PATHNAME) != 0))
            {
                if ((string = strchr(string, '/')) == nullptr)
                {
                    return (FNM_NOMATCH);
                }
                break;
            }

            /* General case, use recursion. */
            while ((test = *string) != EOS)
            {
                if (fnmatch(pattern, string, flags & ~FNM_PERIOD) == 0)
                {
                    return (0);
                }
                if (test == '/' && ((flags & FNM_PATHNAME) != 0))
                {
                    break;
                }
                ++string;
            }
            return (FNM_NOMATCH);
        case '[':
            if (*string == EOS)
            {
                return (FNM_NOMATCH);
            }
            if (*string == '/' && ((flags & FNM_PATHNAME) != 0))
            {
                return (FNM_NOMATCH);
            }
            if ((pattern = rangematch(pattern, *string, flags)) == nullptr)
            {
                return (FNM_NOMATCH);
            }
            ++string;
            break;
        case '\\':
            if ((flags & FNM_NOESCAPE) == 0)
            {
                if ((c = *pattern++) == EOS)
                {
                    c = '\\';
                    --pattern;
                }
            }
            /* FALLTHROUGH */
        default:
            if ((c != *string)
                || (((flags & FNM_CASEFOLD) == 0)
                    && (tolower((unsigned char)c) != tolower((unsigned char)*string))))
            {
                if (((flags & FNM_PREFIX_DIRS) != 0)
                    && *string == EOS
                    && ((c == '/' && string != stringstart)
                        || (string == stringstart + 1 && *stringstart == '/')))
                {
                    return (0);
                }
                return (FNM_NOMATCH);
            }
            string++;
            break;
        }
    }
    /* NOTREACHED */
}

inline auto rangematch(const char* pattern, char test, int flags) -> const char*
{
    int negate;
    int ok;
    char c;
    char c2;

    /*
	 * A bracket expression starting with an unquoted circumflex
	 * character produces unspecified results (IEEE 1003.2-1992,
	 * 3.13.2).  This implementation treats it like '!', for
	 * consistency with the regular expression syntax.
	 * J.T. Conklin (conklin@ngai.kaleida.com)
	 */
    if ((negate = static_cast<int>(*pattern == '!' || *pattern == '^')) != 0)
    {
        ++pattern;
    }

    if ((flags & FNM_CASEFOLD) != 0)
    {
        test = (char)tolower((unsigned char)test);
    }

    for (ok = 0; (c = *pattern++) != ']';)
    {
        if (c == '\\' && ((flags & FNM_NOESCAPE) == 0))
        {
            c = *pattern++;
        }
        if (c == EOS)
        {
            return (nullptr);
        }

        if ((flags & FNM_CASEFOLD) != 0)
        {
            c = (char)tolower((unsigned char)c);
        }

        if (*pattern == '-'
            && (c2 = *(pattern + 1)) != EOS && c2 != ']')
        {
            pattern += 2;
            if (c2 == '\\' && ((flags & FNM_NOESCAPE) == 0))
            {
                c2 = *pattern++;
            }
            if (c2 == EOS)
            {
                return (nullptr);
            }

            if ((flags & FNM_CASEFOLD) != 0)
            {
                c2 = (char)tolower((unsigned char)c2);
            }

            if ((unsigned char)c <= (unsigned char)test && (unsigned char)test <= (unsigned char)c2)
            {
                ok = 1;
            }
        }
        else if (c == test)
        {
            ok = 1;
        }
    }
    return (ok == negate ? nullptr : pattern);
}

#if defined(_MSC_VER)
#pragma warning(default : 4706)
#endif
