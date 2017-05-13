// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#pragma once

#include "Common/FareMarketUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/PricingUnit.h"
#include "FareCalc/CalcTotals.h"
#include "FareCalc/FareCalcHelper.h"
#include "Xform/TaxSplitModel.h"

#include <algorithm>
#include <boost/core/noncopyable.hpp>

namespace tse
{
class FarePath;
class FareUsage;

namespace
{
enum FareCabinChars
{ PREMIUM_FIRST_CLASS = 'P',
  FIRST_CLASS = 'F',
  PREMIUM_BUSINESS_CLASS = 'J',
  BUSINESS_CLASS = 'C',
  PREMIUM_ECONOMY_CLASS = 'S',
  ECONOMY_CLASS = 'Y' };

enum FareCabinCharsAnswer
{ PREMIUM_FIRST_CLASS_ANSWER = 'R',
  FIRST_CLASS_ANSWER = 'F',
  PREMIUM_BUSINESS_CLASS_ANSWER = 'J',
  BUSINESS_CLASS_ANSWER = 'C',
  PREMIUM_ECONOMY_CLASS_ANSWER = 'W',
  ECONOMY_CLASS_ANSWER = 'Y' };

char
getPuType(const PricingUnit& pricingUnit)
{
  switch (pricingUnit.puType())
  {
  case PricingUnit::Type::UNKNOWN:
    return 'U';
  case PricingUnit::Type::OPENJAW:
    return 'J';
  case PricingUnit::Type::ROUNDTRIP:
    return 'R';
  case PricingUnit::Type::CIRCLETRIP:
    return 'C';
  case PricingUnit::Type::ONEWAY:
    return 'O';
  case PricingUnit::Type::ROUNDTHEWORLD_SFC:
    return 'W';
  case PricingUnit::Type::CIRCLETRIP_SFC:
    return 'T';
  default:
    return 'U';
  }
}

std::string
getDirectionality(const FareUsage& fareUsage)
{
  if (fareUsage.isInbound() && !fareUsage.dirChangeFromOutbound())
    return "TO";
  else
    return "FR";
}

} // end of anonymous namespace

class FareCalcModel : private boost::noncopyable
{
  PricingTrx& _pricingTrx;
  const FareUsage& _fareUsage;
  CalcTotals& _calcTotals;
  const FarePath& _farePath;
  const PricingUnit& _pricingUnit;
  const bool _stopoverFlag;
  const uint16_t _puCount;
  const uint16_t _fcCount;

  CalcTotals::FareCompInfo* _fareCompInfo;
  tse::FareBasisCode _fareBasis;
  TravelSeg* _firstTravelSegment;
  TravelSeg* _lastTravelSegment;
  FareBreakPointInfo* _breakPoint;
  TaxSplitModel _taxSplitModel;

  std::vector<MoneyAmount>& _faeValues;
  std::vector<MoneyAmount>& _ftsValues;

  std::string getFareBasis() const;

public:
  FareCalcModel(PricingTrx& pricingTrx,
                const FareUsage& fareUsage,
                CalcTotals& calcTotals,
                const FarePath& farePath,
                const PricingUnit& pricingUnit,
                bool stopoverFlag,
                uint16_t pricingUnitCount,
                uint16_t fcCount,
                std::vector<MoneyAmount>& faeValues,
                std::vector<MoneyAmount>& ftsValues);

  CalcTotals::FareCompInfo* getFareCompInfo() const;

  bool validate() const;

  const LocCode& getFareCalcDepartureCity() const
  {
    return FareMarketUtil::getBoardMultiCity(*_fareUsage.paxTypeFare()->fareMarket(),
                                             *_firstTravelSegment);
  }

  const LocCode& getFareCalcDepartureAirport() const { return _firstTravelSegment->origAirport(); }

  CarrierCode getGoverningCarrier() const;

  CarrierCode getTrueGoverningCarrier() const;

  const LocCode& getFareCalcArrivalCity() const
  {
    return FareMarketUtil::getOffMultiCity(*_fareUsage.paxTypeFare()->fareMarket(),
                                           *_lastTravelSegment);
  }

  const LocCode& getFareCalcArrivalAirport() const { return _lastTravelSegment->destAirport(); }

  bool isDiscounted() const { return _fareUsage.paxTypeFare()->isDiscounted(); }

  const std::string& getDiscountCode() const { return _fareCompInfo->discountCode; }

  double getDiscountPercentage() const { return _fareCompInfo->discountPercentage; }

