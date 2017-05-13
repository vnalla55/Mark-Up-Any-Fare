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
#ifndef ZP_ABSORPTION_H
#define ZP_ABSORPTION_H

#include <log4cxx/helpers/objectptr.h>
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include <vector>

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

class ZPAbsorption
{

public:
  static const std::string TAX_CODE_US1;
  static const std::string TAX_CODE_ZP;

  ZPAbsorption();
  virtual ~ZPAbsorption();

  void applyZPAbsorption(PricingTrx& trx, TaxResponse& taxResponse);

  const uint32_t& noSeg1() const { return _noSeg1; }
  const uint32_t& noSeg2() const { return _noSeg2; }
  const PricingUnit* pricingUnit() const { return _pricingUnit; }
  const FareUsage* fareUsage() const { return _fareUsage; }

private:
  bool locateZPTax(TaxResponse& taxResponse);

  bool locateFareUsage(TaxResponse& taxResponse, TravelSeg& travelSeg);

  bool validZPItin(PricingTrx& trx, TaxResponse& taxResponse, TravelSeg& travelSeg);

  void adjustFare(PricingTrx& trx,
                  TaxResponse& taxResponse,
                  const CurrencyCode& absorbCurrency,
                  MoneyAmount absorbAmount);

  bool locMatch(
      PricingTrx& trx, LocTypeCode& loc1Type, LocCode& loc1, LocTypeCode& loc2Type, LocCode& loc2);

  bool locViaMatch(PricingTrx& trx,
                   TravelSeg& travelSeg,
                   LocTypeCode& loc1Type,
                   LocCode& loc1,
                   LocTypeCode& loc2Type,
                   LocCode& loc2,
                   LocTypeCode& betwAndViaLoc1Type,
                   LocCode& betwAndViaLoc1,
                   LocTypeCode& betwAndViaLoc2Type,
                   LocCode& betwAndViaLoc2);

  bool flightMatch(const AirSeg* airSeg, FlightNumber flight1, FlightNumber flight2);

  MoneyAmount accumulateZP(PricingTrx& trx, TaxResponse& taxResponse);

  uint32_t _noSeg1;
  uint32_t _noSeg2;
  PricingUnit* _pricingUnit;
  FareUsage* _fareUsage;

  ZPAbsorption(const ZPAbsorption& ZP);
  ZPAbsorption& operator=(const ZPAbsorption& ZP);

  static log4cxx::LoggerPtr _logger;
};

} /* end tse namespace */

#endif /* ZP_ABSORPTION_H */
