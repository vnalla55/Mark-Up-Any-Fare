//-------------------------------------------------------------------
//
//  File:        PaxTypeFare.cpp
//  Created:     October 23, 2006
//  Design:      Doug Steeb
//  Authors:     Kavya Katam
//
//  Description: Class represents part of fare depending from
//               passanger Type
//
//  Copyright Sabre 2006
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

#include "BrandedFares/BrandInfo.h"
#include "BrandedFares/BrandProgram.h"
#include "Common/BrandingUtil.h"
#include "Common/CabinType.h"
#include "Common/CurrencyRoundingUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/FareCalcUtil.h"
#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "Common/PaxTypeUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/TSEException.h"
#include "DataModel/CxrPrecalculatedTaxes.h"
#include "DataModel/Fare.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/IndustryFare.h"
#include "DataModel/Itin.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexExchangeTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/ContractPreference.h"
#include "DBAccess/CorpId.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/NegFareRest.h"
#include "DBAccess/NegFareRestExt.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DBAccess/Routing.h"
#include "Pricing/StructuredFareRulesUtils.h"
#include "Routing/RoutingInfo.h"

#include <algorithm>
#include <memory>
#include <xmmintrin.h>

namespace tse
{
FIXEDFALLBACK_DECL(dynamicCastsFBRPTFRuleData)
FALLBACK_DECL(smpShoppingISCoreFix)
FALLBACK_DECL(fallbackHalfRTPricingForIbf)
FALLBACK_DECL(fallbackRTPricingContextFix)
FALLBACK_DECL(fallbackUseFareUsageDirection)

namespace
{

inline PaxTypeFare::BrandStatus toBrandStatus(const boost::logic::tribool& value)
{
  return value ? PaxTypeFare::BS_HARD_PASS : PaxTypeFare::BS_FAIL;
}

}

// static data
Logger
PaxTypeFare::_logger("atseintl.DataModel.PaxTypeFare");

const PaxTypeFare::RuleState PaxTypeFare::_ruleStateMap[] = { RS_CatNotSupported, // cat  0
                                                              RS_Cat01, // cat  1
                                                              RS_DefinedInFare, // cat  2
                                                              RS_DefinedInFare, // cat  3
                                                              RS_DefinedInFare, // cat  4
                                                              RS_DefinedInFare, // cat  5
                                                              RS_DefinedInFare, // cat  6
                                                              RS_DefinedInFare, // cat  7
                                                              RS_DefinedInFare, // cat  8
                                                              RS_DefinedInFare, // cat  9
                                                              RS_DefinedInFare, // cat 10
                                                              RS_DefinedInFare, // cat 11
                                                              RS_DefinedInFare, // cat 12
                                                              RS_Cat13, // cat 13
                                                              RS_DefinedInFare, // cat 14
                                                              RS_DefinedInFare, // cat 15
                                                              RS_DefinedInFare, // cat 16
                                                              RS_CatNotSupported, // cat 17
                                                              RS_Cat18, // cat 18
                                                              RS_Cat19, // cat 19
                                                              RS_Cat20, // cat 20
                                                              RS_Cat21, // cat 21
                                                              RS_Cat22, // cat 22
                                                              RS_DefinedInFare, // cat 23
                                                              RS_CatNotSupported, // cat 24
                                                              RS_Cat25, // cat 25
                                                              RS_CatNotSupported, // cat 26
                                                              RS_DefinedInFare, // cat 27
                                                              RS_CatNotSupported, // cat 28
                                                              RS_CatNotSupported, // cat 29
                                                              RS_CatNotSupported, // cat 30
                                                              RS_Cat31, // cat 31
                                                              RS_CatNotSupported, // cat 32
                                                              RS_Cat33, // cat 33
                                                              RS_CatNotSupported, // cat 34
                                                              RS_Cat35 // cat 35
};

// Constants
const char PaxTypeFare::negFareIsSellingLFare = 'L'; // NegFare is a selling fare
const char PaxTypeFare::negFareIsBundledBFare = 'B'; // NegFare is a bundled fare
const char PaxTypeFare::negFareIsNetTFare = 'T'; // NegFare is a Net fare
const char PaxTypeFare::negFareIsNetCFare = 'C'; // NegFare is a Net fare
const char PaxTypeFare::NonRefundable = 'N';
const std::string PaxTypeFare::MatchNR = "NR";
const PaxTypeCode PaxTypeFare::matchJNNPaxType = "JNN";
const PaxTypeCode PaxTypeFare::matchJNFPaxType = "JNF";
const PaxTypeCode PaxTypeFare::matchUNNPaxType = "UNN";

const uint16_t PaxTypeFare::MAX_STOPOVERS_UNLIMITED = std::numeric_limits<uint16_t>::max();
const uint16_t PaxTypeFare::PAXTYPE_FAIL = std::numeric_limits<uint16_t>::max();
const uint16_t PaxTypeFare::PAXTYPE_NO_MATCHED = (uint16_t)99999;
const uint16_t PaxTypeFare::PAXTYPE_NO_FAREGROUP = (uint16_t)88888;
const Indicator PaxTypeFare::MOD1_MAX_STAY = '1';
const Indicator PaxTypeFare::MOD2_MIN_GROUP = '2';
const Indicator PaxTypeFare::MOD3_PERCENT = '3';
const MoneyAmount PaxTypeFare::NO_PERCENT = (-1.0);

bool
PaxTypeFare::initialize(Fare* fare, PaxType* actualPaxType, FareMarket* fareMarket, PricingTrx& trx)
{
  _fare = fare;
  _actualPaxType = actualPaxType;
  _fareMarket = fareMarket;
  _isShoppingFare = false;

  _durationUsedInFVO = trx.mainDuration();

  _smpShoppingISCoreFixFallback = fallback::smpShoppingISCoreFix(&trx);

  updateCache(fare);
  initializeFlexFareValidationStatus(trx);
  initializeMaxPenaltyStructure(trx.dataHandle());

  return isValid();
}

bool
PaxTypeFare::initialize(Fare* fare, PaxType* actualPaxType, FareMarket* fareMarket)
{
  _fare = fare;
  _actualPaxType = actualPaxType;
  _fareMarket = fareMarket;
  _isShoppingFare = false;

  updateCache(fare);

  return isValid();
}

void
PaxTypeFare::initializeFlexFareValidationStatus(const PricingTrx& trx)
{
  if (UNLIKELY(trx.isFlexFare() && !_flexFaresValidationStatus))
    _flexFaresValidationStatus = std::make_shared<flexFares::ValidationStatus>();
}

const flexFares::ValidationStatusPtr
PaxTypeFare::getFlexFaresValidationStatus() const
{
  TSE_ASSERT(_flexFaresValidationStatus != nullptr);
  return _flexFaresValidationStatus;
}

flexFares::ValidationStatusPtr
PaxTypeFare::getMutableFlexFaresValidationStatus()
{
  TSE_ASSERT(_flexFaresValidationStatus != nullptr);
  return _flexFaresValidationStatus;
}

// remove this function with fallback removal
void
PaxTypeFare::initializeMaxPenaltyStructure(DataHandle& dataHandle)
{
  if (_smpShoppingISCoreFixFallback && _actualPaxType && _actualPaxType->maxPenaltyInfo())
  {
    _penaltyRecordsOld = dataHandle.create<PaxTypeFare::MaxPenaltyInfoRecords>();
  }
}

void
PaxTypeFare::setFare(Fare* fare)
{
  _fare = fare;
  updateCache(fare);
}

void
PaxTypeFare::updateCache(const Fare* fare)
{
  if (LIKELY(fare != nullptr))
  {
    if (LIKELY(_fare->tariffCrossRefInfo() != nullptr)) // for tests not to crash
    {
      _tcrTariffCat = _fare->tcrTariffCat();
      _tcrRuleTariff = _fare->tcrRuleTariff();
    }
    if (LIKELY(_fare->fareInfo() != nullptr))
    {
      _carrier = _fare->carrier();
      _ruleNumber = _fare->ruleNumber();
    }
  }
}

// PaxTypeFare rule command pricing failed flag
bool
PaxTypeFare::setCmdPrcFailedFlag(const unsigned int category, const bool ifFailed)
{
  if (category <= PTFF_Max_Numbered)
  {
    return _cpFailedStatus.set(static_cast<CmdPricingFailedState>(1u << (category - 1u)),
                               ifFailed);
  }
  else if (category == 35)
  {
    return _cpFailedStatus.set(PTFF_Cat35, ifFailed);
  }

  return false;
}

bool
PaxTypeFare::setCmdPrcCurrencySelectionFailedFlag(const bool ifFailed)
{
  return _cpFailedStatus.set(PTFF_CURR_SEL, ifFailed);
}

bool
PaxTypeFare::isFailedCmdPrc(const unsigned int category) const
{
  if (LIKELY(category <= PTFF_Max_Numbered))
  {
    return _cpFailedStatus.isSet(static_cast<CmdPricingFailedState>(1u << (category - 1u)));
  }
  else if (category == 35)
  {
    return _cpFailedStatus.isSet(PTFF_Cat35);
  }

  return false;
}

// rule status
bool
PaxTypeFare::isCategoryValid(const unsigned int category) const
{

  const RuleState categoryBit = mapRuleStateBit(category);

  if (categoryBit == RS_DefinedInFare)
  {
    return _fare->isCategoryValid(category);
  }
  else if (LIKELY(categoryBit != RS_CatNotSupported))
  {
    return !_ruleStatus.isSet(categoryBit);
  }

  return true;
}

bool
PaxTypeFare::setCategoryValid(const unsigned int category, const bool catIsValid)
{
  const RuleState categoryBit = mapRuleStateBit(category);

  if (categoryBit == RS_DefinedInFare)
    return _fare->setCategoryValid(category, catIsValid);

  else if (LIKELY(categoryBit != RS_CatNotSupported))
    return !_ruleStatus.set(categoryBit, !catIsValid);

  else
    return true;
}

bool
PaxTypeFare::areAllCategoryValid() const
{
  if (UNLIKELY(_status.isSet(PTF_SkipCat35ForRex)))
    return true;

  return !_ruleStatus.isSet(RS_AllCat) && _fare->areAllCategoryValid();
}

bool
PaxTypeFare::isCategoryProcessed(const unsigned int category) const
{
  const RuleState categoryBit = mapRuleStateBit(category);

  if (categoryBit == RS_DefinedInFare)
    return _fare->isCategoryProcessed(category);

  if (LIKELY(categoryBit != RS_CatNotSupported))
  {
    return _ruleProcessStatus.isSet(categoryBit);
  }

  return true;
}

bool
PaxTypeFare::setCategoryProcessed(const unsigned int category, const bool catIsProcessed)
{
  const RuleState categoryBit = mapRuleStateBit(category);

  if (categoryBit == RS_DefinedInFare)
    return _fare->setCategoryProcessed(category, catIsProcessed);

  else if (LIKELY(categoryBit != RS_CatNotSupported))
    return _ruleProcessStatus.set(categoryBit, catIsProcessed);

  else
    return true;
}

bool
PaxTypeFare::isSoftPassed() const
{
  if (!areAllCategoryValid())
    return false;

  return _ruleSoftPassStatus.isSet(RS_AllCat) || _fare->isSoftPassed();
}

void
PaxTypeFare::resetRuleStatus()
{
  _ruleProcessStatus.setNull();
  _ruleStatus.setNull();
  _ruleSoftPassStatus.setNull();
  _cpFailedStatus.set(PTFF_ALLRUL, false);
}

// rule softpass/hardpass status
bool
PaxTypeFare::isCategorySoftPassed(const unsigned int category) const
{
  const RuleState categoryBit = mapRuleStateBit(category);

  if (categoryBit == RS_DefinedInFare)
    return _fare->isCategorySoftPassed(category);

  else if (LIKELY(categoryBit != RS_CatNotSupported))
    return _ruleSoftPassStatus.isSet(categoryBit);

  else
    return true;
}

bool
PaxTypeFare::setCategorySoftPassed(const unsigned int category, const bool catSoftPassed)
{
  const RuleState categoryBit = mapRuleStateBit(category);

  if (categoryBit == RS_DefinedInFare)
    return _fare->setCategorySoftPassed(category, catSoftPassed);

  else if (LIKELY(categoryBit != RS_CatNotSupported))
    return _ruleSoftPassStatus.set(categoryBit, catSoftPassed);

  else
    return true;
}

bool
PaxTypeFare::isCmdPricing() const
{
  return (_fareMarket && (!_fareMarket->fareBasisCode().empty() ||
                          _fareMarket->isMultiPaxUniqueFareBasisCodes()) &&
          _fareMarket->fbcUsage() == COMMAND_PRICE_FBC && _fareMarket->fareCalcFareAmt().empty());
}

bool
PaxTypeFare::validForCmdPricing(bool fxCnException, PricingTrx* trx) const
{
  return validForCmdPricing(*_fareMarket, fxCnException, trx);
}

bool
PaxTypeFare::validForCmdPricing(const FareMarket& fareMarket, bool fxCnException, PricingTrx* trx)
    const
{
  return (fareMarket.fbcUsage() == COMMAND_PRICE_FBC &&
          validForFilterAndCmdPricing(fareMarket, fxCnException, trx));
}

bool
PaxTypeFare::isFMValidForFilterAndCmdPricing(const FareMarket& fareMarket) const
{
  if (((fareMarket.travelSeg().front()->origin()->loc() == "HKG" ||
        fareMarket.travelSeg().front()->origin()->loc() == "MFM") &&
       (fareMarket.travelSeg().back()->destination()->nation().equalToConst("CN"))) ||
      ((fareMarket.travelSeg().front()->origin()->nation().equalToConst("CN")) &&
       ((fareMarket.travelSeg().back()->destination()->loc() == "HKG" ||
         fareMarket.travelSeg().back()->destination()->loc() == "MFM"))) ||
      ((fareMarket.travelSeg().front()->origin()->nation().equalToConst("CN")) &&
       (fareMarket.travelSeg().back()->destination()->nation().equalToConst("CN"))))
  {
    return true;
  }
  return false;
}

template <typename FareToCompareType, typename Container>
bool
isFareInsideContainer(const Container& container, const FareToCompareType& fareCompared)
{
  return std::any_of(container.cbegin(),
                     container.cend(),
                     [fareCompared](const std::string& fareBasis)
                     { return fareBasis == fareCompared; });
}

bool
PaxTypeFare::validForSFRMultiPaxRequest(const FareMarket& fareMarket,
                                        bool fxCnException,
                                        PricingTrx* trx) const
{
  const FareClassCode& fareClassCode = fareClass();
  auto& container = fareMarket.getMultiPaxUniqueFareBasisCodes();

  if (!container.empty())
  {
    if (isFareInsideContainer(container, fareClassCode))
      return true;

    std::string fbc;

    createFareBasisRoot(fbc);
    if (isFareInsideContainer(container, fbc))
      return true;

    fbc = createFareBasis(trx);
    if (isFareInsideContainer(container, fbc))
      return true;

    insertBookingCode(fbc);
    if (isFareInsideContainer(container, fbc))
      return true;
  }
  return validForSpecialFareType(fareMarket, fxCnException);
}

bool
PaxTypeFare::validForFilterAndCmdPricingSingleFareBasis(const FareMarket& fareMarket,
                                                        bool fxCnException,
                                                        PricingTrx* trx) const
{
  if (UNLIKELY(!fareMarket.fareBasisCode().empty()))
  {
    if (fareMarket.fareBasisCode() == fareClass())
      return true;

    std::string fbc;
    createFareBasisRoot(fbc);
    if (fbc == fareMarket.fareBasisCode())
      return true;

    fbc = createFareBasis(trx);

    if (fbc == fareMarket.fareBasisCode())
      return true;

    insertBookingCode(fbc);
    if (fbc == fareMarket.fareBasisCode())
      return true;
  }
  return validForSpecialFareType(fareMarket, fxCnException);
}

bool
PaxTypeFare::validForSpecialFareType(const FareMarket& fareMarket, bool fxCnException) const
{
  if (UNLIKELY(fxCnException && isPublished() &&
               // @FIXME: There must be a better way - QT
               (fcaFareType() != "PGV" && fcaFareType() != "PGC")))
  {
    if (isFMValidForFilterAndCmdPricing(fareMarket))
      return true;
  }
  return false;
}

bool
PaxTypeFare::validForFilterAndCmdPricing(const FareMarket& fareMarket,
                                         bool fxCnException,
                                         PricingTrx* trx) const
{
  if (fareMarket.isMultiPaxUniqueFareBasisCodes())
  {
    return validForSFRMultiPaxRequest(fareMarket, fxCnException, trx);
  }
  else
  {
    return validForFilterAndCmdPricingSingleFareBasis(fareMarket, fxCnException, trx);
  }
}

bool
PaxTypeFare::isFilterPricing() const
{
  return (_fareMarket && (!_fareMarket->fareBasisCode().empty() ||
                          _fareMarket->isMultiPaxUniqueFareBasisCodes()) &&
          _fareMarket->fbcUsage() == FILTER_FBC && _fareMarket->fareCalcFareAmt().empty());
}

bool
PaxTypeFare::validForFilterPricing() const
{
  return (_fareMarket->fbcUsage() == FILTER_FBC &&
          validForFilterAndCmdPricing(*_fareMarket, false, nullptr));
}

bool
PaxTypeFare::hasValidFlight() const
{
  return std::any_of(_durationFlightBitmap.cbegin(),
                     _durationFlightBitmap.cend(),
                     [this](const auto& durFlight)
                     { return !isFlightBitmapInvalid(durFlight.second); });
}

bool
PaxTypeFare::isNoDataMissing() const
{
  return (_actualPaxType != nullptr && _fareMarket != nullptr && _fare != Fare::emptyFare() &&
          _fareClassAppInfo != FareClassAppInfo::emptyFareClassApp() &&
          !isFareClassAppSegMissing() && !_fare->isMissingFootnote());
}
void
PaxTypeFare::setBKSNotApplicapleIfBKSNotPASS()
{
  if (!_bookingCodeStatus.isSet(PaxTypeFare::BKS_PASS))
  {
    _bookingCodeStatus = PaxTypeFare::BKS_NOT_YET_PROCESSED;
    _mixClassStatus = PaxTypeFare::MX_NOT_APPLICABLE;
    _segmentStatus.clear();
  }
}

bool
PaxTypeFare::isValidForFareDisplay() const
{
  if (!_isShoppingFare)
  {
    return (isNoDataMissing() && areAllCategoryValid() &&
            (_bookingCodeStatus.isSet(BKS_NOT_YET_PROCESSED) ||
             (!_bookingCodeStatus.isSet(BKS_FAIL))) &&
            ((!isRoutingProcessed()) || isRoutingValid()) && _fare->isValidForFareDisplay() &&
            !failedFareGroup());
  }
  else
    return true;
}

bool
PaxTypeFare::isValid() const
{
  if (_isShoppingFare)
    return true;

  if (!isMainDataValid())
    return false;

  return isValidInternal();
}

bool
PaxTypeFare::isValidForPO() const
{
  if (LIKELY(!_isShoppingFare))
  {
    if (!isMainDataValid())
      return false;

    return isValidInternal(true);
  }
  else
  {
    return true;
  }
}

bool
PaxTypeFare::isValidForDiscount() const
{
  if (LIKELY(!_isShoppingFare))
  {
    if (!isMainDataValid())
      return false;

    return (isCategoryValid(1) &&
            (isCategoryValid(15) || cat15SoftPass()) && // As long as Cat15 softpass
            ((!isRoutingProcessed()) || isRoutingValid()));
  }
  else
  {
    return (true);
  }
}

bool
PaxTypeFare::isValidForRouting() const
{
  if (LIKELY(!_isShoppingFare))
  {
    return (_fareDisplayStatus.isNull() && // no failure codes
            ((!isRoutingProcessed()) || isRoutingValid()) &&
            _fare->isValidForRouting() && !failedFareGroup());
  }
  else
  {
    return (true);
  }
}

bool
PaxTypeFare::isValidNoBookingCode() const
{
  if (UNLIKELY(_isShoppingFare))
    return true;

  if (!(isNoDataMissing() && (areAllCategoryValid()) &&
        ((!isRoutingProcessed()) || isRoutingValid()) && _fare->isValid()))
    return false;

  if (!_fare->isIndustry())
    return true;

  IndustryFare* indf = static_cast<IndustryFare*>(_fare);
  return indf->validForPricing();
}

bool
PaxTypeFare::getPrimeBookingCode(std::vector<BookingCode>& bookingCodeVec) const
{
  bookingCodeVec.clear();
  if (LIKELY(!isFareClassAppSegMissing()))
  {
    for (int i = 0; i < 8; ++i)
    {
      if (!_fareClassAppSegInfo->_bookingCode[i].empty())
      {
        bookingCodeVec.push_back(_fareClassAppSegInfo->_bookingCode[i]);
      }
    }
  }
  return (!bookingCodeVec.empty());
}

bool
PaxTypeFare::isNormal() const
{
  if (LIKELY((fcaPricingCatType() != 0) && (fcaPricingCatType() != ' ')))
    return (fcaPricingCatType() == PRICING_CATTYPE_NORMAL);

  return (fareTypeApplication() == PRICING_CATTYPE_NORMAL);
}

bool
PaxTypeFare::isSpecial() const
{
  if (LIKELY((fcaPricingCatType() != 0) && (fcaPricingCatType() != ' ')))
    return (fcaPricingCatType() == PRICING_CATTYPE_SPECIAL);

  return (fareTypeApplication() == PRICING_CATTYPE_SPECIAL);
}

bool
PaxTypeFare::isMiscFareTagValid(void) const
{
  return (_miscFareTag != nullptr);
}

MiscFareTag*&
PaxTypeFare::miscFareTag()
{
  return _miscFareTag;
}

MiscFareTag*
PaxTypeFare::miscFareTag() const
{
  return _miscFareTag;
}

PaxTypeFareRuleData*
PaxTypeFare::paxTypeFareRuleData(uint16_t cat) const
{
    if(cat == 50)
      cat = 0;
    if(cat >= _paxTypeFareRuleDataMap->size())
      return nullptr;
    PaxTypeFareAllRuleData* allData =  (*_paxTypeFareRuleDataMap)[cat].load(std::memory_order_acquire);
    return allData == nullptr?  nullptr : allData->fareRuleData;
}

PaxTypeFare*
PaxTypeFare::baseFare(int cat) const
{
  PaxTypeFareRuleData* rd = paxTypeFareRuleData(cat);
  if (UNLIKELY(rd == nullptr))
    throw(TSEException(TSEException::NO_ERROR, "NO RULE DATA FOR CATEGORY"));
  PaxTypeFare* sptf = rd->baseFare();
  if (UNLIKELY(sptf == nullptr))
    throw(TSEException(TSEException::NO_ERROR, "NO BASE FARE FOR CATEGORY"));

  return sptf;
}

PaxTypeFare*
PaxTypeFare::getBaseFare(int cat) const
{
  PaxTypeFareRuleData* rd = paxTypeFareRuleData(cat);
  if (rd)
    return rd->baseFare();

  return nullptr;
}

PaxTypeFareRuleData*
PaxTypeFare::paxTypeFareGfrData(uint16_t cat) const
{
  if(cat == 50)
    cat = 0;
  if(cat >= _paxTypeFareRuleDataMap->size())
    return nullptr;
  PaxTypeFareAllRuleData* allData =  (*_paxTypeFareRuleDataMap)[cat].load(std::memory_order_acquire);
  return allData == nullptr ?  nullptr : allData->gfrRuleData;
}

bool
PaxTypeFare::isFareRuleDataApplicableForSimilarItin(uint16_t cat) const
{
  if(cat == 50)
    cat = 0;
  if(cat >= _paxTypeFareRuleDataMap->size())
    return false;
  PaxTypeFareAllRuleData* allData =  (*_paxTypeFareRuleDataMap)[cat].load(std::memory_order_acquire);
  if(allData == nullptr)
    return false;
  return allData->gfrRuleData || allData->fareRuleData;
}

// true if this fare has cat35 rule data
// OR if this was the base fare for cat35
// should be the same as
//       return ( isNegotiated() || somePTF.ptfRuleData(35).baseFare() == this )
//
bool
PaxTypeFare::hasCat35Filed() const
{
  // when not FD trx, need to behave like isNeg()
  return (isNegotiated() || _fareDisplayCat35Type != ' ');
}

// get the 'most base' fare (e.g. published)
// if-sequence based on processing sequence
const PaxTypeFare*
PaxTypeFare::fareWithoutBase() const
{
  // only calculated FBR has base
  if (isFareByRule() && !isSpecifiedFare())
    return baseFare(25);
  else if (isDiscounted())
    return baseFare(19);
  else if (isNegotiated())
    return baseFare(35);
  else
    return this;
}
namespace
{
  PaxTypeFare::PaxTypeFareAllRuleData _paxTypeFareRuleDataBusyFlag;
  struct PaxTypeFareAllRuleDataManager
  {
    PaxTypeFareAllRuleDataManager(PaxTypeFare::PaxTypeFareRuleDataByCatNo::value_type& atom) : a(atom)
    {
      auto guard = &_paxTypeFareRuleDataBusyFlag;
      while((value = a.exchange(guard, std::memory_order_acquire)) == guard)
        _mm_pause();
    }
    ~PaxTypeFareAllRuleDataManager(){ a.store(value, std::memory_order_release); }
    PaxTypeFare::PaxTypeFareRuleDataByCatNo::value_type& a;
    PaxTypeFare::PaxTypeFareAllRuleData* value;
  };

