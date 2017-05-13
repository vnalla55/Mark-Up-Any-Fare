//---------------------------------------------------------------------------
//  Copyright Sabre 2008
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
#ifndef TAX_US2_01_H
#define TAX_US2_01_H

#include <log4cxx/helpers/objectptr.h>

#include "Common/TseCodeTypes.h"
#include "DBAccess/Loc.h"
#include "Taxes/LegacyTaxes/Tax.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/Common/TaxUtility.h"

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}
namespace tse
{
class TaxResponse;
class TaxCodeReg;
class PricingTrx;

class TaxUS2_01 : public Tax
{
  static const std::string AIATA_AREA_1;

  bool _isItineraryValidated;
  bool _validUS2;
  bool _halfTaxFlag;
  bool _soldInUS;
  bool _isInternationalPt;
  bool _intToUS;
  bool _mostDistantUSInitialized;
  bool _mostDistantUS;

public:
  class ItinAnalisis
  {
    bool wasUS;
    bool wasHAWAII;
    bool wasALASKA;

    bool willBeUS;
    bool willBeHAWAII;
    bool willBeALASKA;

  public:
    Loc journeyOriginLoc;
    Loc journeyDestinationLoc;

    bool fromOutsideBufferZone;
    bool toOutsideBufferZone;

    ItinAnalisis()
      : wasUS(false),
        wasHAWAII(false),
        wasALASKA(false),
        willBeUS(false),
        willBeHAWAII(false),
        willBeALASKA(false),
        fromOutsideBufferZone(false),
        toOutsideBufferZone(false)
    {
    }

    void resetWasLocFlags()
    {
      wasUS = false;
      wasHAWAII = false;
      wasALASKA = false;
    }

    void resetWillBeLocFlags()
    {
      willBeUS = false;
      willBeHAWAII = false;
      willBeALASKA = false;
    }

    void copyWasLocFlags(ItinAnalisis& fl)
    {
      wasUS = fl.wasUS;
      wasHAWAII = fl.wasHAWAII;
      wasALASKA = fl.wasALASKA;
      fromOutsideBufferZone = fl.fromOutsideBufferZone;
    }

    void copyWillBeLocFlags(ItinAnalisis& fl)
    {
      willBeUS = fl.willBeUS;
      willBeHAWAII = fl.willBeHAWAII;
      willBeALASKA = fl.willBeALASKA;
      toOutsideBufferZone = fl.toOutsideBufferZone;
    }

    void setToOutsideBufferZoneFlag(const Loc& loc);
    void setFromOutsideBufferZoneFlag(const Loc& loc);
    void setWasLocFlags(enum taxUtil::LocCategory locCat);
    void setWillBeLocFlags(enum taxUtil::LocCategory locCat);
    bool wasLoc(enum taxUtil::LocCategory locCat);
    bool willBeLoc(enum taxUtil::LocCategory locCat);
  };

  std::vector<ItinAnalisis> itinAnalisisVector;

  TaxUS2_01()
    : _isItineraryValidated(false),
      _validUS2(false),
      _halfTaxFlag(false),
      _soldInUS(false),
      _isInternationalPt(false),
      _intToUS(false),
      _mostDistantUSInitialized(false),
      _mostDistantUS(true)
  {
  }

  virtual ~TaxUS2_01() {}

  //---------------------------------------------------------------------------
  // Override of Tax Method
  //---------------------------------------------------------------------------

  bool validateLocRestrictions(PricingTrx& trx,
                               TaxResponse& taxResponse,
                               TaxCodeReg& taxCodeReg,
                               uint16_t& startIndex,
                               uint16_t& endIndex) override
  {
    return true;
  }

  //---------------------------------------------------------------------------
  // Override of Tax Method
  //---------------------------------------------------------------------------

  bool validateTransit(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxCodeReg& taxCodeReg,
                       uint16_t travelSegIndex) override
  {
    return true;
  }

  //---------------------------------------------------------------------------
  // Override of Tax Method
  //---------------------------------------------------------------------------

  bool validateTripTypes(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t& startIndex,
                         uint16_t& endIndex) override;

  //---------------------------------------------------------------------------
  // Override of Tax Method
  //---------------------------------------------------------------------------

  void adjustTax(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg) override;

  static bool validUS2(Itin& itin, bool soldInUS);
  static bool isInternational(const Loc& loc);
  static void calcItinSign(std::string& sign, std::string& signWild, Itin& itin, PricingTrx& trx);
  void analyzeItin(Itin& itin);
  enum TaxDiagnostic::FailCodes validateSegment(Itin& itin, uint16_t segIndex);
  bool isMostDistantUS(PricingTrx& trx, TaxResponse& taxResponse);

private:
  TaxUS2_01(const TaxUS2_01& tax);
  TaxUS2_01& operator=(const TaxUS2_01& tax);
  static void addLocChar(std::string& sign, const Loc& loc, bool soldInUS);
  static log4cxx::LoggerPtr _logger;
};

} /* end tse namespace */

#endif /* TAX_US2_01_H */
