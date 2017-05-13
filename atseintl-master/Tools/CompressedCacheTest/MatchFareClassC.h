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

#include <ctype.h>
#include <cstring>
#include "TseUtil.h"

namespace tse
{
inline bool matchFareClassC (const char *fareClassRuleCStr,
                             const char *fareClassFareCStr)
{
  if (0 == *fareClassRuleCStr || 0 == strcmp(fareClassRuleCStr, fareClassFareCStr))
  {
    return true;
  }
  //note that this function is called very often, and so needs
  //to be fast. The C library string functions (strchr,strstr etc)
  //are faster than the gcc std::string STL functions, so we
  //use the C library functions.

  const char* posHPtr = strchr(fareClassRuleCStr,'-');
  if(posHPtr == 0) {
    return false;
  }

  size_t posH = posHPtr - fareClassRuleCStr;

  const size_t classRuleSize = strlen(fareClassRuleCStr) - 1;
  size_t fareRuleSize = strlen(fareClassFareCStr);

  if ( fareRuleSize < classRuleSize )
    return false;

  size_t posSecond = fareRuleSize - 1;

  if ( posH != classRuleSize )
  {
    char second[9] = {};
    size_t sizeSecond = classRuleSize - posH;

    // Using sizeSecond to copy the right number of bytes.
    memcpy(second, fareClassRuleCStr+posH+1, sizeSecond);
    second[sizeSecond] = 0;
    size_t posSecondEnd = 0;

    // If the hyphen is at the beginning of the generic rule then the
    //  match cannot start at the first character of the fare class code.
    //
    if ( posH == 0 )
    { posSecondEnd = 1; }
    else
    { posSecondEnd = posH; }

    while ( true )
    {
      const char* posSecondPtr = strstr(fareClassFareCStr+posSecondEnd,second);
      if(posSecondPtr == 0) {
        return false;
      }

      posSecond = posSecondPtr - fareClassFareCStr;
      posSecondEnd = posSecond + sizeSecond - 1;

      // When matching on a numeric, if the preceeding
      //  or succeeding characters are also numeric, then
      //  it is not a match.
      // For example: -E70 matches BHE70NR but does not match
      //  BHE701.
      //
      if ( ( posSecond > 0 ) &&
               isDigit(fareClassFareCStr[posSecond - 1]) &&
               isDigit(fareClassFareCStr[posSecond]) )
            {
              ++posSecondEnd;
              continue;
            }

          if ( ( posSecondEnd < ( fareRuleSize - 1 ) ) &&
               isDigit(fareClassFareCStr[posSecondEnd]) &&
               isDigit(fareClassFareCStr[posSecondEnd + 1]) )
            {
              ++posSecondEnd;
              continue;
            }

          // If we get here, then we have a valid match so
          //  break out of the loop.
          break;
        }

      if ( posH == 0 )
        {
          return true;
        }

    }

  if(memcmp(fareClassRuleCStr,fareClassFareCStr,posH) != 0) {
  	return false;
  }

  size_t posFirstEnd = posH - 1;

  if ( isDigit(fareClassFareCStr[posFirstEnd]) &&
       isDigit(fareClassFareCStr[posFirstEnd + 1]) )
    {
      return false;
    }

  if ( posFirstEnd > posSecond ) return false;

  return true;
}
}// tse
