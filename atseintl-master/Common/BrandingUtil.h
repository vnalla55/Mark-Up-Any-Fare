//-------------------------------------------------------------------
//
//  File:        BrandingUtil.h
//  Created:     April 2013
//  Authors:
//
//  Description:
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Common/TseEnums.h"
#include "Common/TseStringTypes.h"

#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>

namespace tse
{
class PricingTrx;
class FareDisplayTrx;
class CalcTotals;

class BrandingUtil : boost::noncopyable
{
  friend class S8BrandingServiceCallerTest;
  friend class CbasBrandingServiceCallerTest;
  friend class S8BrandServiceTest;

public:
  // Method to Check if needs to be processed.
  static bool isBrandGrouping(FareDisplayTrx& trx);
  static bool isPopulateFareDataMapVec(FareDisplayTrx& trx);
  static bool isNoBrandsOffered(const PricingTrx& trx, const CalcTotals& calcTotals);
  static bool isDirectionalityToBeUsed(const PricingTrx& trx);
  static bool isCrossBrandOptionNeeded(const PricingTrx& trx);
  static BrandRetrievalMode getBrandRetrievalMode(const PricingTrx& trx);
  static bool diag892InHardSoftPassMode(const PricingTrx& trx);
  static void processSoftPassFares(PricingTrx& trx);
};
} // tse
