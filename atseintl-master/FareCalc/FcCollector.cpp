//----------------------------------------------------------------------------
//  File:        FcCollector.cpp
//  Authors:     Quan Ta
//  Created:
//
//  Description: Refactored FareCalcCollector
//
//  Updates:
//          date - initials - description.
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
#include "FareCalc/FcCollector.h"

#include "Common/AccTvlDetailOut.h"
#include "Common/BaggageStringFormatter.h"
#include "Common/BSRCollectionResults.h"
#include "Common/BSRCurrencyConverter.h"
#include "Common/CommissionKeys.h"
#include "Common/CurrencyConversionRequest.h"
#include "Common/ErrorResponseException.h"
#include "Common/FallbackUtil.h"
#include "Common/FareMarketUtil.h"
#include "Common/FreeBaggageUtil.h"
#include "Common/ItinUtil.h"
#include "Common/Logger.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/MessageWrapper.h"
#include "Common/Money.h"
#include "Common/NUCCurrencyConverter.h"
#include "Common/PaxTypeUtil.h"
#include "Common/SpecifyMaximumPenaltyCommon.h"
#include "Common/TravelSegUtil.h"
#include "Common/TruePaxType.h"
#include "Common/TrxUtil.h"
#include "Common/ValidatingCxrUtil.h"
#include "Common/Vendor.h"
#include "DataModel/Agent.h"
#include "DataModel/AltPricingTrx.h"
#include "DataModel/CollectedNegFareData.h"
#include "DataModel/ConsolidatorPlusUp.h"
#include "DataModel/DifferentialData.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/InterlineTicketCarrierData.h"
#include "DataModel/MaxPenaltyInfo.h"
#include "DataModel/NetRemitFarePath.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/PrivateIndicator.h"
#include "DataModel/RexExchangeTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/SurchargeData.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/ContractPreference.h"
#include "DBAccess/Customer.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/NegFareRest.h"
#include "DBAccess/PaxTypeInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagManager.h"
#include "FareCalc/CalcTotals.h"
#include "FareCalc/FareAmountsConverter.h"
#include "FareCalc/FareCalcCollector.h"
#include "FareCalc/FareCalcConsts.h"
#include "FareCalc/FareCalcHelper.h"
#include "FareCalc/FcDispItem.h"
#include "FareCalc/FcUtil.h"
#include "FareCalc/IETValidator.h"
#include "FareCalc/NvbNvaOrchestrator.h"
#include "Fares/FareTypeMatcher.h"

#include <functional>
#include <iomanip>
#include <sstream>
#include <vector>

#include <boost/algorithm/string/trim_all.hpp>

