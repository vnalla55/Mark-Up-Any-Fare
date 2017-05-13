//----------------------------------------------------------------------------
//
//  File:        AltPricingUtil.cpp
//  Created:     2/20/2006
//  Authors:     Andrew Ahmad
//
//  Description: Common functions required for ATSE WPA processing.
//
//  Updates:
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
//----------------------------------------------------------------------------

#include "Common/AltPricingUtil.h"

#include "Common/AirlineShoppingUtils.h"
#include "Common/BSRCollectionResults.h"
#include "Common/BSRCurrencyConverter.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/CurrencyConversionRequest.h"
#include "Common/CurrencyConverter.h"
#include "Common/FallbackUtil.h"
#include "Common/FareCalcUtil.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/NUCCurrencyConverter.h"
#include "Common/PaxTypeFareRuleDataCast.h"
#include "Common/TruePaxType.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AltPricingTrx.h"
#include "DataModel/DifferentialData.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/Itin.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/Loc.h"
#include "DBAccess/NoPNROptions.h"
#include "Taxes/LegacyTaxes/TaxRecord.h"

#include <vector>

using namespace std;
namespace tse
{
FALLBACK_DECL(fallbackPriceByCabinActivation)

namespace
{
Logger
logger("atseintl.Common.AltPricingUtil");

ConfigurableValue<uint32_t>
wpaFareOptionMaxDefault("ALT_PRICING_UTIL", "WPA_FARE_OPTION_MAX_DEFAULT", 24);
ConfigurableValue<uint32_t>
wpaFareOptionMaxLimit("ALT_PRICING_UTIL", "WPA_FARE_OPTION_MAX_LIMIT", 50);
}

bool
AltPricingUtil::needToReprocess(PricingTrx& trx)
{
  // Check Fare Calc config option <WP No Match Permitted>
  // If 'N' then there is no need to continue.
  // If 'Y' then check to see if there are any passenger types
  // with a valid fare combination.
  // If all passenger types have no valid fare combinations, then
  // WP No Match processing will be performed.

  const FareCalcConfig* fcConfig = FareCalcUtil::getFareCalcConfig(trx);

  if (!fcConfig)
  {
    LOG4CXX_ERROR(logger,
                  "Error getting Fare Calc Config for PCC: "
                      << trx.getRequest()->ticketingAgent()->tvlAgencyPCC());
    return false;
  }

  if ((trx.altTrxType() == PricingTrx::WP || trx.altTrxType() == PricingTrx::WP_NOMATCH) &&
      fcConfig->wpNoMatchPermitted() != YES)
  {
    return false;
  }

  // Return true for the 1S user and 'NC' is On
  if (trx.isLowestFareOverride() && trx.altTrxType() == AltPricingTrx::WPA &&
      !trx.getRequest()->isLowFareRequested())
  {
    return true;
  }

  vector<Itin*>::const_iterator itinIter = trx.itin().begin();
  vector<Itin*>::const_iterator itinIterEnd = trx.itin().end();

  for (; itinIter != itinIterEnd; ++itinIter)
  {
    if (!(*itinIter)->farePath().empty())
    {
      return false;
    }
  }
  return true;
}

bool
AltPricingUtil::prepareToReprocess(FareMarket& fareMarket)
{
  LOG4CXX_DEBUG(logger,
                "In AltPricingUtil::prepareToReprocess(): "
                    << "\nFare Market: " << fareMarket.origin()->loc() << "-"
                    << fareMarket.destination()->loc());

  ErrorResponseException::ErrorResponseCode& failCode = fareMarket.failCode();

  if (failCode != ErrorResponseException::NO_ERROR &&
      failCode != ErrorResponseException::NO_FARE_FOR_CLASS &&
      failCode != ErrorResponseException::NO_FARE_FOR_CLASS_USED)
  {
    return true;
  }

  // clear validation result map for cat5, we can not reuse the
  // validation result
  fareMarket.resetResultMap(5);

  vector<PaxTypeFare*>::iterator ptfIter = fareMarket.allPaxTypeFare().begin();
  vector<PaxTypeFare*>::iterator ptfIterEnd = fareMarket.allPaxTypeFare().end();

  for (; ptfIter != ptfIterEnd; ++ptfIter)
  {
    PaxTypeFare* ptf = *ptfIter;

    if ((!ptf->areAllCategoryValid()) && (ptf->isCategoryProcessed(5)) &&
        (!ptf->isCategoryValid(5)))
    {
      ptf->setCategoryValid(5);

      // Cat 5 must be the only category that failed.
      if (!ptf->areAllCategoryValid())
      {
        ptf->setCategoryValid(5, false);
        continue;
      }

      if (!ptf->reProcessCat05NoMatch())
      {
        ptf->setCategoryValid(5, false);
        continue;
      }

      ptf->setCategoryProcessed(5, false);
      ptf->setCategorySoftPassed(5, false);

      fareMarket.serviceStatus().set(FareMarket::FareValidator, false);
      fareMarket.failCode() = ErrorResponseException::NO_ERROR;
    }

    if (!ptf->bookingCodeStatus().isSet(PaxTypeFare::BKS_PASS))
    {
      ptf->bookingCodeStatus() = PaxTypeFare::BKS_NOT_YET_PROCESSED;
      ptf->segmentStatus().clear();
      ptf->segmentStatusRule2().clear();
      ptf->mixClassStatus() = PaxTypeFare::MX_NOT_APPLICABLE;

      fareMarket.serviceStatus().set(FareMarket::FareValidator, false);
      fareMarket.failCode() = ErrorResponseException::NO_ERROR;
    }
  }

  return true;
}

bool
AltPricingUtil::prepareToReprocess(PricingTrx& trx)
{
  if (!trx.noPNRPricing())
  {
    if (trx.getRequest()->turnOffNoMatch())
      return false;

    // At time of ticket issuance, do not attempt WP-No-Match.
    if (trx.altTrxType() == AltPricingTrx::WP && trx.getRequest()->isTicketEntry())
    {
      return false;
    }

    if (trx.altTrxType() == AltPricingTrx::WP)
    {
      trx.altTrxType() = AltPricingTrx::WP_NOMATCH;
    }

    trx.getRequest()->lowFareRequested() = 'T';
  }

  vector<Itin*>::const_iterator itinIter = trx.itin().begin();
  vector<Itin*>::const_iterator itinIterEnd = trx.itin().end();

  for (; itinIter != itinIterEnd; ++itinIter)
  {
    (*itinIter)->errResponseCode() = ErrorResponseException::NO_ERROR;
    (*itinIter)->errResponseMsg() = "";
  }

  return true;
}

namespace
{
bool
isSamePaxType(const PaxType* requested, FareUsage* fareUsage)
{
  return requested && fareUsage && fareUsage->paxTypeFare()->fcasPaxType() == requested->paxType();
}

bool
pricingUnitContainsFareWithRequestedPaxType(const PaxType* requested, PricingUnit* pricingUnit)
{
  return pricingUnit->fareUsage().end() !=
         std::find_if(pricingUnit->fareUsage().begin(),
                      pricingUnit->fareUsage().end(),
                      std::bind1st(std::ptr_fun(isSamePaxType), requested));
}

bool
containsFareWithRequestedPaxType(const FarePath* farePath)
{
  return farePath->pricingUnit().end() !=
         std::find_if(farePath->pricingUnit().begin(),
                      farePath->pricingUnit().end(),
                      std::bind1st(std::ptr_fun(pricingUnitContainsFareWithRequestedPaxType),
                                   farePath->paxType()));
}

struct cmpPaxType : public std::binary_function<const FarePath*, const FarePath*, bool>
{
  cmpPaxType(PricingTrx& trx) : _trx(trx) {}
  bool operator()(const FarePath* farePath1, const FarePath* farePath2)
  {
    const PaxTypeCode& paxType1 = farePath1->paxType()->paxType();
    const PaxTypeCode& paxType2 = farePath2->paxType()->paxType();

    if (paxType1 == paxType2)
    {
      // put matches before no-matches
      if (farePath1->noMatchOption() != farePath2->noMatchOption())
        return farePath2->noMatchOption();

      // if any of the solutions contains fare with requested pax type - it should go first
      bool firstFarePathContainsFareWithRequestedPaxType =
          containsFareWithRequestedPaxType(farePath1);
      bool secondFarePathContainsFareWithRequestedPaxType =
          containsFareWithRequestedPaxType(farePath2);

      if (firstFarePathContainsFareWithRequestedPaxType !=
          secondFarePathContainsFareWithRequestedPaxType)
        return firstFarePathContainsFareWithRequestedPaxType;

      // finally - put 'mixed passanger type' fare paths in the back
      TruePaxType tpt2(_trx, *farePath2);
      return tpt2.mixedPaxType();
    }
    return paxType1 < paxType2;
  }
  PricingTrx& _trx;
};
}

bool
AltPricingUtil::finalProcessing(PricingTrx& trx)
{
  if (trx.diagnostic().diagnosticType() == Diagnostic860)
  {
    map<string, string>::iterator diagOption = trx.diagnostic().diagParamMap().find("DD");
    // trx.diagnostic().diagParamMap().find(Diagnostic::DISPLAY_DETAIL);

    if (diagOption != trx.diagnostic().diagParamMap().end())
    {
      if (!((*diagOption).second).empty())
      {
        if ((*diagOption).second == "ALLFARES")
        {
          // Duplicates requested for diagnostic
          return true;
        }
      }
    }
  }

  bool wpasXM = (trx.getRequest()->isWpas() && trx.getRequest()->wpaXm());
  bool allowDuplicateTotals = false;
  uint32_t farePathCount = 0;
  uint32_t fareOptionMaxDefault = getConfigFareOptionMaxDefault();
  uint32_t fareOptionMaxLimit = getConfigFareOptionMaxLimit();

  if (trx.noPNRPricing())
  {
    NoPNRPricingTrx* noPNRPricingTrx = static_cast<NoPNRPricingTrx*>(&trx);
    const NoPNROptions* noPNROptions = noPNRPricingTrx->noPNROptions();
    allowDuplicateTotals = noPNROptions->wqDuplicateAmounts() == YES ? true : false;

    if (allowDuplicateTotals)
      return true;

    farePathCount = noPNROptions->maxNoOptions();
  }
  else
  {
    if (trx.getRequest()->ticketingAgent()->sabre1SUser() && trx.getRequest()->isWpa50())
    {
      farePathCount = AltPricingTrx::WPA_50_OPTIONS;
    }

    const FareCalcConfig* fcConfig = FareCalcUtil::getFareCalcConfig(trx);

    if (fcConfig)
    {
      allowDuplicateTotals = fcConfig->wpaShowDupAmounts() == YES ? true : false;
      if (allowDuplicateTotals && !wpasXM)
      {
        return true;
      }
      if (!farePathCount)
      {
        farePathCount = fcConfig->wpaFareOptionMaxNo();
      }
    }
    else
    {
      LOG4CXX_WARN(logger,
                   "Error getting Fare Calc Config for PCC: "
                       << trx.getRequest()->ticketingAgent()->tvlAgencyPCC()
                       << " Using default values.");
    }
  }

  if (farePathCount <= 0)
  {
    farePathCount = fareOptionMaxDefault;
  }
  else if (farePathCount > fareOptionMaxLimit)
  {
    farePathCount = fareOptionMaxLimit;
  }

  LOG4CXX_DEBUG(logger,
                "\n********** WPA ALTERNATIVE PRICING **********"
                    << "\nCONFIG FARE OPTION MAX DEFAULT: " << fareOptionMaxDefault
                    << "\nCONFIG FARE OPTION MAX LIMIT: " << fareOptionMaxLimit
                    << "\nUSING ALLOW DUP TOTALS: " << (allowDuplicateTotals ? "YES" : "NO")
                    << "\nUSING FP COUNT: " << farePathCount);

  vector<Itin*>::iterator itinIter = trx.itin().begin();
  vector<Itin*>::iterator itinIterEnd = trx.itin().end();

  bool farePathExist = false;
  for (; itinIter != itinIterEnd; ++itinIter)
  {
    Itin* itin = *itinIter;
    if (!itin)
    {
      continue;
    }

    std::set<MoneyAmount> uniqueFarePathTotals;
    vector<FarePath*> uniqueFarePaths;

    if (trx.noPNRPricing())
    {
      std::vector<FarePath*>& farePaths = itin->farePath();
      std::stable_sort(farePaths.begin(), farePaths.end(), cmpPaxType(trx));
    }

    vector<FarePath*>::iterator fpIter = itin->farePath().begin();
    vector<FarePath*>::iterator fpIterEnd = itin->farePath().end();

    uint32_t fpCounter = 0;
    PaxType prevPaxType;
    if (fpIter != fpIterEnd)
    {
      prevPaxType = *((*fpIter)->paxType());
    }

    for (; fpIter != fpIterEnd; ++fpIter)
    {
      FarePath* farePath = *fpIter;
      if (!farePath)
      {
        continue;
      }

      if (!(*(farePath->paxType()) == prevPaxType))
      {
        uniqueFarePathTotals.clear();
        fpCounter = 0;
        prevPaxType = *(farePath->paxType());
      }
      else
      {
        if (fpCounter == farePathCount)
        {
          continue;
        }
      }

      if (trx.getRequest()->isWpas() && trx.getRequest()->wpaXm())
      {
        if (isRBDasBooked(trx, farePath))
        {
          continue;
        }
      }

      std::set<MoneyAmount>::const_reverse_iterator fpTotalsIter = uniqueFarePathTotals.rbegin();
      std::set<MoneyAmount>::const_reverse_iterator fpTotalsIterEnd = uniqueFarePathTotals.rend();

      bool dupFound = false;

      MoneyAmount fpTotalAmt(0);
      MoneyAmount diffAmount(0);
      getAmounts(trx, farePath, fpTotalAmt, diffAmount);

      for (; fpTotalsIter != fpTotalsIterEnd; ++fpTotalsIter)
      {
        if (fabs(fpTotalAmt - *fpTotalsIter) <= diffAmount)
        {
          dupFound = true;
          break;
        }
      }

      if (!dupFound || (allowDuplicateTotals && wpasXM))
      {
        uniqueFarePathTotals.insert(fpTotalAmt);
        uniqueFarePaths.push_back(farePath);
        ++fpCounter;
      }
    }

    itin->farePath().clear();
    if (fpCounter)
    {
      farePathExist = true;
      itin->farePath().insert(
          itin->farePath().end(), uniqueFarePaths.begin(), uniqueFarePaths.end());

      uniqueFarePaths.clear();
      uniqueFarePathTotals.clear();
      fpCounter = 0;
    }
  }

  if (farePathExist)
    return true;
  if (trx.diagnostic().diagnosticType() == DiagnosticNone)
    return false;

  return true;
}

FarePath*
AltPricingUtil::lowestSolution(PricingTrx& trx,
                               const std::vector<FarePath*>& farePaths,
                               PaxType* paxType)
{
  std::vector<FarePath*>::const_iterator iter = farePaths.begin();
  std::vector<FarePath*>::const_iterator iterEnd = farePaths.end();

  FarePath* lowestFarePath = nullptr;
  MoneyAmount lowestAmount = 0;

  while (iter != iterEnd)
  {
    FarePath* farePath = *iter;
    if (farePath->paxType() == paxType)
    {
      MoneyAmount amount = 0;
      MoneyAmount diff = 0;
      getAmounts(trx, farePath, amount, diff);

      if (lowestFarePath == nullptr || lowestAmount > amount)
      {
        lowestAmount = amount;
        lowestFarePath = farePath;
      }
    }
    ++iter;
  }
  return lowestFarePath;
}

void
AltPricingUtil::keepLowestSolutions(PricingTrx& trx, Itin& itin)
{
  std::vector<FarePath*>& farePaths = itin.farePath();
  std::vector<FarePath*> lowestPaths;

  const std::vector<PaxType*>& paxTypes = trx.paxType();
  std::vector<PaxType*>::const_iterator iter = paxTypes.begin();
  std::vector<PaxType*>::const_iterator iterEnd = paxTypes.end();
  while (iter != iterEnd)
  {
    PaxType* paxType = *iter;
    FarePath* farePath = lowestSolution(trx, farePaths, paxType);
    if (farePath)
      lowestPaths.push_back(farePath);
    ++iter;
  }

  farePaths.clear();
  farePaths.insert(farePaths.end(), lowestPaths.begin(), lowestPaths.end());
}

Money
AltPricingUtil::getFarePathTaxTotal(const FarePath& farePath)
{
  Money fpTaxTotal(NUC);

  if (const TaxResponse* taxResponse = TaxResponse::findFor(&farePath))
  {
    vector<TaxRecord*>::const_iterator taxIter = taxResponse->taxRecordVector().begin();
    vector<TaxRecord*>::const_iterator taxIterEnd = taxResponse->taxRecordVector().end();

    if (taxIter != taxIterEnd)
    {
      fpTaxTotal.setCode((*taxIter)->taxCurrencyCode());
    }

    for (; taxIter != taxIterEnd; taxIter++)
    {
      fpTaxTotal.value() += (*taxIter)->getTaxAmount();
    }
  }
  return fpTaxTotal;
}

uint32_t
AltPricingUtil::getConfigFareOptionMaxDefault()
{
  return wpaFareOptionMaxDefault.getValue();
}

uint32_t
AltPricingUtil::getConfigFareOptionMaxLimit()
{
  return wpaFareOptionMaxLimit.getValue();
}

bool
AltPricingUtil::isXMrequest(const PricingTrx& trx)
{
  if (trx.getRequest()->isLowFareRequested())
  {
    return true;
  }

  map<string, string>::const_iterator diagOption = trx.diagnostic().diagParamMap().find("XM");

  if (diagOption != trx.diagnostic().diagParamMap().end())
  {
    return true;
  }
  return false;
}

void
AltPricingUtil::prepareToProcess(PricingTrx& trx)
{
  if (isXMrequest(trx))
  {
    trx.getRequest()->lowFareRequested() = 'T';
  }
}

bool
AltPricingUtil::validateFarePathBookingCode(PricingTrx& trx, FarePath& farePath)
{
  if (UNLIKELY(trx.noPNRPricing()))
  {
    NoPNRPricingTrx* noPNRTrx = static_cast<NoPNRPricingTrx*>(&trx);
    if (!noPNRTrx->isNoMatch())
      return true;
    if (!noPNRTrx->isXM())
      return true;
  }
  else
  {
    AltPricingTrx* altPricingTrx = dynamic_cast<AltPricingTrx*>(&trx);
    if (LIKELY(!altPricingTrx))
      return true;

    if (altPricingTrx->altTrxType() != AltPricingTrx::WPA)
      return true;

    if (!isXMrequest(trx))
      return true;
  }

  vector<PricingUnit*>::const_iterator iter = farePath.pricingUnit().begin();
  vector<PricingUnit*>::const_iterator iterEnd = farePath.pricingUnit().end();

  for (; iter != iterEnd; ++iter)
  {
    const PricingUnit* pu = *iter;

    vector<FareUsage*>::const_iterator fuIter = pu->fareUsage().begin();
    vector<FareUsage*>::const_iterator fuIterEnd = pu->fareUsage().end();

    for (; fuIter != fuIterEnd; ++fuIter)
    {
      const FareUsage* fu = *fuIter;

      PaxTypeFare::SegmentStatusVecCI ssIter = fu->segmentStatus().begin();
      PaxTypeFare::SegmentStatusVecCI ssIterEnd = fu->segmentStatus().end();

      for (; ssIter != ssIterEnd; ++ssIter)
      {
        if ((*ssIter)._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
        {
          // If any segment is rebooked, then we can use this FarePath
          farePath.noMatchOption() = true;
          return true;
        }
      }
      if (!fu->differentialPlusUp().empty())
      {
        PaxTypeFare::SegmentStatusVecCI iter, iterEnd;

        for (const auto diff : fu->differentialPlusUp())
        {
          if (diff == nullptr)
            continue;

          if (diff->status() == DifferentialData::SC_PASSED ||
              diff->status() == DifferentialData::SC_CONSOLIDATED_PASS)
          {
            for (iter = diff->fareHigh()->segmentStatus().begin(),
                iterEnd = diff->fareHigh()->segmentStatus().end();
                 iter != iterEnd;
                 ++iter)
            {
              if ((*iter)._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
              {
                // If any segment is rebooked, then we can use this FarePath
                farePath.noMatchOption() = true;
                return true;
              }
            }
          }
        }
      }
    }
  }
  return false;
}

//---------------------------------------------------------
bool
AltPricingUtil::ignoreAvail(PricingTrx& trx)
{
  if (trx.getTrxType() == PricingTrx::MIP_TRX)
    return false;

  if (UNLIKELY(trx.getRequest()->isWpas()))
    return false;

  // 1. For a WPNC entry NEVER check noMatchAvail from FCC
  // 2. For NOMATCH part of WP or WPA - check noMatchAvail from FCC
  if (trx.getRequest()->isLowFareRequested())
  {
    if (LIKELY(!(trx.altTrxType() == PricingTrx::WPA ||
                 trx.altTrxType() == PricingTrx::WPA_NOMATCH ||
                 trx.altTrxType() == PricingTrx::WP_NOMATCH)))
    {
      return false;
    }
  }

  const FareCalcConfig* fcConfig = getFareCalcConfig(trx);
  if (fcConfig == nullptr)
    return false;

  if (fcConfig->noMatchAvail() != YES)
    return false;

  return true;
}

//---------------------------------------------------------
const FareCalcConfig*
AltPricingUtil::getFareCalcConfig(PricingTrx& trx)
{
  return FareCalcUtil::getFareCalcConfig(trx);
}

bool
AltPricingUtil::intermediateProcessing(PricingTrx& trx,
                                       std::vector<FarePath*>& fPaths,
                                       const uint32_t max)
{
  if (trx.diagnostic().diagnosticType() == Diagnostic860)
  {
    map<string, string>::iterator diagOption = trx.diagnostic().diagParamMap().find("DD");
    // trx.diagnostic().diagParamMap().find(Diagnostic::DISPLAY_DETAIL);

    if (diagOption != trx.diagnostic().diagParamMap().end())
    {
      if (!((*diagOption).second).empty())
      {
        if ((*diagOption).second == "ALLFARES")
        {
          // Duplicates requested for diagnostic
          return true;
        }
      }
    }
  }

  vector<FarePath*>::iterator fpIter = fPaths.begin();
  vector<FarePath*>::iterator fpIterEnd = fPaths.end();

  PaxType currPaxType = *((*fpIter)->paxType());

  std::set<MoneyAmount> matchFarePathTotals;

  if (trx.getRequest()->isLowFareRequested() && !trx.itin()[0]->farePath().empty())
  {
    getMatchTotals(trx, currPaxType, matchFarePathTotals);
  }

  std::set<MoneyAmount> uniqueFarePathTotals;
  vector<FarePath*> uniqueFarePaths;

  uint32_t fpCounter = 0;
  for (; fpIter != fpIterEnd; ++fpIter)
  {
    FarePath* farePath = *fpIter;
    if (!farePath)
    {
      continue;
    }

    if (fpCounter == max)
    {
      continue;
    }

    std::set<MoneyAmount>::const_reverse_iterator fpMatchTotIter = matchFarePathTotals.rbegin();
    std::set<MoneyAmount>::const_reverse_iterator fpMatchTotIterEnd = matchFarePathTotals.rend();

    std::set<MoneyAmount>::const_reverse_iterator fpTotalsIter = uniqueFarePathTotals.rbegin();
    std::set<MoneyAmount>::const_reverse_iterator fpTotalsIterEnd = uniqueFarePathTotals.rend();

    bool dupFound = false;

    MoneyAmount fpTotalAmt(0);
    MoneyAmount diffAmount(0);
    getAmounts(trx, farePath, fpTotalAmt, diffAmount);

    for (; fpTotalsIter != fpTotalsIterEnd; ++fpTotalsIter)
    {
      if (fabs(fpTotalAmt - *fpTotalsIter) <= diffAmount)
      {
        dupFound = true;
        break;
      }
    }

    if (!dupFound)
    {
      for (; fpMatchTotIter != fpMatchTotIterEnd; ++fpMatchTotIter)
      {
        if (fabs(fpTotalAmt - *fpMatchTotIter) <= diffAmount)
        {
          dupFound = true;
          break;
        }
      }
    }

    if (!dupFound)
    {
      uniqueFarePathTotals.insert(fpTotalAmt);
      uniqueFarePaths.push_back(farePath);
      ++fpCounter;
    }
  }
  fPaths.clear();
  fPaths.insert(fPaths.end(), uniqueFarePaths.begin(), uniqueFarePaths.end());

  uniqueFarePaths.clear();
  uniqueFarePathTotals.clear();

  if (fpCounter < max)
  {
    return false;
  }

  return true;
}

void
AltPricingUtil::getMatchTotals(PricingTrx& trx,
                               PaxType& currPaxType,
                               std::set<MoneyAmount>& matchFarePathTotals)
{
  std::vector<Itin*>::const_iterator itM = trx.itin().begin();
  std::vector<Itin*>::const_iterator itMEnd = trx.itin().end();

  for (; itM != itMEnd; ++itM)
  {
    vector<FarePath*>::iterator fpI = (*itM)->farePath().begin();
    vector<FarePath*>::iterator fpIE = (*itM)->farePath().end();

    for (; fpI != fpIE; ++fpI)
    {
      FarePath* farePath = *fpI;
      if (!farePath)
      {
        continue;
      }

      if (!(*(farePath->paxType()) == currPaxType))
      {
        continue;
      }

      MoneyAmount fpTotalAmt(0);
      MoneyAmount diffAmount(0);

      getAmounts(trx, farePath, fpTotalAmt, diffAmount);

      matchFarePathTotals.insert(fpTotalAmt);
    }
  }
  return;
}

void
AltPricingUtil::getAmounts(PricingTrx& trx,
                           FarePath* farePath,
                           MoneyAmount& fpTotalAmt,
                           MoneyAmount& diffAmount)
{
  const Itin* itin = farePath->itin();

  CurrencyNoDec fpTotalNoDec(0);
  CurrencyCode fpTotalCurCode;
  ExchRate exchRate;
  CurrencyNoDec exchNoDec;
  DateTime nucEffectiveDate;
  DateTime nucDiscontinueDate;

  NUCCurrencyConverter conv;
  if (!conv.convertBaseFare(trx,
                            *farePath,
                            farePath->getTotalNUCAmount(),
                            fpTotalAmt,
                            fpTotalNoDec,
                            fpTotalCurCode,
                            exchRate,
                            exchNoDec,
                            nucEffectiveDate,
                            nucDiscontinueDate,
                            farePath->itin()->useInternationalRounding()))
  {
    LOG4CXX_ERROR(logger, "Currency conversion error");
  }

  // Working with BSR

  MoneyAmount equivFareAmount = 0;
  CurrencyCode equivCurrencyCode = trx.getRequest()->ticketingAgent()->currencyCodeAgent();

  if ((!equivCurrencyCode.empty()) && (equivCurrencyCode != itin->originationCurrency()))
  {
    BSRCurrencyConverter bsrConverter;
    bool rc;
    DataHandle dataHandle(trx.ticketingDate());

    const Money sourceMoney(fpTotalAmt, fpTotalCurCode);
    Money targetMoney(equivFareAmount, equivCurrencyCode);
    CurrencyConversionRequest request1(targetMoney,
                                       sourceMoney,
                                       trx.getRequest()->ticketingDT(),
                                       *(trx.getRequest()),
                                       dataHandle,
                                       false,
                                       CurrencyConversionRequest::OTHER,
                                       false,
                                       trx.getOptions(),
                                       false,
                                       true,
                                       &trx);
    BSRCollectionResults bsrResults;
    bsrResults.collect() = true;

    rc = bsrConverter.convert(request1, &bsrResults);
    if (!rc)
    {
      LOG4CXX_FATAL(logger,
                    "BSR Rate:" << fpTotalCurCode << " and:" << equivCurrencyCode
                                << " was not available");
      return;
    }

    fpTotalAmt = targetMoney.value();
  }

  // Get tax total for the farepath
  Money fpTaxTotal = getFarePathTaxTotal(*farePath);

  fpTotalAmt += fpTaxTotal.value();

  diffAmount = 1.0 / pow(10.0, fpTotalNoDec);

  LOG4CXX_DEBUG(logger,
                "\nPAX: " << farePath->paxType()->paxType() << " FARE: "
                          << fpTotalAmt - fpTaxTotal.value() << " TAX: " << fpTaxTotal.value()
                          << " TOTAL: " << fpTotalAmt << " " << fpTotalCurCode
                          << " NO DEC: " << fpTotalNoDec << " DIFF AMT: " << diffAmount);

  return;
}

bool
AltPricingUtil::isCat25SisterFare(const PaxTypeFare& paxTypeFare)
{
  if (paxTypeFare.isFareByRule())
  {
    const PaxTypeFareRuleData* paxTypeFareRuleData = paxTypeFare.paxTypeFareRuleData(25);
    if (paxTypeFareRuleData)
    {
      const FBRPaxTypeFareRuleData* fbrPaxTypeFareRuleData =
          PTFRuleData::toFBRPaxTypeFare(paxTypeFareRuleData);

      if (fbrPaxTypeFareRuleData)
      {
        if (fbrPaxTypeFareRuleData->isBaseFareAvailBkcMatched())
        {
          return true;
        }
      }
    }
  }
  return false;
}

bool
AltPricingUtil::isRBDasBooked(PricingTrx& trx, FarePath* farePath)
{
  typedef std::vector<PaxTypeFare::SegmentStatus>::const_iterator SegmentStatusVecCI;

  std::vector<PricingUnit*>::const_iterator iter = farePath->pricingUnit().begin();
  std::vector<PricingUnit*>::const_iterator iterEnd = farePath->pricingUnit().end();

  std::vector<TravelSeg*>::const_iterator ts, tsEnd;
  std::vector<FareUsage*>::const_iterator fuIter, fuIterEnd;

  for (; iter != iterEnd; ++iter)
  {
    const PricingUnit* pu = *iter;

    fuIter = pu->fareUsage().begin();
    fuIterEnd = pu->fareUsage().end();

    for (; fuIter != fuIterEnd; ++fuIter)
    {
      SegmentStatusVecCI cSt = (*fuIter)->segmentStatus().begin();

      ts = (*fuIter)->travelSeg().begin();
      tsEnd = (*fuIter)->travelSeg().end();

      for (; ts != tsEnd; ++ts, ++cSt)
      {
        const PaxTypeFare::SegmentStatus& segStat = *cSt;

        if (segStat._bkgCodeReBook.empty())
        {
          continue;
        }
        if (!fallback::fallbackPriceByCabinActivation(&trx))
        {
          if ((*ts)->getBookingCode() == segStat._bkgCodeReBook && !(*ts)->rbdReplaced())
          {
            continue;
          }
        }
        else
        {
          if ((*ts)->getBookingCode() == segStat._bkgCodeReBook)
          {
            continue;
          }
        }
        return false; // as booked RBD does not match to rebooked RBD
      }
    }
  }
  return true;
}
}
