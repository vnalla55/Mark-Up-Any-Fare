//-------------------------------------------------------------------
//
//  File:        FareDisplayResponse.cpp
//  Author:      Doug Batchelor
//  Created:     March 14, 2005
//  Description: A class to provide a response object for
//               a FareDisplayTrx.
//
//  Updates:
//
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//
//---------------------------------------------------------------------------

#include "DataModel/FareDisplayResponse.h"

#include "Common/Logger.h"

using namespace tse;
using namespace std;

static Logger
logger("atseintl.DataModel.FareDisplayResponse");

RtgSeqCmp::RtgSeqCmp()
{
  globalOrder_.insert(GlobalOrder::value_type("AP", 1));
  globalOrder_.insert(GlobalOrder::value_type("AT", 2));
  globalOrder_.insert(GlobalOrder::value_type("SA", 3));
  globalOrder_.insert(GlobalOrder::value_type("EH", 4));
  globalOrder_.insert(GlobalOrder::value_type("FE", 5));
  globalOrder_.insert(GlobalOrder::value_type("PA", 6));
  globalOrder_.insert(GlobalOrder::value_type("PN", 7));
  globalOrder_.insert(GlobalOrder::value_type("RU", 8));
  globalOrder_.insert(GlobalOrder::value_type("TS", 9));
  globalOrder_.insert(GlobalOrder::value_type("WH", 10));
}

bool
RtgSeqCmp::
operator()(const RtgSeq& lhs, const RtgSeq& rhs) const
{
  GlobalOrderConstIter l(globalOrder_.find(lhs.substr(0, 2))),
      r(globalOrder_.find(rhs.substr(0, 2)));

  if (l != globalOrder_.end() && r != globalOrder_.end())
  {
    if (l->second < r->second)
      return true;
    if (l->second > r->second)
      return false;
    return lhs.substr(2) < rhs.substr(2);
  }
  return lhs < rhs;
}

FareDisplayResponse::FareDisplayResponse()
  : _badAlphaCodes(0),
    _badCategoryNumbers(0),
    _subCategoryNumbers(0),
    _fareDisplayInclCd(nullptr),
    _groupedByTravelDiscDate(0),
    _displayHeaderCabin(true)
{
}

FareDisplayResponse::~FareDisplayResponse() {}