namespace tse
{
FALLBACK_DECL(fallbackValidatingCxrMultiSp);
FALLBACK_DECL(fallbackCommissionManagement);
FALLBACK_DECL(fallbackVCxrTrailerMsgDisplay);
FALLBACK_DECL(fallbackPriceByCabinActivation);
FALLBACK_DECL(fallbackAlternateVcxrInsteadOptionalVcxr);
FALLBACK_DECL(fallbackAMCPhase2);
FALLBACK_DECL(fallbackNonBSPVcxrPhase1);
FALLBACK_DECL(fallbackValidatingCarrierInItinOrderMultiSp);
FALLBACK_DECL(fallbackFRRProcessingRetailerCode);
FALLBACK_DECL(fallbackNonBSPVcxrPhase1R9);

namespace FareCalc
{
static Logger
logger("atseintl.FareCalc.FcCollector");

FcCollector::FcCollector(PricingTrx* trx,
                         const FarePath* farePath,
                         const FareCalcConfig* fcConfig,
                         FareCalcCollector* fcCollector,
                         CalcTotals* calcTotals)
  : _trx(trx),
    _farePath(farePath),
    _fcConfig(fcConfig),
    _fcCollector(fcCollector),
    _calcTotals(calcTotals)
{
  _calcTotals->farePathInfo.cpFailedStatus.set(PaxTypeFare::PTFF_NONE);

  TruePaxType tpt(*trx, *_farePath);
  _calcTotals->truePaxType = tpt.paxType();
  _calcTotals->mixedPaxType = tpt.mixedPaxType();
  _calcTotals->negFareUsed = tpt.negFareUsed();
  _calcTotals->privateFareUsed = tpt.privateFareUsed();
  _calcTotals->allPaxTypeInSabreGroup = tpt.allInGroup();

  // This little hack is to satisfy Abacus user.
  if (trx->altTrxType() != PricingTrx::WP &&
      _calcTotals->negFareUsed && // This mostly means NetFare
      _calcTotals->truePaxType == CHILD && // CNN
      _calcTotals->requestedPaxType[0] == 'C' &&
      _calcTotals->requestedPaxType[1] < 'a' &&
      _calcTotals->requestedPaxType[2] < 'a' && // Child fare with age encoded - C06
      PaxTypeUtil::isChild(_calcTotals->truePaxType, _calcTotals->vendor))
  {
    _calcTotals->truePaxType = _calcTotals->requestedPaxType;
  }

  // check the Customer table (TJR) to process PVT indicator for Pricing (WP, WQ, WPA)
  if (!_trx->getRequest()->ticketingAgent()->tvlAgencyPCC().empty())
  {
    if (LIKELY(_trx->getRequest()->ticketingAgent()->agentTJR() != nullptr &&
        _trx->getRequest()->ticketingAgent()->agentTJR()->privateFareInd() !=
            'N')) // cat31/33 is out of scope
      _needProcessPvtIndFromTJR = true;
  }
}

void
FcCollector::collect()
{
  ROEDateSetter roeDateSetter(*_trx, const_cast<FarePath&>(*_farePath));

  checkPerDirSurchargeConsecutive();

  const std::vector<PricingUnit*>& pus = _farePath->pricingUnit();
  std::for_each(pus.begin(), pus.end(), *this);

  correctSTOSurOverrideCnt();

  // FarePath total fare and currency info.
  _calcTotals->farePathInfo.nucFareAmount = _farePath->getTotalNUCAmount();

  _calcTotals->farePathInfo.commissionAmount = _farePath->commissionAmount();
  _calcTotals->farePathInfo.commissionPercent = _farePath->commissionPercent();

  // do not call processNVANVBDate() for NetRemit farePath
  if (LIKELY(typeid(*_farePath) != typeid(NetRemitFarePath)))
  {
    NvbNvaOrchestrator nvb(*_trx, *_farePath, *_calcTotals);
    nvb.process();
  }

  convertAmounts();

  if (LIKELY(_firstROE == 0.0f))
  {
    _firstROE = _calcTotals->roeRate; // save first ROE
    _firstROENoDec = _calcTotals->roeRateNoDec; // and no of decimal
  }
  else
  {
    _calcTotals->roeRate = _firstROE; // use first ROE
    _calcTotals->roeRateNoDec = _firstROENoDec; // and no fo decimal
  }

  collectTaxTotals();

  collectAccTvlInfo();

  collectBSR();

  collectMessage();
}

void
FcCollector::
operator()(const PricingUnit* pu)
{
  set(pu);

  std::vector<const FareUsage*> fus(pu->fareUsage().begin(), pu->fareUsage().end());

  if (LIKELY(fus.size() > 0))
  {
    _calcTotals->farePathInfo.fareCurrencyCode = fus.front()->paxTypeFare()->currency();
    _calcTotals->farePathInfo.fareNoDec = fus.front()->paxTypeFare()->numDecimal();
  }

  std::for_each(fus.begin(), fus.end(), *this);

  _calcTotals->farePathInfo.cpFailedStatus.combine(pu->cpFailedStatus());
}

void
FcCollector::
operator()(const FareUsage* fu)
{
  collectFareTotals(fu);

  collectBookingCode(fu);

  collectDifferentials(fu);

  collectSurcharge(fu);

  collectStopOverSurcharge(fu);

  collectTransferSurcharge(fu);

  collectMileageSurcharge(fu);

  collectPvtFareIndicator(fu);

  if (!fu->ignorePTFCmdPrcFailedFlag())
  {
    _calcTotals->farePathInfo.cpFailedStatus.combine(fu->paxTypeFare()->cpFailedStatus());
  }
  set(fu);
}

bool
FcCollector::needProcessPvtIndicator()
{
  // out of scope:
  //   Shopping (MIP),
  //   hosted airline pricing,
  //   Farex.

  if (LIKELY(_trx->getTrxType() != PricingTrx::PRICING_TRX))
    return false;

  if ((_trx->getOptions() && _trx->getOptions()->fareX()) || // FareX entry
      _trx->getRequest()->ticketingAgent()->tvlAgencyPCC().empty()) // carrier partition
    return false;

  if (!needProcessPvtIndFromTJR())
    return false;
  return true;
}

void
FcCollector::collectPvtFareIndicator(const FareUsage* fu)
{
  if (LIKELY(!needProcessPvtIndicator()))
    return;
  if (!fu)
    return;
  const PaxTypeFare* ptFare = fu->paxTypeFare();
  if (!ptFare)
    return;

  if (ptFare ->fcaDisplayCatType() == 'C')
     PrivateIndicator::resolvePrivateFareInd(
       _calcTotals->privateFareIndSeq, PrivateIndicator::getPrivateFareIndicator(*ptFare, false));
  else
    PrivateIndicator::resolvePrivateFareInd(
    _calcTotals->privateFareIndSeq, PrivateIndicator::getPrivateFareIndicatorOld(*ptFare, false));
}

void
FcCollector::collectFareTotals(const FareUsage* fu)
{
  _calcTotals->pricingUnits[fu] = _pricingUnit;

  bool mileageFare = false;
  int16_t mileagePercent = 0;
  if (!fu->paxTypeFare()->isRouting())
  {
    mileageFare = true;
    mileagePercent = fu->paxTypeFare()->mileageSurchargePctg();
  }

  if (UNLIKELY(fu->travelSeg().empty()))
  {
    throw ErrorResponseException(ErrorResponseException::EMPTY_TRAVEL_SEG);
  }

  for (std::vector<TravelSeg*>::const_iterator i = fu->travelSeg().begin(),
                                               iend = fu->travelSeg().end();
       i != iend;
       ++i)
  {
    // Associate with parent FU
    _calcTotals->fareUsages[*i] = fu;

    int segmentOrder = _farePath->itin()->segmentOrder(*i);
    _calcTotals->travelSegs.insert(std::make_pair(segmentOrder, *i));

    if (mileageFare)
    {
      _calcTotals->mileageTravelSegs[*i] = mileagePercent;
    }
  }

  if (fu->penaltyRestInd() > _calcTotals->maxRestCode[_pricingUnit])
  {
    _calcTotals->maxRestCode[_pricingUnit] = fu->penaltyRestInd();
    _calcTotals->appendageCode[_pricingUnit] = fu->appendageCode();
  }

  ////////////////////////////////////////////////////////////////////////////

  FareBreakPointInfo& fbpInfo = _calcTotals->getFareBreakPointInfo(fu);

  fbpInfo.fareAmount = fu->calculateFareAmount();

  if (!(_trx->getRequest()->ticketingAgent()->axessUser() &&
        _trx->getRequest()->isWpNettRequested()) &&
      fu->paxTypeFare()->isNegotiated()) // NEG Fare, check NetRemit

  {
    const NegFareRest& negFareRest = fu->paxTypeFare()->negotiatedInfo();
    switch (negFareRest.tktFareDataInd1())
    {
    case RuleConst::NR_VALUE_F: // Fare Basis only
      if (fu->tktNetRemitFare() != nullptr)
      {
        fbpInfo.netPubFbc = fu->tktNetRemitFare()->createFareBasis(*_trx);
      }
      break;

    case RuleConst::NR_VALUE_B: // Net Fare Basis and Fare Amount
    case RuleConst::NR_VALUE_N: // Net Fare Basis and Net Fare Amount
      if (TrxUtil::optimusNetRemitEnabled(*_trx))
        getNetPubFareAmountAndFbc(fbpInfo, fu, false);
      break;
    case RuleConst::NR_VALUE_A: // Fare Basis and Fare Amount
      getNetPubFareAmountAndFbc(fbpInfo, fu, true);
      break;
    default: // do nothing
      break;
    }
  }

  const TravelSeg* tvlSeg = TravelSegUtil::lastAirSeg(fu->travelSeg());
  if (LIKELY(tvlSeg != nullptr))
  {
    fbpInfo.fareBasisCode = _calcTotals->getFareBasisCode(
        *_trx, tvlSeg, _fcConfig->fareBasisTktDesLng(), _fcConfig->fcChildInfantFareBasis());
  }

  _calcTotals->ticketFareAmount.push_back(
      FcDispItem::convertAmount(fbpInfo.fareAmount, fu->paxTypeFare()->numDecimal()));

  ////////////////////////////////////////////////////////////////////////////

  // Fare Comp Info:
  CalcTotals::FareCompInfo fareCompInfo;

  fareCompInfo.fareType = fu->paxTypeFare()->isSpecial();

  if (LIKELY(fu->adjustedPaxTypeFare() == nullptr))
  {
    fareCompInfo.fareAmount = fu->paxTypeFare()->fareAmount();
    fareCompInfo.fareNoDec = fu->paxTypeFare()->numDecimal();
    fareCompInfo.fareCurrencyCode = fu->paxTypeFare()->currency();
  }
  else
  {
    fareCompInfo.fareAmount = fu->adjustedPaxTypeFare()->fareAmount();
    fareCompInfo.fareNoDec = fu->adjustedPaxTypeFare()->numDecimal();
    fareCompInfo.fareCurrencyCode = fu->adjustedPaxTypeFare()->currency();
  }

  fareCompInfo.pfcAmount = fu->absorptionAdjustment();

  fareCompInfo.isDiscounted = fu->paxTypeFare()->isDiscounted();
  if (fareCompInfo.isDiscounted)
  {
    if (!fu->paxTypeFare()->fcasTktCode().empty())
    {
      fareCompInfo.discountPercentage = fu->paxTypeFare()->discountInfo().discPercent();
      char tktCodeFirstChar = fu->paxTypeFare()->fcasTktCode()[0];
      fareCompInfo.discountCode =
          (tktCodeFirstChar == '-' || tktCodeFirstChar == '*')
              ? fu->paxTypeFare()->fcasTktCode().substr(1).c_str() // skip - or *
              : fu->paxTypeFare()->fcasTktCode().c_str();
    }
  }
  fareCompInfo.vendor = fu->paxTypeFare()->vendor();
  fareCompInfo.fareBasisCode = fbpInfo.fareBasisCode;
  fareCompInfo.globalDirection = fu->paxTypeFare()->globalDirection();
  fareCompInfo.fareCabin = fu->paxTypeFare()->cabin();

  _calcTotals->farePathInfo.fareCompInfo.insert(std::make_pair(fu, fareCompInfo));

  // Fare Path Info:
  _calcTotals->farePathInfo.fareAmount += fu->totalFareAmount();
  _calcTotals->farePathInfo.pfcAmount += fu->absorptionAdjustment();

  // FIXME: This should be removed when Xform is updated to new fields:
  _calcTotals->fareAmount += fu->totalFareAmount();
  _calcTotals->fareCurrencyCode = fareCompInfo.fareCurrencyCode;
}

void
FcCollector::getNetPubFareAmountAndFbc(FareBreakPointInfo& fbpInfo,
                                       const FareUsage* fu,
                                       bool getFbc) const
{
  FareUsage* netRemitFu1 = nullptr;
  FareUsage* netRemitFu2 = nullptr;
  CalcTotals::getNetRemitFareUsage(_farePath, fu, netRemitFu1, netRemitFu2);

  if (netRemitFu1 != nullptr)
  {
    fbpInfo.netPubFareAmount = netRemitFu1->paxTypeFare()->totalFareAmount() -
                               netRemitFu1->getDiscAmount() - netRemitFu1->absorptionAdjustment() +
                               netRemitFu1->minFarePlusUp().getSum(HIP);

    if (netRemitFu2 != nullptr)
    {
      fbpInfo.netPubFareAmount +=
          netRemitFu2->paxTypeFare()->totalFareAmount() - netRemitFu2->getDiscAmount() -
          netRemitFu2->absorptionAdjustment() + netRemitFu2->minFarePlusUp().getSum(HIP);
    }

    if (getFbc)
      fbpInfo.netPubFbc = fu->tktNetRemitFare()->createFareBasis(*_trx);
  }
}

void
FcCollector::collectBookingCode(const FareUsage* fu)
{
  const Itin* itin = _farePath->itin();
  if (UNLIKELY(itin == nullptr))
    return;

  _calcTotals->bookingCodeRebook.resize(itin->travelSeg().size());
  _calcTotals->bkgCodeSegStatus.resize(itin->travelSeg().size());

  bool isLowFareNoAvailability = _trx->getRequest()->isLowFareNoAvailability();

  unsigned int index = 0;
  for (std::vector<TravelSeg*>::const_iterator i = itin->travelSeg().begin(),
                                               iend = itin->travelSeg().end();
       i != iend;
       ++i, ++index)
  {
    if (!_calcTotals->bookingCodeRebook[index].empty())
      continue;

    std::map<const TravelSeg*, const FareUsage*>::iterator iter = _calcTotals->fareUsages.find(*i);
    if (iter == _calcTotals->fareUsages.end())
      continue;

    std::vector<TravelSeg*>::const_iterator tsI =
        std::find(iter->second->travelSeg().begin(), iter->second->travelSeg().end(), *i);
    if (UNLIKELY(tsI == iter->second->travelSeg().end()))
      continue;

    unsigned int tsIndex = std::distance(iter->second->travelSeg().begin(), tsI);

    if (iter->second->segmentStatus().size() > tsIndex)
    {
      if (UNLIKELY(isLowFareNoAvailability && iter->second->segmentStatus()[tsIndex]._bkgCodeReBook.empty()))
      {
        _calcTotals->bookingCodeRebook[index] = (*i)->getBookingCode();
        LOG4CXX_DEBUG(logger,
                      "BKG rebook " << index << " " << _calcTotals->bookingCodeRebook[index]);
      }
      else
      {
        if(!fallback::fallbackPriceByCabinActivation(_trx) && !_trx->getOptions()->cabin().isUndefinedClass() &&
           (*i)->rbdReplaced() && iter->second->segmentStatus()[tsIndex]._bkgCodeReBook.empty())
        {
          _calcTotals->bookingCodeRebook[index] = (*i)->getBookingCode();
          LOG4CXX_DEBUG(logger,
                        "BKG rebook " << index << " " << _calcTotals->bookingCodeRebook[index]);
        }
        else
        {
          _calcTotals->bookingCodeRebook[index] =
          iter->second->segmentStatus()[tsIndex]._bkgCodeReBook;
          LOG4CXX_DEBUG(logger,
                        "BKG rebook " << index << " " << _calcTotals->bookingCodeRebook[index]);
        }
      }
      _calcTotals->bkgCodeSegStatus[index] =
          iter->second->segmentStatus()[tsIndex]._bkgCodeSegStatus;
    }
  }
}

void
FcCollector::collectDifferentials(const FareUsage* fu)
{
  //    if (fu->differentialAmt() == 0)
  //@Change made to allow display ReBooked Booking code for Differential with amount==0
  if (fu->differentialPlusUp().empty())
    return;

  const Itin* itin = _farePath->itin();
  if (itin == nullptr)
    return;

  std::vector<TravelSeg*>::const_iterator i, iEnd, iFu;
  std::vector<PaxTypeFare::SegmentStatus>::const_iterator iter, iterEnd, iterFu;

  bool isLowFareNoAvailability = _trx->getRequest()->isLowFareNoAvailability();
  bool isLowFareRequested = _trx->getRequest()->isLowFareRequested();

  for (std::vector<DifferentialData*>::const_iterator diffI = fu->differentialPlusUp().begin(),
                                                      diffIEnd = fu->differentialPlusUp().end();
       diffI != diffIEnd;
       ++diffI)
  {
    DifferentialData* diff = *diffI;
    if (diff == nullptr)
      continue;

    DifferentialData::STATUS_TYPE aStatus = diff->status();

    if (aStatus == DifferentialData::SC_PASSED || aStatus == DifferentialData::SC_CONSOLIDATED_PASS)
    {
      _calcTotals->differentialData.push_back(diff);

      // @Do not collect differential Fare Basis codes if Diff Amount == 0
      if (diff->amount())
      {
        // Collect differential fare basis code for TravelSeg
        for (i = diff->travelSeg().begin(); i != diff->travelSeg().end(); ++i)
        {
          _calcTotals->differentialFareBasis.insert(
              std::pair<const TravelSeg*, std::string>(*i, diff->fareClassHigh().c_str()));
        }
      }
      // Add rebook Booking Code if exists from the Differential Fare
      for (i = diff->travelSeg().begin(),
          iEnd = diff->travelSeg().end(),
          iter = diff->fareHigh()->segmentStatus().begin(),
          iterEnd = diff->fareHigh()->segmentStatus().end();
           i != iEnd && iter != iterEnd;
           ++i, ++iter)
      {
        int index = itin->segmentOrder(*i) - 1;

        if (!_calcTotals->bookingCodeRebook[index].empty())
          continue;

        if (isLowFareNoAvailability &&
            (*iter)._bkgCodeReBook.empty()) // WPNCS entry and need not rebook
        {
          _calcTotals->bookingCodeRebook[index] = (*i)->getBookingCode();
          LOG4CXX_DEBUG(logger,
                        "D BKG rebook " << index << " " << _calcTotals->bookingCodeRebook[index]);
        }
        else if (isLowFareRequested &&
                 (*iter)._bkgCodeReBook.empty()) // WPNC and no rebook, then check the FU for rebook
        {
          iFu = fu->travelSeg().begin();
          iterFu = fu->segmentStatus().begin();

          for (; iFu != fu->travelSeg().end() && iterFu != fu->segmentStatus().begin();
               ++iFu, ++iterFu)
          {
            if (index != (itin->segmentOrder(*iFu) - 1))
              continue;

            _calcTotals->bookingCodeRebook[index] = (*iterFu)._bkgCodeReBook;
            LOG4CXX_DEBUG(logger,
                          "D BKG rebook " << index << " " << _calcTotals->bookingCodeRebook[index]);
          }
        }
        else
        {
          _calcTotals->bookingCodeRebook[index] = (*iter)._bkgCodeReBook;
          LOG4CXX_DEBUG(logger,
                        "D BKG rebook " << index << " " << _calcTotals->bookingCodeRebook[index]);
        }
        _calcTotals->bkgCodeSegStatus[index] = (*iter)._bkgCodeSegStatus;
      }
    }
  }
}

void
FcCollector::collectSurcharge(const FareUsage* fu)
{
  for (std::vector<SurchargeData*>::const_iterator surchargeIter = fu->surchargeData().begin(),
                                                   surchargeIterEnd = fu->surchargeData().end();
       surchargeIter != surchargeIterEnd;
       ++surchargeIter)
  {
    SurchargeData* surcharge = (*surchargeIter);

    if ((surcharge->travelPortion() == RuleConst::PERTICKET) &&
        (_farePath->pricingUnit().size() > 1 ||
         (_farePath->pricingUnit().size() == 1 &&
          _farePath->pricingUnit().front()->fareUsage().size() > 1)))
    {
      surcharge->fcFpLevel() = true;
    }

    const std::vector<TravelSeg*>& travelSegs = fu->travelSeg();
    for (const auto travelSeg : travelSegs)
    {
      if ((travelSeg->origAirport() == surcharge->brdAirport()) &&
          (travelSeg->destAirport() == surcharge->offAirport()))
      {
        LOG4CXX_DEBUG(logger,
                      "[" << travelSeg->origAirport() << " " << travelSeg->destAirport() << "] "
                          << "Surcharge " << surcharge->brdAirport() << " "
                          << surcharge->offAirport() << " " << surcharge->amountNuc());
        _calcTotals->surcharges[travelSeg].push_back(surcharge);
        if (surcharge->amountNuc() != 0 && surcharge->selectedTkt())
        {
          _calcTotals->totalNetCharges += surcharge->amountNuc();
        }
        break;
      }
    }
  }
}

void
FcCollector::collectStopOverSurcharge(const FareUsage* fu)
{
  const FareUsage::StopoverSurchargeMultiMap& stopoverSurcharges = fu->stopoverSurcharges();
  FareUsage::StopoverSurchargeMultiMap::const_iterator i = stopoverSurcharges.begin();
  for (; i != stopoverSurcharges.end(); ++i)
  {
    _calcTotals->stopoverSurcharges[i->first].push_back(i->second);
    _calcTotals->totalNetCharges += i->second->amount();

    if (!i->second->isSegmentSpecific() && i->second->amount() > 0)
    {
      _calcTotals->stopOverSurcharge.count++;
      _calcTotals->stopOverSurcharge.total += i->second->amount();
      _calcTotals->stopOverSurcharge.pubCurrency.insert(i->second->unconvertedCurrencyCode());

      if (i->second->isFromOverride())
      {
        _calcTotals->stopOverSurchargeByOverride.count++;
        _calcTotals->stopOverSurchargeByOverride.total += i->second->amount();
      }
    }
  }
}

void
FcCollector::correctSTOSurOverrideCnt()
{
  if (LIKELY(_trx->excTrxType() != PricingTrx::PORT_EXC_TRX))
    return;

  const ExchangePricingTrx& excTrx = static_cast<ExchangePricingTrx&>(*_trx);
  int16_t cntAdjust = excTrx.exchangeOverrides().journeySTOOverrideCnt() - 1;
  if (cntAdjust <= 0)
    return;

  _calcTotals->stopOverSurcharge.count += cntAdjust;
  _calcTotals->stopOverSurchargeByOverride.count += cntAdjust;
}

void
FcCollector::collectTransferSurcharge(const FareUsage* fu)
{
  const FareUsage::TransferSurchargeMultiMap& transferSurcharges = fu->transferSurcharges();
  FareUsage::TransferSurchargeMultiMap::const_iterator i = transferSurcharges.begin();
  for (; i != transferSurcharges.end(); ++i)
  {
    _calcTotals->transferSurcharges[i->first].push_back(i->second);
    _calcTotals->totalNetCharges += i->second->amount();

    if (!i->second->isSegmentSpecific() && i->second->amount() > 0)
    {
      _calcTotals->transferSurcharge.count++;
      _calcTotals->transferSurcharge.total += i->second->amount();
      _calcTotals->transferSurcharge.pubCurrency.insert(i->second->unconvertedCurrencyCode());
    }
  }
}

void
FcCollector::collectMileageSurcharge(const FareUsage* fu)
{
  const PaxTypeFare* ptf = fu->paxTypeFare();
  if (UNLIKELY(!ptf))
    return;

  const FareMarket* fm = ptf->fareMarket();
  if (UNLIKELY(!fm))
    return;

  const MileageInfo* mi = ptf->mileageInfo();
  if (!mi)
    return;

  bool tpdSpecificCity = false;

  const std::vector<TravelSeg*>& tvlSeg = fu->travelSeg();
  for (const auto ts : tvlSeg)
  {
    if (mi->tpd()) // TPD applied
    {
      for (const auto& tpd : mi->tpdMatchedViaLocs())
      {
        if (ts->pnrSegment() == tpd)
        {
          tpdSpecificCity = true;
          _calcTotals->extraMileageTravelSegs.insert(ts);
        }
        else
        {
          if (_trx->getTrxType() == PricingTrx::MIP_TRX && !_trx->isAltDates())
          {
            if (_trx->checkPnrSegmentCollocation(tpd, ts->pnrSegment()))
            {
              tpdSpecificCity = true;
              _calcTotals->extraMileageTravelSegs.insert(ts);
            }
          }
        }
      }
    }
  } // end For loop

  // Section 13 ... 5
  if (mi->tpd() && !tpdSpecificCity)
  {
    _calcTotals->extraMileageFareUsages.insert(fu);
  }
}

void
FcCollector::collectTaxTotals()
{
  const TaxResponse* taxResponse = TaxResponse::findFor(_farePath);

  if (!taxResponse)
  {
    _calcTotals->getMutableFcTaxInfo().initialize(_trx, _calcTotals, _fcConfig, nullptr);
  }
  else
  {
    _calcTotals->getMutableFcTaxInfo().initialize(_trx, _calcTotals, _fcConfig, taxResponse);
  }

  //
  // Ticketing Tax Info:
  //

  if (_farePath->calculationCurrency() == NUC)
  {
    _calcTotals->fclROE += " ROE";

    int16_t inNoDec = 6; // Per Darrin Wei No Truncation of ROE. PL#11553
    int16_t outNoDec = 0;

    if (_calcTotals->roeRate == floor(_calcTotals->roeRate))
      outNoDec = 2;

    if (_calcTotals->roeRate == 1.00)
    {
      inNoDec = 2;
      outNoDec = 2;
    }

    _calcTotals->fclROE += FcDispItem::convertAmount(_calcTotals->roeRate, inNoDec, outNoDec);
  }

  const std::vector<std::string>& zpTaxInfo = _calcTotals->zpTaxInfo();
  if (zpTaxInfo.size() > 0)
  {
    std::ostringstream os;
    os << ' ' << FareCalcConsts::TAX_CODE_ZP;
    std::copy(zpTaxInfo.begin(), zpTaxInfo.end(), std::ostream_iterator<std::string>(os));
    _calcTotals->fclZP = os.str();
  }

  LOG4CXX_DEBUG(logger,
                "\nfclROE: " << _calcTotals->fclROE << "\nfclZP: " << _calcTotals->fclZP
                             << "\nfclXT: " << _calcTotals->fclXT
                             << "\nfclXF: " << _calcTotals->fclXF);
}

void
FcCollector::collectBSR()
{
  Agent* agent = _trx->getRequest()->ticketingAgent();
  if (agent == nullptr)
    return;

  if ((_fcConfig->displayBSR() == FareCalcConsts::FC_YES) &&
      agent->hostCarrier().empty() && // Not Airline PCC
      (agent->tvlAgencyPCC().size() == 4) && // 4 AlphaNumeric PCC
      (_fcConfig->userApplType() == CRS_USER_APPL) && // User Sabre
      (_fcConfig->userAppl() == SABRE_USER) &&
      (_fcConfig->loc1().locType() == LOCTYPE_NATION) && // Nation Mexico
      (_fcConfig->loc1().loc() == MEXICO) &&
      (_calcTotals->bsrRate2 == 0.0)) // Only when single conversion happens
  {
    // Display BSR at end of Fare Calc line
    std::string amountBSR;

    // Max 4 decimal points
    unsigned bsrNoDec = (_calcTotals->bsrRate1NoDec < 4 ? _calcTotals->bsrRate1NoDec : 4);
    if ((_calcTotals->equivCurrencyCode == _trx->getOptions()->currencyOverride()) ||
        (_calcTotals->equivCurrencyCode == agent->currencyCodeAgent()))
    {
      amountBSR = FcDispItem::convertAmount(_calcTotals->bsrRate1, bsrNoDec, bsrNoDec);
    }
    else
    {
      amountBSR = FcDispItem::convertAmount(1 / _calcTotals->bsrRate1, bsrNoDec, bsrNoDec);
    }

    _calcTotals->fclBSR = " 1";
    _calcTotals->fclBSR += _calcTotals->convertedBaseFareCurrencyCode;
    _calcTotals->fclBSR += "/";
    _calcTotals->fclBSR += amountBSR;
    _calcTotals->fclBSR += _calcTotals->equivCurrencyCode;
  }
}

bool
FcCollector::convertAmounts()
{
  FareAmountsConverter converter(_trx,
                                 _farePath,
                                 _fcConfig,
                                 _calcTotals,
                                 _lastBaseFareCurrencyCode,
                                 _lastConvertedBaseFareNoDec);
  converter.convertAmounts();

  return true;
}

void
FcCollector::collectAccTvlInfo()
{
  // for both WPA and WP, we need the accompanied travel data now
  // although the data is still saved in wpaInfo
  AccTvlDetailOut<std::ostringstream> accTvlDetailOut;
  std::ostringstream accTvlData;

  accTvlDetailOut.reqAccTvl() = false; // reset
  accTvlDetailOut.msgType() = accTvlDetailOut.WP_MSG_TYPE; // reset
  AltPricingTrx* altPricingTrx = dynamic_cast<AltPricingTrx*>(_trx);

  accTvlDetailOut.storeAccTvlDetail(_trx, accTvlData, _calcTotals->truePaxType, *_farePath);
  _calcTotals->wpaInfo.accTvlData = accTvlData.str();
  _calcTotals->wpaInfo.reqAccTvl = accTvlDetailOut.reqAccTvl();
  _calcTotals->wpaInfo.tktGuaranteed = accTvlDetailOut.tktGuaranteed();

  // Accompanied Travel Trailer Message for AltPricingTrx or cat19 passenger
  // by him self;
  // No warning message for INF, INF already has standard warning message
  // *EACH INF REQUIRES ACCOMPANYING ADULT PASSENGER*
  if (altPricingTrx && (altPricingTrx->altTrxType() == AltPricingTrx::WP_NOMATCH ||
                        altPricingTrx->altTrxType() == AltPricingTrx::WPA) &&
      !_farePath->paxType()->paxTypeInfo().isInfant())
  {
    accTvlDetailOut.appendTrailerMsg(*_calcTotals);
  }
  else if (!_farePath->paxType()->paxTypeInfo().isInfant() && accTvlDetailOut.warnAccTvl())
  {
    accTvlDetailOut.appendTrailerMsg(*_calcTotals);
  }
}

void
FcCollector::collectMessage()
{
  collectFareTypeMessage();
  collectPtcMessage();
  collectPvtIndicatorMessage(); // collect PVT before Cxr on all types of WP

  collectValidatingCarrierMessage();
  collectRetailerRuleMessage();
  if (!fallback::fallbackCommissionManagement(_trx))
    collectCommissionTrailerMessages();

  collectServiceFeesTemplate();
  collectMaxPenaltyMessage();
  collectCat15HasTextTable();
  collectMatchedAccCodeTrailerMessage();

  collectBaggage();
}

void
FcCollector::collectBaggage()
{
  if (TrxUtil::isBaggage302DotActivated(*_trx) && !_trx->noPNRPricing() &&
      !_calcTotals->farePath->baggageResponse().empty())
  {
    std::ostringstream convert;
    convert << std::setw(3) << std::setfill('0') << (_fcCollector->baggageTagMap().size() + 1);
    BaggageFcTagType baggageIndicator = FreeBaggageUtil::BaggageTagHead + convert.str();

    _fcCollector->baggageTagMap().insert(std::make_pair(baggageIndicator, _calcTotals));
    _calcTotals->fcMessage.push_back(FcMessage(FcMessage::BAGGAGE, 0, baggageIndicator));
  }
}

void
FcCollector::collectMaxPenaltyMessage()
{
  const MaxPenaltyResponse* response = _farePath->maxPenaltyResponse();
  if (response && _trx->getTrxType() == PricingTrx::PRICING_TRX &&
      !_trx->getRequest()->isSFR())
  {
    std::ostringstream msg;
    bool somePeriodsNonChg = false;
    bool somePeriodsNonRef = false;
    bool anyMissingData = false;
    bool allMissingData = true;

    auto addMaxPenMessage = [&](const MaxPenaltyResponse::Fee& fee,
                                const smp::RecordApplication cat16App,
                                const std::string& msgString,
                                bool& somePeriodsNon) -> void
    {
      if (cat16App && fee.isMissingData())
      {
        msg << msgString << " DEP VERIFY RULES";
        anyMissingData = true;
      }
      else
      {
        allMissingData = false;
        if (fee._fee)
        {
          msg << msgString << " DEP UP TO " << fee._fee->toString();
          if (fee._non)
          {
            somePeriodsNon = true;
          }
        }
        else
        {
          msg << "NON-" << msgString << " DEP";
        }
      }
      msg << '/';
    };

    addMaxPenMessage(response->_changeFees._before,
                     response->_changeFees._cat16 & smp::BEFORE,
                     "CHG BEF",
                     somePeriodsNonChg);
    addMaxPenMessage(response->_changeFees._after,
                     response->_changeFees._cat16 & smp::AFTER,
                     "CHG AFT",
                     somePeriodsNonChg);
    addMaxPenMessage(response->_refundFees._before,
                     response->_refundFees._cat16 & smp::BEFORE,
                     "REF BEF",
                     somePeriodsNonRef);
    addMaxPenMessage(response->_refundFees._after,
                     response->_refundFees._cat16 & smp::AFTER,
                     "REF AFT",
                     somePeriodsNonRef);

    if (somePeriodsNonChg || somePeriodsNonRef)
    {
      msg << "OTHERWISE";

      if (somePeriodsNonChg)
      {
        msg << " NON-CHG";
      }

      if (somePeriodsNonRef)
      {
        msg << " NON-REF";
      }

      msg << '/';
    }

    if (allMissingData)
    {
      msg.clear();
      msg.str("CAT 16 PENALTIES FREE TEXT FOUND - VERIFY RULES");
    }
    else if (anyMissingData)
    {
      msg << "FREE TEXT FOUND";
    }
    else
    {
      msg << "SEE RULES";
    }

    auto predicate = [](char c)
    {
      return c == ' ' || c == '/';
    };

    std::string::size_type lineWidth = WINDOW_WIDTH;
    // if 'ATTN*' prefix is present we have to consider it when calculating line width
    if (_fcConfig->warningMessages() == FareCalcConsts::FC_YES)
      lineWidth -= 5;

    int typeIndex = 0;
    FcMessage::MessageSubType type[] = {FcMessage::SMPPART1,
                                        FcMessage::SMPPART2,
                                        FcMessage::SMPPART3};

    for (std::string& line : MessageWrapper::wrap(msg.str(), predicate, lineWidth))
    {
      boost::algorithm::trim_all(line);
      TSE_ASSERT(typeIndex < 3);
      _calcTotals->fcMessage.emplace_back(FcMessage::WARNING, 0, std::move(line), true,
                                          FcMessage::UNSPECIFIED, type[typeIndex++]);
    }
  }
}

void
FcCollector::collectFareTypeMessage()
{
  // process fare type message for fare type pricing entry.
  if (LIKELY(!_trx->getOptions()->isFareFamilyType()))
    return;

  FareTypeMatcher ftMatcher(*_trx, _farePath);
  if (ftMatcher(_farePath))
  {
    if (ftMatcher.groupTrailerMsg())
    {
      FcMessage message(FcMessage::WARNING, 0, "VERIFY GROUP SIZE");
      _calcTotals->fcMessage.push_back(message);
      _fcCollector->addMultiMessage(*_trx, _farePath, message);
    }

    if (ftMatcher.itTrailerMsg())
    {
      FcMessage message(FcMessage::WARNING, 0, "IT NUMBER REQUIRED");
      _calcTotals->fcMessage.push_back(message);
      _fcCollector->addMultiMessage(*_trx, _farePath, message);
    }
  }
}

void
FcCollector::collectPtcMessage()
{
  PricingTrx& pricingTrx = *_trx;
  CalcTotals& calcTotals = *_calcTotals;

  LOG4CXX_DEBUG(logger, " REQ. PAX TYPE: " << calcTotals.requestedPaxType);
  LOG4CXX_DEBUG(logger, " PAX TYPE     : " << calcTotals.farePath->paxType()->paxType());
  LOG4CXX_DEBUG(logger, " TRUE PAX TYPE: " << calcTotals.truePaxType);

  if (calcTotals.mixedPaxType)
  {
    calcTotals.fcMessage.push_back(FcMessage(FcMessage::WARNING,
                                             0,
                                             "MIXED PASSENGER TYPES - VERIFY RESTRICTIONS",
                                             true,
                                             FcMessage::TRUE_PTC_MESSAGE));
  }

  ////////////////////////////////////////////////////////////////////////
  //

  {
    bool disp = false;

    if (calcTotals.negFareUsed && (calcTotals.truePaxType == NEG || calcTotals.truePaxType == CNE ||
                                   calcTotals.truePaxType == INE))
    {
      if ((calcTotals.requestedPaxType != NEG) && (calcTotals.requestedPaxType != CNE) &&
          (calcTotals.requestedPaxType != INE))
      {
        if (!pricingTrx.getOptions()->isFareFamilyType() &&
            _fcConfig->truePsgrTypeInd() == FareCalcConsts::FC_YES &&
            _fcConfig->negPermitted() == NEG_FARES_PERMITTED_WITH_MESSAGE)
        {
          calcTotals.fcMessage.push_back(
              FcMessage(FcMessage::WARNING,
                        0,
                        calcTotals.truePaxType + " FARE USED - VERIFY RESTRICTIONS",
                        true,
                        FcMessage::TRUE_PTC_MESSAGE));
          disp = true;
        }
      }

      if (!disp && !_trx->getRequest()->ticketingAgent()->abacusUser() &&
          (calcTotals.requestedPaxType == NEG || calcTotals.requestedPaxType == CNE ||
           calcTotals.requestedPaxType == INE) &&
          calcTotals.requestedPaxType == calcTotals.truePaxType)
      {
        if (!_trx->getRequest()->ticketingAgent()->infiniUser())
        {
          calcTotals.fcMessage.push_back(FcMessage(FcMessage::WARNING,
                                                   0,
                                                   "NEGOTIATED FARES APPLY - VALIDATE ALL RULES",
                                                   true,
                                                   FcMessage::REMAINING_WARNING_MSG));
          disp = true;
        }
      }
    }

    if (!disp && calcTotals.privateFareUsed)
    {
      calcTotals.fcMessage.push_back(
          FcMessage(FcMessage::WARNING,
                    0,
                    "PRIVATE FARE APPLIED - CHECK RULES FOR CORRECT TICKETING",
                    true,
                    FcMessage::REMAINING_WARNING_MSG));
    }

    if (!disp && !requestAndTruePaxTypeSame())
    {
      if (!pricingTrx.getOptions()->isFareFamilyType() &&
          _fcConfig->truePsgrTypeInd() == FareCalcConsts::FC_YES &&
          _fcConfig->negPermitted() == NEG_FARES_PERMITTED_WITH_MESSAGE)
      {
        calcTotals.fcMessage.push_back(
            FcMessage(FcMessage::WARNING,
                      0,
                      calcTotals.truePaxType + " FARE USED - VERIFY RESTRICTIONS",
                      true,
                      FcMessage::TRUE_PTC_MESSAGE));
      }
      else if (!_trx->getRequest()->ticketingAgent()->abacusUser())
      {
        if (!_trx->getRequest()->ticketingAgent()->infiniUser())
        {
          calcTotals.fcMessage.push_back(
              FcMessage(FcMessage::TRAILER,
                        0,
                        calcTotals.requestedPaxType + " NOT APPLICABLE - " +
                            calcTotals.truePaxType + " FARE USED - VERIFY RESTRICTIONS",
                        true,
                        FcMessage::TRUE_PTC_MESSAGE));
        }
      }
    }
  }

  //
  ////////////////////////////////////////////////////////////////////////

  if (calcTotals.farePath->fuelSurchargeIgnored())
  {
    calcTotals.fcMessage.push_back(
        FcMessage(FcMessage::WARNING, 0, "SURCHARGE OVERRIDE APPLIED/FARE NOT GUARANTEED"));
  }

  if (_trx->getRequest()->isPtsOverride())
  {
    if (_trx->getRequest()->ticketingAgent()->abacusUser() ||
        _trx->getRequest()->ticketingAgent()->infiniUser())
    {
      calcTotals.fcMessage.push_back(FcMessage(
          FcMessage::WARNING, 0, "PAPER TKT SURCH OVERRIDE/ FARE NOT GUARANTEED IF TKTD"));
    }
    else
    {
      calcTotals.fcMessage.push_back(
          FcMessage(FcMessage::WARNING,
                    0,
                    "PAPER TKT SURCHARGE OVERRIDE APPLIED/FARE NOT GUARANTEED IF TKT"));
    }
  }

  const PaxType& requestedPaxType = *(calcTotals.farePath->paxType());
  if (PaxTypeUtil::isInfant(requestedPaxType.paxType(), requestedPaxType.vendorCode()))
  {
    calcTotals.fcMessage.push_back(FcMessage(FcMessage::WARNING,
                                             0,
                                             "EACH INF REQUIRES ACCOMPANYING ADT PASSENGER",
                                             true,
                                             FcMessage::REMAINING_WARNING_MSG));
  }
}

void
FcCollector::collectValidatingCarrierMessage()
{
  if (LIKELY(_fcConfig->valCxrDisplayOpt() == YES))
  {
    if (_trx->isValidatingCxrGsaApplicable())
      collectValidatingCarrierMessageForGSA();
    else
      collectValidatingCarrierMessageForNonGSA();
  }
}

/*
 * Create Agency Commission Trailer message with default validating carrier appearing on top
 * We always show default val-cxr when alternates are present, even when def val-cxr does not
 * qualifies for AMC
 */
void
FcCollector::collectCommissionTrailerMessages() const
{
  const FarePath& fp = *_calcTotals->farePath;
  if (!fp.isAgencyCommissionQualifies())
    return;

  const CarrierCode& defValCxr = _trx->isValidatingCxrGsaApplicable() ?
    fp.defaultValidatingCarrier() :
    (fp.itin() ? fp.itin()->validatingCarrier() : "");

  if (!defValCxr.empty())
    constructCommTrailerMsgForDefaultValCxr(fp, defValCxr, "");

  bool isNoDefAndMultiSwaps =
    defValCxr.empty() &&
    _calcTotals->farePath->itin()->hasNeutralValidatingCarrier();

  std::string text(isNoDefAndMultiSwaps ? "OPT" : "ALT");
  if (!fp.validatingCarriers().empty())
    constructCommTrailerMsgForAlternateValCxr(fp, text);
}

void
FcCollector::constructCommTrailerMsgForDefaultValCxr(
    const FarePath& fp,
    const CarrierCode& defValCxr,
    const std::string& text) const

{
  auto defCommIt = fp.valCxrCommissionAmount().find(defValCxr);
  if (defCommIt != fp.valCxrCommissionAmount().end())
    constructAgencyTrailerMsg(defValCxr, text, defCommIt->second);
  else if (fallback::fallbackAMCPhase2(_trx))
  {
    std::string msg;
    createCommNotFoundTrailerMsg(defValCxr, text, msg);
    if (!msg.empty())
      _calcTotals->fcMessage.push_back(FcMessage(FcMessage::AGENCY_COMM_MSG, 0, msg));
  }
}

void
FcCollector::constructCommTrailerMsgForAlternateValCxr(
    const FarePath& fp,
    const std::string& text) const
{
  std::vector<CarrierCode> cxrWithNoComm;
  std::map<MoneyAmount, std::vector<CarrierCode>, amc::compareMoneyAmount> commonCommAmtCxrs;
  for (const CarrierCode& cxr : fp.validatingCarriers())
  {
    auto it = fp.valCxrCommissionAmount().find(cxr);
    it != fp.valCxrCommissionAmount().end() ?
      commonCommAmtCxrs[it->second].push_back(cxr):
      cxrWithNoComm.push_back(cxr);
  }

  // @todo constructAgencyTrailerMsg should be refactored for optimization
  for (auto& p : commonCommAmtCxrs)
  {
    std::sort(p.second.begin(), p.second.end());
    constructAgencyTrailerMsg(
        amc::util::concatenateValCxrForCommMsg(p.second), text, p.first);
  }

  if (fallback::fallbackAMCPhase2(_trx))
  {
    if (!cxrWithNoComm.empty())
    {
      std::string msg;
      std::sort(cxrWithNoComm.begin(), cxrWithNoComm.end());
      for (const CarrierCode& cxr : cxrWithNoComm)
        createCommNotFoundTrailerMsg(cxr, text, msg);
      if (!msg.empty())
        _calcTotals->fcMessage.push_back(FcMessage(FcMessage::AGENCY_COMM_MSG, 0, msg));
    }
  }
}

void
FcCollector::constructAgencyTrailerMsg(const std::string& cxrStr,
    const std::string& text,
    MoneyAmount commAmt) const
{
  _calcTotals->fcMessage.push_back(
      FcMessage(FcMessage::AGENCY_COMM_MSG, 0,
        createAgencyCommTrailerMsg(cxrStr, text, commAmt)));
}

std::string
FcCollector::createAgencyCommTrailerMsg(const std::string& valCxr,
    const std::string& text,
    MoneyAmount commAmt) const
{
  std::string res;
  std::ostringstream os;

  os<< "AGENCY COMMISSION"
    << (text.empty() ? "" : " ")
    << text
    << (text.empty() ? " VAL CARRIER " : " VAL CARRIER/S ")
    << valCxr
    << " - "
    << _calcTotals->equivCurrencyCode
    << std::fixed<<std::setprecision(_calcTotals->equivNoDec) << commAmt;

  res = os.str();
  return res;
}

void
FcCollector::collectValidatingCarrierMessageForNonGSA() const
{
  std::string message = "VALIDATING CARRIER ";
  CarrierCode carrier = _trx->getRequest()->validatingCarrier();

  if (!carrier.empty())
    message += "SPECIFIED ";
  else
  {
    const Itin* itin = _calcTotals->farePath->itin();
    carrier = itin ? itin->validatingCarrier() : CarrierCode();
  }

  message += "- " + MCPCarrierUtil::swapToPseudo(_trx, carrier);
  _calcTotals->fcMessage.push_back(
      FcMessage(FcMessage::WARNING, 0, message, true, FcMessage::CARRIER_MESSAGE));
}

void
FcCollector::processNSPVcxrMsg() const
{
  const FarePath& fp = *_calcTotals->farePath;
  std::vector<std::string> nspValCxrMsgCol;
  std::string nspHeader;
  for(const auto& pair : fp.defaultValCxrPerSp())
  {
    auto its = fp.settlementPlanValidatingCxrs().find(pair.first);
    if ( its != fp.settlementPlanValidatingCxrs().end())
    {
      if(pair.first == NO_SETTLEMENTPLAN)
      {
        buildNSPValidatingCarrierMessage(fp, pair.second, its->second, nspHeader, nspValCxrMsgCol);
        break;
      }
    }
  }
  if(!nspHeader.empty())
  _calcTotals->fcMessage.push_back(
          FcMessage(FcMessage::WARNING, 0, nspHeader, true, FcMessage::CARRIER_MESSAGE, FcMessage::VALIDATINGCXR) );

  for (const auto& trailerMsg : nspValCxrMsgCol)
      _calcTotals->fcMessage.push_back(
          FcMessage(FcMessage::WARNING, 0, trailerMsg, true, FcMessage::CARRIER_MESSAGE, FcMessage::VALIDATINGCXR));
}
void
FcCollector::collectValidatingCarrierMessageForGSA() const
{
  if ((!fallback::fallbackValidatingCxrMultiSp(_trx) || _trx->overrideFallbackValidationCXRMultiSP()) &&
      !_calcTotals->farePath->defaultValCxrPerSp().empty() &&
      _trx->getRequest()->ticketingAgent()->agentTJR()->isMultiSettlementPlanUser())
  {
    prepareValCxrMsgForMultiSp();
  }
  else
  {
    FcMessage::MessageType msgType = FcMessage::WARNING;
    const std::string vcxrMsg = buildValidatingCarrierMessage(msgType);
    if(!vcxrMsg.empty())
      _calcTotals->fcMessage.push_back(FcMessage(
          msgType, 0, vcxrMsg, true, FcMessage::CARRIER_MESSAGE, FcMessage::VALIDATINGCXR));
    if((!fallback::fallbackNonBSPVcxrPhase1(_trx) || _trx->overrideFallbackValidationCXRMultiSP()) &&
        (_trx->getRequest()->spvInd() == tse::spValidator::noSMV_noIEV ||
        _trx->getRequest()->spvInd() == tse::spValidator::noSMV_IEV))
    {
      processNSPVcxrMsg();
    }
  }
}

void
FcCollector::collectRetailerRuleMessage()
{
  if (!fallback::fallbackFRRProcessingRetailerCode(_trx))
  {
     FcMessage::MessageType msgType = FcMessage::GENERAL;
     CalcTotals& calcTotal = *_calcTotals;
     std::string msg = AdjustedSellingUtil::getRetailerCodeFromFRR(calcTotal);

    if (!msg.empty())
    {
      if (msg == " ")
      {
        msg = "MULTIPLE AGENCY RETAILER RULE QUALIFIERS USED";
        if (_trx->getTrxType() == PricingTrx::PRICING_TRX)
        {
          msg += " - SEE WPRD*RR";
        }
        calcTotal.fcMessage.push_back(FcMessage(msgType, 0, msg, true,
              FcMessage::RETAILER_RULE_MESSAGE));
      }
      else
      {
        msg = "AGENCY RETAILER RULE QUALIFIER USED: " + msg;
        calcTotal.fcMessage.push_back(FcMessage(msgType, 0, msg, true,
                                                FcMessage::RETAILER_RULE_MESSAGE));
      }
    }
    return;
  }
  return;
}

/// Collect def, alt, optional val cxr messages
void
FcCollector::prepareValCxrMsgForMultiSp() const
{
  const FarePath& fp = *_calcTotals->farePath;
  std::vector<CarrierCode> marketingCxrs;
  ValidatingCxrUtil::getMarketingCarriersInItinOrder(*fp.itin(), marketingCxrs);

  std::vector<std::string> defValCxrMsgCol;
  std::vector<std::string> altValCxrMsgCol;
  std::vector<std::string> optValCxrMsgCol;
  std::vector<std::string> nspValCxrMsgCol;
  std::string nspHeader;

  for(const auto& pair : fp.defaultValCxrPerSp())
  {
    auto its = fp.settlementPlanValidatingCxrs().find(pair.first);
    if ( its != fp.settlementPlanValidatingCxrs().end())
    {
      if((!fallback::fallbackNonBSPVcxrPhase1(_trx) || _trx->overrideFallbackValidationCXRMultiSP())
        && pair.first == NO_SETTLEMENTPLAN)
      {
        buildNSPValidatingCarrierMessage(fp, pair.second, its->second, nspHeader, nspValCxrMsgCol);
        continue;
      }

      buildValidatingCarrierMessage(
          fp,
          pair.first,   // settlement-plan
          pair.second,  // settlement's def val-cxrs
          marketingCxrs,
          its->second,  // settlment's all cxrs
          defValCxrMsgCol,
          altValCxrMsgCol,
          optValCxrMsgCol);
    }
  }

  std::string header = "VALIDATING CARRIER";
  if (!_trx->getRequest()->validatingCarrier().empty())
    header += " SPECIFIED " ;
  
  setValCxrTrailerMessages(header, defValCxrMsgCol);

  if (!altValCxrMsgCol.empty())
  {
    header = "ALTERNATE VALIDATING CARRIER/S";
    setValCxrTrailerMessages(header, altValCxrMsgCol);
  }

  if (!optValCxrMsgCol.empty())
  {
    header = "OPTIONAL VALIDATING CARRIER";
    setValCxrTrailerMessages(header, optValCxrMsgCol);
  }

  if((!fallback::fallbackNonBSPVcxrPhase1(_trx) || _trx->overrideFallbackValidationCXRMultiSP())
    && !nspHeader.empty())
    setValCxrTrailerMessages(nspHeader, nspValCxrMsgCol);

}

void
FcCollector::setValCxrTrailerMessages(const std::string& header,
    const std::vector<std::string>& trailerMsgCol) const
{

  if(!fallback::fallbackNonBSPVcxrPhase1(_trx) || _trx->overrideFallbackValidationCXRMultiSP())
  {
    if (trailerMsgCol.empty() && header.find("NO IET")== std::string::npos)
      return;
  }
  else if (trailerMsgCol.empty())
    return;


  if(!fallback::fallbackVCxrTrailerMsgDisplay(_trx))
    _calcTotals->fcMessage.push_back(
        FcMessage(FcMessage::WARNING, 0, header, true, FcMessage::CARRIER_MESSAGE, FcMessage::VALIDATINGCXR) );
  else
    _calcTotals->fcMessage.push_back(
        FcMessage(FcMessage::WARNING, 0, header, true, FcMessage::CARRIER_MESSAGE) );
  for (const auto& trailerMsg : trailerMsgCol)
    _calcTotals->fcMessage.push_back(
        FcMessage(FcMessage::WARNING, 0, trailerMsg, true, FcMessage::CARRIER_MESSAGE, FcMessage::VALIDATINGCXR));
}


void
FcCollector::buildNSPValidatingCarrierMessage(const FarePath& fp,
                                              const CarrierCode& defValCxr,
                                              const std::vector<CarrierCode>& valCxrs,
                                              std::string& nspHeader,
                                              std::vector<std::string>& nspts) const
{
  std::map<NationCode, std::vector<CarrierCode>> nspTrailerMsgMap;
  const SpValidatingCxrGSADataMap* nspSpValidatingCxrGSADataMap = fp.itin()->spValidatingCxrGsaDataMap();

  if(_trx->getRequest()->spvInd() == tse::spValidator::noSMV_noIEV)
    nspHeader = "NO SETTLEMENT PLAN/NO IET VALIDATION";
  else if(_trx->getRequest()->spvInd() == tse::spValidator::noSMV_IEV)
  {
    nspHeader = "NO SETTLEMENT PLAN WITH IET";
    if(nspSpValidatingCxrGSADataMap)
    {
      const auto it = nspSpValidatingCxrGSADataMap->find(NO_SETTLEMENTPLAN);
      if(it != nspSpValidatingCxrGSADataMap->end())
      {
        const ValidatingCxrGSAData* nspValidatingCxrGSAData = it->second;
        if(nspValidatingCxrGSAData)
        {
          const ValidatingCxrDataMap& nspValidatingCxrDataMap = nspValidatingCxrGSAData->validatingCarriersData();
          for(const auto& itr : nspValidatingCxrDataMap)
          {
            if(!fallback::fallbackNonBSPVcxrPhase1R9(_trx))
            {
              if((!defValCxr.empty() && itr.first == defValCxr) ||
                 (!valCxrs.empty() && std::find(valCxrs.begin(), valCxrs.end(), itr.first) != valCxrs.end()))
                for(auto itrs : itr.second.interlineValidCountries)
                  nspTrailerMsgMap[itrs].push_back(itr.first);
              else
                continue;
            }
            else
              for(auto itrs : itr.second.interlineValidCountries)
                nspTrailerMsgMap[itrs].push_back(itr.first);
          }
        }
      }
    }
    for(auto& its : nspTrailerMsgMap)
    {
      std::string vcxrMsg;
      if(!fallback::fallbackNonBSPVcxrPhase1R9(_trx) && !its.second.empty())
      {
        std::vector<CarrierCode> marketingCxrs;
        const SettlementPlanType sp = NO_SETTLEMENTPLAN;
        ValidatingCxrUtil::getMarketingCarriersInItinOrder(*fp.itin(), marketingCxrs);
        ValidatingCxrUtil::getAlternateCxrInOrder(*fp.itin(), marketingCxrs, its.second, &sp);
      }
      vcxrMsg = "IET VALIDATION " + its.first + " -";
      for(auto itts = std::begin(its.second); itts != std::end(its.second); ++itts)
        vcxrMsg = vcxrMsg + " " + *itts;
      nspts.push_back(vcxrMsg);
    }
  }

}
void
FcCollector::buildValidatingCarrierMessage(const FarePath& fp,
                                           const SettlementPlanType& sp,
                                           const CarrierCode& defValCxr,
                                           const std::vector<CarrierCode>& mktCxrs,
                                           const std::vector<CarrierCode>& valCxrs,
                                           std::vector<std::string>& defs,
                                           std::vector<std::string>& alts,
                                           std::vector<std::string>& opts) const
{
  bool hasNVC = fp.itin()->hasNeutralValidatingCarrier(sp);
  std::string defaultMsg = createDefaultTrailerMsg(fp, sp, defValCxr);
  std::vector<CarrierCode> altCxrs, optCxrs;
  for(const auto& vcxr : valCxrs)
  {
    if (vcxr == defValCxr)
    {
      if (defaultMsg.empty()) // There can only be one defValCxr per SP
      {
        defaultMsg = sp;
        defaultMsg += " - ";
        defaultMsg +=  vcxr;
      }
      continue;
    }

    if (!hasNVC)
      altCxrs.push_back(vcxr);
    else
      optCxrs.push_back(vcxr);
  }

  if (!defaultMsg.empty())
    defs.push_back(defaultMsg);

  if (!altCxrs.empty()) // sorted in itin order
    alts.push_back(createAltTrailerMsg(fp, sp, mktCxrs, altCxrs));

  if (!optCxrs.empty()) // sorted in alphabetic order
    opts.push_back(createTrailerMsg(sp, optCxrs));
}

void
FcCollector::collectCat15HasTextTable()
{
  if (_trx->isValidatingCxrGsaApplicable())
  {
    const Itin* itin = _calcTotals->farePath->itin();
    if (determineCat15HasTextTable(*itin))
    {
      std::string message = "CAT 15 SALES RESTRICTIONS FREE TEXT FOUND - VERIFY RULES";

      _calcTotals->fcMessage.push_back(FcMessage(FcMessage::WARNING,
                                                 0,
                                                 message,
                                                 true,
                                                 FcMessage::CARRIER_MESSAGE,
                                                 FcMessage::VALIDATINGCXR));
    }
  }
}

void
FcCollector::collectServiceFeesTemplate()
{
  if (_trx->getOptions()->isServiceFeesTemplateRequested() &&
      !_trx->fareCalcCollector().empty() &&
      !_trx->paxType().empty())
  {
    const CalcTotals *ct = _trx->fareCalcCollector().front()->findCalcTotals(_trx->paxType().front());

    if (ct && ct->requestedPaxType == _calcTotals->requestedPaxType)
    {
      std::string totalLine = "TOTAL WITH SERVICE FEE ";
      if (!_calcTotals->equivCurrencyCode.empty())
      {
        totalLine += _calcTotals->equivCurrencyCode;
      }
      else
      {
        totalLine += _calcTotals->convertedBaseFareCurrencyCode;
      }
      totalLine += FareCalcConsts::SERVICE_FEE_TOTAL_LINE;

      _calcTotals->fcMessage.push_back(FcMessage(FcMessage::WARNING,
                                                 0,
                                                 FareCalcConsts::SERVICE_FEE_AMOUNT_LINE,
                                                 false,
                                                 FcMessage::SERVICE_FEE_TEMPLATE));
      _calcTotals->fcMessage.push_back(FcMessage(FcMessage::WARNING,
                                                 0,
                                                 totalLine,
                                                 false,
                                                 FcMessage::SERVICE_FEE_TEMPLATE));
    }
  }
}

bool
FcCollector::isDefaultVcxrFromPreferred(const CarrierCode& defValCxr) const
{
  if(defValCxr.empty())
    return false;
  if(std::find(_trx->getRequest()->preferredVCs().begin(), _trx->getRequest()->preferredVCs().end(), defValCxr) != _trx->getRequest()->preferredVCs().end())
    return true;
  else
    return false;
}

bool
FcCollector::isAcxrsAreNSPcxrs(const std::vector<CarrierCode>& vecNspCxrs, const std::vector<CarrierCode>& vecAlternateValidatingCxrs) const
{
  std::set<CarrierCode> setNspCxrs(vecNspCxrs.begin(), vecNspCxrs.end());
  std::set<CarrierCode> setAlternateValidatingCxrs(vecAlternateValidatingCxrs.begin(), vecAlternateValidatingCxrs.end());

  return(std::includes(setAlternateValidatingCxrs.begin(), setAlternateValidatingCxrs.end(), setNspCxrs.begin(), setNspCxrs.end()));
}

std::string
FcCollector::buildValidatingCarrierMessage(FcMessage::MessageType& msgType) const
{
  const PricingRequest* request = _trx->getRequest();
  const CarrierCode requestedVcxr = request ? request->validatingCarrier() : CarrierCode();
  const Itin* itin = _calcTotals->farePath->itin();
  const CarrierCode defValCxr = _calcTotals->farePath->defaultValidatingCarrier();
  const CarrierCode swappedCxr = _calcTotals->farePath->marketingCxrForDefaultValCxr();
  vcx::ValidationStatus vcxrMsgType;

  if (!swappedCxr.empty())
  {
    vcxrMsgType = vcx::VALID_SINGLE_GSA;
    if (_trx->getTrxType() != PricingTrx::MIP_TRX)
      msgType = FcMessage::VCX_SINGLE_GSA_SWAP;
  }
  else if (!requestedVcxr.empty() && (requestedVcxr == defValCxr))
  {
    vcxrMsgType = vcx::VALID_OVERRIDE;
  }
  else
  {
    vcxrMsgType = vcx::VALID_MSG;
  }

  std::string vcxrMsg;
  if((!fallback::fallbackNonBSPVcxrPhase1(_trx) || _trx->overrideFallbackValidationCXRMultiSP())
    && request)
  {
    if(!request->isNSPCxr(defValCxr))
      vcxrMsg = ValidatingCxrUtil::buildValidationCxrMsg(vcxrMsgType, defValCxr, swappedCxr);
    else
      return vcxrMsg;
  }
  else
    vcxrMsg = ValidatingCxrUtil::buildValidationCxrMsg(vcxrMsgType, defValCxr, swappedCxr);

  vcxrMsgType = vcx::NO_MSG;
  if (!_calcTotals->farePath->validatingCarriers().empty())
  {
    if((!fallback::fallbackNonBSPVcxrPhase1(_trx) || _trx->overrideFallbackValidationCXRMultiSP())
        && request &&
        isAcxrsAreNSPcxrs(_calcTotals->farePath->validatingCarriers(), request->spvCxrsCode()))
        return vcxrMsg;

    if ((!fallback::fallbackAlternateVcxrInsteadOptionalVcxr(_trx) &&
        isDefaultVcxrFromPreferred(defValCxr)) ? true : defValCxr.empty()
             && itin->hasNeutralValidatingCarrier())
    {
      vcxrMsgType = vcx::OPTIONAL_CXR;
    }
    else
    {
      vcxrMsgType = vcx::ALTERNATE_CXR;
    }

    _calcTotals->fcMessage.push_back(
        FcMessage(msgType, 0, vcxrMsg, true, FcMessage::CARRIER_MESSAGE, FcMessage::VALIDATINGCXR));

    vcxrMsg = ValidatingCxrUtil::buildValidationCxrMsg(
        *itin, vcxrMsgType, _calcTotals->farePath->validatingCarriers());

    while (vcxrMsg.size() > 60)
    {
      std::string lineToDisplay = vcxrMsg.substr(0, 59);
      vcxrMsg = vcxrMsg.substr(60);

      _calcTotals->fcMessage.push_back(FcMessage(
          msgType, 0, lineToDisplay, true, FcMessage::CARRIER_MESSAGE, FcMessage::VALIDATINGCXR));
    }
  }

  return vcxrMsg;
}

void
FcCollector::collectMatchedAccCodeTrailerMessage()
{
  if (_trx->altTrxType() == PricingTrx::WP || _trx->altTrxType() == PricingTrx::WP_NOMATCH ||
       _trx->altTrxType() == PricingTrx::WPA || _trx->altTrxType() == PricingTrx::WPA_NOMATCH ||
       (_trx->excTrxType() != PricingTrx::NOT_EXC_TRX && !_trx->getRequest()->ticketingAgent()->tvlAgencyPCC().empty()) ||
       _trx->getRequest()->isTicketEntry())
  {
    std::vector<AccountCode> matchedAccCodeVec;

    if (_trx->getRequest()->isMultiAccCorpId())
    {
      const std::map<const FareUsage*, const PricingUnit*>& pricingUnits =
          _calcTotals->pricingUnits;

      for (const auto pu : pricingUnits)
      {
        const FareUsage* fUsage = pu.first;
        if (!fUsage)
          continue;

        const PaxTypeFare* ptFare = fUsage->paxTypeFare();
        if (!ptFare)
          continue;

        bool processed = false;
        if (ptFare->isFareByRule()) // cat 25 AccCode
        {
          const FBRPaxTypeFareRuleData* fbrData = ptFare->getFbrRuleData(RuleConst::FARE_BY_RULE);
          if (fbrData && !fbrData->fbrApp()->accountCode().empty())
          {
            const AccountCode& accCode = fbrData->fbrApp()->accountCode();
            if (std::find(matchedAccCodeVec.begin(), matchedAccCodeVec.end(), accCode) ==
                matchedAccCodeVec.end())
              matchedAccCodeVec.push_back(accCode);

            processed = true;
          }
        }
        if (!processed && !ptFare->matchedAccCode().empty()) // cat 1 AccCode
        {
          const std::string& accCode = ptFare->matchedAccCode();
          if (std::find(matchedAccCodeVec.begin(), matchedAccCodeVec.end(), accCode) ==
              matchedAccCodeVec.end())
            matchedAccCodeVec.push_back(accCode.c_str());
        }
      }
    }

    if (!matchedAccCodeVec.empty())
    {
      std::string trailerLine;
      std::string::size_type line_size = WINDOW_WIDTH;

      trailerLine += "CORP ID/ACCNT CODE USED:";

      std::vector<AccountCode>::const_iterator accCodeIter = matchedAccCodeVec.begin();
      std::vector<AccountCode>::const_iterator accCodeEnd = matchedAccCodeVec.end();

      while (accCodeIter != accCodeEnd)
      {
        trailerLine = trailerLine + " " + (*accCodeIter);
        ++accCodeIter;
      }

      if (_fcConfig->warningMessages() == FareCalcConsts::FC_YES)
        line_size = line_size - 5; // for 'ATTN*' prefix

      if (trailerLine.size() > line_size)
        trailerLine = trailerLine.substr(0, line_size);

      _calcTotals->fcMessage.push_back(
          FcMessage(FcMessage::WARNING, 0, trailerLine, true, FcMessage::MATCHED_ACCOUNT_MESSAGE));
    }
  }
}

void
FcCollector::collectPvtIndicatorMessage()
{
  if (needProcessPvtIndicator())
  {
    if (_calcTotals->privateFareIndSeq != PrivateIndicator::NotPrivate)
    {
      std::string pvtMsg =
          PrivateIndicator::privateFareIndicatorStr(_calcTotals->privateFareIndSeq);
      _calcTotals->fcMessage.push_back(
          FcMessage(FcMessage::WARNING, 0, pvtMsg, true, FcMessage::PVT_INDICATOR));
    }
  }
}

bool
FcCollector::requestAndTruePaxTypeSame()
{
  const CalcTotals& calcTotals = *_calcTotals;
  if (calcTotals.requestedPaxType == calcTotals.truePaxType)
    return true;

  if (calcTotals.farePath->paxType()->vendorCode() == Vendor::SABRE)
  {
    if (calcTotals.allPaxTypeInSabreGroup)
      return true;
  }
  return false;
}

void
FcCollector::checkPerDirSurchargeConsecutive() const
{
  std::vector<PricingUnit*>::const_iterator pub = _farePath->pricingUnit().begin();
  std::vector<PricingUnit*>::const_iterator pue = _farePath->pricingUnit().end();

  std::vector<FareUsage*> obFUs;
  std::vector<FareUsage*> ibFUs;
  SurchargeData* obSurcharge(nullptr), *ibSurcharge(nullptr);

  for (; pub != pue; ++pub)
  {
    // collect all OB/IB FUs and perDir surcharges
    std::vector<FareUsage*>::const_iterator fub = (*pub)->fareUsage().begin();
    std::vector<FareUsage*>::const_iterator fue = (*pub)->fareUsage().end();
    for (; fub != fue; ++fub)
    {
      if ((*fub)->isInbound())
        ibFUs.push_back(*fub);
      else
        obFUs.push_back(*fub);

      std::vector<SurchargeData*>::const_iterator sdb = (*fub)->surchargeData().begin();
      std::vector<SurchargeData*>::const_iterator sde = (*fub)->surchargeData().end();
      for (; sdb != sde; ++sdb)
      {
        if ((*sdb)->travelPortion() == RuleConst::PERDIRECTION && (*sdb)->selectedTkt())
        {
          if ((*fub)->isInbound())
            ibSurcharge = *sdb;
          else
            obSurcharge = *sdb;
          break;
        }
      }
    }
  }
  // process outbound and inbound separatly
  mergePerDirSurchargeConsecutive(_farePath->itin(), obFUs, obSurcharge);
  mergePerDirSurchargeConsecutive(_farePath->itin(), ibFUs, ibSurcharge);
}

bool
FcCollector::determineCat15HasTextTable(const Itin& itin) const
{
  for (FarePath* fp : itin.farePath())
  {
    for (PricingUnit* pu : fp->pricingUnit())
    {
      for (FareUsage* fu : pu->fareUsage())
      {
        if (fu->paxTypeFare()->isCat15HasT996())
        {
          return true;
        }
      }
    }
  }
  return false;
}

namespace
{
struct FareUsageComp
{
  FareUsageComp(const Itin* itin) : _itin(itin) {}
  bool operator()(FareUsage* f1, FareUsage* f2)
  {
    if (!f1->travelSeg().empty() && !f2->travelSeg().empty())
      return _itin->segmentOrder(f1->travelSeg().front()) <
             _itin->segmentOrder(f2->travelSeg().front());
    else if (f1->travelSeg().empty())
      return false;
    return true;
  }

private:
  const Itin* _itin;
};
}

void
FcCollector::mergePerDirSurchargeConsecutive(const Itin* itin,
                                             std::vector<FareUsage*> fus,
                                             SurchargeData* sd) const
{
  // if no per direction surcharge or 1 fu then there is nothing to process
  if (!sd || fus.size() <= 1)
    return;

  if (itin)
    std::sort(fus.begin(), fus.end(), FareUsageComp(itin));

  // if fu before searched surcharge
  bool beforeSur = true;
  std::vector<FareUsage*>::const_iterator fub = fus.begin();
  std::vector<FareUsage*>::const_iterator fue = fus.end();
  SurchargeData* firstConsecutive(nullptr);
  SurchargeData* lastConsecutive(nullptr);
  bool allConsecutive(true);
  for (; fub != fue; ++fub)
  {
    SurchargeData* matchedSurcharge(nullptr);
    // check if teher is per direction surcharge
    std::vector<SurchargeData*>::const_iterator sdb = (*fub)->surchargeData().begin();
    std::vector<SurchargeData*>::const_iterator sde = (*fub)->surchargeData().end();
    for (; sdb != sde; ++sdb)
    {
      if ((*sdb)->travelPortion() == RuleConst::PERDIRECTION)
      {
        if ((*sdb)->carrier() == sd->carrier() && (*sdb)->surchargeType() == sd->surchargeType())
          matchedSurcharge = *sdb;

        if ((*sdb)->selectedTkt())
          beforeSur = false;
      }
    }
    if (matchedSurcharge == nullptr)
      allConsecutive = false;
    if (beforeSur)
    {
      if (!matchedSurcharge)
        firstConsecutive = nullptr;
      else if (!firstConsecutive)
        firstConsecutive = matchedSurcharge;
    }
    else
    {
      if (!matchedSurcharge)
        break;
      else
        lastConsecutive = matchedSurcharge;
    }
  }
  if (firstConsecutive)
    sd->fcBrdCity() = firstConsecutive->fcBrdCity();
  if (lastConsecutive)
    sd->fcOffCity() = lastConsecutive->fcOffCity();
  sd->fcFpLevel() = allConsecutive;
}

std::string
FcCollector::createDefaultTrailerMsg(const FarePath& fp,
                                     const SettlementPlanType& sp,
                                     const CarrierCode& defValCxr) const
{
  std::string defaultMsg;
  if (defValCxr.empty())
  {
    defaultMsg = sp;
    defaultMsg += " - ";
  }
  else
  {
    auto itCxr = fp.marketingCxrForDefaultValCxrPerSp().find(sp);
    if (itCxr != fp.marketingCxrForDefaultValCxrPerSp().end() )
    {
      const CarrierCode& swappedCxr = itCxr->second;
      if (!swappedCxr.empty())
      {
        defaultMsg = sp;
        defaultMsg += " - ";
        defaultMsg += defValCxr;
        defaultMsg += " PER GSA AGREEMENT WITH ";
        defaultMsg += swappedCxr;
      }
    }
  }
  return defaultMsg;
}

std::string
FcCollector::createAltTrailerMsg(
    const FarePath& fp,
    const SettlementPlanType& sp,
    const std::vector<CarrierCode>& mktCxrs,
    std::vector<CarrierCode>& cxrs) const
{
  if (cxrs.size() > 1)
    !fallback::fallbackValidatingCarrierInItinOrderMultiSp(_trx)?
    ValidatingCxrUtil::getAlternateCxrInOrder(*fp.itin(), mktCxrs, cxrs, &sp):
    ValidatingCxrUtil::getAlternateCxrInOrder(*fp.itin(), mktCxrs, cxrs);

  return createTrailerMsg(sp, cxrs);
}

std::string
FcCollector::createTrailerMsg(const SettlementPlanType& sp, std::vector<CarrierCode>& cxrs) const
{
  if (cxrs.empty())
    return "";

  std::ostringstream oss;
  oss << sp;
  oss << " - ";
  std::copy(cxrs.begin(), cxrs.end()-1,
      std::ostream_iterator<CarrierCode>(oss, " "));
  oss << cxrs.back();
  return oss.str();
}

} // FareCalc
} // tse
