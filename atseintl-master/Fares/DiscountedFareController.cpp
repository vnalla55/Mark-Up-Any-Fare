//-------------------------------------------------------------------
//
//  File:        DiscountedFareController.cpp
//  Created:     May 12, 2004
//  Authors:     Mark Kasprowicz
//
//  Description: Discounted fare factory
//
//  Updates:
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
//-------------------------------------------------------------------
#include "Fares/DiscountedFareController.h"

#include "Common/CurrencyCollectionResults.h"
#include "Common/CurrencyConversionRequest.h"
#include "Common/FallbackUtil.h"
#include "Common/FareDisplayUtil.h"
#include "Common/FareMarketUtil.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/NUCCollectionResults.h"
#include "Common/PaxTypeUtil.h"
#include "Common/Rec2Selector.h"
#include "Common/TseEnums.h"
#include "Common/TSELatencyData.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/TseUtil.h"
#include "Common/Vendor.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/Record2Types.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/DiscountSegInfo.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/FootNoteCtrlInfo.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "DBAccess/GeneralRuleApp.h"
#include "DBAccess/Loc.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/PaxTypeInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag202Collector.h"
#include "Rules/AccompaniedTravel.h"
#include "Rules/FareMarketDataAccess.h"
#include "Rules/RuleUtil.h"

#include <iostream>

