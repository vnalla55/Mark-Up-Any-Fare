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

#include "MinFares/MinFareChecker.h"

#include "Common/TrxUtil.h"
#include "Common/TseEnums.h"
#include "Common/TSELatencyData.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/Loc.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "MinFares/BHCMinimumFare.h"
#include "MinFares/COMMinimumFare.h"
#include "MinFares/COPMinimumFare.h"
#include "MinFares/DMCMinimumFare.h"
#include "MinFares/EPQMinimumFare.h"
#include "MinFares/HIPMinimumFare.h"
#include "MinFares/OJMMinimumFare.h"
#include "MinFares/RTMinimumFare.h"

namespace tse
{

bool
MinFareChecker::process(PricingTrx& trx, FarePath& farePath)
{
  if (UNLIKELY(trx.getOptions()->isRtw()))
    return true;

  if (UNLIKELY(trx.excTrxType() == PricingTrx::PORT_EXC_TRX))
  {
    ExchangePricingTrx& excTrx = static_cast<ExchangePricingTrx&>(trx);
    if (!excTrx.exchangeOverrides().dummyFCSegs().empty())
      return true;
  }

  TSELatencyData metrics(trx, "PO MINFARE PROCESS");
  farePath.minFareCheckDone() = true;

  DCFactory* factory = DCFactory::instance();
  DiagCollector& diag = *(factory->create(trx));
  diag.enable(Diagnostic700);

  if (UNLIKELY(diag.isActive()))
  {
    diag << "\n****************** MINIMUM FARE CHECKER ********************" << '\n';
    diag << "FARE PATH - ";
    diag << farePath;
  }

  EPQMinimumFare epq;
  bool epqSet = epq.process(trx, farePath);

  if (UNLIKELY(diag.isActive()))
    diag << "PROCESSED EPQ CHECK - SEE DIAG 711" << '\n';

  if (UNLIKELY(farePath.pricingUnit().empty()))
  {
    return true;
  }

  MoneyAmount totalPlusUp = 0; // accumulate all the plus up for the fare path
  MinFareChecks mfChecks;

  if (UNLIKELY(!processHIPMinimumFare(trx, farePath, mfChecks, totalPlusUp, diag)))
  {
    diag.flushMsg();
    if (totalPlusUp > 0)
      setTotalPlusUp(farePath, totalPlusUp);
    return false; // This fare path should fail.
  }

  // Check if some PUs will be exempted
  checkExemption(trx, farePath);

  //--- Process all Priorities according to each minimum fare modules (CTM, COM, OSC,,,)
  for (int priority = PRIORITY_LEVEL1; priority <= PRIORITY_LEVEL3; priority++)
  {
    processMinFareCheckOnPU(trx, farePath, mfChecks, totalPlusUp, diag, priority);

    //--- The end of priority 2 then process across pricing units minimum fare checks
    if (priority == PRIORITY_LEVEL2)
    {
      if (UNLIKELY(diag.isActive()))
        diag << "PROCESSED ACROSS PRICING UNITS" << '\n';

      if (mfChecks.processOSC)
      {
        if (diag.isActive())
          diag << "OSC CHECK DOES NOT APPLY" << '\n';
      }

      if (mfChecks.processRSC)
      {
        if (diag.isActive())
          diag << "RSC CHECK DOES NOT APPLY" << '\n';
      }

      if (mfChecks.processCOM)
      {
        processCOMMinimumFare(trx, farePath, mfChecks, totalPlusUp, diag);
      }

      if (mfChecks.processDMC)
      {
        if (!epqSet)
        {
          processDMCMinimumFare(trx, farePath, totalPlusUp, diag);
        }
      }

      if (UNLIKELY(diag.isActive()))
        diag << '\n';
    }
  }

  if (mfChecks.processCOP)
  {
    processCOPMinimumFare(trx, farePath, totalPlusUp, diag);
  }

  diag.flushMsg();

  if (totalPlusUp > 0)
    setTotalPlusUp(farePath, totalPlusUp);

  return true;
}

MoneyAmount
MinFareChecker::processNormalOpenJaw(
    PricingTrx& trx, FarePath& farePath, PricingUnit& pu, int priority, DiagCollector& diag)
{
  MoneyAmount plusUp = 0;

  if (priority == PRIORITY_LEVEL1)
  {
    processOJMMinimumFare(trx, farePath, pu, plusUp, diag);

    if (diag.isActive())
      diag << "CPM CHECK DOES NOT APPLY \n"
           << " \n";
  }

  return plusUp;
}

void
MinFareChecker::checkExemption(PricingTrx& trx, FarePath& farePath)
{
  std::vector<PricingUnit*>& puVec = farePath.pricingUnit();
  std::vector<PricingUnit*>::iterator puIter = puVec.begin();

  for (puIter = puVec.begin(); puIter != puVec.end(); puIter++)
  {
    checkExemption(trx, **puIter);
  }
}

// Check if a PU should be exempted from minfare check
// due to cat25 or DA/DP.
void
MinFareChecker::checkExemption(PricingTrx& trx, PricingUnit& pu)
{
  bool validateDiscount = false;
  if (TrxUtil::newDiscountLogic(trx))
  {
    validateDiscount = TrxUtil::validateDiscountNew(trx, pu);
  }
  else
  {
    Percent dp = 0;
    const DiscountAmount* da = nullptr;
    validateDiscount = TrxUtil::getDiscountOld(trx, pu, dp, da);
  }

  if (UNLIKELY(!validateDiscount))
  {
    // Exempt minfare check for conflict DA or DP within the PU
    pu.exemptMinFare() = true;
    pu.exemptOpenJawRoundTripCheck() = true;
  }

  if (!pu.exemptMinFare())
  {
    const std::vector<FareUsage*>& fuVec = pu.fareUsage();
    std::vector<FareUsage*>::const_iterator fuIter = fuVec.begin();

    for (fuIter = fuVec.begin(); fuIter != fuVec.end(); fuIter++)
    {
      PaxTypeFare* paxTypeFare = (*fuIter)->paxTypeFare();
      // Exempt minfare check for cat25/cat35 fare
      if (paxTypeFare->isFareByRule() || paxTypeFare->isNegotiated())
      {
        pu.exemptMinFare() = true;
        break;
      }
    }
  }

  // Check side trip PUs
  std::vector<PricingUnit*>::const_iterator stPuI = pu.sideTripPUs().begin();
  for (; stPuI != pu.sideTripPUs().end(); ++stPuI)
  {
    checkExemption(trx, **stPuI);
  }
}

void
MinFareChecker::setTotalPlusUp(FarePath& farePath, MoneyAmount totalPlusUp)
{
  farePath.plusUpFlag() = true;
  farePath.plusUpAmount() = totalPlusUp;
  farePath.increaseTotalNUCAmount(totalPlusUp);
}

// Process all Pricing Units in fare path
void
MinFareChecker::processMinFareCheckOnPU(PricingTrx& trx,
                                        FarePath& farePath,
                                        MinFareChecks& mfChecks,
                                        MoneyAmount& totalPlusUp,
                                        DiagCollector& diag,
                                        int priority)
{
  const std::vector<PricingUnit*>& puVec = farePath.pricingUnit();
  processMinFareCheckOnPUCommon(trx, farePath, puVec, mfChecks, totalPlusUp, diag, priority, false);
}

// Check side trip PUs
void
MinFareChecker::processMinFareCheckOnSideTripPU(PricingTrx& trx,
                                                FarePath& farePath,
                                                PricingUnit& pu,
                                                MinFareChecks& mfChecks,
                                                MoneyAmount& totalPlusUp,
                                                DiagCollector& diag,
                                                int priority)
{
  const std::vector<PricingUnit*>& stPuI = pu.sideTripPUs();
  processMinFareCheckOnPUCommon(trx, farePath, stPuI, mfChecks, totalPlusUp, diag, priority, true);
}

void
MinFareChecker::processMinFareCheckOnPUCommon(PricingTrx& trx,
                                              FarePath& farePath,
                                              const std::vector<PricingUnit*>& puVec,
                                              MinFareChecks& mfChecks,
                                              MoneyAmount& totalPlusUp,
                                              DiagCollector& diag,
                                              int priority,
                                              bool checkOnSideTripPU)
{
  std::vector<PricingUnit*>::const_iterator puIter = puVec.begin();

  for (; puIter != puVec.end(); puIter++)
  {
    PricingUnit& pu = **puIter;
    if (!checkOnSideTripPU && pu.isSideTripPU())
      continue;

    if (pu.puFareType() == PricingUnit::NL)
    {
      if (pu.puType() == PricingUnit::Type::ROUNDTRIP)
      {
        if (priority == PRIORITY_LEVEL1)
          processRTMinimumFare(trx, farePath, pu, totalPlusUp, diag);

        if ((priority == PRIORITY_LEVEL2) && (farePath.itin()->geoTravelType() == GeoTravelType::International))
          mfChecks.processRSC = true;

        if (pu.geoTravelType() == GeoTravelType::International)
          mfChecks.processCOP = true;
      }
      else if (pu.puType() == PricingUnit::Type::CIRCLETRIP)
      {
        if (priority == PRIORITY_LEVEL1)
        {
          if (pu.geoTravelType() == GeoTravelType::International)
          {
            processCTMMinimumFare(trx, farePath, pu, mfChecks, totalPlusUp, diag);
          }
        }

        mfChecks.processCOP = true;
        if ((priority == PRIORITY_LEVEL2) && (farePath.itin()->geoTravelType() == GeoTravelType::International))
          mfChecks.processRSC = true;
      }
      else if (pu.puType() == PricingUnit::Type::ONEWAY)
      {
        mfChecks.processOSC = true;
        if (pu.geoTravelType() == GeoTravelType::International)
        {
          mfChecks.processCOM = true;
          mfChecks.processDMC = true;
        }
      }
      else if (pu.puType() == PricingUnit::Type::OPENJAW)
      {
        if ((priority == PRIORITY_LEVEL2) && (farePath.itin()->geoTravelType() == GeoTravelType::International))
          mfChecks.processRSC = true;

        totalPlusUp = totalPlusUp + processNormalOpenJaw(trx, farePath, pu, priority, diag);
        mfChecks.processDMC = true;
      }
    } // end if normal PU
    else if (pu.geoTravelType() == GeoTravelType::International)
    {
      if (pu.puType() == PricingUnit::Type::ROUNDTRIP)
      {
        if (priority == PRIORITY_LEVEL1)
          processRTMinimumFare(trx, farePath, pu, totalPlusUp, diag);
        mfChecks.processCOP = true;
      }
      else if (pu.puType() == PricingUnit::Type::CIRCLETRIP)
      {
        if (priority == PRIORITY_LEVEL1)
          processCTMMinimumFare(trx, farePath, pu, mfChecks, totalPlusUp, diag);

        mfChecks.processCOP = true;
      }
      else if (pu.puType() == PricingUnit::Type::ONEWAY)
      {
        mfChecks.processCOM = true;
        mfChecks.processDMC = true;
      }
      else if (LIKELY(pu.puType() == PricingUnit::Type::OPENJAW))
      {
        if (priority == PRIORITY_LEVEL1)
          processOJMMinimumFare(trx, farePath, pu, totalPlusUp, diag);
      }
    } // end if International PU
    else if ( (priority == PRIORITY_LEVEL1) && (pu.puType() == PricingUnit::Type::OPENJAW) )
    {
      processOJMMinimumFare(trx, farePath, pu, totalPlusUp, diag);
    }
    else if (pu.puFareType() == PricingUnit::SP && priority == PRIORITY_LEVEL1 &&
             pu.puType() == PricingUnit::Type::ROUNDTRIP)
    {
      processRTMinimumFare(trx, farePath, pu, totalPlusUp, diag);
    }

    if (!checkOnSideTripPU)
    {
      processMinFareCheckOnSideTripPU(trx, farePath, pu, mfChecks, totalPlusUp, diag, priority);
    }
  }
}

bool
MinFareChecker::processHIPMinimumFare(PricingTrx& trx,
                                      FarePath& farePath,
                                      MinFareChecks& mfChecks,
                                      MoneyAmount& totalPlusUp,
                                      DiagCollector& diag)
{
  HIPMinimumFare hip(trx, MinimumFare::DEFAULT_HIP_PROCESSING);
  totalPlusUp =
      totalPlusUp +
      hip.process(farePath, mfChecks.ruleLevelMap, mfChecks.applMap, mfChecks.defaultLogicMap);
  if (UNLIKELY(diag.isActive()))
    diag << "PROCESSED HIP CHECK - SEE DIAG 718 \n"
         << " \n";

  return hip.passedLimitation();
}

void
MinFareChecker::processCTMMinimumFare(PricingTrx& trx,
                                      FarePath& farePath,
                                      PricingUnit& pu,
                                      MinFareChecks& mfChecks,
                                      MoneyAmount& totalPlusUp,
                                      DiagCollector& diag)
{
  CTMMinimumFare ctm(
      trx, farePath, pu, mfChecks.ruleLevelMap, mfChecks.applMap, mfChecks.defaultLogicMap);

  totalPlusUp = totalPlusUp + ctm.process();
  if (diag.isActive())
  {
    diag << pu;
    diag << "PROCESSED CTM CHECK - SEE DIAG 719 \n"
         << " \n";
  }
}

void
MinFareChecker::processOJMMinimumFare(PricingTrx& trx,
                                      FarePath& farePath,
                                      PricingUnit& pu,
                                      MoneyAmount& totalPlusUp,
                                      DiagCollector& diag)
{
  OJMMinimumFare ojm(trx, farePath);
  totalPlusUp = totalPlusUp + ojm.process(pu);
  if (UNLIKELY(diag.isActive()))
  {
    diag << pu;
    diag << "PROCESSED HIGHEST RT OJ CHECK - SEE DIAG 767 \n"
         << " \n";
  }
}

void
MinFareChecker::processRTMinimumFare(PricingTrx& trx,
                                     FarePath& farePath,
                                     PricingUnit& pu,
                                     MoneyAmount& totalPlusUp,
                                     DiagCollector& diag)

{
  RTMinimumFare rtm(trx, farePath);
  totalPlusUp = totalPlusUp + rtm.process(pu);
  if (UNLIKELY(diag.isActive()))
  {
    diag << pu;
    diag << "PROCESSED HIGHEST RT CHECK - SEE DIAG 768 \n"
         << " \n";
  }
}

void
MinFareChecker::processCOPMinimumFare(PricingTrx& trx,
                                      FarePath& farePath,
                                      MoneyAmount& totalPlusUp,
                                      DiagCollector& diag)
{
  COPMinimumFare cop(trx, farePath);
  totalPlusUp = totalPlusUp + cop.process();
  if (UNLIKELY(diag.isActive()))
    diag << "PROCESSED COP CHECK - SEE DIAG 765" << '\n';
}

void
MinFareChecker::processCOMMinimumFare(PricingTrx& trx,
                                      FarePath& farePath,
                                      MinFareChecks& mfChecks,
                                      MoneyAmount& totalPlusUp,
                                      DiagCollector& diag)
{
  //        IMPORTANT !!!
  // This is the only check with no dates adjustment in case of REX trx
  COMMinimumFare com(
      trx, farePath, mfChecks.ruleLevelMap, mfChecks.applMap, mfChecks.defaultLogicMap);

  totalPlusUp = totalPlusUp + com.process();
  if (diag.isActive())
    diag << "PROCESSED COM CHECK - SEE DIAG 760" << '\n';
}

void
MinFareChecker::processDMCMinimumFare(PricingTrx& trx,
                                      FarePath& farePath,
                                      MoneyAmount& totalPlusUp,
                                      DiagCollector& diag)
{
  DMCMinimumFare dmc;
  totalPlusUp = totalPlusUp + dmc.process(trx, farePath);
  if (diag.isActive())
    diag << "PROCESSED DMC CHECK - SEE DIAG 762" << '\n';
}
}