  MoneyAmount getFareAmount() const { return _breakPoint->fareAmount; }

  const tse::FareBasisCode& getFareBasisCode() const { return _fareBasis; }

  const FareUsage* getNetFU() const { return _calcTotals.getNetFareUsage(&_farePath, &_fareUsage); }

  bool isNetFUNeeded() const;

  double getNetTktFareAmount() const;

  bool isNetRemitPubFareAmountNeeded() const { return getNetRemitPubFareAmount() != 0; }

  double getNetRemitPubFareAmount() const { return _breakPoint->netPubFareAmount; }

  bool isNetRemitPubFareBasisCodeNeeded() const { return !_breakPoint->netPubFbc.empty(); }

  const std::string& getNetRemitPubFareBasisCode() const { return _breakPoint->netPubFbc; }

  const PaxTypeCode& getRequestedPassengerType() const;

  char getFareCalcCabinCode() const;

  const NationCode& getDepartureCountry() const { return _firstTravelSegment->origin()->nation(); }

  const IATAAreaCode& getDepartureIATA() const { return _firstTravelSegment->origin()->area(); }

  const StateCode& getDepartureStateOrProvince() const
  {
    return _firstTravelSegment->origin()->state();
  }

  const NationCode& getArrivalCountry() const
  {
    return _lastTravelSegment->destination()->nation();
  }

  const IATAAreaCode& getArrivalIATA() const { return _lastTravelSegment->destination()->area(); }

  const StateCode& getArrivalStateOrProvince() const
  {
    return _lastTravelSegment->destination()->state();
  }

  MoneyAmount getPublishedFareAmount() const { return _fareCompInfo->fareAmount; }

  CurrencyNoDec getPublishedFarePrecision() const { return _fareCompInfo->fareNoDec; }

  CurrencyCode getFareComponentCurrencyCode() const { return _fareCompInfo->fareCurrencyCode; }

  bool isOneWayFare() const { return _pricingUnit.puType() == PricingUnit::Type::ONEWAY; }

  bool isRoundTripFare() const;

  std::string getCommencementDate() const;

  char getTypeOfFare() const { return _fareUsage.paxTypeFare()->isNegotiated() ? 'N' : 'P'; }

  bool isStopoverSegment() const { return _stopoverFlag; }

  bool isIsMileageRouting() const;

  bool isDomesticTravel() const;

  uint16_t getMileageSurchargePctg() const
  {
    return _fareUsage.paxTypeFare()->mileageSurchargePctg();
  }

  bool isSpecialFare() const { return _pricingUnit.puFareType() == PricingUnit::SP ? true : false; }

  uint16_t getPricingUnitCount() const { return _puCount; }

  char getPricingUnitType() const { return getPuType(_pricingUnit); }

  std::string getFareCalcDirectionality() const { return getDirectionality(_fareUsage); }

  bool isDummyFare() const { return _fareUsage.paxTypeFare()->isDummyFare(); }

  std::string getGlobalDirectionInd() const;

  bool hasSideTravels() const
  {
    return !_fareUsage.paxTypeFare()->fareMarket()->sideTripTravelSeg().empty();
  }

  std::vector<TravelSeg*>::size_type getSegmentsCount() const
  {
    return _fareUsage.travelSeg().size();
  }

  bool isValidCorpId(const AccountCode& corpId) const
  {
    return corpId.length() == 5 && isalpha(corpId[0]) && isalpha(corpId[1]) && isalpha(corpId[2]) &&
           isdigit(corpId[3]) && isdigit(corpId[4]);
  }

  AccountCode getCorpId() const;

  std::string getOCFareStat() const;

  const FareType getFCAFareType() const { return _fareUsage.paxTypeFare()->fcaFareType(); }

  bool isSplitTaxAttributesNeeded() const
  {
    return TrxUtil::isSplitTaxesByFareComponentEnabled(_pricingTrx) &&
           _pricingTrx.getOptions()->isSplitTaxesByFareComponent();
  }

  MoneyAmount getTotalTaxesPerFareComponent() const;

  MoneyAmount getTotalSurchargesPerFareComponent() const;

  MoneyAmount getFareComponentPlusTaxesPlusSurcharges() const;

  MoneyAmount getFareComponentInEquivalentCurrency() const;

private:
  Money convertToEquivalent(const MoneyAmount& amount,
                            const CurrencyCode& currency,
                            CurrencyConversionRequest::ApplicationType applType) const;

  void addFaeValue();
  void addFtsValue();
};
}
