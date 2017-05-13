//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.

//-------------------------------------------------------------------

#pragma once

#include <algorithm>
#include "TseUtil.h"

/*
Coding a fare family in the Fare Class field allows a series of fare class codes to be
identified via a wildcard multi-use entry that represents anything.

When a fare family is used in the Fare Class field, the system will identify all
qualifying fare families. The Fare Class field accepts one special character, a hyphen
(-). The system can accept a hyphen in any of the eight Fare Class field positions,
but cannot accept two or more hyphens. Interpret the hyphen as a break of any
length in the value between what is before and after the hyphen. The matching fare
class must contain the characters and be in the same order as the fare family to be
considered a match.

For example, fare family Y-E matches YHE, YHXE, Y8E, YLXE1A, and YE.

When the hyphen appears at the beginning, at least one character must be in its
place. For example, fare family -YE will match to AYE, B7YE, and ABCYE, but
does not match YE. The numeric value of the fare family cannot be altered by the
characters replacing the hyphen. For example, Y-8 matches YH8. It does not match
Y18 or YH89. Y8- matches Y8E, but does not match Y89, or Y80E.

A hyphen at the beginning of a fare class must be replaced with a letter. It will not
match to a blank space. For example -E70 will match to BE70 but not to E70.

A hyphen may appear in either the beginning or the middle of the fare family. There
is always an assumed hyphen at the end.

For example, fare family H-EE matches to HLEE1M, HKEE, and HAPEE3M
*/

namespace tse
{

inline bool preselectByFareClass (const char *fareClassRule,
                                  const char *fareClassFare)
{
  if (0 == *fareClassRule
      || (0 == *(fareClassRule + 1) && '-' == *fareClassRule))
  {
    return true;
  }
  const char *hyphen(fareClassRule);
  const char *fareIt(fareClassFare);
  while (*hyphen && *fareIt && *hyphen == *fareIt)// find a mismatch
  {
    ++hyphen;
    ++fareIt;
  }
  if (0 == *fareIt && 0 == *hyphen)// equal strings
  {
    return true;
  }
  if ('-' != *hyphen)
  {
    return false;
  }
  bool hyphenAtBeginning(hyphen == fareClassRule);
  if (!hyphenAtBeginning)
  {
    char beginningOfInsert(*(fareClassFare + (hyphen - fareClassRule)));
    if (isDigit(*(hyphen - 1)) && isDigit(beginningOfInsert))
    {
      return false;
    }
  }
  return true;
}

inline bool matchFareClassN (const char *fareClassRule,
                             const char *fareClassFare)
{
  char c(*fareClassRule);
  if (0 == c
      || (0 == *(fareClassRule + 1) && '-' == c))
  {
    return true;
  }
  const char *hyphen(fareClassRule);
  const char *fareIt(fareClassFare);
  while (*hyphen && *fareIt && *hyphen == *fareIt)// find a mismatch
  {
    ++hyphen;
    ++fareIt;
  }
  if (0 == *fareIt && 0 == *hyphen)// equal strings
  {
    return true;
  }
  if ('-' != *hyphen)
  {
    return false;
  }
  bool hyphenAtBeginning(hyphen == fareClassRule);
  if (!hyphenAtBeginning)
  {
    char beginningOfInsert(*(fareClassFare + (hyphen - fareClassRule)));
    if (isDigit(*(hyphen - 1)) && isDigit(beginningOfInsert))
    {
      return false;
    }
  }
  const char * const ruleAfterHyphen(hyphen + 1);
  if (0 == *ruleAfterHyphen)
  {
    return true;
  }
  const char *ruleEnd(ruleAfterHyphen);
  while (*ruleEnd)
  {
    ++ruleEnd;
  }
  const char *fareEnd(fareIt);
  while (*fareEnd)
  {
    ++fareEnd;
  }
  bool lastInRuleDigit(isDigit(*(ruleEnd - 1)));
  const char *end(0);
  const char *searchStart(fareIt + (hyphenAtBeginning ? 1 : 0));
  while ((end = std::search(searchStart, fareEnd, ruleAfterHyphen, ruleEnd)) != fareEnd)
  {
    bool matchAtInsertion(true);
    if (end > fareClassFare)
    {
      matchAtInsertion = !(isDigit(*end) && isDigit(*(end - 1)));
    }
    if (matchAtInsertion
        && !(lastInRuleDigit && isDigit(*(end + (ruleEnd - ruleAfterHyphen)))))
    {
      return true;
    }
    searchStart = end + 1;
  }
  return false;
}
}// tse
