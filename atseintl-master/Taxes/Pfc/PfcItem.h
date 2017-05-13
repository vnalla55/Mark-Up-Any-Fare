//----------------------------------------------------------------------------
//  File:           PfcItem.h
//  Description:    PfcItem header file for ATSE International Project
//  Created:        2/11/2004
//  Authors:        Dean Van Decker
//
//  Description: This Object will be used for all Passenger Facility Charges.
//          Tax Package will build TaxOut Objects which will include a vector of
//          Passenger Facility Charges.
//
//  Updates:
//          2/11/04 - DVD - updated for model changes.
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

#ifndef PFC_ITEM_H
#define PFC_ITEM_H

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

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
class Loc;
class FareUsage;

// ----------------------------------------------------------------------------
// <PRE>
//
// @class PfcItem
// Description:  Handles all Passenger Facility Charges
//
// </PRE>
// ----------------------------------------------------------------------------

class PfcItem
{
  friend class PfcItemTest;

public:
  static const uint16_t MAX_INFANT_DISCOUNT = 10;
  static const uint16_t INFANT_DISCOUNT_PERCENTAGE = 50;

  static const uint8_t GEO_TYPE_1 = '1';
  static const uint8_t GEO_TYPE_2 = '2';
  static const uint8_t GEO_TYPE_3 = '3';
  static const uint8_t GEO_TYPE_4 = '4';

  static const uint8_t YES = 'Y';
  static const uint8_t NO = 'N';

  static const uint8_t COLLECTION_TYPE_1 = '1';
  static const uint8_t COLLECTION_TYPE_2 = '2';

  PfcItem();
  virtual ~PfcItem();

  //-----------------------------------------------------------------------------
  // Copy Constructor must be in Public for Pooled Objects
  //-----------------------------------------------------------------------------
  PfcItem(const PfcItem& data);
  PfcItem& operator=(const PfcItem& data);
  //-----------------------------------------------------------------------------
  // build will move PFC items into TaxResponse
  //-----------------------------------------------------------------------------
  void build(PricingTrx& trx, TaxResponse& taxResponse);

  const bool& absorptionInd() const { return _absorptionInd; }
  const bool& allTravelDomesticUS() const { return _allTravelDomesticUS; }
  const MoneyAmount& pfcAmount() const { return _pfcAmt; }
  const CurrencyCode& pfcCurrencyCode() const { return _pfcCurrency; }
  const uint32_t& pfcDecimals() const { return _pfcNumDec; }
  const uint32_t& couponNumber() const { return _couponNumber; }
  const LocCode& pfcAirportCode() const { return _pfcAirport; }
  void setPfcAirportCode(const LocCode& code) { _pfcAirport = code; }
  const int16_t& legId() const { return _legId; }
  int16_t& legId() { return _legId; }
  CarrierCode& carrierCode() { return _carrierCode; }
  const CarrierCode& carrierCode() const { return _carrierCode; }

  void setAbsorptionInd(bool v) { _absorptionInd = v; }
  void setPfcAmount(const MoneyAmount& v) { _pfcAmt = v; }
  void setPfcCurrencyCode(const CurrencyCode& v) { _pfcCurrency = v; }
  void setPfcDecimals(uint32_t v) { _pfcNumDec = v; }
  void setCouponNumber(uint32_t v) { _couponNumber = v; }
  const TravelSeg* travelSeg() const { return _travelSeg; }

protected:
  bool isInfantExempt(PricingTrx& trx, TaxResponse& taxResponse, TravelSeg& travelSeg);

  uint8_t setPfcLocType(PricingTrx& trx, TaxResponse& taxResponse);

  void domesticFormula(PricingTrx& trx, TaxResponse& taxResponse);

  FareUsage* findFareUsage(TravelSeg& travelSeg, TaxResponse& taxResponse) const;

  void
  internationalFormula(PricingTrx& trx, TaxResponse& taxResponse, const Loc* pointOfSaleLocation);

  bool isValidPfc(PricingTrx& trx, TaxResponse& taxResponse, TravelSeg& travelSeg);

  void initializePfc(PricingTrx& trx, TaxResponse& taxResponse, TravelSeg& travelSeg);

  bool _absorptionInd; // Absorption Indicator
  bool _allTravelDomesticUS; // Domestic US no matter POS must do domestic PFC
  bool _roundTrip;
  bool _bypassTktDes;
  MoneyAmount _pfcAmt; // PFC Amount
  CurrencyCode _pfcCurrency; // PFC Currency
  uint32_t _pfcNumDec; // Number of Decimal Places
  uint32_t _couponNumber; // Coupon Number where PFC applies
  LocCode _pfcAirport; // PFC Airport
  int16_t _legId;
  CarrierCode _carrierCode;
  TravelSeg* _travelSeg { nullptr };

private:
  static log4cxx::LoggerPtr _logger;
};
} // tse namespace
#endif // PFC_ITEM_H
