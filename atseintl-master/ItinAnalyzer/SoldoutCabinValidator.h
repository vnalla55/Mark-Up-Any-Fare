//----------------------------------------------------------------------------
//  Code extracted directly from ItinAnalyzerService.cpp
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
//----------------------------------------------------------------------------

#pragma once

#include "Common/TsePrimitiveTypes.h"
#include "Common/TseDateTimeTypes.h"
#include "DataModel/PricingTrx.h"

#include <map>

namespace tse
{
class Itin;
class PricingTrx;

namespace iadetail
{

class SoldoutCabinValidator
{
public:
  typedef std::map<std::pair<DateTime, FlightNumber>, bool> StatusMap;
  typedef std::pair<DatePair, PricingTrx::AltDateInfo*> AltDatePair;

  virtual ~SoldoutCabinValidator() {};

  void validateSoldouts(PricingTrx& trx);
  void validateFlightCabin(PricingTrx& trx);

private:
  void validateAltDatePairs(PricingTrx& trx, CabinType& preferredCabinClass,
                            StatusMap& processedItinsMap);

  void validateItin(PricingTrx& trx, CabinType& preferredCabinClass,
                    AltDatePair& altDateIt,
                    StatusMap& processedItinsMap,
                    const bool originBasedRTPricing,
                    const bool reqAwardAltDatesRT);

  bool validateEmptyAltDatePairs(PricingTrx& trx, CabinType& preferredCabinClass,
                                 StatusMap& processedItinsMap);

  virtual bool isReqAwardAltDatesRT(PricingTrx& trx);

  virtual bool isCabinAvailable(PricingTrx& trx,
                                CabinType& preferredCabinClass,
                                Itin& curItin,
                                StatusMap& statusMap);

  virtual bool isOriginBasedOrReqAwardAltDatesRT(const bool originBasedRTPricing,
                                                 const bool reqAwardAltDatesRT,
                                                 const bool depInDateEq);
}; // End class SoldoutCabinValidator

} // End namespace iadetail
} // End namespace tse

