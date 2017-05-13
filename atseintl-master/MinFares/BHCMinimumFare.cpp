/* vim: set ts=3 sw=3 sts=3 et: */
//----------------------------------------------------------------------------
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

#include "MinFares/BHCMinimumFare.h"

#include "Common/Logger.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/MinFarePlusUp.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "DBAccess/MinFareAppl.h"
#include "DBAccess/MinFareDefaultLogic.h"
#include "DBAccess/MinFareRuleLevelExcl.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "MinFares/MinFareLogic.h"
#include "MinFares/MinFareNormalFareSelection.h"
#include "MinFares/MinFareSpecialFareSelection.h"
#include "Util/BranchPrediction.h"

#include <iomanip>

namespace tse
{
static Logger
logger("atseintl.MinFares.BHCMinimumFare");

MoneyAmount
BHCMinimumFare::process(PricingTrx& trx,
                        const FarePath& farePath,
                        const PricingUnit& pu,
                        FareUsage& fu,
                        DiagCollector* diag)
{
  if (pu.exemptMinFare())
    return 0.0;

  bool diagEnabled = false;
  if (UNLIKELY(diag))
  {
    diag->enable(Diagnostic718);
    if (diag->isActive())
      diagEnabled = true;
  }

  MoneyAmount hipPlusUp = fu.minFarePlusUp().getSum(HIP);

  if (hipPlusUp == 0.0)
  {
    // If there is no HIP plus up, do not process BHC.
    return 0.0;
  }

  const Itin& itin = *farePath.itin();

  PaxTypeFare& thruFare = *fu.paxTypeFare();

  _travelDate = itin.travelDate();

  // here rex adjustment
  if (trx.excTrxType() == PricingTrx::AR_EXC_TRX)
    adjustRexPricingDates(trx, thruFare.retrievalDate());
  else if (trx.excTrxType() == PricingTrx::PORT_EXC_TRX)
    adjustPortExcDates(trx);

  MoneyAmount thruFareAmount = thruFare.nucFareAmount() + thruFare.mileageSurchargeAmt();

  // Check if it is a Nigeria adjusted fare:
  if (fu.adjustedPaxTypeFare())
  {
    thruFareAmount =
        fu.adjustedPaxTypeFare()->nucFareAmount() + fu.adjustedPaxTypeFare()->mileageSurchargeAmt();
  }

  MinFarePlusUpItem curPlusUp;
  curPlusUp.baseAmount = thruFareAmount + hipPlusUp;

  LOG4CXX_DEBUG(logger, "BaseAmount (with HIP+EMS) = " << curPlusUp.baseAmount);

  if (UNLIKELY(diagEnabled))
  {
    *diag << " \n"
          << "         ONEWAY BACKHAUL CHECK" << '\n';
  }

  bool reverseFcOrigin = fu.isInbound();
  if (curPlusUp.baseAmount > 0 && !checkTableItem(trx, farePath, itin, thruFare, diag))
  {
    MinFareFareSelection::FareDirectionChoice outbInb = thruFare.directionality() == FROM
                                                            ? MinFareFareSelection::OUTBOUND
                                                            : MinFareFareSelection::INBOUND;

    processIntermediate(BHC,
                        MinFareFareSelection::ONE_WAY,
                        trx,
                        itin,
                        thruFare,
                        *farePath.paxType(),
                        diag,
                        thruFare.isNormal(),
                        outbInb,
                        _normalExemptSet,
                        curPlusUp,
                        &farePath,
                        &pu,
                        &fu,
                        "",
                        reverseFcOrigin);
  }

  const FareMarket* fareMarket = thruFare.fareMarket();

  BhcPlusUpItem* _plusUp = nullptr;
  if (curPlusUp.plusUpAmount > 0)
  {
    trx.dataHandle().get(_plusUp);
    if (_plusUp)
    {
      _plusUp->plusUpAmount = curPlusUp.plusUpAmount;
      _plusUp->baseAmount = curPlusUp.baseAmount;
      _plusUp->fareBoardPoint =
          (reverseFcOrigin ? fareMarket->offMultiCity() : fareMarket->boardMultiCity());
      _plusUp->fareOffPoint =
          (reverseFcOrigin ? fareMarket->boardMultiCity() : fareMarket->offMultiCity());
      _plusUp->boardPoint = (reverseFcOrigin ? curPlusUp.offPoint : curPlusUp.boardPoint);
      _plusUp->offPoint = (reverseFcOrigin ? curPlusUp.boardPoint : curPlusUp.offPoint);
      _plusUp->constructPoint = curPlusUp.constructPoint;
      fu.minFarePlusUp().addItem(BHC, _plusUp);
    }
  }

  if (UNLIKELY(diagEnabled))
  {
    *diag << "  BASE FARE NUC ";
    diag->setf(std::ios::right, std::ios::adjustfield);
    diag->setf(std::ios::fixed, std::ios::floatfield);
    diag->precision(2);
    *diag << std::setw(8);
    *diag << curPlusUp.baseAmount;
    *diag << " ";

    if (_plusUp && _plusUp->plusUpAmount > 0)
      *diag << _plusUp->boardPoint << _plusUp->offPoint;
    else
      *diag << "      ";

    *diag << " " << (reverseFcOrigin ? fareMarket->offMultiCity() : fareMarket->boardMultiCity())
          << (reverseFcOrigin ? fareMarket->boardMultiCity() : fareMarket->offMultiCity())
          << " BHC PLUS UP ";

    if (curPlusUp.plusUpAmount <= 0)
    {
      diag->setf(std::ios::right, std::ios::adjustfield);
      diag->setf(std::ios::fixed, std::ios::floatfield);
      diag->precision(2);
      *diag << std::setw(8);
      *diag << 0.0 << " \n";
    }
    else
    {
      diag->setf(std::ios::right, std::ios::adjustfield);
      diag->setf(std::ios::fixed, std::ios::floatfield);
      diag->precision(2);
      *diag << std::setw(8);
      *diag << curPlusUp.plusUpAmount << " \n";
    }
  }

  if (trx.excTrxType() == PricingTrx::AR_EXC_TRX)
    restorePricingDates(trx, trx.ticketingDate());

  if (curPlusUp.plusUpAmount <= 0)
    return 0;

  return curPlusUp.plusUpAmount;
}

bool
BHCMinimumFare::checkTableItem(PricingTrx& trx,
                               const FarePath& farePath,
                               const Itin& itin,
                               const PaxTypeFare& paxTypeFare,
                               DiagCollector* diag)
{
  std::multimap<uint16_t, const MinFareRuleLevelExcl*> ruleMap;
  std::multimap<uint16_t, const MinFareAppl*> applMap;
  std::map<uint16_t, const MinFareDefaultLogic*> defLogicMap;

  _matchedRuleItem =
      MinFareLogic::getRuleLevelExcl(trx, itin, paxTypeFare, BHC, ruleMap, _travelDate);
  if (_matchedRuleItem && _matchedRuleItem->backhaulMinFareAppl() == YES)
  {
    if (UNLIKELY(diag))
    {
      printExceptionInfo(paxTypeFare, *diag);
      *diag << "EXEMPT BY RULE LEVEL EXCL TABLE - " << _matchedRuleItem->seqNo() << '\n';
    }
    return true;
  }

  _matchedDefaultItem = nullptr;
  _matchedApplItem = MinFareLogic::getApplication(
      trx, farePath, itin, paxTypeFare, BHC, applMap, defLogicMap, _travelDate);

  if (_matchedApplItem == nullptr)
  {
    if (UNLIKELY(diag))
    {
      printExceptionInfo(paxTypeFare, *diag);
      *diag << "NO MATCH IN APPLICATION TABLE - RETURN.\n";
    }
    return true;
  }
  else if (_matchedApplItem->applyDefaultLogic() == YES)
  {
    _matchedDefaultItem = MinFareLogic::getDefaultLogic(BHC, trx, itin, paxTypeFare, defLogicMap);
    if (_matchedDefaultItem == nullptr)
    {
      if (UNLIKELY(diag))
      {
        printExceptionInfo(paxTypeFare, *diag);
        *diag << "NO MATCH IN DEFAULT LOGIC TABLE - RETURN.\n";
      }
      return true;
    }
  }

  return false;
}

bool
BHCMinimumFare::compareAndSaveFare(const PaxTypeFare& paxTypeFare,
                                   const PaxTypeFare& interFare,
                                   MinFarePlusUpItem& curPlusUp,
                                   bool useInternationalRounding,
                                   bool outbound)
{
  // 1. thru fare including HIP and mileage surcharge
  MoneyAmount thruFareAmt = curPlusUp.baseAmount;

  // 2. intermediate fare (excluding hip)
  MoneyAmount interFareAmt = interFare.nucFareAmount();

  // 3. diff amount from highest interm. fare to thru fare
  MoneyAmount diffAmount = interFareAmt - paxTypeFare.nucFareAmount();

  if (diffAmount < 0)
    diffAmount = 0;

  // 4. BHC plus up:
  MoneyAmount bhcPlusUp = (interFareAmt + diffAmount) - thruFareAmt;

  if (bhcPlusUp > curPlusUp.plusUpAmount)
  {
    LOG4CXX_DEBUG(logger,
                  "\n1. acc. thru fare: "
                      << thruFareAmt << "\n2. highest fare (no hip): " << interFareAmt
                      << "\n3. for bhc "
                         "\n   " << interFareAmt << "\n  -" << paxTypeFare.nucFareAmount()
                      << "\n   " << diffAmount << "\n  +" << interFareAmt << "\n   "
                      << (interFareAmt + diffAmount) << "\n4."
                                                        "\n  -" << thruFareAmt << "\n   "
                      << (interFareAmt + diffAmount - thruFareAmt));

    // Save the largest BHC plus up amount
    curPlusUp.plusUpAmount = bhcPlusUp;
    curPlusUp.boardPoint = interFare.fareMarket()->boardMultiCity();
    curPlusUp.offPoint = interFare.fareMarket()->offMultiCity();
    curPlusUp.constructPoint = "";

    return true;
  }

  return false;
}

bool
BHCMinimumFare::compareAndSaveFare(const PaxTypeFare& paxTypeFare,
                                   const PtfPair& ptfPair,
                                   MinFarePlusUpItem& curPlusUp,
                                   bool useInternationalRounding,
                                   bool outbound)
{
  MoneyAmount unPubAmount = ptfPair.first->nucFareAmount() + ptfPair.second->nucFareAmount();

  const LocCode& boardPoint = ptfPair.first->fareMarket()->boardMultiCity();
  const LocCode& constPoint = ptfPair.first->fareMarket()->offMultiCity();
  const LocCode& offPoint = ptfPair.second->fareMarket()->offMultiCity();

  // 1. thru fare including HIP and mileage surcharge
  MoneyAmount thruFareAmt = curPlusUp.baseAmount;

  // 2 & 3 diff. amount from highest interm. fare to thru fare
  MoneyAmount diffAmount = unPubAmount - paxTypeFare.nucFareAmount();

  if (diffAmount < 0)
    diffAmount = 0;

  // 4. BHC plus up:
  MoneyAmount bhcPlusUp = (unPubAmount + diffAmount) - thruFareAmt;

  if (bhcPlusUp > curPlusUp.plusUpAmount)
  {
    LOG4CXX_DEBUG(logger,
                  "\n1. acc. thru fare: "
                      << thruFareAmt << "\n2. highest fare (no hip): " << unPubAmount
                      << "\n3. for bhc "
                         "\n   " << unPubAmount << "\n  -" << paxTypeFare.nucFareAmount() << "\n   "
                      << diffAmount << "\n  +" << unPubAmount << "\n   "
                      << (unPubAmount + diffAmount) << "\n4."
                                                       "\n  -" << thruFareAmt << "\n   "
                      << (unPubAmount + diffAmount - thruFareAmt));

    // Save the largest BHC plus up amount
    curPlusUp.plusUpAmount = bhcPlusUp;
    curPlusUp.boardPoint = boardPoint;
    curPlusUp.offPoint = offPoint;
    curPlusUp.constructPoint = constPoint;

    curPlusUp.currency = ptfPair.first->currency();

    return true;
  }

  return false;
}
}