namespace tse
{
const char DiscountedFareController::CALCULATED = 'C';
const char DiscountedFareController::SPECIFIED = 'S';
const char DiscountedFareController::ADD_CALCULATED_TO_SPECIFIED = 'A';
const char DiscountedFareController::SUBTRACT_SPECIFIED_FROM_CALCULATED = 'M';

const char DiscountedFareController::MATCH_ALWAYS = ' ';
const char DiscountedFareController::MATCH_FARE_CLASS = 'F';
const char DiscountedFareController::MATCH_FARE_TYPE = 'T';
const char DiscountedFareController::MATCH_PAX_TYPE = 'P';
const char DiscountedFareController::MUST_MATCH = 'Y';
const char DiscountedFareController::MUST_NOT_MATCH = 'N';

const std::string DiscountedFareController::BLANK_CURRENCY = "***";
const Indicator DiscountedFareController::REQ_ACC_TVL = ' ';
const Indicator DiscountedFareController::NOT_APPLY = ' ';
const MoneyAmount DiscountedFareController::NO_MIN_AMT = 1E20;

static Logger
logger("atseintl.Fares.DiscountedFareController");

FALLBACK_DECL(fallbackAPO37838Record1EffDateCheck);
FALLBACK_DECL(fallbackFootNoteR2Optimization);

namespace
{

//-------------------------------------------------------------------
// misc utilities
//-------------------------------------------------------------------

bool
eq_pt(const PaxTypeCode& x, const PaxTypeCode& y)
{
  if ((x.empty() && y == ADULT) || (y.empty() && x == ADULT))
    return true;
  return x == y;
}

bool
less_pt(const PaxTypeCode& x, const PaxTypeCode& y)
{
  if (UNLIKELY((x.empty() && y == ADULT) || (y.empty() && x == ADULT)))
    return false;
  return x < y;
}

struct less_paxtype : public std::binary_function<const DiscountedFareController::DiscRule&,
                                                  const DiscountedFareController::DiscRule&,
                                                  bool>
{
  bool operator()(const DiscountedFareController::DiscRule& x,
                  const DiscountedFareController::DiscRule& y)
  {
    return less_pt(x.first->paxType(), y.first->paxType());
  }
};

} // namespace anon

void
DiscountedFareController::FallBackOn::
operator()(DiscountedFareController& dfc,
           FootNoteCtrlInfoVec& fnCtrlInfoVec,
           PricingTrx& trx,
           PaxTypeFare& paxTypeFare,
           const TariffNumber& fareTariff,
           const Footnote& footnote,
           const uint16_t categoryNumber)
{
  dfc.getFootNoteCtrlInfos(paxTypeFare, categoryNumber, footnote, fareTariff, fnCtrlInfoVec);
}

void
DiscountedFareController::FallBackOff::
operator()(DiscountedFareController& dfc,
           FootNoteCtrlInfoVec& fnCtrlInfoVec,
           PricingTrx& trx,
           PaxTypeFare& paxTypeFare,
           const TariffNumber& fareTariff,
           const Footnote& footnote,
           const uint16_t categoryNumber)
{
  fnCtrlInfoVec = Rec2Selector::getFootNoteCtrl(trx,
                                                paxTypeFare,
                                                fareTariff,
                                                footnote,
                                                categoryNumber,
                                                paxTypeFare.fareMarket()->travelDate());
  fnCtrlInfoVec = Rec2Selector::getExactFareMatchFootNoteCtrl(trx, fnCtrlInfoVec, paxTypeFare);
}

void
DiscountedFareController::DiscRules::addRule(PricingTrx& trx,
                                             const DiscountInfo* x,
                                             PaxTypeFareRuleData* y)
{
#ifdef DEBUG_PERF
  TSELatencyData metrics(trx, "PROC ADD DISCRULE");
#endif

  _vec.push_back(DiscRule(x, y));
}

bool
DiscountedFareController::DiscRules::hasPax(const PaxTypeCode& p, int16_t searchVecSz)
{
  if (searchVecSz == 0)
    return false;

  int16_t index = 0;

  for (auto& discRule : *this)
  {
    if (eq_pt(p, discRule.first->paxType()))
      return true;

    if (searchVecSz != DiscountedFareController::DiscRules::CHK_ALL_EXIST_DISCRULE)
    {
      if (++index == searchVecSz)
        break;
    }
  }
  return false;
}

DiscountedFareController::DiscountedFareController(PricingTrx& trx,
                                                   Itin& itin,
                                                   FareMarket& fareMarket)
  : FareController(trx, itin, fareMarket), _ageDefinedInTrxPax(PaxTypeUtil::definedPaxAge(_trx))
{
}

void
DiscountedFareController::getFootNoteCtrlInfos(PaxTypeFare& paxTypeFare,
                                               const uint16_t categoryNumber,
                                               const Footnote& footnote,
                                               const TariffNumber& fareTariff,
                                               FootNoteCtrlInfoVec& fnCtrlInfoVec)
{
#ifdef DEBUG_PERF
  TSELatencyData metrics(_trx, "PROC ONE DISC FOOT");
#endif

  CarrierCode carrier = paxTypeFare.carrier();
  if (paxTypeFare.fare()->isIndustry())
    carrier = INDUSTRY_CARRIER;

  const std::vector<FootNoteCtrlInfo*>& ftnList =
      _trx.dataHandle().getFootNoteCtrl(paxTypeFare.vendor(),
                                        carrier,
                                        fareTariff,
                                        footnote,
                                        categoryNumber,
                                        paxTypeFare.fareMarket()->travelDate());

  // scan the vector and find the first match
  std::vector<FootNoteCtrlInfo*>::const_iterator i = ftnList.begin();
  std::vector<FootNoteCtrlInfo*>::const_iterator j = ftnList.end();

  const FootNoteCtrlInfo* fnci = nullptr;
  bool isLocationSwapped;
  bool isHistorical = _trx.dataHandle().isHistorical();
  size_t findFootNoteCount = 0;
  DateTime& tktDate = _trx.ticketingDate();

  TvlDatesSet tvlDatesSet;
  tvlDatesSet.insert(paxTypeFare.fareMarket()->travelDate());

  std::vector<char> matchedDates(tvlDatesSet.size(), 0); // should be faster than vector<bool>,
  // and more flexible than bitset which is fixed-size

  std::set<FootNoteCtrlInfoPair> uniqueFnci; // to disallow duplicates

  for (; i != j; ++i)
  {
    isLocationSwapped = false;
    fnci = *i;

    if (RuleUtil::matchOneWayRoundTrip(fnci->owrt(), paxTypeFare.owrt()) &&
        RuleUtil::matchFareClass(fnci->fareClass().c_str(), paxTypeFare.fareClass().c_str()) &&
        RuleUtil::matchFareRouteNumber(fnci->routingAppl(), fnci->routing(), paxTypeFare.routingNumber()) &&
        RuleUtil::matchJointCarrier(_trx, paxTypeFare, fnci->jointCarrierTblItemNo()) &&
        RuleUtil::matchLoc_R1_2_6(_trx, fnci->loc1(), fnci->loc2(), paxTypeFare, isLocationSwapped))
    {
      TvlDatesSet::iterator tvlDatesIt = tvlDatesSet.begin();
      for (size_t dit = 0; dit < tvlDatesSet.size(); ++dit)
      {
        if ((fnci->effDate() <= (*tvlDatesIt)) && (fnci->discDate() >= (*tvlDatesIt)))
        {
          if (matchedDates[dit] == 0)
          {
            ++findFootNoteCount;
            uniqueFnci.insert(std::make_pair(fnci, isLocationSwapped));
            matchedDates[dit] = 1;
          }
          else
          {
            if (isHistorical &&
                RuleUtil::matchCreateOrExpireDate(tktDate, fnci->createDate(), fnci->expireDate()))
              uniqueFnci.insert(std::make_pair(fnci, isLocationSwapped));
          }
        }
        tvlDatesIt++;
      }
      if (!isHistorical && findFootNoteCount == tvlDatesSet.size())
        break;
    }
  } // endfor - all fn ctrl infos

  fnCtrlInfoVec.assign(uniqueFnci.begin(), uniqueFnci.end());
}

//-------------------------------------------------------------------
// process
//-------------------------------------------------------------------
bool
DiscountedFareController::process()
{
  DCFactory* factory = DCFactory::instance();
  _diagPtr = factory->create(_trx);
  DiagCollector& diag = *_diagPtr;

  if (UNLIKELY(diag.diagnosticType() == Diagnostic219))
    _diag219On = true;
  else if (UNLIKELY(diag.diagnosticType() == Diagnostic319))
    _diag319On = true;

  // Get all PaxTypeFares from each bucket
  std::vector<PaxTypeFare*>& allFares = _fareMarket.allPaxTypeFare();

  for (auto ptFare : allFares)
  {
    if (checkFare(ptFare, diag))
      fallback::fallbackFootNoteR2Optimization(&_trx)
          ? processFare<DiscountedFareController::FallBackOn>(*ptFare)
          : processFare<DiscountedFareController::FallBackOff>(*ptFare);
  } // endfor - all fares

  LOG4CXX_INFO(logger, "discPaxTypeFares() contains " << _discPaxTypeFares.size() << " fares");

  // If we dont have any discounted fares, we dont output any 219 information
  if (_discPaxTypeFares.empty())
  {
    if (UNLIKELY(_diag219On || _diag319On))
    {
      diag.enable(Diagnostic219, Diagnostic319);
      diag << "**********************************************" << std::endl;
      diag << "NO DISCOUNTED FARES CREATED FOR " << FareMarketUtil::getDisplayString(_fareMarket)
           << " MARKET" << std::endl;
      diag << "**********************************************" << std::endl;
    }
  }
  else
    updateFareMarket(diag);

  _diagPtr->flushMsg();
  return true;
}

bool
DiscountedFareController::getDiscInfoFromRule(PaxTypeFare& ptFare,
                                              const CategoryRuleInfo* ruleInfo,
                                              const bool isLocationSwapped,
                                              bool allowDup)
{
#ifdef DEBUG_PERF
  TSELatencyData metrics(_trx, "PROC GET DISCINFO");
#endif
  size_t searchVecSz = DiscountedFareController::DiscRules::CHK_ALL_EXIST_DISCRULE;
  if (!allowDup)
  {
    // we still allowDUP created with this ruleInfo, so we need to remember end of
    // the existing _discRule
    searchVecSz = _discRules.size();
  }

  std::set<PaxTypeCode> noDiscPax;
  bool existNoDiscPax = false;

  const std::vector<CategoryRuleItemInfoSet*>& ruleItemInfoSets =
      ruleInfo->categoryRuleItemInfoSet();
  std::vector<CategoryRuleItemInfoSet*>::const_iterator m = ruleItemInfoSets.begin();
  std::vector<CategoryRuleItemInfoSet*>::const_iterator n = ruleItemInfoSets.end();

  std::vector<CarrierCode> inputValCxrList;

  inputValCxrList = ptFare.validatingCarriers().empty() ?
                   ptFare.fareMarket()->validatingCarriers() : ptFare.validatingCarriers();

  const DiscountInfo* discountInfo = nullptr;
  bool found = false;
  for (; m != n; ++m)
  {
    CategoryRuleItemInfoSet* ruleItemInfoSet = *m;

    if (UNLIKELY(ruleItemInfoSet == nullptr))
    {
      LOG4CXX_DEBUG(logger,
                    "getDiscInfoFromRule() - Skipping null CategoryRuleItemInfoSet pointer");
      continue;
    }

    bool isSoftPass = false;
    std::vector<CategoryRuleItemInfo>* segQual = nullptr;
    // lint --e{413}
    _trx.dataHandle().get(segQual);

    std::vector<CategoryRuleItemInfo>::iterator o =
        (*ruleItemInfoSet).begin();
    std::vector<CategoryRuleItemInfo>::iterator p = (*ruleItemInfoSet).end();

    processRuleItemInfoSet(*ruleItemInfoSet, *segQual, o, p);
    std::vector<CarrierCode> validValCxrList;

    if (segQual->empty())
    {
      validValCxrList = inputValCxrList;

      LOG4CXX_DEBUG(logger, "getDiscInfoFromRule() - No 'IF' condition found");
    }
    else
    {
      // Validate the if conditions
      if (!validateQualifiers(ptFare, *segQual, ruleInfo->vendorCode(), inputValCxrList, validValCxrList,
                              ruleInfo, nullptr, &isSoftPass))
      {
        LOG4CXX_DEBUG(logger,
                      "getDiscInfoFromRule() - validateQualifiers returned false, skipping");
        continue; // bump to next R2 set
      }

      if(!validValCxrList.empty() && !isSoftPass)
        isSoftPass = isCat15Qualified(*segQual);
    }
    // Now loop through all the records before the if and build
    // discounted fares
    o = (*ruleItemInfoSet).begin();

    for (; o != p; ++o)
    {
      CategoryRuleItemInfo* catRuleItemInfo = &(*o);

      if (UNLIKELY(catRuleItemInfo == nullptr))
      {
        LOG4CXX_DEBUG(logger,
                      "getDiscInfoFromRule() - Skipping null CategoryRuleItemInfo pointer");
        continue;
      }

      // We want to process the then's, or's and else's
      if (UNLIKELY((catRuleItemInfo->relationalInd() == CategoryRuleItemInfo::IF) ||
          (catRuleItemInfo->relationalInd() == CategoryRuleItemInfo::AND)))
      {
        LOG4CXX_DEBUG(logger,
                      "getDiscInfoFromRule() - Skipping unapplicable CategoryRuleItemInfo pointer");
        continue;
      }

      Record3ReturnTypes directionCode;

      // Check for Fare Display transaction
      if (UNLIKELY(isFdTrx()))
      {
        directionCode =
            CategoryRuleItem::isDirectionPassForFD(ptFare, catRuleItemInfo, isLocationSwapped);
      }
      else
      {
        directionCode =
            CategoryRuleItem::isDirectionPass(_trx,
                                              ptFare,
                                              catRuleItemInfo,
                                              isLocationSwapped,
                                              (_trx.itin().size() == 1) ? _trx.itin().front() : nullptr);
      }

      if (directionCode == FAIL)
      {
        LOG4CXX_DEBUG(logger, "getDiscInfoFromRule() - Skipping unapplicable CategoryRuleItemInfo "
                               "failed directionality check");
        continue;
      }

      if (directionCode == SOFTPASS)
        isSoftPass = true;

      discountInfo = getDiscountInfo(ruleInfo, catRuleItemInfo);

      if (discountInfo == nullptr)
      {
        LOG4CXX_DEBUG(logger, "getDiscInfoFromRule() - Skipping null DiscountInfo pointer");
        continue;
      }
      else
      {
        if (!allowDup && _discRules.hasPax(discountInfo->paxType(), searchVecSz))
          continue;

        if (UNLIKELY(existNoDiscPax && noDiscPax.find(discountInfo->paxType()) != noDiscPax.end()))
          continue;

        found = true;
        // may find rule but leave _discRules untouched
        // (needed for footnote precedence)
        if (validate(*_diagPtr, ptFare, *ruleInfo, *discountInfo, *ruleItemInfoSet))
        {
          PaxTypeFareRuleData* ruleData = nullptr;
          _trx.dataHandle().get(ruleData);
          if (UNLIKELY(ruleData == nullptr))
          {
            LOG4CXX_ERROR(logger, "bad alloc for ruleData");
            continue;
          }
          ruleData->categoryRuleInfo() = ruleInfo;
          ruleData->isLocationSwapped() = isLocationSwapped;
          ruleData->categoryRuleItemInfoSet() = ruleItemInfoSet;
          ruleData->categoryRuleItemInfoVec() = segQual;
          ruleData->categoryRuleItemInfo() = catRuleItemInfo;
          ruleData->ruleItemInfo() = discountInfo;
          ruleData->setSoftPassDiscount(isSoftPass);
          ruleData->validatingCarriers() = validValCxrList;

          _discRules.addRule(_trx, discountInfo, ruleData);
          if (UNLIKELY(!isSoftPass && discountInfo->overrideDateTblItemNo() != 0 &&
              discountInfo->discAppl() == 'X'))
          {
            // When 994 used, there could be default rule set allows
            // discount, but we would not want to use
            noDiscPax.insert(discountInfo->paxType());
            existNoDiscPax = true;
          }
        }
      } // endif - good disc info
    } // endfor - all crii
  } // endfor - all in set

  return found;
}

//-------------------------------------------------------------------
// validate()
//-------------------------------------------------------------------
bool
DiscountedFareController::validate(DiagCollector& diag,
                                   PaxTypeFare& ptFare,
                                   const CategoryRuleInfo& ruleInfo,
                                   const DiscountInfo& discountInfo,
                                   const CategoryRuleItemInfoSet& catRuleItemInfoSet)
{
#ifdef DEBUG_PERF
  TSELatencyData metrics(_trx, "FCO DISCFARE VALIDATION");
#endif

  /// @todo does this need to be moved to support dumping all the
  //        fares in the ptFares vector?
  if (UNLIKELY(_diag319On))
    writeDiag319(diag, ptFare, ruleInfo, discountInfo);

  diag.enable(Diagnostic319);
  // unavailtag check
  const Indicator unavailTag = discountInfo.unavailtag();
  if (unavailTag == RuleApplicationBase::dataUnavailable)
  {
    diag << "DISCOUNT DATA UNAVAILABLE\n";
    return false;
  }
  else if (unavailTag == RuleApplicationBase::textOnly)
  {
    diag << "DISCOUNT DATA TEXT ONLY\n";
    return false;
  }

  //----------------------------------------------------------------------------------
  // Loop thru all requested pax type .. check if this record3 discount can be applied
  //----------------------------------------------------------------------------------
  //   const PaxTypeCode& r3PaxType = discountInfo.paxType();
  const CarrierCode& ptCarrier = ptFare.carrier();

  const PaxTypeCode& discPaxType =
      (discountInfo.paxType().empty() || (ptFare.isWebFare() && ruleInfo.categoryNumber() == 22))
          ? tse::ADULT
          : discountInfo.paxType();

  const PaxType* ptItem = PaxTypeUtil::isAnActualPaxInTrx(
      _trx, ptCarrier, discPaxType, discountInfo.minAge(), discountInfo.maxAge());

  if (ptItem == nullptr)
  {
    diag << "DISCOUNT NOT APPLIED TO ANY REQUESTED PSG TYPE" << std::endl;
    return false;
  }

  const PaxTypeInfo& paxTypeInfo = ptItem->paxTypeInfo();
  const PaxTypeCode& requestPsg = ptItem->paxType();
  const uint16_t requestAge = ptItem->age();

  diag << "REQUESTED PAX TYPE MATCH " << requestPsg << " AGE:" << requestAge << std::endl;

  //--------------------------------------------------------------------------
  // Validate Date Override Table
  //--------------------------------------------------------------------------

  if (discountInfo.firstOccur() > 1)
  {
    diag << "PASSENGER OCCURENCES : " << discountInfo.firstOccur()
         << " TO: " << discountInfo.lastOccur() << " - FAIL" << std::endl;

    return false;
  }

  if (discountInfo.overrideDateTblItemNo() != 0)
  {
    diag << "Table994 : " << discountInfo.overrideDateTblItemNo() << std::endl;

    DateTime travelDate;
    RuleUtil::getFirstValidTvlDT(travelDate, ptFare.fareMarket()->travelSeg(), true);

    if (UNLIKELY(travelDate.isOpenDate()))
    {
      // use first travelSeg departure date(+1 logic), setby PricingModelMap
      travelDate = ptFare.fareMarket()->travelSeg().front()->departureDT();
    }
    DateTime bookingDate;
    RuleUtil::getLatestBookingDate(_trx, bookingDate, ptFare);

    if (!(RuleUtil::validateDateOverrideRuleItem(_trx,
                                                 discountInfo.overrideDateTblItemNo(),
                                                 ptFare.vendor(),
                                                 travelDate,
                                                 _trx.getRequest()->ticketingDT(),
                                                 bookingDate,
                                                 &diag,
                                                 Diagnostic319)))
    {
      return false;
    }
  }

  if (UNLIKELY(discountInfo.geoTblItemNo() != 0))
  {
    diag << "Table995 : " << discountInfo.overrideDateTblItemNo() << std::endl;

    RuleUtil::TravelSegWrapperVector applTravelSegment;
    bool origCheck = true;
    bool destCheck = false;
    bool fltStopCheck = false;
    TSICode tsiReturn;
    LocKey locKey1Return;
    LocKey locKey2Return;
    FarePath* fp = nullptr;
    PricingUnit* pu = nullptr;

    if (!(RuleUtil::validateGeoRuleItem(discountInfo.geoTblItemNo(),
                                        ruleInfo.vendorCode(),
                                        RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                                        false,
                                        false,
                                        false,
                                        _trx,
                                        fp,
                                        nullptr,
                                        pu,
                                        &_fareMarket,
                                        _trx.getRequest()->ticketingDT(),
                                        applTravelSegment,
                                        origCheck,
                                        destCheck,
                                        fltStopCheck,
                                        tsiReturn,
                                        locKey1Return,
                                        locKey2Return,
                                        Diagnostic319)))
    {
      return false;
    }
  }

  //--------------------------------------------------------------------------
  // Validate PASSENGER TYPE and AGE in REC3 (cat19, cat20, cat21, cat22)
  //--------------------------------------------------------------------------
  bool match = false;
  if (LIKELY(discPaxType == requestPsg || ((_trx.getOptions()->xoFares() == 'T' && // MAK
                                   (paxTypeInfo.adultInd() == 'Y' && discPaxType == ADULT)) ||
                                   (paxTypeInfo.childInd() == 'Y' && discPaxType == CHILD) ||
                                   (paxTypeInfo.infantInd() == 'Y' && discPaxType == INFANT))))
  {
    if (requestAge == 0)
    {
      match = true;
    }

    if ((discountInfo.minAge() == 0 || requestAge >= discountInfo.minAge()) &&
        (discountInfo.maxAge() == 0 || requestAge <= discountInfo.maxAge()))
    {
      match = true;
    }
  }

  if (UNLIKELY(!match))
  {
    diag << "DISCOUNT NOT APPLIED TO ANY REQUESTED PSG TYPE" << std::endl;
    return false;
  }

  if (discountInfo.discAppl() == 'X') // no Applicable discount?
  {
    diag << "NO APPLICABLE DISCOUNT FOR " << discPaxType << " MIN AGE : " << discountInfo.minAge()
         << " MAX AGE : " << discountInfo.maxAge() << std::endl;
    if (LIKELY(typeid(ruleInfo) != typeid(FootNoteCtrlInfo)))
    { // When FareRule and GenRule have a discount for the same PaxType and
      // FareRule is not applicable, a discount from GenRule should be not applicable too.

      return true;
    }
    return false; // Record 3s within this data string
  }

  bool ret = validateAccompanied(diag, discountInfo, ptFare, ruleInfo, *ptItem) &&
             hasGoodBaseFare(discountInfo, ptFare, diag) &&
             validateCurrency(diag, discountInfo, catRuleItemInfoSet);

  if (ret)
     printPassValidate(diag, ptFare, discountInfo, *ptItem);

  return ret;
}

bool
DiscountedFareController::validateAccompanied(DiagCollector& diag,
                                              const DiscountInfo& discountInfo,
                                              PaxTypeFare& ptFare,
                                              const CategoryRuleInfo& ruleInfo,
                                              const PaxType& ptItem) const
{
  //-------------------------------------------------------------------
  // Validate Accompanied Travel Restriction
  //-------------------------------------------------------------------
  if (discountInfo.accInd() == REQ_ACC_TVL)
  {
    AccompaniedTravel accTvl;

    if (LIKELY(!isFdTrx()))
    {
      diag.flushMsg();
      if (accTvl.validate(_trx, ptItem, discountInfo, ptFare) != PASS)
      {
        if (ptFare.isCmdPricing())
        {
          diag << std::endl << "PASS DISCOUNTED FARE ACCOMPANIED TVL REQ BY CMD PRICING"
               << std::endl;
          // ptFare is not the discounted fare, but we set this flag for
          // the discounted fare to be created
          ptFare.setCmdPrcFailedFlag(ruleInfo.categoryNumber());
          // we can set the flag in base ptFare because we can only have
          // one discountInfo for each category between 19 and 22.
        }
        else
        {
          diag << std::endl << "NO ACCOMPANIED TVL REQ MATCH" << std::endl;
          return false;
        }
      }
    }
    else
    {
      if (accTvl.validate(discountInfo, ptFare) != PASS)
      {
        LOG4CXX_DEBUG(logger,
                      "validate() - Discount not applied "
                          << ptFare.actualPaxType()->paxType() << " DiscPsgType="
                          << discountInfo.paxType() << " fcc=" << ptFare.fareClass());
        return false;
      }
      else
        LOG4CXX_DEBUG(logger,
                      "validate() - Discount applied " << discountInfo.paxType() << " for "
                                                       << ptFare.actualPaxType()->paxType()
                                                       << " fcc=" << ptFare.fareClass());
    }
  }
  else
    LOG4CXX_DEBUG(logger,
                  "validate() - NO ACC Discount applied " << discountInfo.paxType() << " for "
                                                          << ptFare.actualPaxType()->paxType()
                                                          << " fcc=" << ptFare.fareClass());
  return true;
}

bool
DiscountedFareController::validateCurrency(DiagCollector& diag,
                                           const DiscountInfo& discountInfo,
                                           const CategoryRuleItemInfoSet& catRuleItemInfoSet) const
{
  if (discountInfo.fareAmt1() > 0 &&
      discountInfo.cur1() != _itin.originationCurrency() &&
      (discountInfo.fareAmt2() == 0 || discountInfo.cur2() != _itin.originationCurrency()))
  {
    // check we have R3 records with another currency
    for (const CategoryRuleItemInfo& cri : catRuleItemInfoSet)
    {
      const DiscountInfo* di= _trx.dataHandle().getDiscount(discountInfo.vendor(), cri.itemNo(), cri.itemcat());

      if (LIKELY(di))
      {
        if ((!di->cur1().empty() && di->cur1() !=  discountInfo.cur1()) ||
            (!di->cur2().empty() && di->cur2() !=  discountInfo.cur1()))
        {
          diag << "DISCOUNT INFO FAILED MATCH CURRENCY" << std::endl;
          return false;
        }
      }
    }
  }

  return true;
}

void
DiscountedFareController::printPassValidate(DiagCollector& diag,
                                            PaxTypeFare& ptFare,
                                            const DiscountInfo& discountInfo,
                                            const PaxType& ptItem) const
{
  if (UNLIKELY(diag.isActive()))
  {
    if (discountInfo.resultOwrt() != NOT_APPLY && ptFare.owrt() != discountInfo.resultOwrt())
    {
      diag << "ORIGIN OWRT TAG: " << ptFare.owrt()
           << " RESULT OWRT TAG: " << discountInfo.resultOwrt() << std::endl;
    }
    if (!discountInfo.resultBookingCode().empty())
      diag << "RESULT BOOKING CODE: " << discountInfo.resultBookingCode() << std::endl;

    diag << "PASS - DISCOUNT APPLICABLE FOR " << ptItem.paxType() << std::endl;
  }
}

//-------------------------------------------------------------------
// calcAmount()
// results in _calcMoney
//-------------------------------------------------------------------
void
DiscountedFareController::calcAmount(PaxTypeFare& paxTypeFare, const DiscountInfo& discountInfo)
{
#ifdef DEBUG_PERF
  TSELatencyData metrics(_trx, "FCO DISCFARE CALCAMT");
#endif

  _calcMoney.getFromPTF(paxTypeFare);
  switch (discountInfo.farecalcInd())
  {
  case CALCULATED:
    _calcMoney.doPercent(discountInfo.discPercent());
    break;

  case SPECIFIED:
    _calcMoney.getFromSpec(
        discountInfo.fareAmt1(), discountInfo.cur1(), discountInfo.fareAmt2(), discountInfo.cur2());
    break;

  case ADD_CALCULATED_TO_SPECIFIED:
    _calcMoney.doPercent(discountInfo.discPercent());
    _calcMoney.doAdd(
        discountInfo.fareAmt1(), discountInfo.cur1(), discountInfo.fareAmt2(), discountInfo.cur2());
    break;

  case SUBTRACT_SPECIFIED_FROM_CALCULATED:
    _calcMoney.doPercent(discountInfo.discPercent());
    _calcMoney.doMinus(
        discountInfo.fareAmt1(), discountInfo.cur1(), discountInfo.fareAmt2(), discountInfo.cur2());
    break;

  default: // NOP
    break;
  }
}

//-------------------------------------------------------------------
// calcPercentage
//-------------------------------------------------------------------
MoneyAmount
DiscountedFareController::calcPercentage(const MoneyAmount fareAmt, const MoneyAmount percent)
{
  return (fareAmt * (percent / 100.0f));
}

//-------------------------------------------------------------------
// writeDiag319
//-------------------------------------------------------------------
void
DiscountedFareController::writeDiag319(DiagCollector& diag,
                                       const PaxTypeFare& paxTypeFare,
                                       const CategoryRuleInfo& ruleInfo,
                                       const DiscountInfo& discountInfo) const
{
  if (!_diag319On)
    return;

  diag.enable(Diagnostic319);

  diag << "***************************************************************" << std::endl;
  diag << "            ATSE CAT19-22 DISCOUNTS DIAGNOSTICS                " << std::endl;

  //   PHASE:FARE CREATION   R3 ITEM NO:0000008766 CNN
  diag << "  PHASE:FARE CREATION   R3 ITEM NO: " << discountInfo.itemNo() << " "
       << discountInfo.paxType() << " " << std::endl;

  // SMF MSP TE14W9N   R2 : CAT 19: ATP NW 10      8000

  if (paxTypeFare.fare()->isReversed())
    diag << paxTypeFare.destination() << "" << paxTypeFare.origin() << " ";
  else
    diag << paxTypeFare.origin() << "" << paxTypeFare.destination() << " ";

  diag << paxTypeFare.fareClass() << "    R2 : CAT " << ruleInfo.categoryNumber() << " : "
       << ruleInfo.vendorCode() << " " << ruleInfo.carrierCode() << " " << ruleInfo.tariffNumber()
       << "   " << ruleInfo.ruleNumber() << " " << ruleInfo.sequenceNumber() << std::endl;

  // PTC - CNN     MIN AGE - 03  MAX AGE - 11
  diag << "R3 PTC - " << discountInfo.paxType() << " MIN AGE : " << discountInfo.minAge()
       << " MAX AGE :  " << discountInfo.maxAge() << std::endl;

  if (discountInfo.discAppl() == 'X') // no Applicable discount
  {
    diag << "DISCOUNT APPL : X\n";
  }

  // DISCOUNT TYPE : xxxxxx
  diag << "DISCOUNT TYPE : " << discountInfo.farecalcInd() << std::endl;

  // PERCENTAGE : nnn.nn  AMOUNT : nnn.nn
  diag.setf(std::ios::fixed, std::ios::floatfield);
  diag.precision(2);
  diag << " DISC PERCENTAGE : " << std::setw(6) << discountInfo.discPercent() << std::endl;

  // CURR1 : xxx  AMOUNT : nnn.nn
  diag << " DISC AMT - CUR1: " << discountInfo.cur1() << " AMT1: " << discountInfo.fareAmt1()
       << " CUR2: " << discountInfo.cur2() << " AMT2: " << discountInfo.fareAmt2() << std::endl;

  if (discountInfo.owrt() != MATCH_ALWAYS || discountInfo.baseFareInd() != MATCH_ALWAYS ||
      discountInfo.bookingAppl() != MATCH_ALWAYS)
  {
    diag << "DISC CALC WITH FARE OF SAME TARIFF CXR AND\n";
    if (discountInfo.owrt() != MATCH_ALWAYS)
      diag << " OW/RT : " << discountInfo.owrt();
    if (discountInfo.baseFareInd() == MATCH_FARE_CLASS)
      diag << " FARCLS: " << discountInfo.baseFareClass();
    else if (discountInfo.baseFareInd() == MATCH_PAX_TYPE)
      diag << " PAXTYPE: " << discountInfo.basePsgType();
    else if (discountInfo.baseFareInd() == MATCH_FARE_TYPE)
      diag << " FARETYPE: " << discountInfo.baseFareType();
    else if (discountInfo.baseFareInd() != MATCH_ALWAYS)
      diag << " BAD BF IND: " << discountInfo.baseFareInd();

    if (discountInfo.bookingAppl() == MUST_MATCH)
      diag << " MUST BOOK IN: " << discountInfo.bookingCode()[0];
    else if (discountInfo.bookingAppl() == MUST_NOT_MATCH)
      diag << " MUST NOT BOOK IN: " << discountInfo.bookingCode()[0];
    else if (discountInfo.bookingAppl() != MATCH_ALWAYS)
      diag << " BAD BKG IND: " << discountInfo.bookingAppl();

    diag << "\n";
  }
}

//-------------------------------------------------------------------
// writeDiag219
//-------------------------------------------------------------------
void
DiscountedFareController::writeDiag219(DiagCollector& diag, PaxTypeFare& paxTypeFare) const
{
  if (!_diag219On)
    return;

  diag.enable(Diagnostic219);

  diag << "***************************************************************" << std::endl;
  diag << "            CAT19-22 FARE CREATION DIAGNOSITCS                 " << std::endl;

  const PaxTypeFareRuleData* ptfRuleData =
      paxTypeFare.paxTypeFareRuleData(RuleConst::CHILDREN_DISCOUNT_RULE);
  if ((ptfRuleData == nullptr) || (ptfRuleData->categoryRuleInfo() == nullptr))
    return;

  const CategoryRuleItemInfo& catRuleItemInfo = *ptfRuleData->categoryRuleItemInfo();
  const CategoryRuleInfo& categoryRuleInfo = *ptfRuleData->categoryRuleInfo();
  const std::vector<CategoryRuleItemInfo>& qualifiers = *ptfRuleData->categoryRuleItemInfoVec();

  const DiscountInfo& discountInfo =
      dynamic_cast<const DiscountInfo&>(*ptfRuleData->ruleItemInfo());

  diag.setf(std::ios::fixed, std::ios::floatfield);

  diag.precision(2);

  // C09 MSP-SMF NW  CNN 03 11       KE14NRN      345.00
  if (paxTypeFare.fare()->isReversed())
    diag << paxTypeFare.destination() << "" << paxTypeFare.origin() << " ";
  else
    diag << paxTypeFare.origin() << "" << paxTypeFare.destination() << " ";

  diag << paxTypeFare.carrier() << "   " << discountInfo.paxType() << " " << discountInfo.minAge()
       << " " << discountInfo.maxAge() << "       " << paxTypeFare.fareClass() << "      "
       << Money(paxTypeFare.fareAmount(), paxTypeFare.currency()) << std::endl;

  // R2: ATP NW 10      8000 6000000
  diag << "R2: " << categoryRuleInfo.vendorCode() << " " << categoryRuleInfo.carrierCode() << " "
       << categoryRuleInfo.tariffNumber() << "      " << categoryRuleInfo.ruleNumber() << " "
       << categoryRuleInfo.sequenceNumber() << std::endl;

  // R3: THEN  019  0000093322
  if (&catRuleItemInfo == nullptr) // lint !e506
  {
    diag << "R3: MISSING" << std::endl;
    return;
  }

  diag << "R3: " << diag.getRelationString(catRuleItemInfo.relationalInd()) << " CAT"
       << catRuleItemInfo.itemcat() << "  ITEM:" << catRuleItemInfo.itemNo()
       << "   DIR:" << catRuleItemInfo.directionality() << std::endl;

  // QUALIFIER: NONE
  // QUALIFIER: IF 4-1111 OR 4-2222
  diag << "QUALIFIER:";

  if (qualifiers.empty())
  {
    diag << " NONE";
  }
  else
  {
    std::vector<CategoryRuleItemInfo>::const_iterator i = qualifiers.begin();
    std::vector<CategoryRuleItemInfo>::const_iterator j = qualifiers.end();

    for (; i != j; ++i)
    {
      const CategoryRuleItemInfo* info = &(*i);

      if (info == nullptr)
        continue;

      diag << " " << diag.getRelationString(info->relationalInd()) << " " << info->itemcat() << "-"
           << info->itemNo();
    }
  }

  diag << std::endl;

  // R3 CATEGORY 19-22 RESULTING TICKETING DESIGNATOR: CH75

  const TktCodeModifier& tktCodeModifier = discountInfo.tktCodeModifier();
  if (!tktCodeModifier.empty())
  {
    diag << "R3 CATEGORY 19-22 RESULTING TICKETING DESIGNATOR: " << tktCodeModifier << std::endl;
  }
}

void
DiscountedFareController::addFare(PaxTypeFare* newPTFare)
{
#ifdef DEBUG_PERF
  TSELatencyData metrics(_trx, "PROC ADD DISCFARE");
#endif

  _discPaxTypeFares.push_back(newPTFare);
}

PaxTypeFare*
DiscountedFareController::makeFare(PaxTypeFare* currPTFare, DiscRule& discRule, CalcMoney& cMoney)
{
#ifdef DEBUG_PERF
  TSELatencyData metrics(_trx, "PROC MAKE DISCFARE");
#endif
  if (cMoney.nucValue() >= NO_MIN_AMT)
    return nullptr;

  const DiscountInfo* discountInfo = discRule.first;

  // Create a new PaxTypeFare
  PaxTypeFare* newPTFare = nullptr;
  Fare* newFare = nullptr;
  FareInfo* newFareInfo = nullptr;
  FareClassAppSegInfo* fcasInfo = nullptr;
  PaxTypeFareRuleData* ruleData = nullptr;

  bool rexSecondAmtNeeded =
      RexPricingTrx::isRexTrxAndNewItin(_trx) &&
      static_cast<RexPricingTrx&>(_trx).applyReissueExchange() &&
      !static_cast<RexBaseTrx&>(_trx).newItinSecondROEConversionDate().isEmptyDate();

  makeAlloc(currPTFare, &newPTFare, &newFare, &newFareInfo, &fcasInfo, &ruleData);

  // put discount details in rec1 area if not already set by FBR
  fcasInfo->_minAge = discountInfo->minAge();
  fcasInfo->_maxAge = discountInfo->maxAge();
  if (!discountInfo->tktCode().empty() &&
      (fcasInfo->_tktCode.empty() || !newPTFare->isFareByRule()))
  {
    fcasInfo->_tktCode = discountInfo->tktCode();
    fcasInfo->_tktCodeModifier = discountInfo->tktCodeModifier();
  }
  if (!discountInfo->tktDesignator().empty())
  {
    fcasInfo->_tktDesignator = discountInfo->tktDesignator();
    fcasInfo->_tktDesignatorModifier = discountInfo->tktDesignatorModifier();
  }
  if (!discountInfo->paxType().empty() && discountInfo->paxType() != ADULT)
  {
    fcasInfo->_paxType = discountInfo->paxType();

    PaxType* paxType = const_cast<PaxType*>(
        PaxTypeUtil::isAnActualPaxInTrx(_trx, newPTFare->carrier(), fcasInfo->_paxType));
    if (LIKELY(paxType))
    {
      newPTFare->actualPaxType() = paxType;
    }
  }

  if (UNLIKELY(discountInfo->resultOwrt() != NOT_APPLY && newFareInfo->owrt() != discountInfo->resultOwrt()))
  {
    if (discountInfo->resultOwrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
    {
      // tag1 or 3 OW changed to tag 2 RT
      newFareInfo->originalFareAmount() = 2.0f * newFareInfo->originalFareAmount();
      newFare->nucOriginalFareAmount() = 2.0f * newFare->nucOriginalFareAmount();
      if (rexSecondAmtNeeded)
        newFare->rexSecondNucOriginalFareAmount() =
            2.0f * newFare->rexSecondNucOriginalFareAmount();
    }
    else if (newFareInfo->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
    {
      // RT change to tag 1 or 3 OW
      newFareInfo->originalFareAmount() = newFareInfo->fareAmount();
      newFare->nucOriginalFareAmount() = newFare->nucFareAmount();
      if (rexSecondAmtNeeded)
        newFare->rexSecondNucOriginalFareAmount() = newFare->rexSecondNucFareAmount();
    }

    newFareInfo->owrt() = discountInfo->resultOwrt();
  }

  if (UNLIKELY(!discountInfo->resultBookingCode().empty()))
  {
    fcasInfo->_bookingCode[0] = discountInfo->resultBookingCode();
    for (int index = 1; index < FareClassAppSegInfo::BK_CODE_SIZE; index++)
    {
      fcasInfo->_bookingCode[index] = "";
    }
  }

  ruleData->categoryRuleInfo() = discRule.second->categoryRuleInfo();
  ruleData->isLocationSwapped() = discRule.second->isLocationSwapped();
  ruleData->categoryRuleItemInfoSet() = discRule.second->categoryRuleItemInfoSet();
  ruleData->categoryRuleItemInfoVec() = discRule.second->categoryRuleItemInfoVec();
  ruleData->categoryRuleItemInfo() = discRule.second->categoryRuleItemInfo();
  ruleData->ruleItemInfo() = discRule.first;
  ruleData->setSoftPassDiscount(discRule.second->isSoftPassDiscount());
  newPTFare->validatingCarriers() = discRule.second->validatingCarriers();

  if (discountInfo->accInd() == REQ_ACC_TVL)
  {
    if (discountInfo->fareClassBkgCodeInd() != NOT_APPLY ||
        discountInfo->accTvlSameRule() != NOT_APPLY)
    {
      // need revalidation during combinability process
      newPTFare->setAccSameFareBreak(true);
    }
    else if (discountInfo->accTvlSameCpmt() != NOT_APPLY)
    {
      newPTFare->setAccSameCabin(true);
    }

    newPTFare->setAccTvlWarning(true);
  }
  cMoney.setRT(newPTFare->isRoundTrip());

  cMoney.putIntoPTF(*newPTFare, *newFareInfo);
  return newPTFare;
}
//-------------------------------------------------------------------
// makeAlloc()
//-------------------------------------------------------------------

void
DiscountedFareController::makeAlloc(PaxTypeFare* ptFare,
                                    PaxTypeFare** newPTFare,
                                    Fare** newFare,
                                    FareInfo** fareInfo,
                                    FareClassAppSegInfo** fcasInfo,
                                    PaxTypeFareRuleData** ruleData)
{
#ifdef DEBUG_PERF
  TSELatencyData metrics(_trx, "DISC ALLOC");
#endif
  *newPTFare = ptFare->clone(_trx.dataHandle());
  *newFare = ptFare->fare()->clone(_trx.dataHandle());
  *fareInfo = ptFare->fare()->fareInfo()->clone(_trx.dataHandle());
  *fcasInfo = ptFare->fareClassAppSegInfo()->clone(_trx.dataHandle());

  _trx.dataHandle().get(*ruleData);

  if (UNLIKELY(newPTFare == nullptr || *newFare == nullptr || *fareInfo == nullptr || *fcasInfo == nullptr || *ruleData == nullptr))
  {
    LOG4CXX_ERROR(logger, "makeAlloc() - bad alloc");
    throw("bad alloc in DFC::makeAlloc()");
  }

  (*newFare)->setFareInfo(*fareInfo);
  (*newPTFare)->setFare(*newFare);
  (*newPTFare)->fareClassAppSegInfo() = *fcasInfo;
  (*ruleData)->baseFare() = ptFare;

  // If faredisplay trx, create a FareDisplayInfo in the new paxtypefare.
  // Clone FareDisplayInfo object from the basefare so the new paxtypefare
  // do not share this with base paxtypefare
  if (UNLIKELY(isFdTrx()))
  {
    if (!FareDisplayUtil::initFareDisplayInfo(_fdTrx, **newPTFare))
    {
      LOG4CXX_ERROR(logger,
                    "DiscountedFareController::createFares - Unable to init FareDisplayInfo");
    }
    else
    {
      (ptFare->fareDisplayInfo())->clone((*newPTFare)->fareDisplayInfo(), *newPTFare);
    }
  }

  // all discount categories stored in one place (cat19)
  (*newPTFare)->setRuleData(RuleConst::CHILDREN_DISCOUNT_RULE, _trx.dataHandle(), *ruleData);

  (*newPTFare)->status().set(PaxTypeFare::PTF_Discounted);

  // turn off revalidation
  (*newPTFare)->setCategoryProcessed(RuleConst::CHILDREN_DISCOUNT_RULE, true);
  (*newPTFare)->setCategoryProcessed(RuleConst::TOUR_DISCOUNT_RULE, true);
  (*newPTFare)->setCategoryProcessed(RuleConst::AGENTS_DISCOUNT_RULE, true);
  (*newPTFare)->setCategoryProcessed(RuleConst::OTHER_DISCOUNT_RULE, true);
}

bool
DiscountedFareController::makeInfFare(PaxTypeFare& ptFare)
{
#ifdef DEBUG_PERF
  TSELatencyData metrics(_trx, "MAKEINF FARE");
#endif

  PaxTypeFare* newPTFare = nullptr;
  Fare* newFare = nullptr;
  FareInfo* fareInfo = nullptr;
  FareClassAppSegInfo* fcasInfo = nullptr;
  PaxTypeFareRuleData* ruleData = nullptr;

  makeAlloc(&ptFare, &newPTFare, &newFare, &fareInfo, &fcasInfo, &ruleData);

  fareInfo->originalFareAmount() = 0;
  fareInfo->fareAmount() = 0;
  newFare->nucFareAmount() = 0;
  newFare->rexSecondNucFareAmount() = 0;

  DiscountInfo* di;
  _trx.dataHandle().get(di); // lint !e530
  if (di == nullptr)
  {
    LOG4CXX_ERROR(logger, "makeInfFare() - bad discInfo alloc for default INF fare");
    return true;
  }
  ruleData->ruleItemInfo() = di;

  fcasInfo->_paxType = INFANT;
  fcasInfo->_minAge = 0;
  fcasInfo->_maxAge = 0;
  const PaxType* infPax =
      PaxTypeUtil::isAnActualPaxInTrx(_trx, _fareMarket.governingCarrier(), INFANT);
  if (!infPax)
  {
    LOG4CXX_WARN(logger, "MakeInfFare() - could not get valid INF PaxType");
    return false;
  }
  newPTFare->paxType() = const_cast<PaxType*>(infPax);
  newPTFare->validatingCarriers() = ptFare.validatingCarriers();

  addFare(newPTFare);

  return true; // Success
}

bool
DiscountedFareController::hasGoodBaseFare(const DiscountInfo& discountInfo,
                                          const PaxTypeFare& ptFare,
                                          DiagCollector& diag)
{
  if (LIKELY(discountInfo.betweenMarket().empty()))
  {
    if (discountInfo.owrt() == MATCH_ALWAYS && discountInfo.baseFareInd() == MATCH_ALWAYS &&
        discountInfo.bookingAppl() == MATCH_ALWAYS)
    {
      return true;
    }

    // will use match fare specified
    // discountInfo must use percentage
    if (discountInfo.farecalcInd() == CALCULATED)
    {
      PaxTypeFare* calcBaseFare = LIKELY(_trx.isFootNotePrevalidationAllowed()) ?
          findBaseFare(ptFare, discountInfo) : findCheapestMatchFare(ptFare.fareMarket()->allPaxTypeFare(), ptFare, discountInfo);

      if (calcBaseFare)
      {
        _discountCalcHelper.insert(
            std::pair<const DiscountInfo*, PaxTypeFare*>(&discountInfo, calcBaseFare));

        if (diag.isActive())
          diag << " MATCHED BASE FARE FOR DISCOUNT CALCULATION: " << calcBaseFare->fareClass() << "\n";

        return true;
      }
    }
  }

  diag << "DISCOUNT INFO FAILED MATCH BASE FARE" << std::endl;
  return false; // do not support Between City And City now
}

//-------------------------------------------------------------------
// writeBaseFaresDiag319
//-------------------------------------------------------------------
void
DiscountedFareController::writeBaseFaresDiag319(DiagCollector& diag,
                                                std::vector<PaxTypeFare*>& ptFares) const
{
  if (!_diag319On)
    return;

  diag.enable(Diagnostic319);

  std::vector<PaxTypeFare*>::iterator i = ptFares.begin();
  std::vector<PaxTypeFare*>::iterator j = ptFares.end();

  for (; i != j; ++i)
  {
    PaxTypeFare* paxFare = *i;

    diag << "  " << std::setw(2)
         << (paxFare->vendor() == Vendor::ATPCO ? "A"
                                                : (paxFare->vendor() == Vendor::SITA ? "S" : "?"))
         << std::setw(5) << paxFare->ruleNumber() << std::setw(13) << paxFare->fareClass()
         << std::setw(4) << paxFare->fareTariff() << std::setw(2)
         << (paxFare->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED ? "R" : "O") << std::setw(2)
         << (paxFare->directionality() == FROM ? "O" : "I");

    diag.setf(std::ios::right, std::ios::adjustfield);
    diag.setf(std::ios::fixed, std::ios::floatfield);
    diag.precision(2);
    diag << std::setw(8) << paxFare->fareAmount();

    diag.setf(std::ios::left, std::ios::adjustfield);
    diag << " " << std::setw(4) << paxFare->currency();

    diag << std::setw(4) << paxFare->fcaFareType();

    diag << std::setw(4) << paxFare->fcasPaxType();
    diag << std::endl;
  }
}

//-------------------------------------------------------------------
// writeDiagnostics()
//-------------------------------------------------------------------
void
DiscountedFareController::writeDiagnostics() const
{
  Diagnostic& trxDiag = _trx.diagnostic();

  if (trxDiag.diagnosticType() != Diagnostic219)
    return;

  DCFactory* factory = DCFactory::instance();
  DiagCollector* diagPtr = factory->create(_trx);
  DiagCollector& diag = *diagPtr;

  std::vector<PaxTypeFare*>& allFares = _fareMarket.allPaxTypeFare();

  std::vector<PaxTypeFare*>::iterator i = allFares.begin();
  std::vector<PaxTypeFare*>::iterator j = allFares.end();

  for (; i != j; ++i)
  {
    PaxTypeFare* ptFare = *i;
    if (ptFare == nullptr)
      continue;

    writeDiag219(diag, *ptFare);
  }
}

bool
DiscountedFareController::getGenParam(const GeneralFareRuleInfo* ruleInfo,
                                      PaxTypeFare& paxTypeFare,
                                      const uint16_t categoryNumber,
                                      TariffNumber& genTariff,
                                      RuleNumber& genNumber)
{
  if (ruleInfo == nullptr)
  {
    CarrierCode carrier = paxTypeFare.carrier();
    if (paxTypeFare.fare()->isIndustry())
      carrier = INDUSTRY_CARRIER;

    // Try the 'default' values
    if(!fallback::fallbackAPO37838Record1EffDateCheck(&_trx))
    {
      return _trx.dataHandle().getGeneralRuleAppTariffRuleByTvlDate(paxTypeFare.vendor(),
                                                                    carrier,
                                                                    paxTypeFare.tcrRuleTariff(),
                                                                    RuleConst::NULL_GENERAL_RULE,
                                                                    categoryNumber,
                                                                    genNumber,
                                                                    genTariff,
                                                                    paxTypeFare.fareMarket()->travelDate());
    }
    else
    {
      return _trx.dataHandle().getGeneralRuleAppTariffRule(paxTypeFare.vendor(),
                                                           carrier,
                                                           paxTypeFare.tcrRuleTariff(),
                                                           RuleConst::NULL_GENERAL_RULE,
                                                           categoryNumber,
                                                           genNumber,
                                                           genTariff);
    }
  }

  if (ruleInfo->applInd() == RuleConst::STRING_DOES_NOT_APPLY)
    return false;

  // from record0...
  if (ruleInfo->generalRule() == RuleConst::NULL_GENERAL_RULE &&
      ruleInfo->generalRuleAppl() != RuleConst::RECORD_0_NOT_APPLY)
  {
    GeneralRuleApp* gra = RuleUtil::getGeneralRuleApp(_trx, paxTypeFare, categoryNumber);
    if (gra == nullptr)
      return false;

    genTariff = gra->generalRuleTariff();
    genNumber = gra->generalRule();
  }
  // ...or from record2
  else
  {
    genTariff = ruleInfo->generalRuleTariff();
    genNumber = ruleInfo->generalRule();
  }
  return true;
}

PaxTypeFare*
DiscountedFareController::findBaseFare(const PaxTypeFare& rulePTF,
                                       const DiscountInfo& discountInfo)
{
  PaxTypeFare* matchedPTF = findCheapestMatchFare(_fareMarket.allPaxTypeFare(), rulePTF, discountInfo);

  if (_fareMarket.footNoteFailedFares().size())
  {
    struct
    {
        bool operator()(const std::pair<Fare*, bool>& lhf, const std::pair<Fare*, bool>& rhf)
        {
          return lhf.first->nucFareAmount() < rhf.first->nucFareAmount();
        }
    } fareComparator;

    if (UNLIKELY(!_footNoteFailedFaresSorted))
    {
      if (_fareMarket.footNoteFailedFares().size() > 1)
      {
        std::sort(_fareMarket.footNoteFailedFares().begin(),
                  _fareMarket.footNoteFailedFares().end(),
                  fareComparator
                  );
      }
      _footNoteFailedFaresSorted = true;
    }

    auto cheapestFaresEnd =  matchedPTF ? std::lower_bound(
          _fareMarket.footNoteFailedFares().begin(),
          _fareMarket.footNoteFailedFares().end(),
          std::make_pair(matchedPTF->fare(), false),
          fareComparator) : _fareMarket.footNoteFailedFares().end();

    for (auto fareIter = _fareMarket.footNoteFailedFares().begin(); fareIter < cheapestFaresEnd; ++fareIter)
    {
      if (fareIter->second)
        continue;

      resolveFareClassApp(*(fareIter->first), _fareMarket.footNoteFailedPaxTypeFares());
      fareIter->second = true;
    }

    PaxTypeFare* matchedInvalidPTF = findCheapestMatchFare(_fareMarket.footNoteFailedPaxTypeFares(), rulePTF, discountInfo);

    if (!matchedPTF || (matchedInvalidPTF && matchedInvalidPTF->nucFareAmount() < matchedPTF->nucFareAmount()))
      return matchedInvalidPTF;
  }
  return matchedPTF;
}


PaxTypeFare*
DiscountedFareController::findCheapestMatchFare(const std::vector<PaxTypeFare*>& allPTF,
                                                const PaxTypeFare& rulePTF,
                                                const DiscountInfo& discountInfo) const
{
  PaxTypeFare* matchedPTF = nullptr;

  std::vector<PaxTypeFare*>::const_iterator ptfIter = allPTF.begin();
  std::vector<PaxTypeFare*>::const_iterator ptfIterEnd = allPTF.end();

  for (; ptfIter != ptfIterEnd; ptfIter++)
  {
    PaxTypeFare& ptFare = **ptfIter;

    // when this is called no ptFare should be discounted fare, so no need
    // if (ptFare.isDiscounted()) continue;

    if (matchedPTF != nullptr)
    {
      if (matchedPTF->nucFareAmount() < (ptFare.nucFareAmount() + EPSILON))
        continue;
    }

    if (ptFare.vendor() != rulePTF.vendor() || ptFare.tcrRuleTariff() != rulePTF.tcrRuleTariff() ||
        ptFare.carrier() != rulePTF.carrier() || ptFare.currency() != rulePTF.currency() ||
        !matchFareDirectionality(ptFare.fare()->fareInfo()->directionality(),
                                 rulePTF.fare()->fareInfo()->directionality()))
      continue;

    // now if this ptFare matches it will be used as matchedPTF for discount
    // calculation
    bool matched = false;

    switch (discountInfo.baseFareInd())
    {
    case MATCH_ALWAYS:
      matched = true;
      break;

    case MATCH_FARE_CLASS:
      matched = RuleUtil::matchFareClass(discountInfo.baseFareClass().c_str(),
                                         ptFare.fareClass().c_str());
      break;

    case MATCH_PAX_TYPE:
      matched = (ptFare.fcasPaxType() == discountInfo.basePsgType()) ||
                (ptFare.fcasPaxType().empty() && discountInfo.basePsgType() == ADULT);
      break;

    case MATCH_FARE_TYPE:
      matched = RuleUtil::matchFareType(discountInfo.baseFareType(), ptFare.fcaFareType());
      break;

    default:
      LOG4CXX_INFO(logger, "findCheapestMatchFare() - rejecting match of unknown baseFareInd");
      return nullptr;
    }

    if (!matched)
      continue;

    switch (discountInfo.bookingAppl())
    {
    case MATCH_ALWAYS:
      break;

    case MUST_MATCH:
      // check only the 1st char of fare class
      matched = (ptFare.fareClass()[0] == discountInfo.bookingCode()[0]);
      break;

    case MUST_NOT_MATCH:
      // check only the 1st char of fare class
      matched = (ptFare.fareClass()[0] != discountInfo.bookingCode()[0]);
      break;

    default:
      LOG4CXX_INFO(logger, "findCheapestMatchFare() - rejecting match of unknown bookingAppl");
      return nullptr;
    }

    if (!matched)
      continue;

    if (discountInfo.owrt() != ' ')
    {
      if (((ptFare.owrt() == ONE_WAY_MAY_BE_DOUBLED ||
            ptFare.owrt() == ONE_WAY_MAYNOT_BE_DOUBLED) &&
           (discountInfo.owrt() != ONE_WAY_MAY_BE_DOUBLED)) ||
          (ptFare.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED &&
           discountInfo.owrt() != ROUND_TRIP_MAYNOT_BE_HALVED))
      {
        matched = false;
      }
    }

    if (matched)
      matchedPTF = &ptFare;
  }

  return matchedPTF;
}

bool
DiscountedFareController::matchFareDirectionality(const Directionality& basePTFDir,
                                                  const Directionality& rulePTFDir) const
{
  if (basePTFDir != BOTH && rulePTFDir != BOTH && basePTFDir != rulePTFDir)
    return false;
  return true;
}

bool
DiscountedFareController::discAgeSensitive(const DiscountInfo& discInfo, const CarrierCode& carrier)
{
  if (UNLIKELY(discInfo.minAge() == 0 && discInfo.maxAge() == 0))
    return false;

  const PaxTypeCode& discPaxCode = discInfo.paxType();

  std::map<PaxTypeCode, bool>::iterator sensitiveMapI = _paxAgeSensitiveMap.find(discPaxCode);
  if (sensitiveMapI != _paxAgeSensitiveMap.end())
    return (sensitiveMapI->second);

  std::vector<PaxType*> matchedPaxTypeInTrx;

  if (!PaxTypeUtil::getActualPaxInTrx(_trx, carrier, discPaxCode, matchedPaxTypeInTrx))
  {
    _paxAgeSensitiveMap[discPaxCode] = false;
    return false;
  }

  std::vector<PaxType*>::const_iterator paxTypeI = matchedPaxTypeInTrx.begin();
  const std::vector<PaxType*>::const_iterator paxTypeIEnd = matchedPaxTypeInTrx.end();
  for (; paxTypeI != paxTypeIEnd; paxTypeI++)
  {
    if ((*paxTypeI)->age() != 0)
    {
      _paxAgeSensitiveMap[discPaxCode] = true;
      return true;
    }
  }
  _paxAgeSensitiveMap[discPaxCode] = false;
  return false;
}

void
DiscountedFareController::processGeneralRule(const GeneralFareRuleInfo* gfrInfo,
                                             PaxTypeFare& ptFare,
                                             const uint16_t cat)
{
  TariffNumber genTariff;
  RuleNumber genNumber;
  const GeneralFareRuleInfo* ruleInfo = nullptr;
  bool isLocationSwapped = false;
  if (getGenParam(gfrInfo, ptFare, cat, genTariff, genNumber))
  {
    // get discounts from general (global) rule
    GeneralFareRuleInfoVec grInfoVec;

    RuleUtil::getGeneralFareRuleInfo(_trx, ptFare, cat, grInfoVec, &genTariff, &genNumber);

    if (!grInfoVec.empty())
    {
      GeneralFareRuleInfoVec::iterator grIter = grInfoVec.begin();
      GeneralFareRuleInfoVec::iterator grIterEnd = grInfoVec.end();
      for (; grIter != grIterEnd; ++grIter)
      {
        ruleInfo = (*grIter).first;
        isLocationSwapped = (*grIter).second;

        getDiscInfoFromRule(ptFare, ruleInfo, isLocationSwapped, false);
        if (UNLIKELY(isFdTrx()))
          break;
      }
    }
  }
}

const DiscountInfo*
DiscountedFareController::getDiscountInfo(const CategoryRuleInfo* ruleInfo,
                                          const CategoryRuleItemInfo* catRuleItemInfo)
{
  return _trx.dataHandle().getDiscount(
      ruleInfo->vendorCode(), catRuleItemInfo->itemNo(), catRuleItemInfo->itemcat());
}

template <typename FallBackSwitch>
void
DiscountedFareController::processFare(PaxTypeFare& ptFare)
{
  uint16_t beginCat = RuleConst::CHILDREN_DISCOUNT_RULE;
  uint16_t endCat = RuleConst::OTHER_DISCOUNT_RULE;

  /* runs twice as fast if can only do cat20-22 */
  if (!_fareMarket.isChildNeeded() && !_fareMarket.isInfantNeeded())
  {
    LOG4CXX_DEBUG(
        logger, "DFC::processGenFareRule() - Skipping cat 19, no child or infant pax types exist");
    beginCat = RuleConst::TOUR_DISCOUNT_RULE;
  }
  // start new list of discInfo for this PTF
  _discRules.clear();
  _discountCalcHelper.clear();

  const GeneralFareRuleInfo* ruleInfo = nullptr;
  std::vector<Footnote> footnoteCodes;
  std::vector<TariffNumber> fareTariffs;
  bool isLocationSwapped = false;
  uint16_t cat = beginCat;
  bool isFootFound = false;

  RuleUtil::getFootnotes(ptFare, footnoteCodes, fareTariffs);

  bool hasPaxInf = false;

  FallBackSwitch fallBackSwitch;

  // if a footnote applies, no fare or general rule used for that category
  for (; cat <= endCat; cat++)
  {
    isLocationSwapped = false;
    isFootFound = false;
    ruleInfo = nullptr;

    if (!footnoteCodes.empty())
    {
      const FootNoteCtrlInfo* footNote = nullptr;
      std::vector<Footnote>::const_iterator ftnt = footnoteCodes.begin();
      std::vector<Footnote>::const_iterator ftntEnd = footnoteCodes.end();
      std::vector<TariffNumber>::const_iterator fareTariffI = fareTariffs.begin();
      std::vector<TariffNumber>::const_iterator fareTariffEnd = fareTariffs.end();

      // get ALL footnotes
      for (; (ftnt != ftntEnd) && (fareTariffI != fareTariffEnd); ++ftnt, ++fareTariffI)
      {
        FootNoteCtrlInfoVec fnCtrlInfoVec;
        fallBackSwitch(*this, fnCtrlInfoVec, _trx, ptFare, *fareTariffI, *ftnt, cat);

        Diag202Collector* dc =
            Diag202Collector::makeMe(_trx, &Diag202Collector::FNT, ptFare.fareMarket(), &ptFare);
        if (UNLIKELY(dc))
        {
          const CarrierCode carrier =
              ptFare.fare()->isIndustry() ? INDUSTRY_CARRIER : ptFare.carrier();

          const std::vector<FootNoteCtrlInfo*>& ftnList =
              (fallback::fallbackFootNoteR2Optimization(&_trx))
                  ? _trx.dataHandle().getFootNoteCtrl(ptFare.vendor(),
                                                      carrier,
                                                      *fareTariffI,
                                                      *ftnt,
                                                      cat,
                                                      ptFare.fareMarket()->travelDate())
                  : _trx.dataHandle().getAllFootNoteCtrl(
                        ptFare.vendor(), carrier, *fareTariffI, *ftnt, cat);

          dc->printR2sMatchDetails<FootNoteCtrlInfo>(FB_FOOTNOTE_RECORD_2,
                                                     _trx,
                                                     ftnList,
                                                     fnCtrlInfoVec,
                                                     *ptFare.fareMarket(),
                                                     carrier,
                                                     ptFare.fareMarket()->travelDate());
        }

        if (LIKELY(fnCtrlInfoVec.empty()))
          continue;

        FootNoteCtrlInfoVec::iterator iter = fnCtrlInfoVec.begin();
        FootNoteCtrlInfoVec::iterator iterEnd = fnCtrlInfoVec.end();
        for (; iter != iterEnd; ++iter)
        {
          footNote = (*iter).first;
          isLocationSwapped = (*iter).second;

          if (getDiscInfoFromRule(ptFare, footNote, isLocationSwapped))
          {
            isFootFound = true;
            break;
          }
        }
      } // endfor - fn codes
    } // endif - fn exist

    if (LIKELY(!isFootFound))
    {
      isLocationSwapped = false;

      // get discounts from fare rule
      GeneralFareRuleInfoVec gfrInfoVec;
      bool isHistorical = _trx.dataHandle().isHistorical();
      DateTime& tktDate = _trx.ticketingDate();

      RuleUtil::getGeneralFareRuleInfo(_trx, ptFare, cat, gfrInfoVec);

      if (!gfrInfoVec.empty())
      {
        GeneralFareRuleInfoVec::iterator iter = gfrInfoVec.begin();
        GeneralFareRuleInfoVec::iterator iterEnd = gfrInfoVec.end();
        // looping through General Fare Rule vector
        for (; iter != iterEnd; ++iter)
        {
          ruleInfo = (*iter).first;
          isLocationSwapped = (*iter).second;
          getDiscInfoFromRule(ptFare, ruleInfo, isLocationSwapped);

          // Process General Rule record 2s
          processGeneralRule(ruleInfo, ptFare, cat);

          if (LIKELY(!isHistorical || (tktDate.date() != ruleInfo->createDate().date() &&
                                tktDate.date() != ruleInfo->expireDate().date())))
          {
            break;
          }

          // Will remove the below 2 line for fare display
          if (isFdTrx())
            break;
        } // for loop
      }
      else
      {
        // this fare rule info determines where to find the general rule
        // Process General Rule record 2s
        processGeneralRule(ruleInfo, ptFare, cat);
      }

    } // endif - fare/gen rule

    // a bit premature, but it's a good as done...
    ptFare.setCategoryProcessed(cat, true);

    if (!hasPaxInf)
      hasPaxInf = _discRules.hasPax(INFANT);

    // make fares from discInfos
    createFares(ptFare);

    if (UNLIKELY(_trx.getRequest()->isSFR()))
      ptFare.clearStructuredRuleData();

    _discRules.clear();
  } // endfor - allcat

  // free babies
  if (UNLIKELY(_fareMarket.travelBoundary().isSet(FMTravelBoundary::TravelWithinUSCA) &&
      _fareMarket.isInfNeeded() && ptFare.paxType()->paxType() == ADULT && !hasPaxInf))
  {
    makeInfFare(ptFare);
  }
}

bool
DiscountedFareController::checkFare(PaxTypeFare* ptFare, DiagCollector& diag)
{
  // for diag txn, skip fares based on filter
  if (_diag219On || _diag319On)
  {
    if (!diag.rootDiag()->shouldDisplay(*ptFare))
      return false;
  }
  if (ptFare == nullptr || !shouldProcessFare(*ptFare))
  {
    LOG4CXX_DEBUG(logger, "DFC::process() - Skipping processed or null ptFare");
    return false;
  }

  // continue the process if fare that fail fare group but eligible to be a based for discounted
  // fare
  if (UNLIKELY(ptFare->failedFareGroup()))
  {
    if (ptFare->actualPaxTypeItem()[0] != PaxTypeFare::PAXTYPE_NO_MATCHED)
    {
      LOG4CXX_DEBUG(logger, "DFC::process() - Skipping processed or null ptFare");
      return false;
    }
  }

  // Apply discount from Cat 19-22 to Cat 25 fare that was created from Record 8
  // with the Secondary Passenger Type which matches to the request
  if (ptFare->isFareByRule() && !shouldProcessFbrFare(*ptFare))
    return false;
  return true;
}

void
DiscountedFareController::createFares(PaxTypeFare& ptFare)
{
  if (_discRules.empty())
    return;

  std::sort(_discRules.begin(), _discRules.end(), less_paxtype());

  // sorted so can find smallest discount for each pax type
  DiscRules::iter dr = _discRules.begin();
  DiscRules::iter drEnd;

  CalcMoney minCM(_trx, _ccFacade, _itin);
  DiscRule& minDR = *dr;
  PaxTypeFare* newPTFare = nullptr;

  // do for each block of paxtypes
  while (dr != _discRules.end())
  {
    // different aged same PaxType might use different discRule
    if ((isFdTrx() && _fdTrx->isERD()) ||
        (_ageDefinedInTrxPax && discAgeSensitive(*dr->first, ptFare.carrier())))
    {
      drEnd = dr + 1;
    }
    else
    {
      drEnd = std::upper_bound(dr, _discRules.end(), *dr, less_paxtype());
    }

    minCM.nucValue() = NO_MIN_AMT;
    minDR = *dr;
    // loop thru all with the same PaxType
    for (; dr != drEnd; dr++)
    {
      // PaxTypeFare should not be created when a discount is not applicable.

      if (dr->first->discAppl() == 'X')
        continue;

      // results from last calcAmount() in _calcMoney
      std::map<const DiscountInfo*, PaxTypeFare*>::iterator discMapI =
          _discountCalcHelper.find(dr->first);
      if (UNLIKELY(discMapI != _discountCalcHelper.end()))
        calcAmount(*discMapI->second, *dr->first);
      else
        calcAmount(ptFare, *dr->first);

      if (UNLIKELY(_calcMoney.nucValue() < 0))
        continue;

      // If there is softpass qualifer, we always create;
      // others are candidates for the minimum
      const uint32_t catNumber = dr->second->categoryRuleItemInfo()->itemcat();

      if (dr->second->isSoftPassDiscount())
      {
        newPTFare = makeFare(&ptFare, *dr, _calcMoney);
        if (LIKELY(newPTFare != nullptr))
        {
          setNewPTFareCategoryStatus(*newPTFare, ptFare, catNumber);
          if (UNLIKELY(ptFare.isFailedCmdPrc(catNumber)))
          {
            newPTFare->setCmdPrcFailedFlag(catNumber);
            ptFare.setCmdPrcFailedFlag(catNumber, false);
          }

          if (UNLIKELY(newPTFare->fareDisplayInfo() && ptFare.fareDisplayInfo()))
          {
            // We have to copy from base fare rule record data from fbDisplay for cat15 to
            // maintain consistency in displaying rule info (SPR#110178).
            newPTFare->fareDisplayInfo()->fbDisplay().setRuleRecordData(
                15, ptFare.fareDisplayInfo()->fbDisplay().getRuleRecordData(15));
          }
          addFare(newPTFare);
        }

        continue;
      }
      if (_calcMoney.nucValue() < minCM.nucValue())
      {
        minCM = _calcMoney;
        minDR = *dr;
      }
    } // endfor - find min in block with same PaxType

    // found min, so make it
    newPTFare = makeFare(&ptFare, minDR, minCM);
    if (newPTFare != nullptr)
    {
      const uint32_t catNumber = minDR.second->categoryRuleItemInfo()->itemcat();

      setNewPTFareCategoryStatus(*newPTFare, ptFare, catNumber);
      if (UNLIKELY(ptFare.isFailedCmdPrc(catNumber)))
      {
        newPTFare->setCmdPrcFailedFlag(catNumber);
        ptFare.setCmdPrcFailedFlag(catNumber, false);
      }

      if (UNLIKELY(newPTFare->fareDisplayInfo() && ptFare.fareDisplayInfo()))
      {
        // We have to copy from base fare rule record data from fbDisplay for cat15 to
        // maintain consistency in displaying rule info (SPR#110178).
        newPTFare->fareDisplayInfo()->fbDisplay().setRuleRecordData(
            15, ptFare.fareDisplayInfo()->fbDisplay().getRuleRecordData(15));
      }

      addFare(newPTFare);
    }
  } // endwhile - all discInfos
}

void
DiscountedFareController::updateFareMarket(DiagCollector& diag)
{
  std::vector<PaxTypeFare*>& allPTFares = _fareMarket.allPaxTypeFare();

  for (PaxTypeFare* ptFare : _discPaxTypeFares)
  {
    if (UNLIKELY(!ptFare))
      continue;

    if (_diag219On)
      writeDiag219(diag, *ptFare);

    allPTFares.push_back(ptFare);
    addFareToPaxTypeBucket(*ptFare);
  }
  _discPaxTypeFares.clear();
}

bool
DiscountedFareController::shouldProcessFare(const PaxTypeFare& ptFare) const
{
  if ((!ptFare.isValidForDiscount() && !ptFare.failedFareGroup()))
    return false;

  if (UNLIKELY((ptFare.isCategoryProcessed(RuleConst::CHILDREN_DISCOUNT_RULE)) &&
      (ptFare.isCategoryProcessed(RuleConst::TOUR_DISCOUNT_RULE)) &&
      (ptFare.isCategoryProcessed(RuleConst::AGENTS_DISCOUNT_RULE)) &&
      (ptFare.isCategoryProcessed(RuleConst::OTHER_DISCOUNT_RULE))))
  {
    return false;
  }

  return true;
}

bool
DiscountedFareController::shouldProcessFbrFare(const PaxTypeFare& ptFare) const
{
  FBRPaxTypeFareRuleData* fbrPTFare = ptFare.getFbrRuleData(RuleConst::FARE_BY_RULE);
  TSE_ASSERT(fbrPTFare);

  if (fbrPTFare->fbrApp()->segCnt() == 0)
  {
    return false; // No discount from Cat 19-22 if no Secondary Passenger Type coded in Record 8
  }
  else
  {
    const std::vector<PaxTypeCode>& secPaxTypesVec = fbrPTFare->fbrApp()->secondaryPaxTypes();

    std::vector<PaxTypeCode>::const_iterator pt_i = secPaxTypesVec.begin();
    std::vector<PaxTypeCode>::const_iterator pt_j = secPaxTypesVec.end();

    const CarrierCode& ptCarrier = ptFare.carrier();

    for (; pt_i != pt_j; ++pt_i)
    {
      const PaxTypeCode& secPaxType = *pt_i;

      const PaxType* ptItem = PaxTypeUtil::isAnActualPaxInTrx(_trx, ptCarrier, secPaxType);
      if (ptItem != nullptr)
      {
        break; // Apply discount from Cat 19-22 to Cat 25 fare
      }
    }
    if (pt_i == pt_j)
    {
      return false; // No discount from Cat 19-22 if Secondary Passenger Type coded in Record 8
      // does not match the request
    }
  }

  return true;
}

void
DiscountedFareController::setNewPTFareCategoryStatus(PaxTypeFare& newPTFare,
                                                     const PaxTypeFare& basePTFare,
                                                     const uint32_t catNumber) const
{
  // Copy Cat15 status from base fare
  newPTFare.setCategoryValid(15, basePTFare.isCategoryValid(15));
  newPTFare.setCat15SoftPass(basePTFare.cat15SoftPass());
  newPTFare.setCategoryProcessed(15);
  // need revalidate (FPath) cat15 of the base fare when SOFTPASS
  // and use this status for the Discounted fare.
  // do not need validate cat15 for the Discounted fare
  newPTFare.setCategorySoftPassed(15, basePTFare.isCategorySoftPassed(15));

  newPTFare.setCategorySoftPassed(catNumber);
}
}