  inline PaxTypeFareRuleData* getOrInitialize(PaxTypeFareRuleData*& rd, DataHandle& dh)
  {
    if(rd == nullptr)
      dh.get(rd);
    return rd;
  }
}

void
PaxTypeFare::setRuleData(uint16_t cat, DataHandle& dh, PaxTypeFareRuleData* rd, bool isFareRule)
{
  if(cat == 50)
    cat = 0;
  if(cat >= _paxTypeFareRuleDataMap->size())
    return;

  PaxTypeFareAllRuleDataManager allRuleData((*_paxTypeFareRuleDataMap)[cat]);

  if(allRuleData.value == nullptr)
  {
    dh.get(allRuleData.value);
  }

  if (LIKELY(isFareRule))
  {
    allRuleData.value->chkedRuleData = true;
    allRuleData.value->fareRuleData = rd;
  }
  else
  {
    allRuleData.value->chkedGfrData = true;
    allRuleData.value->gfrRuleData = rd;
  }
}

PaxTypeFareRuleData*
PaxTypeFare::setCatRuleInfo(const CategoryRuleInfo& cri,
                            DataHandle& dh,
                            bool isLocationSwapped,
                            bool isFareRule)
{
  const uint16_t cat = (cri.categoryNumber() != 50) ? cri.categoryNumber() : 0;
  if(cat >= _paxTypeFareRuleDataMap->size())
    return nullptr;

  PaxTypeFareRuleData* ruleData = nullptr;

  PaxTypeFareAllRuleDataManager allRuleData((*_paxTypeFareRuleDataMap)[cat]);

  if(allRuleData.value == nullptr)
  {
    dh.get(allRuleData.value);
  }

  if (LIKELY(isFareRule))
  {
    allRuleData.value->chkedRuleData = true;
    ruleData = getOrInitialize(allRuleData.value->fareRuleData, dh);
  }
  else
  {
    // GeneralFareRule
    allRuleData.value->chkedGfrData = true;
    ruleData = getOrInitialize(allRuleData.value->gfrRuleData, dh);
  }
  ruleData->categoryRuleInfo() = &cri;
  ruleData->isLocationSwapped() = isLocationSwapped;

  return ruleData;
}

PaxTypeFareRuleData*
PaxTypeFare::setCatRuleInfo(const CategoryRuleInfo* cri,
                            uint16_t categoryNumber,
                            DataHandle& dh,
                            bool isLocationSwapped,
                            bool isFareRule)
{
  if(categoryNumber == 50)
    categoryNumber = 0;
  if(categoryNumber >= _paxTypeFareRuleDataMap->size())
    return nullptr;

  PaxTypeFareRuleData* ruleData = nullptr;
  PaxTypeFareAllRuleDataManager allRuleData((*_paxTypeFareRuleDataMap)[categoryNumber]);

  if (allRuleData.value == nullptr)
  {
    dh.get(allRuleData.value);
  }

  if (isFareRule)
  {
    allRuleData.value->chkedRuleData = true;
    if (cri == nullptr)
    {
      allRuleData.value->fareRuleData = nullptr;
      return nullptr;
    }
    ruleData = getOrInitialize(allRuleData.value->fareRuleData, dh);
  }
  else
  {
    // GeneralFareRule
    allRuleData.value->chkedGfrData = true;
    if (cri == nullptr)
    {
      allRuleData.value->gfrRuleData = nullptr;
      return nullptr;
    }
    ruleData = getOrInitialize(allRuleData.value->gfrRuleData, dh);
  }

  ruleData->categoryRuleInfo() = cri;
  ruleData->isLocationSwapped() = isLocationSwapped;

  return ruleData;
}

bool
PaxTypeFare::alreadyChkedFareRule(uint16_t categoryNumber,
                                  DataHandle& dataHandle,
                                  PaxTypeFareRuleData*& paxTypeFareRuleData) const
{
  if(categoryNumber == 50)
    categoryNumber = 0;
  if(categoryNumber >= _paxTypeFareRuleDataMap->size())
    return false;

  PaxTypeFareAllRuleData* allRuleData = (*_paxTypeFareRuleDataMap)[categoryNumber].load(std::memory_order_acquire);
  if(allRuleData == nullptr)
    return false;

  if (UNLIKELY(!allRuleData->chkedRuleData))
    return false;

  paxTypeFareRuleData = allRuleData->fareRuleData;
  return true;
}

bool
PaxTypeFare::alreadyChkedGfrRule(uint16_t categoryNumber,
                                 DataHandle& dataHandle,
                                 PaxTypeFareRuleData*& paxTypeFareRuleData) const
{
  if(categoryNumber == 50)
    categoryNumber = 0;
  if(categoryNumber >= _paxTypeFareRuleDataMap->size())
    return false;

  PaxTypeFareAllRuleData* allRuleData = (*_paxTypeFareRuleDataMap)[categoryNumber].load(std::memory_order_acquire);
  if(allRuleData == nullptr)
    return false;

  if (!allRuleData->chkedGfrData)
    return false;

  paxTypeFareRuleData = allRuleData->gfrRuleData;
  return true;
}

const DiscountInfo&
PaxTypeFare::discountInfo() const
{
  PaxTypeFareRuleData* ptfRuleData = paxTypeFareRuleData(19);
  if (UNLIKELY(ptfRuleData == nullptr))
    throw(TSEException(TSEException::NO_ERROR, "NO RULE_DATA FOR CAT19-22"));

  const DiscountInfo* di = dynamic_cast<const DiscountInfo*>(ptfRuleData->ruleItemInfo());
  if (UNLIKELY(di == nullptr))
    throw(TSEException(TSEException::NO_ERROR, "NO DISCOUNT INFO"));
  return *di;
}

const NegFareRest&
PaxTypeFare::negotiatedInfo() const
{
  PaxTypeFareRuleData* ptfRuleData = paxTypeFareRuleData(35);
  if (UNLIKELY(ptfRuleData == nullptr))
    throw(TSEException(TSEException::NO_ERROR, "NO RULE_DATA FOR CAT35"));

  const NegFareRest* x = dynamic_cast<const NegFareRest*>(ptfRuleData->ruleItemInfo());
  if (UNLIKELY(x == nullptr))
    throw(TSEException(TSEException::NO_ERROR, "NO NEG-FARE-REST"));
  return *x;
}

const FareByRuleItemInfo&
PaxTypeFare::fareByRuleInfo() const
{
  PaxTypeFareRuleData* ptfRuleData = paxTypeFareRuleData(RuleConst::FARE_BY_RULE);
  if (UNLIKELY(ptfRuleData == nullptr))
    throw(TSEException(TSEException::NO_ERROR, "NO RULE-DATA FO CAT25"));

  const FareByRuleItemInfo* x =
      dynamic_cast<const FareByRuleItemInfo*>(ptfRuleData->ruleItemInfo());
  if (UNLIKELY(x == nullptr))
    throw(TSEException(TSEException::NO_ERROR, "NO FARE-BY-BY-RULE-ITEM-INFO"));
  return *x;
}

FBRPaxTypeFareRuleData*
PaxTypeFare::getFbrRuleData() const
{
  FBRPaxTypeFareRuleData* ret = nullptr;

  if (fallback::fixed::dynamicCastsFBRPTFRuleData())
  {
    ret = dynamic_cast<FBRPaxTypeFareRuleData*>(paxTypeFareRuleData(RuleConst::FARE_BY_RULE));
  }
  else
  {
    if (auto fbrData = paxTypeFareRuleData(RuleConst::FARE_BY_RULE))
      ret = fbrData->toFBRPaxTypeFareRuleData();
  }
  if (UNLIKELY(ret == nullptr))
    throw(TSEException(TSEException::NO_ERROR, "NO RULE DATA FOR FBR"));
  return ret;
}

FBRPaxTypeFareRuleData*
PaxTypeFare::getFbrRuleData(uint16_t category) const
{
  if (fallback::fixed::dynamicCastsFBRPTFRuleData())
    return dynamic_cast<FBRPaxTypeFareRuleData*>(paxTypeFareRuleData(category));

  if (auto ptfRule = paxTypeFareRuleData(category))
    return ptfRule->toFBRPaxTypeFareRuleData();
  return nullptr;
}

NegPaxTypeFareRuleData*
PaxTypeFare::getNegRuleData() const
{
  if (fallback::fixed::dynamicCastsFBRPTFRuleData())
    return dynamic_cast<NegPaxTypeFareRuleData*>(paxTypeFareRuleData(RuleConst::NEGOTIATED_RULE));

  if (auto ptfRule = paxTypeFareRuleData(RuleConst::NEGOTIATED_RULE))
    return ptfRule->toNegPaxTypeFareRuleData();
  return nullptr;
}

bool
PaxTypeFare::isSpecifiedFare() const
{
  const FareByRuleItemInfo* fbrItemInfo =
      dynamic_cast<const FareByRuleItemInfo*>(getFbrRuleData()->ruleItemInfo());
  if (UNLIKELY(fbrItemInfo == nullptr))
    throw(TSEException(TSEException::NO_ERROR, "NO ITEM-INFO FOR FBR"));

  return (fbrItemInfo->fareInd() == FareByRuleItemInfo::SPECIFIED ||
          fbrItemInfo->fareInd() == FareByRuleItemInfo::SPECIFIED_K ||
          fbrItemInfo->fareInd() == FareByRuleItemInfo::SPECIFIED_E ||
          fbrItemInfo->fareInd() == FareByRuleItemInfo::SPECIFIED_F);
}

bool
PaxTypeFare::isSpecifiedCurFare() const
{
  if (!isFareByRule())
    return false;

  const FareByRuleItemInfo* fbrItemInfo =
      dynamic_cast<const FareByRuleItemInfo*>(getFbrRuleData()->ruleItemInfo());
  if (UNLIKELY(fbrItemInfo == nullptr))
    return false;

  return (fbrItemInfo->fareInd() == FareByRuleItemInfo::SPECIFIED_K ||
          fbrItemInfo->fareInd() == FareByRuleItemInfo::SPECIFIED_E ||
          fbrItemInfo->fareInd() == FareByRuleItemInfo::SPECIFIED_F);
}

const FareByRuleApp&
PaxTypeFare::fbrApp() const
{
  return *(getFbrRuleData()->fbrApp());
}

// ----------------------------------------------------------------------------
// @function PaxTypeFare::adoptBaseFareRuleStat
//
// Description: discounted fare share have same rule restrictions as its base
//          fare, will see if it has been processed and adopt the result
//          updateBaseFareRuleStat will set the base fare rule validation
//          result so that
//
// @param category - uint16_t the main category that result are to be shared
//
// return:
//         true     - found and adopt the base fare rule validation result
//         false    - could not adopt the base fare rule validation result
//
// ----------------------------------------------------------------------------
bool
PaxTypeFare::adoptBaseFareRuleStat(DataHandle& dataHandle, const uint16_t category)
{
  if (UNLIKELY(!this->isDiscounted()))
  {
    return false;
  }

  try
  {
    PaxTypeFare* baseFare = this->baseFare(19);
    // exception would be thrown if baseFare == 0

    if (!baseFare || !baseFare->isCategoryProcessed(category))
    {
      return false;
    }

    setCategoryValid(category, baseFare->isCategoryValid(category));

    setCategorySoftPassed(category, baseFare->isCategorySoftPassed(category));

    setCategoryProcessed(category);

    PaxTypeFareRuleData* baseFareRuleData = baseFare->paxTypeFareRuleData(category);
    if (baseFareRuleData)
      setRuleData(category, dataHandle, baseFareRuleData);

    if ((category == 5) && !this->reProcessCat05NoMatch())
    {
      this->reProcessCat05NoMatch() = baseFare->reProcessCat05NoMatch();
    }
  }
  catch (...) { return false; }

  return true;
}

// ----------------------------------------------------------------------------
// @function PaxTypeFare::updateBaseFareRuleStat
//
// Description: discounted fare share have same rule restrictions as its base
//          fare;
//          updateBaseFareRuleStat will set the base fare rule validation
//          result so that base fare and other discounted fare can share the
//          result
//
// @param category - uint16_t the main category that result are to be shared
//
// return:
//         true     - updated the base fare rule validation result
//         false    - could not update the base fare rule validation result
//
// ----------------------------------------------------------------------------
bool
PaxTypeFare::updateBaseFareRuleStat(DataHandle& dataHandle, const uint16_t category)
{
  if (UNLIKELY(!this->isDiscounted()))
  {
    return false;
  }

  try
  {
    PaxTypeFare* baseFare = this->baseFare(19);
    // exception would be thrown if baseFare == 0
    if (UNLIKELY(!baseFare))
      return false;

    baseFare->setCategoryValid(category, isCategoryValid(category));

    baseFare->setCategorySoftPassed(category, isCategorySoftPassed(category));

    baseFare->setCategoryProcessed(category);

    PaxTypeFareRuleData* ruleData = paxTypeFareRuleData(category);
    if (ruleData)
      baseFare->setRuleData(category, dataHandle, ruleData);

    if ((category == 5) && !baseFare->reProcessCat05NoMatch())
    {
      baseFare->reProcessCat05NoMatch() = this->reProcessCat05NoMatch();
    }
    if ((category == 14) && _NVAData)
    {
      baseFare->_NVAData = _NVAData;
    }
  }
  catch (...) { return false; }

  return true;
}

// ----------------------------------------------------------------------------
// @function string PaxTypeFare::copyBasis
//
// Description:  Given a fare Class and ticketing Code (fc & tc),
//               copy to resulting fareBasis fields (basis & tktCode).
//               Mainly for preventing overwrites.
// ----------------------------------------------------------------------------
void
PaxTypeFare::copyBasis(std::string& basis,
                       std::string& tktCode,
                       const FareClassCode& fc,
                       const TktDesignator& tc) const
{
  if (!tc.empty() && tktCode.empty())
  {
    // regardless of append/replace, cause lower precence tktCode's to be ignored
    tktCode = tc.c_str();

    if (tc[0] != '-' && basis.empty())
    {
      basis = tc.c_str();
      return;
    }
  }
  if (basis.empty())
  {
    if (!fc.empty())
      basis = fc.c_str();
  }
}

// ----------------------------------------------------------------------------
// @function void PaxTypeFare::createModifier
//
// Description: create formatted modifier string in given string
//              (works for both tktCodeModifier & tktDesignatorModifier)
//              if previously created, skip
//              TODO change TseTypes.h and Infos for consisent Modifiers
// ----------------------------------------------------------------------------
void
PaxTypeFare::createModifier(std::string& str,
                            const TktDesignatorModifier& mod,
                            const MoneyAmount& percent,
                            const bool asDiscount) const
{
  if (!mod.empty())
    createModifier(str, mod[0], percent, asDiscount);
}

void
PaxTypeFare::createModifier(std::string& str,
                            const Indicator& mod,
                            const MoneyAmount& percent,
                            const bool asDiscount) const
{
  if (!str.empty())
    return;

  MoneyAmount dspNum = (asDiscount ? (100.0 - percent) : (percent));
  std::ostringstream fmtPercent;

  switch (mod)
  {
  case ' ': // for speed, put usual case first
    break;

  case MOD3_PERCENT:
    if (dspNum < 100.0 && dspNum > 0)
    {
      fmtPercent << std::setw(2) << std::setfill('0') << std::fixed << std::setprecision(0)
                 << dspNum;
    }
    else if (dspNum == 0)
    {
      fmtPercent << "0";
    }
    else if (dspNum == 100)
    {
      fmtPercent << "00";
    }

    str = fmtPercent.str();
    break;

  case MOD2_MIN_GROUP:
    // not yet
    break;
  case MOD1_MAX_STAY:
    // not yet
    break;
  default:
    break;
  }
}

// ----------------------------------------------------------------------------
// @function string createFareBasisCodeFD
//
// Description:  create Fare Basis Code from Fare Class, ticketing code,
//               ticket designator and non-refund indicator
//
// ----------------------------------------------------------------------------
std::string
PaxTypeFare::createFareBasisCodeFD(FareDisplayTrx& trx) const
{
  std::string fareBasis = createFareBasis(trx);
  if (!trx.isERD())
  {
    insertBookingCode(fareBasis);
  }

  return fareBasis;
}

void
PaxTypeFare::insertBookingCode(std::string& fareBasisCode) const
{
  if (getChangeFareBasisBkgCode().empty())
    return;

  if (fareBasisCode.size() < 2)
    return;

  if (fareBasisCode.at(1) == '/')
    return;

  std::string& fbc = fareBasisCode;
  if (fbc.compare(1, 2, "CH") == 0 || fbc.compare(1, 2, "IN") == 0 || fbc.compare(1, 3, CNE) == 0 ||
      fbc.compare(1, 3, INE) == 0)
    return;

  const IndustryFare* indFare = dynamic_cast<const IndustryFare*>(fare());
  if (indFare != nullptr && indFare->changeFareClass())
  {
    fareBasisCode.replace(0, 1, getChangeFareBasisBkgCode());
  }
}

// ----------------------------------------------------------------------------
// @function void PaxTypeFare::ChInDefault
//
// Description:  based on childInfantCode in FareCalcConfig, make default
//               modifiers.  When no default, empty string returned.
//               Caller has logic to determine when default applies.
//
// ----------------------------------------------------------------------------
void
PaxTypeFare::ChInDefault(PricingTrx& trx,
                         Indicator fcChildInfantInd,
                         std::string& tktCodeDefault,
                         std::string& tktDesDefault,
                         bool& appendDiscount) const
{
  if (UNLIKELY(!actualPaxType()))
    return;

  std::string* target = nullptr;
  switch (fcChildInfantInd)
  {
  case '3':
    appendDiscount = true; // intentional fall-thru
  case '2':
    target = &tktDesDefault;
    break;
  case '1':
  default:
    target = &tktCodeDefault;
    break;
  }

  if (PaxTypeUtil::isChild(trx, actualPaxType()->paxType(), vendor()) ||
      actualPaxType()->paxType() == "INS" || actualPaxType()->paxType() == "UNN")
  {
    *target = "CH";
    return;
  }

  if (PaxTypeUtil::isInfant(trx, actualPaxType()->paxType(), vendor()))
    *target = "IN";

  return;
}

void
PaxTypeFare::appendTktDesig(std::string& fareBasisCode,
                            std::string& designator,
                            char tktDesLength,
                            std::string::size_type maxLenAll,
                            std::string::size_type maxLenFB) const
{
  // see if need to trunc fare class *before* appending designator
  // TODO check req on what these FareCalcConst mean for tktDesLength
  if (UNLIKELY(tktDesLength != ' ' && tktDesLength != '4'))
  {
    if (fareBasisCode.length() > maxLenFB)
      fareBasisCode.erase(maxLenFB);
  }
  if (!designator.empty())
  {
    fareBasisCode += "/";
    fareBasisCode += designator;
  }
  if (fareBasisCode.length() > maxLenAll)
    fareBasisCode.erase(maxLenAll);
}

void
PaxTypeFare::createFareBasisRoot(std::string& basis) const
{
  basis = createFareBasis(nullptr);
  std::string::size_type posBasis = basis.find("/");
  if (posBasis != std::string::npos)
  {
    basis.erase(posBasis);
  }
}

void
PaxTypeFare::createTktDesignator(std::string& des) const
{
  des = createFareBasis(nullptr);
  std::string::size_type posBasis = des.find("/");

  if ((posBasis != std::string::npos) && (posBasis < des.size() - 1))
    des.erase(0, posBasis + 1); // "/" exists
  else
    des.clear();
}

bool
PaxTypeFare::isFareBasisEqual(const std::string& fareBasis)
{
  const std::string createdFareBasis = createFareBasis(nullptr);
  return createdFareBasis == fareBasis;
}

// ----------------------------------------------------------------------------
// @function string PaxTypeFare::createFareBasis
//
// Description:  create Fare Basis Code from Fare Class, ticketing code,
//               ticket designator and non-refund indicator
//
// @param fareBasisCode - reference to a CalcFareBasisCode to be loaded by this method
// ----------------------------------------------------------------------------
std::string
PaxTypeFare::createFareBasis(PricingTrx& trx, bool ignCat19ChInf /* = false */) const
{
  return createFareBasis(&trx, ignCat19ChInf);
}

std::string
PaxTypeFare::createFareBasis(PricingTrx& trx, const std::string& ufbCode) const
{
  bool ignCat19ChInf = false;
  FareBasisCodes fbCodes;
  fbCodes.ufbCode = ufbCode;
  return setFareBasis(&trx, fbCodes, ignCat19ChInf);
}

std::string
PaxTypeFare::createFareBasis(PricingTrx* trx, bool ignCat19ChInf /* = false */) const
{
  FareBasisCodes fbCodes;
  return setFareBasis(trx, fbCodes, ignCat19ChInf);
}

std::string
PaxTypeFare::setFareBasis(PricingTrx* trx, FareBasisCodes& fbCodes, bool ignCat19ChInf) const
{
  try
  {
    std::string modString; // string based on tktDesignatorModifier
    MoneyAmount percent = NO_PERCENT;
    const FareCalcConfig* fareCalcConfig = nullptr;
    char trimTktDes = ' '; // don't configure fare basis/tktdes lengths

    if (trx)
    {
      fareCalcConfig = FareCalcUtil::getFareCalcConfig(*trx);
      if (LIKELY(fareCalcConfig))
      {
        trimTktDes = fareCalcConfig->fareBasisTktDesLng();
      }
    }

    bool needTktCodeDefault = false;

    if (isNegotiated())
      setCat35Designator(trx, fbCodes.des);

    if (isDiscounted())
      setDiscountedDesignator(
          trx, percent, fareCalcConfig, ignCat19ChInf, needTktCodeDefault, modString, fbCodes);

    if (isFareByRule())
      setFareByRuleDesignator(percent, modString, fbCodes);

    const PaxTypeFare* base = fareWithoutBase();

    if (UNLIKELY(trx != nullptr && !fbCodes.ufbCode.empty()))
    {
      fbCodes.basis.assign(fbCodes.ufbCode);
    }
    else
    {
      copyBasis(fbCodes.basis, fbCodes.tktCode, base->fareClass(), base->fcasTktCode());
    }
    createModifier(fbCodes.modCode, base->fcasTktCodeModifier(), percent, false);

    if (fbCodes.des.empty())
    {
      fbCodes.des = base->fcasTktDesignator().c_str();
      createModifier(modString, base->fcasTktDesignatorModifier(), percent, true);
      fbCodes.des += modString;
    }

    ///////////////////////////////////////
    //  put the parts together
    ///////////////////////////////////////

    if (!fbCodes.tktCode.empty() && fbCodes.tktCode[0] == '-')
      fbCodes.basis.append(fbCodes.tktCode, 1, fbCodes.tktCode.size() - 1);

    fbCodes.basis += fbCodes.modCode;

    if (needTktCodeDefault)
      fbCodes.basis += fbCodes.tktCodeDefault;

    fbCodes.des += modString;
    fbCodes.basis += _spanishResidentFareBasisSuffix;
    appendTktDesig(fbCodes.basis, fbCodes.des, trimTktDes);
  }
  catch (...)
  {
    fbCodes.basis = "**";
    fbCodes.basis += fareClass().c_str();
  }

  return fbCodes.basis;
}

bool
PaxTypeFare::isChild() const
{
  if (_actualPaxType->paxTypeInfo() == nullptr)
  {
    return false;
  }

  if ((_actualPaxType->paxTypeInfo()->childInd() == 'Y' &&
       _actualPaxType->paxTypeInfo()->infantInd() != 'Y') ||
      _actualPaxType->paxTypeInfo()->paxType() == matchJNNPaxType ||
      _actualPaxType->paxTypeInfo()->paxType() == matchUNNPaxType)
  {
    return true;
  }

  return false;
}

bool
PaxTypeFare::isInfant() const
{
  if (_actualPaxType->paxTypeInfo()->infantInd() == 'Y' ||
      _actualPaxType->paxTypeInfo()->paxType() == matchJNFPaxType)
  {
    return true;
  }

  return false;
}

const MoneyAmount
PaxTypeFare::totalFareAmount() const
{
  MoneyAmount totalAmount = mileageSurchargeAmt();
  totalAmount += nucFareAmount();

  // Add Minimum Fare plus up by requested Pax Type

  return totalAmount;
}

PaxTypeFare*
PaxTypeFare::clone(DataHandle& dataHandle, bool resetStatus, FareMarket* newFm, Fare* newFare) const
{
  PaxTypeFare* cloneObj = nullptr;
  dataHandle.get(cloneObj);

  clone(dataHandle, *cloneObj, resetStatus, newFm, newFare);

  return cloneObj;
}

void
PaxTypeFare::clone(DataHandle& dataHandle,
                   PaxTypeFare& cloneObj,
                   bool resetStatus,
                   FareMarket* newFm,
                   Fare* newFare) const
{
  cloneObj._status = _status;
  cloneObj._isValidForBranding = _isValidForBranding;
  cloneObj._cpFailedStatus = _cpFailedStatus;
  cloneObj._ruleStatus = _ruleStatus;
  cloneObj._ruleProcessStatus = _ruleProcessStatus;
  cloneObj._ruleSoftPassStatus = _ruleSoftPassStatus;
  cloneObj._bookingCodeStatus = _bookingCodeStatus;
  cloneObj._segmentStatus = _segmentStatus;
  cloneObj._segmentStatusRule2 = _segmentStatusRule2;
  cloneObj._mixClassStatus = _mixClassStatus;
  cloneObj._fareDisplayStatus = _fareDisplayStatus;
  cloneObj._fare = newFare ? newFare : _fare;
  cloneObj._bookingCode = _bookingCode;
  cloneObj._actualPaxType = _actualPaxType;
  cloneObj._actualPaxTypeItem = _actualPaxTypeItem;
  cloneObj._fareMarket = newFm ? newFm : _fareMarket;
  cloneObj._fareClassAppInfo = _fareClassAppInfo;
  cloneObj._fareClassAppSegInfo = _fareClassAppSegInfo;
  cloneObj._fcasOfOverFlownBkgCds = _fcasOfOverFlownBkgCds;
  cloneObj._cabin = _cabin;
  cloneObj._fareTypeDesignator = _fareTypeDesignator;
  cloneObj._fareTypeApplication = _fareTypeApplication;
  cloneObj._convertedFareAmount = _convertedFareAmount;
  cloneObj._soloSurcharges = _soloSurcharges;
  for(unsigned i = 0; i<_paxTypeFareRuleDataMap->size(); ++i)
    (*cloneObj._paxTypeFareRuleDataMap)[i].store(
             (*_paxTypeFareRuleDataMap)[i].load(std::memory_order_relaxed),std::memory_order_relaxed);

  cloneObj._fareStatusPerInclCode.insert(_fareStatusPerInclCode.begin(),
                                          _fareStatusPerInclCode.end());
  cloneObj._maxStopoversPermitted = _maxStopoversPermitted;
  cloneObj._puRuleValNeeded = _puRuleValNeeded;

  cloneObj._flightBitmap = _flightBitmap;
  cloneObj._flightBitmapESV = _flightBitmapESV;
  cloneObj._durationFlightBitmapForCarrier = _durationFlightBitmapForCarrier;
  cloneObj._flightBitmapForCarrier = _flightBitmapForCarrier;
  cloneObj._flightBitmapPerCarrier = _flightBitmapPerCarrier;
  cloneObj._fareCallbackFVOForCarrier = _fareCallbackFVOForCarrier;
  cloneObj._durationFlightBitmapPerCarrier = _durationFlightBitmapPerCarrier;
  cloneObj._fltIndependentValidationFVO = _fltIndependentValidationFVO;
  cloneObj._fltIndependentValidationFVOForCarrier = _fltIndependentValidationFVOForCarrier;
  cloneObj._fltIndependentValidationFVOPerCarrier = _fltIndependentValidationFVOPerCarrier;
  cloneObj._shoppingComponentValidationFailed = _shoppingComponentValidationFailed;
  cloneObj._shoppingComponentValidationPerformed = _shoppingComponentValidationPerformed;
  cloneObj._flightBitmapAllInvalid = _flightBitmapAllInvalid;
  cloneObj._isShoppingFare = _isShoppingFare;

  cloneObj._penaltyRestInd = _penaltyRestInd;
  cloneObj._changePenaltyApply = _changePenaltyApply;
  cloneObj._csTextMessages = _csTextMessages;
  cloneObj._cat25BasePaxFare = _cat25BasePaxFare;
  cloneObj._fareDisplayCat35Type = _fareDisplayCat35Type;

  cloneObj._NVAData = _NVAData;
  cloneObj._failedByJalAxessRequest = _failedByJalAxessRequest;
  cloneObj._ticketedFareForAxess = _ticketedFareForAxess;
  cloneObj._nationFranceInCat35 = _nationFranceInCat35;

  cloneObj._altDateStatus = _altDateStatus;
  cloneObj._durationFlightBitmap = _durationFlightBitmap;
  cloneObj._altDateFltBitStatus = _altDateFltBitStatus;
  if (_structuredRuleData != nullptr)
    cloneObj._structuredRuleData.reset(new PaxTypeFareStructuredRuleData(*_structuredRuleData));

  cloneObj._retrievalInfo = _retrievalInfo;
  cloneObj._adjSellCalcData = _adjSellCalcData;
  cloneObj._cat25Fare = _cat25Fare;
  cloneObj._miscFareTag = _miscFareTag;

  cloneObj._matchedAccCode = _matchedAccCode;
  cloneObj._mileage = _mileage;
  cloneObj._surchargeData = _surchargeData;
  cloneObj._needRecalculateCat12 = _needRecalculateCat12;
  cloneObj._brandStatusWithDirection = _brandStatusWithDirection;
  cloneObj._validatingCarriers = _validatingCarriers;
  cloneObj._cat15HasT996FT = _cat15HasT996FT;
  cloneObj._cat15HasT996FR = _cat15HasT996FR;
  cloneObj._cat15HasT996GR = _cat15HasT996GR;
  cloneObj._cat15SecurityFail = _cat15SecurityFail;
  cloneObj._cat15SoftPass = _cat15SoftPass;

  // cached fields
  cloneObj._tcrTariffCat = _tcrTariffCat;
  cloneObj._tcrRuleTariff = _tcrRuleTariff;
  cloneObj._carrier = _carrier;
  cloneObj._ruleNumber = _ruleNumber;

  if (_flexFaresValidationStatus)
    cloneObj._flexFaresValidationStatus =
        std::make_shared<flexFares::ValidationStatus>(*_flexFaresValidationStatus);

  cloneObj._matchFareFocusRule = _matchFareFocusRule;
  cloneObj._routingInvalidByMPM = _routingInvalidByMPM;
  cloneObj._requestedFareBasisInvalid = _requestedFareBasisInvalid;
  cloneObj._spanishDiscountEnabled = _spanishDiscountEnabled;

  if (_penaltyRecordsOld)
  {
    MaxPenaltyInfoRecords* maxPen = dataHandle.create<MaxPenaltyInfoRecords>();
    cloneObj._penaltyRecordsOld = maxPen;
    cloneObj._penaltyRecordsOld->_penaltyInfo = _penaltyRecordsOld->_penaltyInfo;
    cloneObj._penaltyRecordsOld->_voluntaryRefundsInfo = _penaltyRecordsOld->_voluntaryRefundsInfo;
    cloneObj._penaltyRecordsOld->_voluntaryChangesInfo = _penaltyRecordsOld->_voluntaryChangesInfo;
    cloneObj._penaltyRecordsOld->_penaltyRecordsLoaded = _penaltyRecordsOld->_penaltyRecordsLoaded;
  }
  else
  {
    cloneObj._penaltyRecords._voluntaryRefundsInfo = _penaltyRecords._voluntaryRefundsInfo;
    cloneObj._penaltyRecords._voluntaryChangesInfo = _penaltyRecords._voluntaryChangesInfo;
    cloneObj._penaltyRecords._penaltyInfo = _penaltyRecords._penaltyInfo;
    cloneObj._penaltyRecords._penaltyRecordsLoaded = _penaltyRecords._penaltyRecordsLoaded;
  }

  cloneObj._smpShoppingISCoreFixFallback = _smpShoppingISCoreFixFallback;

  if (resetStatus)
  {
    cloneObj.resetRuleStatus();

    // Keep softpass status for cat19-22/25 fare
    if (isFareByRule())
    {
      if (isCategorySoftPassed(25))
        cloneObj.setCategorySoftPassed(25);
    }

    if (isDiscounted())
    {
      if (LIKELY(isCategorySoftPassed(19)))
        cloneObj.setCategorySoftPassed(19);
      if (UNLIKELY(isCategorySoftPassed(20)))
        cloneObj.setCategorySoftPassed(20);
      if (UNLIKELY(isCategorySoftPassed(21)))
        cloneObj.setCategorySoftPassed(21);
      if (UNLIKELY(isCategorySoftPassed(22)))
        cloneObj.setCategorySoftPassed(22);
    }
  }

#ifdef TRACKING
  cloneObj._trackCollector.assign(_trackCollector);
#endif
}

PaxTypeFare::CabinComparator::CabinComparator(bool compareCabins)
{
  _compareCabins = compareCabins;
}

bool
PaxTypeFare::CabinComparator::areCabinsEqual(const CabinType& leftCabin,
                                             const CabinType& rightCabin) const
{
  if (UNLIKELY(leftCabin.isValidCabin() && rightCabin.isValidCabin()))
    return leftCabin == rightCabin;

  return !leftCabin.isValidCabin() && !rightCabin.isValidCabin();
}

bool
PaxTypeFare::CabinComparator::isLessCabin(const CabinType& leftCabin, const CabinType& rightCabin)
    const
{
  const bool leftCabinValid = leftCabin.isValidCabin();
  const bool rightCabinValid = rightCabin.isValidCabin();

  if (leftCabinValid && !rightCabinValid)
    return true;

  if (!leftCabinValid && rightCabinValid)
    return false;

  return leftCabin > rightCabin;
}

PaxTypeFare::FareComparator::FareComparator(PaxTypeBucket& cortege,
                                            uint16_t paxTypeNum,
                                            bool compareCabins,
                                            const bool isCmdPricing)
  : _paxTypeNum(paxTypeNum), _cortege(cortege), _cabinComparator(compareCabins),
    _isCmdPricing(isCmdPricing)
{
}

bool
PaxTypeFare::FareComparator::
operator()(const PaxTypeFare* lptf, const PaxTypeFare* rptf) const
{
  if (UNLIKELY(lptf == nullptr || rptf == nullptr))
  {
    return (lptf < rptf);
  }

  MoneyAmount lNucAmount = lptf->totalFareAmount();
  MoneyAmount rNucAmount = rptf->totalFareAmount();

  lNucAmount += lptf->baggageLowerBound();
  rNucAmount += rptf->baggageLowerBound();

  // add surcharges and yqyr to NucAmounts
  {
    const CxrPrecalculatedTaxes& taxes = _cortege.cxrPrecalculatedTaxes();
    if (!taxes.empty())
    {
      lNucAmount += taxes.getLowerBoundTotalTaxAmount(*lptf);
      rNucAmount += taxes.getLowerBoundTotalTaxAmount(*rptf);
    }
  }

  MoneyAmount diff = std::abs(rNucAmount - lNucAmount);
  if (diff > EPSILON)
    return (lNucAmount < rNucAmount);

  if (UNLIKELY(_cabinComparator.shouldCompareCabins() &&
      !_cabinComparator.areCabinsEqual(lptf->cabin(), rptf->cabin())))
  {
    return _cabinComparator.isLessCabin(lptf->cabin(), rptf->cabin());
  }

  if (UNLIKELY(lptf->mileage() != rptf->mileage()))
    return (lptf->mileage() < rptf->mileage());

  // nucFareAmount equal (or close enough)

  // left < right MUST imply !right < left
  if (lptf->isFareByRule() != rptf->isFareByRule())
  {
    return lptf->isFareByRule();
  }

  // may need to change for fare group number.
  if (UNLIKELY(lptf->actualPaxTypeItem().size() > _paxTypeNum &&
      rptf->actualPaxTypeItem().size() > _paxTypeNum))
  {
    if (lptf->actualPaxTypeItem()[_paxTypeNum] != rptf->actualPaxTypeItem()[_paxTypeNum])
    {
      return (lptf->actualPaxTypeItem()[_paxTypeNum] < rptf->actualPaxTypeItem()[_paxTypeNum]);
    }
  }

  if (lptf->isWebFare() != rptf->isWebFare())
  {
    return lptf->isWebFare();
  }

  // may need to change for group fare, etc.
  if (lptf->fcasPaxType() != rptf->fcasPaxType())
  {
    const PaxTypeCode paxCode = _cortege.requestedPaxType()->paxType();
    if (paxCode == lptf->fcasPaxType())
      return true;
    else if (paxCode == rptf->fcasPaxType())
      return false;
    // both not requested, so put ADT at end
    if (rptf->fcasPaxType() == ADULT)
      return true;
    else if (lptf->fcasPaxType() == ADULT)
      return false;
  }

  if ((lptf->carrier() == INDUSTRY_CARRIER) != (rptf->carrier() == INDUSTRY_CARRIER))
  {
    // put Carrier fare above YY fare
    return (rptf->carrier() == INDUSTRY_CARRIER);
  }

  if (lptf->paxType()->paxTypeInfo().paxTypeStatus() !=
      rptf->paxType()->paxTypeInfo().paxTypeStatus())
  {
    // higher the value higher the priority
    return lptf->paxType()->paxTypeInfo().paxTypeStatus() >
           rptf->paxType()->paxTypeInfo().paxTypeStatus();
  }

  if (lptf->fareClass() != rptf->fareClass())
    return (lptf->fareClass() < rptf->fareClass());
  if (lptf->ruleNumber() != rptf->ruleNumber())
    return (lptf->ruleNumber() < rptf->ruleNumber());

  if (lptf->isNegotiated() && rptf->isNegotiated())
  {
    const PaxTypeFareRuleData* lptfRuleData =
        lptf->paxTypeFareRuleData(35); // RuleConst::NEGOTIATED_RULE
    const PaxTypeFareRuleData* rptfRuleData =
        rptf->paxTypeFareRuleData(35); // RuleConst::NEGOTIATED_RULE
    if (LIKELY(lptfRuleData && rptfRuleData))
    {
      const CategoryRuleItemInfo* lptfCrItemInfo = lptfRuleData->categoryRuleItemInfo();
      const CategoryRuleItemInfo* rptfCrItemInfo = rptfRuleData->categoryRuleItemInfo();

      if(lptfCrItemInfo->itemNo() != rptfCrItemInfo->itemNo())
        return lptfCrItemInfo->itemNo() < rptfCrItemInfo->itemNo();
    }
  }

  return _isCmdPricing && lptf->cmdPricedWFail() < rptf->cmdPricedWFail();
}

bool
PaxTypeFare::isPSRApplied() const
{
  if (LIKELY(_fareMarket != nullptr && mileageInfo()))
  {
    return mileageInfo()->psrApplies();
  }
  return false;
}

bool
PaxTypeFare::isValidForPricing() const
{
  if (!isValid())
    return false;

  if (!_fare->isIndustry())
    return true;

  IndustryFare* indf = static_cast<IndustryFare*>(_fare);
  return indf->validForPricing();
}

bool
PaxTypeFare::isValidForCombination() const
{
  if (!isValidForPO())
    return false;

  if (!_fare->isIndustry())
    return true;

  IndustryFare* indf = static_cast<IndustryFare*>(_fare);
  return indf->validForPricing();
}

//-------------------------------------------------------------------------//
//-- Start shopping methods
//-------------------------------------------------------------------------//
bool
PaxTypeFare::setFlightBitmapSize(const uint32_t& flightBitmapSize)
{
  if (_flightBitmapForCarrier)
  {
    if (UNLIKELY(!_flightBitmapForCarrier->empty()))
      return false;

    _flightBitmapForCarrier->resize(flightBitmapSize);
    return true;
  }

  if (UNLIKELY(_flightBitmapSize > 0))
  {
    // Only allow the resize to occur once
    return (false);
  }

  _flightBitmap.resize(flightBitmapSize);
  _flightBitmapSize = flightBitmapSize;

  return true;
}

// Returns validity of a certain flight in spiecified durationFlightBitmap
// false means the flight is not valid
// true means the flight is valid
bool
PaxTypeFare::isFlightValid(const uint64_t& duration, const uint32_t& i) const
{
  bool bRet = false;

  VecMap<uint64_t, PaxTypeFare::FlightBitmap>::const_iterator itCurDurFlBitmap =
      durationFlightBitmap().find(duration);

  if (itCurDurFlBitmap != durationFlightBitmap().end() && !(itCurDurFlBitmap->second.empty()))
  {
    if (itCurDurFlBitmap->second.size() > i && itCurDurFlBitmap->second[i]._flightBit == 0)
    {
      bRet = true;
    }
  }
  return bRet;
}

bool
PaxTypeFare::setFlightInvalid(const uint32_t& i, uint8_t errorNum /*= '1'*/)
{
  if (_flightBitmapForCarrier)
    return setFlightInvalid(*_flightBitmapForCarrier, i, errorNum);

  if (UNLIKELY(i >= _flightBitmapSize))
  {
    return (false);
  }

  _flightBitmap[i]._flightBit = errorNum;

  return true;
}

bool
PaxTypeFare::setFlightInvalid(FlightBitmap& flightBitmap, const uint32_t& i, uint8_t errorNum)
{
  if (UNLIKELY(i >= flightBitmap.size()))
  {
    return (false);
  }

  flightBitmap[i]._flightBit = errorNum;

  return (true);
}

uint8_t*
PaxTypeFare::getFlightBit(const uint32_t& i)
{
  if (_flightBitmapForCarrier)
  {
    if (UNLIKELY(_flightBitmapForCarrier->size() <= i))
      return nullptr;

    return &((*_flightBitmapForCarrier)[i]._flightBit);
  }

  if (i >= _flightBitmapSize)
  {
    return (nullptr);
  }

  return (&(_flightBitmap[i]._flightBit));
}

const uint8_t*
PaxTypeFare::getFlightBit(const uint64_t& duration, const uint32_t& i)
{
  VecMap<uint64_t, PaxTypeFare::FlightBitmap>::const_iterator itCurDurFlBitmap =
      durationFlightBitmap().find(duration);

  if (itCurDurFlBitmap != durationFlightBitmap().end() && !(itCurDurFlBitmap->second.empty()))
  {
    if (itCurDurFlBitmap->second.size() > i)
    {
      return (&(itCurDurFlBitmap->second[i]._flightBit));
    }
  }

  return nullptr;
}
bool
PaxTypeFare::isFlightBitmapInvalid(bool skippedAsInvalid) const
{
  if (_flightBitmapForCarrier)
    return isFlightBitmapInvalid(*_flightBitmapForCarrier, skippedAsInvalid);

  return isFlightBitmapInvalid(_flightBitmap, skippedAsInvalid);
}

bool
PaxTypeFare::isFlightBitmapInvalid(const FlightBitmap& flightBitmap,
                                   bool skippedAsInvalid)
{
  FlightBitEqualTo isBitValid(0);
  FlightBitEqualTo isBitSkipped('S');

  return std::none_of(flightBitmap.begin(), flightBitmap.end(), [&](const FlightBit& fb)
  {
    if (isBitValid(fb))
      return true;
    return !skippedAsInvalid && isBitSkipped(fb);
  });
}

bool
PaxTypeFare::setFlightBookingCodeStatus(const uint32_t& i,
                                        const BookingCodeStatus& bookingCodeStatus)
{
  if (_flightBitmapForCarrier)
  {
    if (UNLIKELY(_flightBitmapForCarrier->size() <= i))
      return false;

    (*_flightBitmapForCarrier)[i]._bookingCodeStatus = bookingCodeStatus;
    return true;
  }

  if (UNLIKELY(i >= _flightBitmapSize))
  {
    return false;
  }

  _flightBitmap[i]._bookingCodeStatus = bookingCodeStatus;

  return true;
}

bool
PaxTypeFare::setFlightMPMPercentage(const uint32_t& i, const uint16_t& mpmPercentage)
{
  if (_flightBitmapForCarrier)
  {
    if (UNLIKELY(_flightBitmapForCarrier->size() <= i))
      return false;

    (*_flightBitmapForCarrier)[i]._mpmPercentage = mpmPercentage;
    return true;
  }

  if (UNLIKELY(i >= _flightBitmapSize))
  {
    return (false);
  }

  _flightBitmap[i]._mpmPercentage = mpmPercentage;

  return true;
}

bool
PaxTypeFare::swapInFlightSegmentStatus(const uint32_t& i, std::vector<SegmentStatus>& segmentStatus)
{
  if (_flightBitmapForCarrier)
  {
    if (UNLIKELY(_flightBitmapForCarrier->size() <= i))
      return false;

    (*_flightBitmapForCarrier)[i]._segmentStatus.swap(segmentStatus);
  }

  if (i >= _flightBitmap.size())
    return false;

  _flightBitmap[i]._segmentStatus.swap(segmentStatus);

  return true;
}

std::vector<PaxTypeFare::SegmentStatus>*
PaxTypeFare::getFlightSegmentStatus(const uint32_t& i)
{
  if (_flightBitmapForCarrier)
  {
    if (_flightBitmapForCarrier->size() <= i)
      return nullptr;

    return &((*_flightBitmapForCarrier)[i]._segmentStatus);
  }

  if (i >= _flightBitmapSize)
  {
    return nullptr;
  }

  return (&(_flightBitmap[i]._segmentStatus));
}

const std::vector<PaxTypeFare::SegmentStatus>*
PaxTypeFare::getFlightSegmentStatus(const uint32_t& i) const
{
  if (_flightBitmapForCarrier)
  {
    if (_flightBitmapForCarrier->size() <= i)
      return nullptr;

    return &((*_flightBitmapForCarrier)[i]._segmentStatus);
  }

  if (i >= _flightBitmapSize)
  {
    return nullptr;
  }

  return (&(_flightBitmap[i]._segmentStatus));
}

void
PaxTypeFare::initAltDates(const ShoppingTrx& trx)
{
  for (const auto& elem : trx.altDatePairs())
  {
    setAltDateStatus(elem.first, 0);
  }
}

uint8_t
PaxTypeFare::getAltDateStatus(const DatePair& dates) const
{
  const VecMap<DatePair, uint8_t>::const_iterator i = _altDateStatus.find(dates);
  if (LIKELY(i != _altDateStatus.end()))
  {
    return i->second;
  }
  else
  {
    return 'X';
  }
}

uint8_t
PaxTypeFare::getAltDateStatus(const DateTime& date, uint8_t leg) const
{
  for (const auto& elem : _altDateStatus)
  {
    DatePair datePair = elem.first;
    if (leg == 0)
    {
      if (datePair.first.get64BitRepDateOnly() == date.get64BitRepDateOnly())
        return elem.second;
    }
    else
    {
      if (datePair.second.get64BitRepDateOnly() == date.get64BitRepDateOnly())
        return elem.second;
    }
  }
  return 'X';
}

void
PaxTypeFare::setAltDateStatus(const DatePair& dates, uint8_t status)
{
  _altDateStatus[dates] = status;
}

bool
PaxTypeFare::getAltDatePass(const DatePair& dates) const
{
  return getAltDateStatus(dates) == 0;
}

bool
PaxTypeFare::getAltDatePass(const DateTime& date, uint8_t leg) const
{
  return getAltDateStatus(date, leg) == 0;
}

bool
PaxTypeFare::isAltDateValid()
{
  for (VecMap<DatePair, uint8_t>::const_iterator i = _altDateStatus.begin();
       i != _altDateStatus.end();
       ++i)
  {
    uint8_t j = i->second;
    if (j == 0)
      return true;
  }
  return false;
}

//  ALTDATE REUSE PROCESSED
bool
PaxTypeFare::setAltDateFltBitStatusSize()
{
  if (_flightBitmapSize <= 0)
  {
    // Only resize when _flightBitmapSize is already set
    return false;
  }

  _altDateFltBitStatus.resize(_flightBitmapSize);
  return true;
}

bool
PaxTypeFare::setAltDateFltBitBKCProcessed(const uint32_t& fltBit)
{
  if (fltBit >= _flightBitmapSize)
  {
    return false;
  }

  _altDateFltBitStatus[fltBit].set(BKC_PROCESSED);
  return true;
}

bool
PaxTypeFare::setAltDateFltBitBKCFailed(const uint32_t& fltBit)
{
  if (fltBit >= _flightBitmapSize)
  {
    return false;
  }

  return _altDateFltBitStatus[fltBit].set(BKC_FAILED);
}

bool
PaxTypeFare::isAltDateFltBitBKCProcessed(const uint32_t& fltBit)
{
  if (fltBit >= _flightBitmapSize)
  {
    return false;
  }

  return _altDateFltBitStatus[fltBit].isSet(BKC_PROCESSED);
}

bool
PaxTypeFare::isAltDateFltBitBKCPassed(const uint32_t& fltBit)
{
  if (fltBit >= _flightBitmapSize)
  {
    return false;
  }

  return !_altDateFltBitStatus[fltBit].isSet(BKC_FAILED);
}

bool
PaxTypeFare::setAltDateFltBitRTGProcessed(const uint32_t& fltBit)
{
  if (fltBit >= _flightBitmapSize)
  {
    return false;
  }

  return _altDateFltBitStatus[fltBit].set(RTG_PROCESSED);
}

bool
PaxTypeFare::setAltDateFltBitRTGFailed(const uint32_t& fltBit)
{
  if (fltBit >= _flightBitmapSize)
  {
    return false;
  }

  return _altDateFltBitStatus[fltBit].set(RTG_FAILED);
}

bool
PaxTypeFare::isAltDateFltBitRTGProcessed(const uint32_t& fltBit)
{
  if (fltBit >= _flightBitmapSize)
  {
    return false;
  }

  return _altDateFltBitStatus[fltBit].isSet(RTG_PROCESSED);
}

bool
PaxTypeFare::isAltDateFltBitRTGPassed(const uint32_t& fltBit)
{
  if (fltBit >= _flightBitmapSize)
  {
    return false;
  }

  return !_altDateFltBitStatus[fltBit].isSet(RTG_FAILED);
}

bool
PaxTypeFare::setAltDateFltBitGLBProcessed(const uint32_t& fltBit)
{
  if (fltBit >= _flightBitmapSize)
  {
    return false;
  }

  return _altDateFltBitStatus[fltBit].set(RTG_PROCESSED);
}

bool
PaxTypeFare::setAltDateFltBitGLBFailed(const uint32_t& fltBit)
{
  if (fltBit >= _flightBitmapSize)
  {
    return false;
  }

  return _altDateFltBitStatus[fltBit].set(RTG_FAILED);
}

bool
PaxTypeFare::isAltDateFltBitGLBProcessed(const uint32_t& fltBit)
{
  if (fltBit >= _flightBitmapSize)
  {
    return false;
  }

  return _altDateFltBitStatus[fltBit].isSet(RTG_PROCESSED);
}

bool
PaxTypeFare::isAltDateFltBitGLBPassed(const uint32_t& fltBit)
{
  if (fltBit >= _flightBitmapSize)
  {
    return false;
  }

  return !_altDateFltBitStatus[fltBit].isSet(RTG_FAILED);
}

//-------------------------------------------------------------------------//
//-- End shopping methods
//-------------------------------------------------------------------------//
bool
PaxTypeFare::fareSort(const PaxTypeFare* lptf, const PaxTypeFare* rptf)
{
  if (UNLIKELY(lptf == nullptr))
    return true;

  if (UNLIKELY(rptf == nullptr))
    return false;

  if (lptf->totalFareAmount() != rptf->totalFareAmount())
    return (lptf->totalFareAmount() < rptf->totalFareAmount());

  if (lptf->isWebFare() != rptf->isWebFare())
    return lptf->isWebFare();

  return (lptf->fareClass() < rptf->fareClass());
}

const TariffCode*
PaxTypeFare::getRuleTariffCode(FareDisplayTrx& trx)
{

  TariffCode* ruleTariffCode = nullptr;
  const Itin* itin = trx.itin().front();

  if (itin == nullptr)
  {
    return ruleTariffCode;
  }

  const bool isInternational = (this->isInternational() || this->isForeignDomestic());

  const FareByRuleApp& fbrApp = this->fbrApp();

  const std::vector<TariffCrossRefInfo*>& tcrList =
      trx.dataHandle().getTariffXRefByRuleTariff(fbrApp.vendor(),
                                                 fbrApp.carrier(),
                                                 (isInternational ? INTERNATIONAL : DOMESTIC),
                                                 fbrApp.ruleTariff(),
                                                 itin->travelDate());
  size_t tcrSize = tcrList.size();
  if (tcrSize == 0)
  {
    return nullptr;
  }
  else if (tcrSize > 1)
  {
    //------------------------------------------------------------------------
    // For Private Non-Carrier Tariffs, we must apply the logic to match the
    // Rule Number and the RuleTariffCode in the TariffCrossRef table.
    //
    // For EX: Rule 'XX21' (770) will match to RuleTariffCode 'FXX----' (770)
    //         Rule '4353' (770) will match to RuleTariffCode 'F43----' (770)
    //
    // For other Cat 25 rule tariffs, do not apply that special logic.
    // For EX: Rule 'XX09' (191) will match to ruletariff 191, ruletariffcode could be 'FBRNAPV'.
    //         Rule 'NA71' (191) will match to ruletariff 191, ruletariffcode could be 'FBRNAPV'.
    //------------------------------------------------------------------------
    std::vector<TariffCrossRefInfo*>::const_iterator tcr = tcrList.begin();

    for (; tcr != tcrList.end(); tcr++)
    {
      if (((*tcr)->tariffCat() == PRIVATE_TARIFF) && ((*tcr)->ruleTariffCode().size() > 2))
      {
        if (fbrApp.ruleNo()[0] == (*tcr)->ruleTariffCode()[1] &&
            fbrApp.ruleNo()[1] == (*tcr)->ruleTariffCode()[2])
        {
          ruleTariffCode = &((*tcr)->ruleTariffCode());
          break;
        }
      }
    }
    if (tcr == tcrList.end())
    {
      tcr = tcrList.begin();
      ruleTariffCode = &((*tcr)->ruleTariffCode());
    }
  }
  else
  {
    std::vector<TariffCrossRefInfo*>::const_iterator tcr = tcrList.begin();
    ruleTariffCode = &((*tcr)->ruleTariffCode());
  }

  return ruleTariffCode;
}

void
PaxTypeFare::setCat15SoftPass(bool cat15SoftPass)
{
  _cat15SoftPass = cat15SoftPass;
}

void
PaxTypeFare::addNVAData(DataHandle& dataHandle, uint16_t segOrder, const DateTime* nvaDate)
{
  if (!_NVAData)
  {
    dataHandle.get(_NVAData);
  }
  if (UNLIKELY(!_NVAData))
    return;

  (*_NVAData)[segOrder] = nvaDate;
}

std::map<uint16_t, const DateTime*>*
PaxTypeFare::getNVAData() const
{
  return _NVAData;
}

bool
PaxTypeFare::isDummyFare() const
{
  return _fareMarket->useDummyFare();
}

bool
PaxTypeFare::isCWTMarketFare() const
{
  if (tcrTariffCat() == PRIVATE_TARIFF && !matchedCorpID() &&
      PaxTypeUtil::isAdultOrNegotiatedOrAssociatedType(fcasPaxType()))
  {
    if (fare()->isNationFRInCat15() || isNationFRInCat35())
      return true;
    else if (isFareByRule()) // need to check the calculated FBR
    {
      try
      {
        const FareByRuleItemInfo* fbrItemInfo =
            dynamic_cast<const FareByRuleItemInfo*>(getFbrRuleData()->ruleItemInfo());
        if (fbrItemInfo == nullptr || isSpecifiedFare())
          return false;

        Indicator i = fbrItemInfo->ovrdcat15();
        if (i == 'B' || i == ' ') // need to check cat15 for the base fare
        {
          PaxTypeFare* bFare = baseFare(25);
          if (bFare && bFare->fare()->isNationFRInCat15())
            return true;
        }
      }
      catch (...) { return false; }
    }
  }
  return false;
}

bool
PaxTypeFare::isValidAsKeepFare(const PricingTrx& trx) const
{
  if (_retrievalInfo != nullptr)
  {
    if (_retrievalInfo->keep())
    {
      // Keep fare still need to pass routing
      return (!isRoutingProcessed() || isRoutingValid()) && _isValidForBranding;
    }
    else if (!isValidAccountCode(trx))
    {
      return false;
    }
  }
  return isValidForPricing();
}

bool
PaxTypeFare::isValidAccountCode(const PricingTrx& trx) const
{
  if (isFareByRule()) // Fiter out non-keep fare created from different account code
  {
    const AccountCode& acctCode = fbrApp().accountCode();
    if (!acctCode.empty() && (acctCode != trx.getRequest()->accountCode()))
    {
      if (!trx.getRequest()->corporateID().empty())
      {
        const std::vector<tse::CorpId*>& corporateIds =
            trx.dataHandle().getCorpId(trx.getRequest()->corporateID(), _carrier, trx.travelDate());
        if (std::none_of(corporateIds.begin(),
                         corporateIds.end(),
                         [&acctCode](const tse::CorpId* corpId)
                         { return corpId->accountCode() == acctCode; }))
          return false;
      }
      else
        return false;
    }
  }
  return true;
}

bool
PaxTypeFare::isNonRefundableByFareType() const
{
  static const FareType fType[] = { "XAN", "XBN", "XPN", "XPV", "EIP", "SIP" };
  static const FareType* fTypeEnd = fType + sizeof(fType) / sizeof(FareType);
  return std::find(fType, fTypeEnd, fcaFareType()) != fTypeEnd;
}

MileageInfo*&
PaxTypeFare::mileageInfo()
{
  return _fareMarket->mileageInfo(directionality() == FROM);
}

const MileageInfo*
PaxTypeFare::mileageInfo() const
{
  return _fareMarket->mileageInfo(directionality() == FROM);
}

uint32_t
PaxTypeFare::getRebookedClassesStatus()
{
  if (_bkgCodeTypeForRex != BKSS_NOT_YET_PROCESSED)
    return _bkgCodeTypeForRex;

  size_t size = _segmentStatus.size();
  for (size_t idx = 0; idx < size; idx++)
  {
    if (!_segmentStatus[idx]._bkgCodeSegStatus.isSet(BKSS_REX_WP_LOCALMARKET_VALIDATION) &&
        _segmentStatus[idx]._bkgCodeSegStatus.isSet(BKSS_REBOOKED) &&
        !_segmentStatus[idx]._bkgCodeReBook.empty())
      return BKSS_REBOOKED;
  }
  return BKSS_PASS; // it marks as Booked
}

MoneyAmount&
PaxTypeFare::nucFareAmount()
{
  return needSecondRoeDateAmount() ? _fare->rexSecondNucFareAmount() : _fare->nucFareAmount();
}

const MoneyAmount
PaxTypeFare::nucFareAmount() const
{
  return needSecondRoeDateAmount() ? _fare->rexSecondNucFareAmount() : _fare->nucFareAmount();
}

MoneyAmount
PaxTypeFare::nucAmountWithEstimatedTaxes(const PaxTypeBucket& cortege) const
{
  MoneyAmount amount = totalFareAmount();

  amount += cortege.cxrPrecalculatedTaxes().getLowerBoundTotalTaxAmount(*this);
  amount += _baggageLowerBound;

  return amount;
}

MoneyAmount&
PaxTypeFare::mileageSurchargeAmt()
{
  return needSecondRoeDateAmount() ? _fare->rexSecondMileageSurchargeAmt()
                                   : _fare->mileageSurchargeAmt();
}

const MoneyAmount&
PaxTypeFare::mileageSurchargeAmt() const
{
  return needSecondRoeDateAmount() ? _fare->rexSecondMileageSurchargeAmt()
                                   : _fare->mileageSurchargeAmt();
}

bool
PaxTypeFare::needSecondRoeDateAmount() const
{
  if (UNLIKELY(_fareMarket && _fareMarket->rexBaseTrx() != nullptr &&
      static_cast<RexBaseTrx*>(_fareMarket->rexBaseTrx())->useSecondROEConversionDate()))
    return true;
  return false;
}

bool
PaxTypeFare::hasConstructedRouting() const
{
  if (_fare->globalDirection() == GlobalDirection::CT ||
      _fare->globalDirection() == GlobalDirection::RW)
    return false;

  if (!isFareByRule()) // Fare not build by CAT 25
  {
    bool isConstr = true;
    if (!isConstructed())
      isConstr = false;
    else
    {
      if (UNLIKELY(SITA_VENDOR_CODE == _fare->fareInfo()->vendor()))
      {
        const SITAConstructedFareInfo* sitaCfi =
            dynamic_cast<const SITAConstructedFareInfo*>(_fare->constructedFareInfo());
        if (sitaCfi && (MILEAGE_ROUTING != sitaCfi->throughFareRouting()))
          isConstr = false;
      }
    }
    return isConstr;
  }

  const FareByRuleItemInfo& fbrItemInfo = fareByRuleInfo();
  if (fbrItemInfo.resultRouting().empty())
  {
    return !_fare->isConstructedFareInfoMissing();
  }

  return false;
}

bool
PaxTypeFare::isMainDataValid() const
{
  return (isNoDataMissing() && _fareDisplayStatus.isNull() && // no failure codes
          _fare->isValid() &&
          !failedFareGroup() && _isValidForBranding && isValidForBaggage());
}

bool
PaxTypeFare::isValidInternal(bool needBookingProcessed) const
{
  if ( areAllCategoryValid() &&
       _isMipUniqueFare &&
       !isNotValidForCP() &&
       ((! isRoutingProcessed()) || isRoutingValid()) &&
       !isRequestedFareBasisInvalid() &&
       _isValidForRepricing)
  {
    if (needBookingProcessed)
    {
      if (!_bookingCodeStatus.isSet(BKS_NOT_YET_PROCESSED) && !_bookingCodeStatus.isSet(BKS_FAIL))
        return true;
    }
    else
    {
      if (_bookingCodeStatus.isSet(BKS_NOT_YET_PROCESSED) || !_bookingCodeStatus.isSet(BKS_FAIL))
        return true;
    }
  }

  return false;
}

bool
PaxTypeFare::applyNonIATAR(const PricingTrx& trx,
                           const VendorCode& vendor,
                           const CarrierCode& carrier,
                           const RuleNumber& ruleNumber)
{
  return CurrencyRoundingUtil::applyNonIATARounding(trx, vendor, carrier, ruleNumber);
}

bool
PaxTypeFare::isNetRemitTktIndicatorAorB() const
{
  if (isNegotiated())
  {
    const NegPaxTypeFareRuleData* negPaxTypeFareRuleData = getNegRuleData();
    if (negPaxTypeFareRuleData != nullptr)
    {
      const NegFareRest* negFareRest =
          dynamic_cast<const NegFareRest*>(negPaxTypeFareRuleData->ruleItemInfo());
      if (negFareRest && (negFareRest->tktFareDataInd1() == RuleConst::NR_VALUE_A ||
                          negFareRest->tktFareDataInd1() == RuleConst::NR_VALUE_B))
        return true;
      const NegFareRestExt* negFareRestExt = negPaxTypeFareRuleData->negFareRestExt();
      if (negFareRestExt && (negFareRestExt->fareBasisAmtInd() == RuleConst::NR_VALUE_A ||
                             negFareRestExt->fareBasisAmtInd() == RuleConst::NR_VALUE_B))
        return true;
    }
  }
  return false;
}

bool
PaxTypeFare::applyNonIATARounding(const PricingTrx& trx)
{
  if (_applyNonIATARounding != BLANK)
    return _applyNonIATARounding == YES;

  if (applyNonIATAR(trx, vendor(), carrier(), ruleNumber()))
  {
    _applyNonIATARounding = YES;
    return true;
  }
  _applyNonIATARounding = NO;
  return false;
}

void
PaxTypeFare::setCat35Designator(PricingTrx* trx, std::string& des) const
{
  bool axessCat35Fare = false;

  const NegPaxTypeFareRuleData* negPaxTypeFareRuleData = getNegRuleData();

  if (LIKELY(negPaxTypeFareRuleData != nullptr))
  {
    axessCat35Fare = negPaxTypeFareRuleData->axessCat35Fare();
  }

  if (LIKELY(!axessCat35Fare))
  {
    NegFareRest info = negotiatedInfo();
    des = info.tktDesignator2().c_str();
    // des1 gets final say (precedence)
    if (!info.tktDesignator1().empty())
      des = info.tktDesignator1().c_str();
    // no cat35 modifier
  }
}

void
PaxTypeFare::setDiscountedDesignator(PricingTrx* trx,
                                     MoneyAmount& percent,
                                     const FareCalcConfig* fareCalcConfig,
                                     bool ignCat19ChInf,
                                     bool& needTktCodeDefault,
                                     std::string& modString,
                                     FareBasisCodes& fbCodes) const
{
  bool appendDiscount = false;
  if (discountInfo().farecalcInd() == 'C')
    percent = discountInfo().discPercent();

  if (trx != nullptr)
  {
    if (fareCalcConfig && !ignCat19ChInf)
    {
      ChInDefault(*trx,
                  fareCalcConfig->fcChildInfantFareBasis(),
                  fbCodes.tktCodeDefault,
                  fbCodes.tktDesDefault,
                  appendDiscount);
    }
  }
  if (fbCodes.des.empty())
  {
    fbCodes.des = discountInfo().tktDesignator().c_str();

    if (UNLIKELY(fbCodes.des.empty() && !fbCodes.tktDesDefault.empty()))
    {
      fbCodes.des = fbCodes.tktDesDefault;
      if (fareCalcConfig && fareCalcConfig->fcChildInfantFareBasis() != '2')
      {
        createModifier(modString, MOD3_PERCENT, percent, appendDiscount);
      }
    }
    else
    {
      createModifier(modString, discountInfo().tktDesignatorModifier(), percent, true);
    }
  }

  copyBasis(
      fbCodes.basis, fbCodes.tktCode, discountInfo().resultFareClass(), discountInfo().tktCode());
  createModifier(fbCodes.modCode, discountInfo().tktCodeModifier(), percent, false);

  if (discountInfo().tktDesignator().empty() && discountInfo().tktCode().empty())
    needTktCodeDefault = true;
}

void
PaxTypeFare::setFareByRuleDesignator(MoneyAmount& percent,
                                     std::string& modString,
                                     FareBasisCodes& fbCodes) const
{
  if (fareByRuleInfo().fareInd() == 'C' && percent == NO_PERCENT)
    percent = fareByRuleInfo().percent();

  if (fbCodes.des.empty())
  {
    fbCodes.des = fareByRuleInfo().tktDesignator().c_str();
    if (isDiscounted())
    {
      createModifier(modString, fareByRuleInfo().tktDesignatorModifier(), percent, true);
    }
  }

  if (fareByRuleInfo().resultFareClass1().empty())
  {
    // no basis change, just process tktCode
    copyBasis(fbCodes.basis, fbCodes.tktCode, "", fareByRuleInfo().tktCode());
  }
  else
  {
    // Cat 25 resultFareClass may have "*xyz"; basis change already formatted in fareClass()
    copyBasis(fbCodes.basis, fbCodes.tktCode, fareClass(), fareByRuleInfo().tktCode());
  }

  if (isDiscounted())
  {
    createModifier(fbCodes.modCode, fareByRuleInfo().tktCodeModifier(), percent, false);
  }
}

void
PaxTypeFare::setComponentValidationForCarrier(const uint32_t carrierKey,
                                              bool isAltDates,
                                              const uint64_t& duration)
{
  if (isAltDates)
  {
    _durationFlightBitmapForCarrier = &_durationFlightBitmapPerCarrier[carrierKey];
    _flightBitmapForCarrier = &(*_durationFlightBitmapForCarrier)[duration];

    VecMap<uint64_t, bool>* fltIndependentValidationCallbackPerDuration =
        &_durationFltIndependentValidationCallbackPerCarrier[carrierKey];
    _fareCallbackFVOForCarrier = &(*fltIndependentValidationCallbackPerDuration)[duration];

    VecMap<uint64_t, bool>* fltIndependentValidationPerDuration =
        &_durationFltIndependentValidationPerCarrier[carrierKey];
    _fltIndependentValidationFVOForCarrier = &(*fltIndependentValidationPerDuration)[duration];
  }
  else
  {
    _flightBitmapForCarrier = &_flightBitmapPerCarrier[carrierKey];
    _fareCallbackFVOForCarrier = &_fareCallbackFVOPerCarrier[carrierKey];
    _fltIndependentValidationFVOForCarrier = &_fltIndependentValidationFVOPerCarrier[carrierKey];
  }
}

bool
PaxTypeFare::isFareAndProgramDirectionConsistent(Direction programDirection) const
{
  Direction direction = getDirection();

  return ((direction == Direction::BOTHWAYS) ||
          (programDirection == Direction::BOTHWAYS) ||
          (direction == programDirection));
}

PaxTypeFare::BrandStatus
PaxTypeFare::getBestStatusInAnyBrand(bool useDirectionality) const
{
  PaxTypeFare::BrandStatus bestStatus = BS_FAIL;
  for (const BrandStatusWithDirection& status : _brandStatusWithDirection)
  {
    if (useDirectionality && !isFareAndProgramDirectionConsistent(status.second))
      continue;

    if (status.first == BS_HARD_PASS)
      return BS_HARD_PASS;
    else if (status.first == BS_SOFT_PASS)
      bestStatus = BS_SOFT_PASS;
  }
  return bestStatus;
}

PaxTypeFare::BrandStatus
PaxTypeFare::getBrandStatus(const PricingTrx& trx, const BrandCode* brandCode) const
{
  if(trx.isExchangeTrx() && trx.getRequest()->isBrandedFaresRequest())
    return isValidForExchangeWithBrands(trx, *brandCode);

  if (UNLIKELY(_fareMarket->useDummyFare() && !fallback::fallbackHalfRTPricingForIbf(&trx)))
  {
    // dummy fares are used on artificial legs in half round trip Pricing feature
    // we don't care which brand is used on those legs so we pass all of them
    return BS_HARD_PASS;
  }

  const bool useDirectionality = BrandingUtil::isDirectionalityToBeUsed(trx);

  // in ANY_BRAND_LEG_PARITY path we still need to have at least one hard pass on each leg
  // no matter what brand it is.
  if (*brandCode == ANY_BRAND_LEG_PARITY || *brandCode == ANY_BRAND)
    return getBestStatusInAnyBrand(useDirectionality);

  for (uint16_t i = 0; i < _fareMarket->brandProgramIndexVec().size(); ++i)
  {
    const uint16_t currentBrandIdx = _fareMarket->brandProgramIndexVec()[i];
    if (ShoppingUtil::getBrandCode(trx, currentBrandIdx) == *brandCode)
    {
      TSE_ASSERT(i < _brandStatusWithDirection.size());
      if (useDirectionality)
      {
        if (isFareAndProgramDirectionConsistent(_brandStatusWithDirection[i].second))
          return _brandStatusWithDirection[i].first;
      }
      else
      {
        return _brandStatusWithDirection[i].first;
      }
    }
  }
  return BS_FAIL;
}

PaxTypeFare::BrandStatus
PaxTypeFare::isValidForExchangeWithBrands(const PricingTrx& trx,
                                          const BrandCode& brandCode) const
{
  const RexExchangeTrx& rTrx = static_cast<const RexExchangeTrx&>(trx);

  if (rTrx.getKeepOriginal() && isFromFlownOrNotShoppedFM())
  {
    if (rTrx.getKeepBrandStrategy() == RexExchangeTrx::KEEP_BRAND &&
        notValidated(_keepBrandValidationStatus))
    {
      _keepBrandValidationStatus = false;
      const BrandCode& bc = rTrx.getBrandsUsedInOriginalTicket(_fareMarket);

      for (uint16_t i = 0; i < _fareMarket->brandProgramIndexVec().size(); ++i)
      {
        if (ShoppingUtil::getBrandCode(trx, _fareMarket->brandProgramIndexVec()[i]) == bc)
        {
          if (_brandStatusWithDirection[i].first == BS_HARD_PASS)
            _keepBrandValidationStatus = true;
          break;
        }
      }
    }
    else if (rTrx.getKeepBrandStrategy() == RexExchangeTrx::KEEP_FARE)
    {
      return toBrandStatus(true);
    }

    return toBrandStatus(_keepBrandValidationStatus &&
                         fareAmount() <= rTrx.getMaxPriceForFM(_fareMarket));
  }


  if (!isFromFlownOrNotShoppedFM())
  {
    for (uint16_t i = 0; i < _fareMarket->brandProgramIndexVec().size(); ++i)
    {
      if (ShoppingUtil::getBrandCode(trx, _fareMarket->brandProgramIndexVec()[i]) == brandCode)
        return _brandStatusWithDirection[i].first;
    }
  }
  else
  {
    return toBrandStatus(fareAmount() <= rTrx.getMaxPriceForFM(_fareMarket));
  }

  return BS_FAIL;
}

bool
PaxTypeFare::isFromFlownOrNotShoppedFM() const
{
  //fares in flown FM and not shopped FM should be matched to all brands
  return _fareMarket->isFlown() || !_fareMarket->isShopped();
}

void
PaxTypeFare::setStructuredRuleData(const PaxTypeFareStructuredRuleData& structuredRuleData)
{
  createStructuredRuleDataIfNonexistent();
  _structuredRuleData->setAdvanceReservationWithEarlier(structuredRuleData._advanceReservation);
  _structuredRuleData->setAdvanceTicketingWithEarlier(structuredRuleData._advanceTicketing);
  for (const auto& elem : structuredRuleData._maxStayMap)
    structuredFareRulesUtils::updateMaxStayTrvData(_structuredRuleData->_maxStayMap, elem);
}

int
PaxTypeFare::getValidBrandIndex(const PricingTrx& trx, const BrandCode* brandCode,
                                Direction fareUsageDirection) const
{
  TSE_ASSERT(!isDummyFare()); // Dummy fares are not branded and have no brandStatusVec
  const bool useDirectionality = BrandingUtil::isDirectionalityToBeUsed(trx);
  // if directionality enabled and fare direction is BOTHWAYS (verified inside the loop)
  // we need fare usage (if given != BOTHWAYS) to properly find program (the same brand
  // can be used in two programs in opposite directions, both matching BOTHWAYS fare)
  const bool validateFareUsageDirection = useDirectionality &&
      !fallback::fallbackUseFareUsageDirection(&trx) &&
      (fareUsageDirection != Direction::BOTHWAYS);

  for (uint16_t i = 0; i < _fareMarket->brandProgramIndexVec().size(); ++i)
  {
    if (_brandStatusWithDirection[i].first != BS_FAIL)
    {
      if (!useDirectionality ||
          isFareAndProgramDirectionConsistent(_brandStatusWithDirection[i].second))
      {
        if (validateFareUsageDirection && (getDirection() == Direction::BOTHWAYS))
        {
          // if brand is not bidirectional it must be consistent with fareUsage, otherwise
          // search for next brand
          if ((_brandStatusWithDirection[i].second != Direction::BOTHWAYS) &&
              (_brandStatusWithDirection[i].second != fareUsageDirection))
            continue;
        }
        int brandIndex = _fareMarket->brandProgramIndexVec()[i];
        if (ShoppingUtil::getBrandCode(trx, brandIndex) == *brandCode)
          return brandIndex;
      }
    }
  }

  return INVALID_BRAND_INDEX;
}

bool
PaxTypeFare::isValidForBrand(const PricingTrx& trx, const BrandCode* brandCode,
  bool hardPassedOnly) const
{
  if (UNLIKELY(isDummyFare() && !fallback::fallbackHalfRTPricingForIbf(&trx)))
    return true;

  if (*brandCode == NO_BRAND)
    return true;

  if ((*brandCode == ANY_BRAND) || (*brandCode == ANY_BRAND_LEG_PARITY))
    return hasValidBrands();

  PaxTypeFare::BrandStatus brandStatus = getBrandStatus(trx, brandCode);

  if (brandStatus == BS_FAIL)
    return false;

  if (hardPassedOnly && (brandStatus != BS_HARD_PASS))
    return false;

  return true;
}

bool
PaxTypeFare::isValidForRequestedBrands(const PricingTrx& trx,
                                       const BrandFilterMap& requestedBrands,
                                       bool hardPassedOnly) const
{
  // if brand filter is empty all brands are allowed
  if (requestedBrands.empty())
    return true;

  if (UNLIKELY(isDummyFare() && !fallback::fallbackHalfRTPricingForIbf(&trx)))
    return true;

  for (auto brand : requestedBrands)
  {
    if (isValidForBrand(trx, &(brand.first), hardPassedOnly))
      return true;
  }

  return false;
}

bool
PaxTypeFare::isValidForCabin(const CabinType& reqCabin, bool stayInCabin) const
{
  if (!reqCabin.isValidCabin())
    return true;

  if (_cabin.isValidCabin())
  {
    if (stayInCabin)
    {
      if (_cabin.generalIndex() == reqCabin.generalIndex())
        return true;
    }
    else
    {
      if (_cabin <= reqCabin)
        return true;
    }
  }

  return false;
}

void
PaxTypeFare::getValidBrands(const PricingTrx& trx, std::vector<int>& brands,
  bool hardPassedOnly) const
{
  if (UNLIKELY(isDummyFare() && !fallback::fallbackRTPricingContextFix(&trx)))
  {
    brands = _fareMarket->brandProgramIndexVec();
  }

  TSE_ASSERT(_fareMarket->brandProgramIndexVec().size() == _brandStatusWithDirection.size());

  const bool useDirectionality = BrandingUtil::isDirectionalityToBeUsed(trx);

  for (uint16_t i = 0; i < _fareMarket->brandProgramIndexVec().size(); ++i)
  {
    if (_brandStatusWithDirection[i].first == BS_FAIL)
      continue;
    if ((_brandStatusWithDirection[i].first == BS_SOFT_PASS) && hardPassedOnly)
      continue;
    if (useDirectionality &&
        !isFareAndProgramDirectionConsistent(_brandStatusWithDirection[i].second))
      continue;

    brands.push_back(_fareMarket->brandProgramIndexVec()[i]);
  }
}

void
PaxTypeFare::getValidBrands(const PricingTrx& trx,
                            std::vector<BrandCode>& brands,
                            bool hardPassedOnly) const
{
  if (UNLIKELY(isDummyFare() && !fallback::fallbackRTPricingContextFix(&trx)))
  {
    for (uint16_t i = 0; i < _fareMarket->brandProgramIndexVec().size(); ++i)
      brands.push_back(ShoppingUtil::getBrandCode(trx, _fareMarket->brandProgramIndexVec()[i]));
    return;
  }

  TSE_ASSERT(_fareMarket->brandProgramIndexVec().size() == _brandStatusWithDirection.size());

  const bool useDirectionality = BrandingUtil::isDirectionalityToBeUsed(trx);

  for (uint16_t i = 0; i < _fareMarket->brandProgramIndexVec().size(); ++i)
  {
    if (_brandStatusWithDirection[i].first == BS_FAIL)
      continue;
    if (hardPassedOnly && (_brandStatusWithDirection[i].first == BS_SOFT_PASS))
      continue;
    if (useDirectionality &&
        !isFareAndProgramDirectionConsistent(_brandStatusWithDirection[i].second))
      continue;

    brands.push_back(ShoppingUtil::getBrandCode(trx, _fareMarket->brandProgramIndexVec()[i]));
  }
}

void
PaxTypeFare::getValidBrands(const PricingTrx& trx,
                            std::vector<skipper::BrandCodeDirection>& brands,
                            bool hardPassedOnly) const
{
  if (UNLIKELY(isDummyFare() && !fallback::fallbackRTPricingContextFix(&trx)))
  {
    for (uint16_t brandIndex : _fareMarket->brandProgramIndexVec())
    {
        brands.push_back(skipper::BrandCodeDirection(
                ShoppingUtil::getBrandCode(trx, brandIndex),
                Direction::BOTHWAYS));
    }
    return;
  }

  TSE_ASSERT(_fareMarket->brandProgramIndexVec().size() == _brandStatusWithDirection.size());

  const bool useDirectionality = BrandingUtil::isDirectionalityToBeUsed(trx);

  for (uint16_t i = 0; i < _fareMarket->brandProgramIndexVec().size(); ++i)
  {
    if (_brandStatusWithDirection[i].first == BS_FAIL)
      continue;
    if (hardPassedOnly && (_brandStatusWithDirection[i].first == BS_SOFT_PASS))
      continue;
    if (useDirectionality &&
        !isFareAndProgramDirectionConsistent(_brandStatusWithDirection[i].second))
      continue;

    if (useDirectionality)
      brands.push_back(skipper::BrandCodeDirection(
        ShoppingUtil::getBrandCode(trx, _fareMarket->brandProgramIndexVec()[i]),
        _brandStatusWithDirection[i].second));
    else
      brands.push_back(skipper::BrandCodeDirection(
        ShoppingUtil::getBrandCode(trx, _fareMarket->brandProgramIndexVec()[i]),
        Direction::BOTHWAYS));
  }
}

bool
PaxTypeFare::hasValidBrands(bool hardPassedOnly) const
{
  // Dummy Fares apply to all brands in a given fare market
  if (UNLIKELY(isDummyFare()))
    return !_fareMarket->brandProgramIndexVec().empty();

  for (const BrandStatusWithDirection& status : _brandStatusWithDirection)
  {
    if(status.first == BS_HARD_PASS ||
      (!hardPassedOnly && (status.first == BS_SOFT_PASS)))
      return true;
  }
  return false;
}

BrandCode
PaxTypeFare::getFirstValidBrand(const PricingTrx& trx, Direction fareUsageDirection) const
{
  TSE_ASSERT(_fareMarket->brandProgramIndexVec().size() == _brandStatusWithDirection.size());
  BrandCode validBrand;

  const bool useDirectionality = BrandingUtil::isDirectionalityToBeUsed(trx);
  // if directionality enabled and fare direction is BOTHWAYS (verified inside the loop)
  // we need fare usage (if given != BOTHWAYS) to properly find program (the same brand
  // can be used in two programs in opposite directions, both matching BOTHWAYS fare)
  const bool validateFareUsageDirection = useDirectionality &&
      !fallback::fallbackUseFareUsageDirection(&trx) &&
      (fareUsageDirection != Direction::BOTHWAYS);

  for (uint16_t i = 0; i < _fareMarket->brandProgramIndexVec().size(); ++i)
  {
    if (_brandStatusWithDirection[i].first == BS_SOFT_PASS && validBrand.empty())
    {
      if (!useDirectionality ||
          isFareAndProgramDirectionConsistent(_brandStatusWithDirection[i].second))
      {
        if (validateFareUsageDirection && (getDirection() == Direction::BOTHWAYS))
        {
          // if brand is not bidirectional it must be consistent with fareUsage, otherwise
          // search for next brand
          if ((_brandStatusWithDirection[i].second != Direction::BOTHWAYS) &&
              (_brandStatusWithDirection[i].second != fareUsageDirection))
            continue;
        }
        validBrand = ShoppingUtil::getBrandCode(trx, _fareMarket->brandProgramIndexVec()[i]);
      }
    }
    else if(_brandStatusWithDirection[i].first == BS_HARD_PASS)
    {
      if (!useDirectionality ||
          isFareAndProgramDirectionConsistent(_brandStatusWithDirection[i].second))
      {
        // if directionality enabled and fare direction is bothways we need fare usage
        // (if given) to properly find program (the same brand can be used in two programs
        // in opposite directions, both matching BOTHWAYS fare)
        if (useDirectionality && fareUsageDirection != Direction::BOTHWAYS &&
            getDirection() == Direction::BOTHWAYS)
        {
          // if brand is not bidirectional it must be consistent with fareUsage, otherwise
          // search for next brand
          if ((_brandStatusWithDirection[i].second != Direction::BOTHWAYS) &&
              (_brandStatusWithDirection[i].second != fareUsageDirection))
            continue;
        }
        return ShoppingUtil::getBrandCode(trx, _fareMarket->brandProgramIndexVec()[i]);
      }
    }
  }
  return validBrand;
}

void
PaxTypeFare::setValidatingCxrInvalid(const CarrierCode& cxr, uint16_t failedCatNum)
{
  const RuleState categoryBit = mapRuleStateBit(failedCatNum);

  if (categoryBit == RS_DefinedInFare)
  {
    _fare->addInvalidValidatingCxr(cxr);
  }

  removeFromValidatingCarrierList(cxr);
}

void
PaxTypeFare::setValidatingCxrInvalid(const std::vector<CarrierCode>& cxrs, uint16_t failedCatNum)
{
  const size_t numOfFail = cxrs.size();
  if (numOfFail == 0)
    return;

  const RuleState categoryBit = mapRuleStateBit(failedCatNum);

  if (categoryBit == RS_DefinedInFare)
  {
    _fare->addInvalidValidatingCxr(cxrs);
  }

  if(numOfFail == _validatingCarriers.size())
    _validatingCarriers.clear();
  else
  {
    for (const CarrierCode& cxr : cxrs)
    {
      removeFromValidatingCarrierList(cxr);
    }
  }
}

void
PaxTypeFare::removeFromValidatingCarrierList(const CarrierCode& cxr)
{
  std::vector<CarrierCode>::iterator it = std::find(_validatingCarriers.begin(),
      _validatingCarriers.end(), cxr);
  if (it != _validatingCarriers.end())
    _validatingCarriers.erase(it);
}

void
PaxTypeFare::consolidValidatingCxrList()
{
  uint16_t numOfInvalidVCXR = static_cast<uint16_t>(_fare->invalidValidatingCarriers().size());
  for (uint16_t index = _numOfValidatingCxrFailedFareCatRule;
       index < numOfInvalidVCXR; ++index)
  {
    removeFromValidatingCarrierList(*(_fare->invalidValidatingCarriers().begin() + index));
  }

  _numOfValidatingCxrFailedFareCatRule = numOfInvalidVCXR;
}

uint16_t
PaxTypeFare::getFailedCatNum(std::vector<uint16_t>& catSeq) const
{
  for (uint16_t catNum : catSeq)
  {
    if (!isCategoryValid(catNum))
      return catNum;
  }
  return 0;
}

void
PaxTypeFare::cloneMeForAsBooked(PricingTrx& trx)
{
  if (_rebookInfo._asBookedClone)
    return;

  _rebookInfo._asBookedClone = clone(trx.dataHandle(), false);
  _rebookInfo._asBookedClone->_rebookInfo._iAmAsBookedClone = true;
}

bool
PaxTypeFare::isSoldOut() const
{
  return (_ibfErrorMessage == IbfErrorMessage::IBF_EM_NOT_AVAILABLE || _ibfErrorMessage == IbfErrorMessage::IBF_EM_NOT_OFFERED);
}

bool
PaxTypeFare::canTreatAsWithSameCabin(const PaxTypeFare& rhs) const
{
  return _cabin.canTreatAsSame(rhs._cabin);
}

Direction
PaxTypeFare::getDirection() const
{
  Direction fareDirection = Direction::BOTHWAYS;

  bool isReversedVsFM = false;

  if ((fare()->fareInfo()->market1() == fareMarket()->destination()->loc()) ||
      (fare()->fareInfo()->market1() == fareMarket()->offMultiCity()))
    isReversedVsFM = true;

  if (fare()->fareInfo()->directionality() == FROM)
    fareDirection = Direction::ORIGINAL;
  else if (fare()->fareInfo()->directionality() == TO)
    fareDirection = Direction::REVERSED;

  if (isReversedVsFM)
  {
    if (fareDirection == Direction::ORIGINAL)
      fareDirection = Direction::REVERSED;
    else if (fareDirection == Direction::REVERSED)
      fareDirection = Direction::ORIGINAL;
  }

  return fareDirection;
}

const std::unordered_set<const PenaltyInfo*>&
PaxTypeFare::getPenaltyInfo() const
{
  const MaxPenaltyInfoRecords& recordsReference =
      (!_smpShoppingISCoreFixFallback ? _penaltyRecords : *_penaltyRecordsOld);

  TSE_ASSERT(!_smpShoppingISCoreFixFallback || _penaltyRecordsOld);
  return (isFareByRule() && fareByRuleInfo().ovrdcat16() == 'B' ? baseFare()->getPenaltyInfo()
                                                                : recordsReference._penaltyInfo);
}

const std::unordered_set<const VoluntaryChangesInfoW*>&
PaxTypeFare::getVoluntaryChangesInfo() const
{
  const MaxPenaltyInfoRecords& recordsReference =
      (!_smpShoppingISCoreFixFallback ? _penaltyRecords : *_penaltyRecordsOld);

  TSE_ASSERT(!_smpShoppingISCoreFixFallback || _penaltyRecordsOld);
  return (isFareByRule() && fareByRuleInfo().ovrdcat31() == 'B'
              ? baseFare()->getVoluntaryChangesInfo()
              : recordsReference._voluntaryChangesInfo);
}

const std::unordered_set<const VoluntaryRefundsInfo*>&
PaxTypeFare::getVoluntaryRefundsInfo() const
{
  const MaxPenaltyInfoRecords& recordsReference =
      (!_smpShoppingISCoreFixFallback ? _penaltyRecords : *_penaltyRecordsOld);

  TSE_ASSERT(!_smpShoppingISCoreFixFallback || _penaltyRecordsOld);
  return (isFareByRule() && fareByRuleInfo().ovrdcat33() == 'B'
              ? baseFare()->getVoluntaryRefundsInfo()
              : recordsReference._voluntaryRefundsInfo);
}

bool
PaxTypeFare::isFarePassForInclCode(uint8_t inclusionNumber) const
{
    std::map<uint8_t, bool>::const_iterator i;
    i = _fareStatusPerInclCode.find(inclusionNumber);
    if (i == _fareStatusPerInclCode.end())
      return false;
    else
      return (i->second);
}

} // tse
