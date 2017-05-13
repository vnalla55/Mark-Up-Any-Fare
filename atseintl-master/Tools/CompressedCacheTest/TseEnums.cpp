//----------------------------------------------------------------------------
//
//  File:           TseEnums.C
//  Created:        2/4/2004
//  Authors:        
//
//  Description:    Common enums required for ATSE shopping/pricing.
//
//  Updates:
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include <iostream>

#include "TseEnums.h"

namespace tse {

const int GlobalDirectionItemsCnt = 34;
const char* globalDirectionItems[GlobalDirectionItemsCnt] =
  { "AF", "AL", "AP", "AT", "CA", "CT", "DI", "DO", "DU", "EH",
    "EM", "EU", "FE", "IN", "ME", "NA", "NP", "PA", "PE", "PN",
    "PO", "PV", "RU", "RW", "SA", "SN", "SP", "TB", "TS", "TT",
    "US", "WH", "XX", ""
  };

const std::string globalDirectionItemsStr[GlobalDirectionItemsCnt] =
  { "AF", "AL", "AP", "AT", "CA", "CT", "DI", "DO", "DU", "EH",
    "EM", "EU", "FE", "IN", "ME", "NA", "NP", "PA", "PE", "PN",
    "PO", "PV", "RU", "RW", "SA", "SN", "SP", "TB", "TS", "TT",
    "US", "WH", "XX", ""
  };
  
bool globalDirectionToStr( std::string& dst, const GlobalDirection src )
{
  int gdIdx = src - 1;
  if( (gdIdx < 0) || (gdIdx >= GlobalDirectionItemsCnt) )
  {
    return false;
  }
  
  dst = globalDirectionItems[gdIdx];
  
  return true;
}

const std::string* globalDirectionToStr(const GlobalDirection src)
{
  int gdIdx = src - 1;
  if( (gdIdx < 0) || (gdIdx >= GlobalDirectionItemsCnt) )
  {
    return NULL;
  }

  return &globalDirectionItemsStr[gdIdx];
}

bool strToGlobalDirection( GlobalDirection& dst, const std::string& src )
{
  for( int gdIdx = 0; gdIdx < GlobalDirectionItemsCnt; gdIdx++ )
  {
    if( src == globalDirectionItems[gdIdx] )
    {
      dst = static_cast<GlobalDirection>( gdIdx + 1 );
      return true;
    }
  }
  
  return false;
}

} // end tse namespace
