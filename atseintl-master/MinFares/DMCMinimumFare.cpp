//----------------------------------------------------------------------------
//
//  File:           DMCMinimumFare.cpp
//  Created:        3/4/2004
//  Authors:
//
//  Description:    A class to represent data and methods for DMC
//
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
//----------------------------------------------------------------------------

#include "MinFares/DMCMinimumFare.h"

#include "Common/DiagMonitor.h"
#include "Common/ErrorResponseException.h"
#include "Common/Logger.h"
#include "Common/TravelSegUtil.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/TseUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"
#include "DBAccess/MinFareAppl.h"
#include "DBAccess/MinFareRuleLevelExcl.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "MinFares/COMMinimumFare.h"
#include "MinFares/HIPMinimumFare.h"
#include "MinFares/MatchExceptionTable.h"
#include "MinFares/MatchRuleLevelExclTable.h"
#include "MinFares/MinFareLogic.h"
#include "MinFares/MinFareNormalFareSelection.h"
#include "MinFares/MinFareSpecialFareSelection.h"

#include <iomanip>
#include <iostream>
#include <vector>

namespace tse
{
static Logger
logger("atseintl.MinFares.DMCMinimumFare");

MoneyAmount
DMCMinimumFare::process(PricingTrx& trx, FarePath& farePath)
{
  DiagMonitor diagMonitor(trx, getDiagnostic());
  DiagCollector& diag = diagMonitor.diag();

  _travelDate = farePath.itin()->travelDate();
  if (trx.excTrxType() == PricingTrx::PORT_EXC_TRX)
    adjustPortExcDates(trx);

  if (qualifyMHException(diag, trx, farePath))
  {
    return 0.0;
  }

  MoneyAmount fpDmcPlusUp = 0.0; // fare path's accumulated the dmc plus ups

  if (farePath.oscPlusUp().size() > 0)
  {
    // Notice: be advised that the puI might be updated (additional advance)
    //         inside the loop body if there is multi-pu involved.
    //
    for (std::vector<PricingUnit*>::const_iterator puI = farePath.pricingUnit().begin(),
                                                   puEnd = farePath.pricingUnit().end();
         puI != puEnd;
         ++puI)
    {
      if ((*puI)->exemptMinFare())
        continue;

      if (puI == puEnd || !*puI)
        break;

      if ((*puI)->puType() != PricingUnit::Type::ONEWAY)
        continue;

      if (trx.excTrxType() == PricingTrx::AR_EXC_TRX)
        adjustRexPricingDates(trx, (*puI)->fareUsage().front()->paxTypeFare()->retrievalDate());

      // process the special pu as usual - multi-pu only consists of normal pu
      if ((*puI)->puFareType() == PricingUnit::SP)
      {
        fpDmcPlusUp += process(trx, farePath, **puI);
        continue;
      }

      /////////////////////////////////////////////////////////////////////////
      // TODO: Factoring Candidate

      // pu origin
      const TravelSeg* orig = (*puI)->travelSeg().front();
      // lint -e{530}
      const std::vector<FarePath::OscPlusUp*>& oscPlusUp = farePath.oscPlusUp();

      // tracking the oscPlusUp with origEqual and has the max plusUp
      TravelSegUtil::OrigEqual origEqual(orig->origin());
      const FarePath::OscPlusUp* oscPlusUpPtr = nullptr;

      for (const auto plusUp : oscPlusUp)
      {
        if (!plusUp->thruFare)
          continue;

        if (!plusUp->thruFare->fareMarket())
          continue;

        const std::vector<TravelSeg*>& tvlSeg = plusUp->thruFare->fareMarket()->travelSeg();

        // check if the the current pu is part of osc multi-pu, and find the
        // multi-pu with the highest plusUp
        if (find_if(tvlSeg.begin(), tvlSeg.end(), origEqual) != tvlSeg.end())
        {
          if (!oscPlusUpPtr ||
              oscPlusUpPtr->thruFare->fareMarket()->travelSeg().size() < tvlSeg.size())
          // oscPlusUpPtr->plusUpAmount < (*plusUpI)->plusUpAmount)
          {
            oscPlusUpPtr = plusUp;
          }
        }
      }

      // determine the pus from the osc plus up thruFare
      std::vector<PricingUnit*> multiPu;
      if (oscPlusUpPtr)
      {
        const std::vector<TravelSeg*>& tvlSeg = oscPlusUpPtr->thruFare->fareMarket()->travelSeg();
        // save the current puI iterator - it is the start of the multi-pu
        std::vector<PricingUnit*>::const_iterator multiPuI = puI;

        // make puI point to the last pu of the multi-pu
        for (; puI != puEnd; ++puI)
        {
          if ((*puI)->travelSeg().back()->destination() == tvlSeg.back()->destination())
          {
            break;
          }
        }
        std::copy(multiPuI, puI + 1, back_inserter(multiPu));
      }

      /////////////////////////////////////////////////////////////////////////
      //

      if (multiPu.size() > 0)
      {
        fpDmcPlusUp += process(trx, farePath, multiPu, *oscPlusUpPtr, diag);
      }
      else
      {
        fpDmcPlusUp += process(trx, farePath, **puI, diag);
      }

      if (puI == puEnd)
      {
        break;
      }
    }
  }
  else
  { // there is no OSC plus (no multi-pu), process the pus as usual.
    for (std::vector<PricingUnit*>::const_iterator i = farePath.pricingUnit().begin(),
                                                   end = farePath.pricingUnit().end(); // lint !e530
         i != end;
         ++i)
    {
      if (trx.excTrxType() == PricingTrx::AR_EXC_TRX)
      {
        _travelDate = farePath.itin()->travelDate();
        adjustRexPricingDates(trx, (*i)->fareUsage().front()->paxTypeFare()->retrievalDate());
      }

      fpDmcPlusUp += process(trx, farePath, **i);

      // Check side trip PUs
      std::vector<PricingUnit*>::const_iterator stPuI = (**i).sideTripPUs().begin();
      for (; stPuI != (**i).sideTripPUs().end(); ++stPuI)
      {

        if (trx.excTrxType() == PricingTrx::AR_EXC_TRX)
        {
          _travelDate = farePath.itin()->travelDate();
          adjustRexPricingDates(trx, (*stPuI)->fareUsage().front()->paxTypeFare()->retrievalDate());
        }

        fpDmcPlusUp += process(trx, farePath, **stPuI);
      }
    }
  }

  if (trx.excTrxType() == PricingTrx::AR_EXC_TRX)
    restorePricingDates(trx, trx.ticketingDate());

  return fpDmcPlusUp;
}

MoneyAmount
DMCMinimumFare::process(PricingTrx& trx,
                        FarePath& farePath,
                        const std::vector<PricingUnit*>& multiPu,
                        const FarePath::OscPlusUp& oscPlusUp,
                        DiagCollector& diag)
{
  PaxTypeFare* paxTypeFare = const_cast<PaxTypeFare*>(oscPlusUp.thruFare);
  if (!paxTypeFare)
    return 0.0;

  // Check if this is a domestic (multi)PU
  if (paxTypeFare->fareMarket() && paxTypeFare->fareMarket()->travelSeg().size() > 0 &&
      isWithinLoc(trx,
                  paxTypeFare->fareMarket()->travelSeg(),
                  LOCTYPE_NATION,
                  paxTypeFare->fareMarket()->travelSeg().front()->origin()->nation()))
  {
    return 0.0;
  }

  diag << "\n"
          "*************** DIRECTIONAL MINIMUM CHECK - " << farePath.paxType()->paxType()
       << " ****************\n"
          "CITY      GOV    CLASS                       DIR FARE  GLOB EXCK\n"
          "PAIR      CXR           CUR   AMOUNT RTG TAG I/O TYPE  IND  IND \n";

  MoneyAmount plusUp = 0.0;

  // Print through fare
  printFareInfo(*paxTypeFare, (diag << "* "), DMC);

  // Check if this fare could be excluded from DMC check.
  if (checkThroughMarketExclusion(diag, trx, farePath, *paxTypeFare))
  {
    printExceptionInfo(*paxTypeFare, diag);
    diag << " EXEMPT BY RULE LEVEL EXCL TABLE - " << (*_matchedRuleItem).seqNo() << '\n';

    return 0.0;
  }

  if (checkExceptionTable(diag, trx, farePath, *paxTypeFare))
  {
    trx.dataHandle().get(_plusUp);
    if (!_plusUp)
    {
      LOG4CXX_ERROR(logger, "DataHandle failed to allocate memory (_plusUp)");
      return 0.0;
    }

    // Process HIP check on the new multi-pu thru fare
    FareUsage tmpFareUsage;
    tmpFareUsage.paxTypeFare() = const_cast<PaxTypeFare*>(paxTypeFare);

    HIPMinimumFare hip(trx);
    hip.process(tmpFareUsage, *multiPu.front(), farePath);

    calcBaseFare(farePath, tmpFareUsage);

    processIntermediate(diag, trx, farePath, *multiPu.front(), tmpFareUsage);

    diag.enable(Diagnostic762);
    diag << "BASE FARE NUC" << _plusUp->baseAmount << "   " << _plusUp->boardPoint
         << _plusUp->offPoint << "  PLUS UP " << _plusUp->plusUpAmount << "\n";

    if (_plusUp->plusUpAmount > 0.0)
    {
      plusUp = _plusUp->plusUpAmount;

      // Save the plus up in FarePath:
      FarePath::PlusUpInfo* plusUpInfo = nullptr;
      trx.dataHandle().get(plusUpInfo);
      if (!plusUpInfo)
      {
        LOG4CXX_ERROR(logger, "DataHandle failed to allocate memory (plusUpInfo)");
        return 0.0;
      }
      // lint --e{413}
      plusUpInfo->module() = DMC;
      plusUpInfo->startSeg() =
          farePath.itin()->segmentOrder(paxTypeFare->fareMarket()->travelSeg().front());
      plusUpInfo->endSeg() =
          farePath.itin()->segmentOrder(paxTypeFare->fareMarket()->travelSeg().back());
      plusUpInfo->minFarePlusUp() = _plusUp;
      farePath.plusUpInfoList().push_back(plusUpInfo);
    }

    _plusUp = nullptr;
  }

  return plusUp;
}

MoneyAmount
DMCMinimumFare::process(PricingTrx& trx, FarePath& farePath, const PricingUnit& pu)
{
  DiagMonitor diagMonitor(trx, getDiagnostic());
  DiagCollector& diag = diagMonitor.diag();

  // this version of process() will be called by RSC (or other module), so
  // MH exception need to be done.  (this is normally done once for the fare
  // path)
  if (qualifyMHException(diag, trx, farePath))
  {
    return 0.0;
  }

  if (pu.exemptMinFare())
  {
    return 0.0;
  }

  _travelDate = farePath.itin()->travelDate();

  if (trx.excTrxType() == PricingTrx::AR_EXC_TRX)
    adjustRexPricingDates(trx, pu.fareUsage().front()->paxTypeFare()->retrievalDate());
  else if (trx.excTrxType() == PricingTrx::PORT_EXC_TRX)
    adjustPortExcDates(trx);

  MoneyAmount dmcPuMoneyAmount = process(trx, farePath, pu, diag);

  if (trx.excTrxType() == PricingTrx::AR_EXC_TRX)
    restorePricingDates(trx, trx.ticketingDate());

  return dmcPuMoneyAmount;
}

MoneyAmount
DMCMinimumFare::process(PricingTrx& trx,
                        FarePath& farePath,
                        const PricingUnit& pu,
                        DiagCollector& diag)
{
  if (!qualify(pu))
  {
    return 0.0;
  }

  // Check if this is a domestic PU.
  if (pu.travelSeg().size() > 0 &&
      isWithinLoc(trx, pu.travelSeg(), LOCTYPE_NATION, pu.travelSeg().front()->origin()->nation()))
  {
    return 0.0;
  }

  diag << "\n"
          "*************** DIRECTIONAL MINIMUM CHECK - " << farePath.paxType()->paxType()
       << " ****************\n"
          "CITY      GOV    CLASS                       DIR FARE  GLOB EXCL\n"
          "PAIR      CXR    CUR          AMOUNT RTG TAG I/O TYPE  IND  IND \n";

  MoneyAmount plusUp = 0.0;

  const std::vector<FareUsage*>& fu = pu.fareUsage();
  std::vector<FareUsage*>::const_iterator fuIter, fuIterEnd;

  for (fuIter = fu.begin(), fuIterEnd = fu.end(); fuIter != fuIterEnd; ++fuIter)
  {
    if (!(*fuIter))
      continue;

    FareUsage& fareUsage = **fuIter;
    // lint -e{530}
    PaxTypeFare* paxTypeFare = const_cast<PaxTypeFare*>(fareUsage.paxTypeFare());

    // Print through fare
    printFareInfo(*paxTypeFare, (diag << "* "), DMC);

    if (checkThroughMarketExclusion(diag, trx, farePath, *paxTypeFare))
    {
      printExceptionInfo(*paxTypeFare, diag);
      diag << " EXEMPT BY RULE LEVEL EXCL TABLE - " << (*_matchedRuleItem).seqNo() << '\n';

      // this fare component is excluded from DMC check. Proceed to the
      // next fare component
      continue;
    }

    if (checkExceptionTable(diag, trx, farePath, *paxTypeFare))
    {
      if (_plusUp == nullptr)
      {
        trx.dataHandle().get(_plusUp);
        if (!_plusUp)
        {
          LOG4CXX_ERROR(logger, "DataHandle failed to allocate memory (_plusUp)");
          return 0.0;
        }
      }

      calcBaseFare(farePath, fareUsage);

      processIntermediate(diag, trx, farePath, pu, fareUsage);

      diag.enable(Diagnostic762);
      diag << "BASE FARE NUC" << _plusUp->baseAmount << "   " << _plusUp->boardPoint
           << _plusUp->offPoint << "  PLUS UP " << _plusUp->plusUpAmount << "\n";

      if (_plusUp->plusUpAmount > 0.0)
      {
        plusUp += _plusUp->plusUpAmount;
        (**fuIter).minFarePlusUp().addItem(DMC, _plusUp);
        (**fuIter).accumulateMinFarePlusUpAmt(_plusUp->plusUpAmount);
      }

      _plusUp = nullptr;
    }
  }

  return plusUp;
}

MoneyAmount
DMCMinimumFare::calcBaseFare(const FarePath& farePath, const FareUsage& fareUsage)
{
  const PaxTypeFare& thruFare = *fareUsage.paxTypeFare();
  const FareMarket& fareMarket = *thruFare.fareMarket();
  const std::vector<TravelSeg*>& tvlSegs = fareMarket.travelSeg();

  // Accummulate any plus-ups from HIP, EMS, BHC, OSC, COM
  //
  MoneyAmount HIP_PlusUp = fareUsage.minFarePlusUp().getSum(HIP);
  MoneyAmount BHC_PlusUp = fareUsage.minFarePlusUp().getSum(BHC);

  MoneyAmount COM_PlusUp = 0.0;
  int16_t startSeg = farePath.itin()->segmentOrder(tvlSegs.front());
  int16_t endSeg = farePath.itin()->segmentOrder(tvlSegs.back());
  const MinFarePlusUpItem* comPlusUp = farePath.minFarePlusUp(COM, startSeg, endSeg);
  if (comPlusUp)
  {
    COM_PlusUp = comPlusUp->plusUpAmount;
  }

  MoneyAmount OSC_PlusUp = 0.0;

  _plusUp->baseAmount = thruFare.nucFareAmount() + thruFare.mileageSurchargeAmt() + HIP_PlusUp +
                        BHC_PlusUp + OSC_PlusUp + COM_PlusUp;

  _plusUp->plusUpAmount = 0.0;
  _plusUp->boardPoint = "";
  _plusUp->offPoint = "";

  LOG4CXX_DEBUG(logger,
                "DMC::calcBaseFare: "
                    << "\tthruFare.nucFareAmount() = " << thruFare.nucFareAmount()
                    << "\tmileage surcharge amt = " << thruFare.mileageSurchargeAmt()
                    << "\tHIP PlusUP = " << HIP_PlusUp << "\tBHC PlusUP = " << BHC_PlusUp
                    << "\tOSC PlusUP = " << OSC_PlusUp << "\tCOM PlusUP = " << COM_PlusUp
                    << "\tDMC Base Amount = " << _plusUp->baseAmount);

  return _plusUp->baseAmount;
}

/**
 * Check the "Minimum Fare Rule Level Exclusion Table"
 *
 * @return: true - if there is a matched and DMC Min Fare Check "Do not apply"
 *          is "Y", false otherwise.
 *
 *          If the return value is true, DMC check should not be apply to the
 *          through market specified by the paxTypeFare
 **/
bool
DMCMinimumFare::checkThroughMarketExclusion(DiagCollector& diag,
                                            PricingTrx& trx,
                                            const FarePath& farePath,
                                            const PaxTypeFare& paxTypeFare)
{
  std::multimap<uint16_t, const MinFareRuleLevelExcl*> ruleLevelMap;

  _matchedRuleItem = MinFareLogic::getRuleLevelExcl(
      trx, *farePath.itin(), paxTypeFare, DMC, ruleLevelMap, _travelDate);
  if (_matchedRuleItem)
  {
    if (_matchedRuleItem->dmcMinFareAppl() == YES)
    {
      return true;
    }
  }

  return false;
}

bool
DMCMinimumFare::checkExceptionTable(DiagCollector& diag,
                                    PricingTrx& trx,
                                    const FarePath& farePath,
                                    const PaxTypeFare& paxTypeFare)
{
  std::multimap<uint16_t, const MinFareAppl*> applMap;
  std::map<uint16_t, const MinFareDefaultLogic*> defaultLogicMap;

  _matchedDefaultItem = nullptr;
  _matchedApplItem = MinFareLogic::getApplication(
      trx, farePath, *farePath.itin(), paxTypeFare, DMC, applMap, defaultLogicMap, _travelDate);

  if (_matchedApplItem == nullptr)
  {
    printExceptionInfo(paxTypeFare, diag);
    diag << "NO MATCH APPL TABLE\n";
    return false;
  }

  if (_matchedApplItem && MinFareLogic::getMinFareApplIndicator(DMC, *_matchedApplItem) != YES)
  {
    printExceptionInfo(paxTypeFare, diag);
    diag << "MATCHED APPL. DO NOT APPLY DMC - " << _matchedApplItem->seqNo() << "\n";
    return false;
  }

  if (_matchedApplItem->applyDefaultLogic() == YES)
  {
    _matchedDefaultItem =
        MinFareLogic::getDefaultLogic(DMC, trx, *farePath.itin(), paxTypeFare, defaultLogicMap);
    if (_matchedDefaultItem == nullptr)
    {
      printExceptionInfo(paxTypeFare, diag);
      diag << "NO MATCH DEF LOGIC TABLE\n";
      return false;
    }
  }

  return true;
}

PtfPair
DMCMinimumFare::selectInboundThruFare(PricingTrx& trx,
                                      const FarePath& farePath,
                                      const PricingUnit& pu,
                                      const FareUsage& fu) const
{
  const PaxTypeFare& thruFare = *fu.paxTypeFare();

  PtfPair ptfPair = MinFareLogic::selectQualifyConstFare(
      DMC,
      trx,
      *(farePath.itin()),
      thruFare,
      *farePath.paxType(),
      thruFare.cabin(),
      thruFare.isNormal(),
      (fu.isOutbound() ? MinFareFareSelection::INBOUND : MinFareFareSelection::OUTBOUND),
      MinFareLogic::eligibleFare(pu),
      thruFare.fareMarket()->travelSeg(),
      _travelDate,
      _matchedApplItem,
      _matchedDefaultItem,
      nullptr, // repricingTrx
      &farePath,
      &pu);

  return ptfPair;
}

void
DMCMinimumFare::processIntermediate(DiagCollector& diag,
                                    PricingTrx& trx,
                                    const FarePath& farePath,
                                    const PricingUnit& pu,
                                    FareUsage& fu)
{
  const PaxTypeFare& paxTypeFare = *fu.paxTypeFare(); // lint !e530

  std::set<uint32_t> normalExemptSet;
  MinFareFareSelection::EligibleFare eligibleFare = MinFareLogic::eligibleFare(pu);

  diag.enable(Diagnostic762);
  MinFarePlusUpItem curPlusUp;
  bool reverseFcOrigin = fu.isInbound();
  MinimumFare::processIntermediate(
      DMC,
      eligibleFare,
      trx,
      *(farePath.itin()),
      paxTypeFare,
      *(farePath.paxType()),
      &diag,
      paxTypeFare.isNormal(),
      (fu.isOutbound() ? MinFareFareSelection::OUTBOUND : MinFareFareSelection::INBOUND),
      normalExemptSet,
      curPlusUp,
      &farePath,
      &pu,
      &fu,
      "",
      reverseFcOrigin);

  const FareMarket* fareMarket = paxTypeFare.fareMarket();
  if (fareMarket == nullptr || fareMarket->travelSeg().empty())
    return;

  const AirSeg* aSeg = dynamic_cast<AirSeg*>(fareMarket->travelSeg().front());
  const AirSeg* zSeg = dynamic_cast<AirSeg*>(fareMarket->travelSeg().back());
  if (aSeg == nullptr || zSeg == nullptr)
    return;

  // select the inbound through fare
  PtfPair inboundPtfPair = selectInboundThruFare(trx, farePath, pu, fu);

  const MinFareFareSelection::FareDirectionChoice revFareDir =
      (fu.isOutbound() ? MinFareFareSelection::INBOUND : MinFareFareSelection::OUTBOUND);

  if (inboundPtfPair.first || inboundPtfPair.second)
  {
    savePlusUpInfo(paxTypeFare, inboundPtfPair, curPlusUp, false, revFareDir);
    printFareInfo(inboundPtfPair, revFareDir, (diag << "  "), DMC);
  }
  else
  {
    printExceptionInfo(diag, fareMarket->travelSeg(), revFareDir, "NO FARE FOUND");
  }

  diag.enable(Diagnostic762);
  MinimumFare::processIntermediate(DMC,
                                   eligibleFare,
                                   trx,
                                   *(farePath.itin()),
                                   paxTypeFare,
                                   *(farePath.paxType()),
                                   &diag,
                                   paxTypeFare.isNormal(),
                                   revFareDir,
                                   normalExemptSet,
                                   curPlusUp,
                                   &farePath,
                                   &pu,
                                   &fu);
}

bool
DMCMinimumFare::compareAndSaveFare(const PaxTypeFare& thruFare,
                                   const PaxTypeFare& interFare,
                                   MinFarePlusUpItem& curPlusUp,
                                   bool useInternationalRounding,
                                   bool outbound)
{
  MoneyAmount interFareAmt = interFare.nucFareAmount();

  MoneyAmount plusUp = interFareAmt - _plusUp->baseAmount;

  if (plusUp > _plusUp->plusUpAmount)
  {
    _plusUp->plusUpAmount = plusUp;

    const FareMarket& fareMarket = *interFare.fareMarket();
    const LocCode& orig = fareMarket.boardMultiCity();
    const LocCode& dest = fareMarket.offMultiCity();

    const bool reversedDirection = interFare.directionality() == TO;

    _plusUp->boardPoint = reversedDirection ? dest : orig;
    _plusUp->offPoint = reversedDirection ? orig : dest;
    _plusUp->constructPoint.clear();

    _plusUp->currency = interFare.currency();

    return true;
  }

  return false;
}

bool
DMCMinimumFare::compareAndSaveFare(const PaxTypeFare& paxTypeFare,
                                   const PtfPair& ptfPair,
                                   MinFarePlusUpItem& curPlusUp,
                                   bool useInternationalRounding,
                                   bool outbound)
{
  MoneyAmount unPubAmount = ptfPair.first->nucFareAmount() + ptfPair.second->nucFareAmount();

  const LocCode& boardPoint = ptfPair.first->fareMarket()->boardMultiCity();
  const LocCode& constPoint = ptfPair.first->fareMarket()->offMultiCity();
  const LocCode& offPoint = ptfPair.second->fareMarket()->offMultiCity();

  MoneyAmount plusUp = unPubAmount - _plusUp->baseAmount;

  if (plusUp > _plusUp->plusUpAmount)
  {
    const bool reversedDirection = ptfPair.first->directionality() == TO;

    _plusUp->plusUpAmount = plusUp;

    _plusUp->boardPoint = reversedDirection ? offPoint : boardPoint;
    _plusUp->offPoint = reversedDirection ? boardPoint : offPoint;
    _plusUp->constructPoint = constPoint;

    _plusUp->currency = ptfPair.first->currency();

    return true;
  }

  return false;
}

bool
DMCMinimumFare::qualifyMHException(DiagCollector& diag,
                                   const PricingTrx& trx,
                                   const FarePath& farePath) const
{
  const Itin* itin = farePath.itin(); // lint !e530
  if (itin == nullptr)
  {
    return false;
  }

  if ((itin->travelSeg().size() != 2) || (farePath.pricingUnit().size() != 2))
  {
    return false;
  }

  PricingUnit& pu1 = *farePath.pricingUnit()[0];
  PricingUnit& pu2 = *farePath.pricingUnit()[1];
  if (pu1.puFareType() != PricingUnit::NL || pu1.puType() != PricingUnit::Type::ONEWAY ||
      pu1.geoTravelType() != GeoTravelType::International || pu2.puFareType() != PricingUnit::NL ||
      pu2.puType() != PricingUnit::Type::ONEWAY || pu2.geoTravelType() != GeoTravelType::International)
  {
    return false;
  }

  AirSeg* tvlSeg1 = dynamic_cast<AirSeg*>(itin->travelSeg().front());
  AirSeg* tvlSeg2 = dynamic_cast<AirSeg*>(itin->travelSeg().back());

  if (tvlSeg1 == nullptr || tvlSeg2 == nullptr)
  {
    return false;
  }

  if ((tvlSeg1->carrier() != MH_CARRIER) || (tvlSeg2->carrier() != MH_CARRIER))
  {
    return false;
  }

  if ((pu1.fareUsage()[0]->travelSeg().size() != 1) ||
      (pu2.fareUsage()[0]->travelSeg().size() != 1))
  {
    // FareUsage contains intermediate point(s)
    return false;
  }

  const Loc* tvlSeg1Orig = tvlSeg1->origin();
  const Loc* tvlSeg1Dest = tvlSeg1->destination();
  const Loc* tvlSeg2Orig = tvlSeg2->origin();
  const Loc* tvlSeg2Dest = tvlSeg2->destination();
  if ((tvlSeg1Orig == nullptr) || (tvlSeg1Dest == nullptr) || (tvlSeg2Orig == nullptr) || (tvlSeg2Dest == nullptr))
  {
    return false;
  }

  const LocCode& seg1Orig = tvlSeg1Orig->loc();
  const LocCode& seg1Dest = tvlSeg1Dest->loc();
  const LocCode& seg2Orig = tvlSeg2Orig->loc();
  const LocCode& seg2Dest = tvlSeg2Dest->loc();

  if (!(((seg1Orig == LOC_SIN) && (seg2Dest == LOC_BWN)) ||
        ((seg1Orig == LOC_BWN) && (seg2Dest == LOC_SIN))))
  {
    return false;
  }

  if (((seg1Dest == LOC_KUL) && (seg2Orig == LOC_KUL)) ||
      ((seg1Dest == LOC_KCH) && (seg2Orig == LOC_KCH)) ||
      ((seg1Dest == LOC_BKI) && (seg2Orig == LOC_BKI)))
  {
    diag << "MH EXCEPTION APPLIED\n";
    return true;
  }

  return false;
}

bool
DMCMinimumFare::qualify(const PricingUnit& pu) const
{
  if ((pu.puType() == PricingUnit::Type::ONEWAY) ||
      (pu.puFareType() == PricingUnit::NL && pu.puType() == PricingUnit::Type::OPENJAW))
  {
    return true;
  }

  return false;
}
}
