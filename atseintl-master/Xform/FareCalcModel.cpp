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

#include "Common/FallbackUtil.h"
#include "Common/GlobalDirectionFinderV2Adapter.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/ServiceFeeUtil.h"
#include "Common/TravelSegUtil.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/AdjustedSellingCalcData.h"
#include "DBAccess/FareRetailerRuleInfo.h"  
#include "DBAccess/EligibilityInfo.h"
#include "DBAccess/FareByRuleApp.h"
#include "FareCalc/FareCalcConsts.h"
#include "Pricing/FarePathUtils.h"
#include "Xform/FareCalcModel.h"

#include <boost/range/numeric.hpp>
#include <memory>

namespace tse
{
FALLBACK_DECL(fae_checkFareCompInfo);

namespace
{
char
fareCabinChar(CabinType& cabin)
{
  if (cabin.isPremiumFirstClass())
    return PREMIUM_FIRST_CLASS;
  if (cabin.isFirstClass())
    return FIRST_CLASS;
  if (cabin.isPremiumBusinessClass())
    return PREMIUM_BUSINESS_CLASS;
  if (cabin.isBusinessClass())
    return BUSINESS_CLASS;
  if (cabin.isPremiumEconomyClass())
    return PREMIUM_ECONOMY_CLASS;
  return ECONOMY_CLASS;
}

char
fareCabinCharAnswer(CabinType& cabin)
{
  if (cabin.isPremiumFirstClass())
    return PREMIUM_FIRST_CLASS_ANSWER;
  if (cabin.isFirstClass())
    return FIRST_CLASS_ANSWER;
  if (cabin.isPremiumBusinessClass())
    return PREMIUM_BUSINESS_CLASS_ANSWER;
  if (cabin.isBusinessClass())
    return BUSINESS_CLASS_ANSWER;
  if (cabin.isPremiumEconomyClass())
    return PREMIUM_ECONOMY_CLASS_ANSWER;
  return ECONOMY_CLASS_ANSWER;
}
void
tuneLastValue(const MoneyAmount expectedSum,
              const std::vector<MoneyAmount>& values1,
              std::vector<MoneyAmount>& values2)
{
  if (values2.empty())
  {
    return;
  }

  const MoneyAmount diff =
      expectedSum - boost::accumulate(values1, 0.0) - boost::accumulate(values2, 0.0);

  values2.back() += diff;
}
}

FareCalcModel::FareCalcModel(PricingTrx& pricingTrx,
                             const FareUsage& fareUsage,
                             CalcTotals& calcTotals,
                             const FarePath& farePath,
                             const PricingUnit& pricingUnit,
                             bool stopoverFlag,
                             uint16_t pricingUnitCount,
                             uint16_t fcCount,
                             std::vector<MoneyAmount>& faeValues,
                             std::vector<MoneyAmount>& ftsValues)
  : _pricingTrx(pricingTrx),
    _fareUsage(fareUsage),
    _calcTotals(calcTotals),
    _farePath(farePath),
    _pricingUnit(pricingUnit),
    _stopoverFlag(stopoverFlag),
    _puCount(pricingUnitCount),
    _fcCount(fcCount),
    _fareCompInfo(nullptr),
    _firstTravelSegment(nullptr),
    _lastTravelSegment(nullptr),
    _breakPoint(nullptr),
    _taxSplitModel(pricingTrx, calcTotals),
    _faeValues(faeValues),
    _ftsValues(ftsValues)
{
  if (validate())
  {
    _fareCompInfo = getFareCompInfo();
    _fareBasis = getFareBasis();
    _firstTravelSegment = _fareUsage.travelSeg().front();
    _lastTravelSegment = _fareUsage.travelSeg().back();
    _breakPoint = &_calcTotals.getFareBreakPointInfo(&_fareUsage);
  }

  addFtsValue();
  addFaeValue();

  if (farepathutils::fareComponentCount(_farePath) == _faeValues.size())
  {
    tuneLastValue(_calcTotals.getTotalFareAmount(_pricingTrx.getOptions()->currencyOverride()),
                  _ftsValues,
                  _faeValues);
  }
}

std::string
FareCalcModel::getFareBasis() const
{
  std::string fareBasis(_fareCompInfo->fareBasisCode.c_str());
  SpanishFamilyDiscountDesignator appender = spanishFamilyDiscountDesignatorBuilder(
      _pricingTrx, _calcTotals, FareCalcConsts::MAX_FARE_BASIS_SIZE);
  appender(fareBasis);

  return fareBasis;
}

CalcTotals::FareCompInfo*
FareCalcModel::getFareCompInfo() const
{
  if (_fareCompInfo)
    return _fareCompInfo;

  auto fareCompI = _calcTotals.farePathInfo.fareCompInfo.find(&_fareUsage);
  return &fareCompI->second;
}

bool
FareCalcModel::validate() const
{
  auto fareCompI = _calcTotals.farePathInfo.fareCompInfo.find(&_fareUsage);
  return fareCompI != _calcTotals.farePathInfo.fareCompInfo.end();
}

CarrierCode
FareCalcModel::getGoverningCarrier() const
{
  if (_fareUsage.paxTypeFare()->fareMarket()->governingCarrier() != INDUSTRY_CARRIER)
  {
    return MCPCarrierUtil::swapToPseudo(&_pricingTrx,
                                        _fareUsage.paxTypeFare()->fareMarket()->governingCarrier());
  }
  else
  {
    const AirSeg* airSeg = nullptr;
    if (_fareUsage.paxTypeFare()->fareMarket()->primarySector() != nullptr)
      airSeg = dynamic_cast<const AirSeg*>(_fareUsage.paxTypeFare()->fareMarket()->primarySector());
    if (airSeg == nullptr)
      airSeg = TravelSegUtil::firstAirSeg(_fareUsage.travelSeg());
    if (airSeg != nullptr)
      return MCPCarrierUtil::swapToPseudo(&_pricingTrx, airSeg->carrier());
  }
  return "";
}

CarrierCode
FareCalcModel::getTrueGoverningCarrier() const
{
  CarrierCode governingCarrier =
      MCPCarrierUtil::swapToPseudo(&_pricingTrx, _fareUsage.paxTypeFare()->carrier());
  if (!governingCarrier.equalToConst("YY"))
    governingCarrier = MCPCarrierUtil::swapToPseudo(
        &_pricingTrx, _fareUsage.paxTypeFare()->fareMarket()->governingCarrier());
  return governingCarrier;
}

bool
FareCalcModel::isNetFUNeeded() const
{
  if (TrxUtil::isCat35TFSFEnabled(_pricingTrx) && // cat35 tfsf
      nullptr != _calcTotals.netCalcTotals &&
      nullptr != _farePath.netFarePath())
  {
    return getNetFU() != nullptr;
  }
  return false;
}

double
FareCalcModel::getNetTktFareAmount() const
{
  const FareUsage* netFU = getNetFU();
  if (netFU == nullptr)
    return 0.0;

  return netFU->calculateFareAmount();
}

char
FareCalcModel::getFareCalcCabinCode() const
{
  if (TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(_pricingTrx) &&
      TrxUtil::isRbdByCabinValueInPriceResponse(_pricingTrx))
    return fareCabinCharAnswer(_fareCompInfo->fareCabin);
  else
    return fareCabinChar(_fareCompInfo->fareCabin);
}

bool
FareCalcModel::isRoundTripFare() const
{
  return _pricingUnit.puType() == PricingUnit::Type::ROUNDTRIP ||
         _pricingUnit.puType() == PricingUnit::Type::CIRCLETRIP ||
         _pricingUnit.puType() == PricingUnit::Type::OPENJAW;
}

std::string
FareCalcModel::getCommencementDate() const
{
  const DateTime& travelDate =
      (_pricingTrx.excTrxType() != PricingTrx::AR_EXC_TRX)
          ? _pricingTrx.travelDate()
          : (static_cast<RexPricingTrx&>(_pricingTrx)).originalTravelDate();
  return travelDate.dateToSqlString();
}

bool
FareCalcModel::isIsMileageRouting() const
{
  bool mileageInd =
      !(_fareUsage.paxTypeFare()->isRouting() || _fareUsage.paxTypeFare()->isPSRApplied());

  if (_pricingTrx.excTrxType() == PricingTrx::PORT_EXC_TRX)
  {
    if (_fareUsage.paxTypeFare()->isDummyFare())
      mileageInd = false;
  }

  return mileageInd;
}

std::string
FareCalcModel::getGlobalDirectionInd() const
{
  GlobalDirection gd = GlobalDirection::XX;

  if (_pricingTrx.noPNRPricing())
  {
    gd = _fareUsage.paxTypeFare()->fareMarket()->getGlobalDirection();
  }
  else
  {
    GlobalDirectionFinderV2Adapter::getGlobalDirection(
        &_pricingTrx, _calcTotals.farePath->itin()->travelDate(), _fareUsage.travelSeg(), gd);
  }

  std::string result = "";
  globalDirectionToStr(result, gd);

  return result;
}

AccountCode
FareCalcModel::getCorpId() const
{
  AccountCode corpId = "";
  if (_fareUsage.paxTypeFare()->isFareByRule())
  {
    // for Cat 25, check record 8
    corpId = _fareUsage.paxTypeFare()->fbrApp().accountCode();
  }

  if (isValidCorpId(corpId))
  {
    return corpId;
  }
  else
  {
    // check category 1
    PaxTypeFareRuleData* ruleData = _fareUsage.paxTypeFare()->paxTypeFareRuleData(1);
    if (ruleData != nullptr)
    {
      const RuleItemInfo* ruleItemInfo = ruleData->ruleItemInfo();
      if (ruleItemInfo != nullptr)
      {
        const EligibilityInfo* eligibility = dynamic_cast<const EligibilityInfo*>(ruleItemInfo);
        corpId = eligibility->acctCode();
      }
    }
    if (isValidCorpId(corpId))
    {
      return corpId;
    }
    else
    {
      const PaxTypeFare* paxTypeFare = _fareUsage.paxTypeFare();
      if (!paxTypeFare->matchedAccCode().empty()) // cat 1 AccCode
      {
        return paxTypeFare->matchedAccCode().c_str();
      }
    }
  }
  return "";
}

std::string
FareCalcModel::getOCFareStat() const
{
  std::string result;
  ServiceFeeUtil::fareStatToString(*(_fareUsage.paxTypeFare()), result);
  return result;
}

MoneyAmount
FareCalcModel::getTotalTaxesPerFareComponent() const
{
  MoneyAmount result{0};
  if (!_taxSplitModel.contains(&_fareUsage))
    return result;

  for (const std::shared_ptr<AbstractTaxSplitData>& each :
       _taxSplitModel.getTaxBreakdown(_fareUsage))
    result += each->getTaxValue();

  return result;
}

MoneyAmount
FareCalcModel::getTotalSurchargesPerFareComponent() const
{
  return _ftsValues[_fcCount];
}

MoneyAmount
FareCalcModel::getFareComponentInEquivalentCurrency() const
{
  return _faeValues[_fcCount];
}

MoneyAmount
FareCalcModel::getFareComponentPlusTaxesPlusSurcharges() const
{
  MoneyAmount result = 0;


  result += getFareComponentInEquivalentCurrency();
  result += getTotalTaxesPerFareComponent();
  result += getTotalSurchargesPerFareComponent();

  return result;
}

void
FareCalcModel::addFtsValue()
{
  Money fts = convertToEquivalent(_fareUsage.surchargeAmt(),
                                  _farePath.calculationCurrency(),
                                  CurrencyConversionRequest::SURCHARGES);

  CurrencyConversionFacade().round(fts, _pricingTrx, _farePath.itin()->useInternationalRounding());
  _ftsValues.push_back(fts.value());
}

void
FareCalcModel::addFaeValue()
{
  if (!fallback::fae_checkFareCompInfo(&_pricingTrx))
  {
    if (!_fareCompInfo)
    {
      return;
    }
  }

  Money fae = convertToEquivalent(
      _fareCompInfo->fareAmount, _fareCompInfo->fareCurrencyCode, CurrencyConversionRequest::FARES);

  CurrencyConversionFacade().round(fae, _pricingTrx, _farePath.itin()->useInternationalRounding());
  _faeValues.push_back(fae.value());
}

Money
FareCalcModel::convertToEquivalent(const MoneyAmount& amount,
                                   const CurrencyCode& currency,
                                   CurrencyConversionRequest::ApplicationType applType) const
{
  const Money source(amount, currency);
  const CurrencyCode& currencyTo = _calcTotals.equivCurrencyCode;

  Money result(amount, currencyTo);
  if (_calcTotals.convertedBaseFareCurrencyCode != _calcTotals.equivCurrencyCode || currency == NUC)
  {
    CurrencyConversionFacade().convert(
        result, source, _pricingTrx, _farePath.itin()->useInternationalRounding(), applType);
  }
  else
  {
    Money targetNuc(amount, NUC);
    CurrencyConversionFacade().convert(targetNuc,
                                       source,
                                       _pricingTrx,
                                       result.code(),
                                       result.value(),
                                       _farePath.itin()->useInternationalRounding(),
                                       applType);
  }

  return result;
}

const PaxTypeCode&
FareCalcModel::getRequestedPassengerType() const
{
  return _calcTotals.farePath->paxType()->paxType();
}

bool
FareCalcModel::isDomesticTravel() const
{
  return _calcTotals.farePath->itin()->geoTravelType() == GeoTravelType::Domestic;
}


}
