//---------------------------------------------------------------------------
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
#ifndef PFC_ABSORPTION_H
#define PFC_ABSORPTION_H

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

#include <log4cxx/helpers/objectptr.h>
namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class PricingTrx;
class TaxResponse;
class TravelSeg;
class AirSeg;
class FareUsage;
class PricingUnit;

class PfcAbsorption
{

public:
  static const std::string TAX_CODE_US1;

  static const uint8_t GEO_TYPE_1 = '1';
  static const uint8_t GEO_TYPE_2 = '2';
  static const uint8_t GEO_TYPE_3 = '3';
  static const uint8_t GEO_TYPE_4 = '4';

  PfcAbsorption();
  virtual ~PfcAbsorption();

  //-----------------------------------------------------------------------------
  // Determine if Fare should be Absorbed for PFC Item.
  //-----------------------------------------------------------------------------

  bool pfcAbsorptionApplied(PricingTrx& trx,
                            TaxResponse& taxResponse,
                            TravelSeg& travelSeg,
                            const CurrencyCode& absorbCurrency,
                            MoneyAmount absorbAmount,
                            uint8_t locType);

  const bool& firstFareUsage() const { return _firstFareUsage; }
  const bool& type1() const { return _type1; }
  const PricingUnit* pricingUnit() const { return _pricingUnit; }
  const FareUsage* fareUsage() const { return _fareUsage; }

private:
  static log4cxx::LoggerPtr _logger;

  //-----------------------------------------------------------------------------
  // Find and Set the Fare Usage pointer
  //-----------------------------------------------------------------------------

  bool locateFareUsage(TaxResponse& taxResponse, TravelSeg& travelSeg);

  //-----------------------------------------------------------------------------
  // Validate the Travel Segment against the PFC Database
  //-----------------------------------------------------------------------------

  bool
  validPfcItin(PricingTrx& trx, TaxResponse& taxResponse, TravelSeg& travelSeg, uint8_t locType);

  //-----------------------------------------------------------------------------
  // Set Fare Usage with Absorption Amount.
  //-----------------------------------------------------------------------------

  void adjustFare(PricingTrx& trx,
                  TaxResponse& taxResponse,
                  const CurrencyCode& absorbCurrency,
                  MoneyAmount absorbAmount);

  //-----------------------------------------------------------------------------
  // Multi-Airport travel Segment validation test against PFC Database.. .
  //-----------------------------------------------------------------------------

  bool cityMatch(PricingTrx& trx, LocCode& absorbCity1, LocCode& absorbCity2);

  //-----------------------------------------------------------------------------
  // Flight Number travel Segment validation test against PFC Database.. .
  //-----------------------------------------------------------------------------

  bool flightMatch(const AirSeg* airSeg, FlightNumber flight1, FlightNumber flight2);

  //-----------------------------------------------------------------------------
  // Geographic Location Setting Method.to Compare against PFC DataBase .
  //-----------------------------------------------------------------------------

  uint8_t setPfcLocType(PricingTrx& trx, TaxResponse& taxResponse);

  bool _firstFareUsage;
  bool _type1;
  bool _type234;
  PricingUnit* _pricingUnit;
  FareUsage* _fareUsage;

  PfcAbsorption(const PfcAbsorption& pfc);
  PfcAbsorption& operator=(const PfcAbsorption& pfc);
};

} /* end tse namespace */

#endif /* PFC_ABSORPTION_H */
