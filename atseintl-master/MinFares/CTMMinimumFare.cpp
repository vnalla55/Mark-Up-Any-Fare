/* vim: set ts=2 sts=2 sw=2: */
//----------------------------------------------------------------------------
//
//  File:     CTMMinimumFare.cpp
//  Created:  4/20/2004
//  Authors:
//
//  Description: This is the class to represent Circle Trip Minimum Check
//               methods.
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

#include "MinFares/CTMMinimumFare.h"

#include "Common/DiagMonitor.h"
#include "Common/Logger.h"
#include "Common/TravelSegUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Cabin.h"
#include "DBAccess/Loc.h"
#include "DBAccess/MinFareAppl.h"
#include "DBAccess/MinFareDefaultLogic.h"
#include "DBAccess/MinFareRuleLevelExcl.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "MinFares/HIPMinimumFare.h"
#include "MinFares/MinFareFareSelection.h"
#include "MinFares/MinFareLogic.h"
#include "MinFares/MinFareNormalFareSelection.h"
#include "MinFares/RoundTripFareSelection.h"

#include <iomanip>
#include <iostream>
#include <sstream>

namespace tse
{
static Logger
logger("atseintl.MinFares.CTMMinimumFare");

CTMMinimumFare::CTMMinimumFare(PricingTrx& trx,
                               FarePath& farePath,
                               PricingUnit& pu,
                               std::multimap<uint16_t, const MinFareRuleLevelExcl*>& ruleLevelMap,
                               std::multimap<uint16_t, const MinFareAppl*>& applMap,
                               std::map<uint16_t, const MinFareDefaultLogic*>& defaultLogicMap)
  : MinimumFare(farePath.itin()->travelDate()),
    _trx(trx),
    _farePath(farePath),
    _pu(pu),
    _ruleLevelMap(ruleLevelMap),
    _applMap(applMap),
    _defaultLogicMap(defaultLogicMap)

{
  LOG4CXX_DEBUG(logger, "Construct CTMMinimumFare");

  if (_trx.excTrxType() == PricingTrx::PORT_EXC_TRX)
    adjustPortExcDates(_trx);
}

CTMMinimumFare::~CTMMinimumFare()
{
  LOG4CXX_DEBUG(logger, "Destruct CTMMinimumFare");
}

MoneyAmount
CTMMinimumFare::process()
{
  if (!qualifyCtmCheck())
  {
    return 0.0;
  }

  DiagMonitor diagMonitor(_trx, Diagnostic719);
  DiagCollector& diag = diagMonitor.diag();

  diag << " \nCIRCLE TRIP MINIMUM - " << _farePath.paxType()->paxType() << '\n';
  diag << "CITY      GOV  CLASS                         DIR FARE  GLOG EXCL\n"
       << "PAIR      CXR           CUR   AMOUNT RTG TAG I/O TYPE  IND  IND \n";

  return process(diag, false);
}

MoneyAmount
CTMMinimumFare::process(DiagCollector& diag, bool cpmCheck)
{
  if (cpmCheck && !qualifyCtmCheck()) // to avoid calling qualifyCtmCheck() again
  {
    return 0.0;
  }

  const Itin& itin = *_farePath.itin();
  const PaxType& requestedPaxType = *_farePath.paxType();
  const std::vector<FareUsage*>& fuVec = _pu.fareUsage();

  CabinType lowestCabin; // = UNKNOWN_CLASS; done in constructor
  bool mixedCabin = MinFareLogic::isMixedCabin(_pu, lowestCabin);
  LOG4CXX_DEBUG(logger, "mixed Cabin: " << mixedCabin);

  // Display the thru fares.
  printThruFare(diag);

  // Check if this PU is being exempted by DA/DP Discount Pricing or by Cat25 fare
  if (_pu.exemptMinFare())
  {
    LOG4CXX_INFO(logger, "PU is exempted.");
    diag << "PU IS EXEMPTED\n";
    return 0.0;
  }

  // Check if around the world travel originating in Australia or New Zealand,
  // shall not apply CTM check
  if (checkRtwException())
  {
    diag << "RTW TRAVEL ORIGINATING IN AUSTRALIA/NEW ZEALAND - NO CTM CHECK\n";
    return 0.0;
  }

  // Check if PU contains infant domestic 0 amount then not apply CTM check
  if (checkInfantException())
  {
    diag << "ZERO AMOUNT INFANT DOMESTIC FARE FOUND - NO CTM CHECK\n";
    return 0.0;
  }

  if (!getThruMarketFare())
  {
    if (_trx.excTrxType() == PricingTrx::AR_EXC_TRX)
      restorePricingDates(_trx, _trx.ticketingDate());

    return 0.0;
  }

  bool sameCarrier = isPuSameCarrier();
  unsigned int excl_count = checkRuleLevelExcl(sameCarrier, diag);
  if ((excl_count == _ctmFareList.size()) || (sameCarrier && excl_count >= 1))
  {
    if (_trx.excTrxType() == PricingTrx::AR_EXC_TRX)
      restorePricingDates(_trx, _trx.ticketingDate());

    return 0.0;
  }

  checkApplication();

  // Moved out of the for loop, check heck HIP for the whole PU once, instead
  // of calling for each fare component inside the for loop.
  HIPMinimumFare normalHip(_trx, MinimumFare::NORMAL_HIP_PROCESSING);
  if (_pu.puFareType() == PricingUnit::SP)
  {
    std::multimap<uint16_t, const MinFareRuleLevelExcl*> ruleLevelMap;
    std::multimap<uint16_t, const MinFareAppl*> applMap;
    std::map<uint16_t, const MinFareDefaultLogic*> defaultLogicMap;

    normalHip.process(_pu, _farePath, ruleLevelMap, applMap, defaultLogicMap);
  }

  _baseFare = 0;
  std::set<const TravelSeg*> processedTvlSeg;

  if (_trx.excTrxType() == PricingTrx::AR_EXC_TRX)
  {
    _travelDate = _farePath.itin()->travelDate();

    adjustRexPricingDates(_trx, _pu.fareUsage().front()->paxTypeFare()->retrievalDate());
  }

  for (auto& ctmFare : _ctmFareList)
  {

    if (!ctmFare.thruMarketFare)
      continue;

    if (!ctmFare.fareUsage)
      continue;

    const PaxTypeFare& paxTypeFare(*ctmFare.thruMarketFare);

    if (!cpmCheck && ctmFare.ruleLevelExcl && ctmFare.excludedByRuleLevelExcl)
    {
      printExceptionInfo(paxTypeFare, diag);
      diag << " EXEMPTED BY RULE LEVEL EXCL TABLE - " << ctmFare.ruleLevelExcl->seqNo() << "\n";

      // If excluded by Rule Level Excl table, we will not check the interm.
      // stopover/ticket point in this fare component. Go to the next one.
      continue;
    }

    if (_baseFare == 0)
    {
      if (mixedCabin)
      {
        LOG4CXX_DEBUG(logger, "Mixed Cabin: Reprice to lowest cabin");
        _baseFare = MinFareLogic::selectLowestCabinFare(&diag,
                                                        CTM,
                                                        _pu,
                                                        lowestCabin,
                                                        _trx,
                                                        _farePath,
                                                        requestedPaxType,
                                                        _travelDate,
                                                        (*(fuVec.begin()))->isPaxTypeFareNormal(),
                                                        ctmFare.appl,
                                                        ctmFare.defaultLogic);

        if (_baseFare == 0)
          calculateBaseFare();
      }
      else
      {
        calculateBaseFare();
      }
    }

    std::vector<TravelSeg*>& puTvlSeg = _pu.travelSeg();

    std::vector<TravelSeg*>::iterator tvlBoard = puTvlSeg.begin();
    std::vector<TravelSeg*>::iterator tvlOff = ctmFare.tvlOff;

    std::vector<TravelSeg*> obTvlSeg(tvlBoard, tvlOff + 1);
    std::vector<TravelSeg*> ibTvlSeg(tvlOff + 1, puTvlSeg.end());

    if ((_pu.puFareType() == PricingUnit::SP) && (!normalHip.isSpecialHipSpecialOnly()) &&
        normalHip.isNormalExempted(obTvlSeg))
    {
      printExceptionInfo(diag,
                         obTvlSeg.front()->boardMultiCity(),
                         obTvlSeg.back()->offMultiCity(),
                         "EXEMPT BY NORMAL HIP\n");
      continue;
    }

    {

      if (!cpmCheck)
      {
        if (ctmFare.appl->ctmStopTktInd() == 'S' &&
            !isStopOver(itin, ctmFare.fareUsage, **tvlOff, **(tvlOff + 1)))
        {
          printExceptionInfo(diag, obTvlSeg, MinFareFareSelection::OUTBOUND);
          diag << "/" << paxTypeFare.fareMarket()->governingCarrier()
               << "/ EXEMPT BY NON-STOPOVER POINT - APPL. TABLE " << ctmFare.appl->seqNo() << "\n";
          continue;
        }

        if (checkDomesticExclusion(
                itin, paxTypeFare, ctmFare.appl, ctmFare.defaultLogic, tvlBoard, tvlOff))
        {
          printExceptionInfo(
              diag, obTvlSeg, MinFareFareSelection::OUTBOUND, "EXEMPT BY DOMESTIC\n");
          continue;
        }

        if (checkIntermediateExclusion(_trx,
                                       CTM,
                                       *ctmFare.appl,
                                       ctmFare.defaultLogic,
                                       paxTypeFare.isNormal(),
                                       puTvlSeg,
                                       itin,
                                       &_pu,
                                       ctmFare.fareUsage,
                                       MinFareFareSelection::OUTBOUND,
                                       tvlBoard,
                                       tvlOff))
        {
          printExceptionInfo(diag, obTvlSeg, MinFareFareSelection::OUTBOUND);
          diag << "EXEMPT BY INTERM. LOC. EXCL. APPL. TABLE " << ctmFare.appl->seqNo() << " "
               << (*tvlOff)->destination()->loc() << '\n';
          continue;
        }

        if (!(checkIntermediateCityPair(CTM,
                                        *ctmFare.appl,
                                        ctmFare.defaultLogic,
                                        paxTypeFare.isNormal(),
                                        puTvlSeg,
                                        itin,
                                        tvlBoard,
                                        tvlOff)))
        {
          printExceptionInfo(diag, obTvlSeg, MinFareFareSelection::OUTBOUND);
          if ((ctmFare.appl->applyDefaultLogic() == YES) && (ctmFare.defaultLogic != nullptr))
          {
            diag << "EXEMPT BY DETAIL COMPO. CHK DEFAULT LOGIC TBL "
                 << ctmFare.defaultLogic->seqNo() << '\n';
          }
          else
          {
            diag << "EXEMPT BY DETAIL COMPO CHK APPL. TBL " << ctmFare.appl->seqNo() << '\n';
          }
          continue;
        }
      }

      if ((ctmFare.selectFareForDirection == FMDirection::UNKNOWN) &&
          (processedTvlSeg.find(*tvlOff) != processedTvlSeg.end()))
      {
        continue;
      }
      else
      {
        processedTvlSeg.insert(*tvlOff);
      }

      /////////////////////////////////////////////////////////////////////
      // Using RoundTripFareSelection:
      LOG4CXX_DEBUG(logger, "Calling RoundTripFareSelection");

      RepricingTrx* obRepriceTrx = nullptr;
      RepricingTrx* ibRepriceTrx = nullptr;

      // Circle trip provision: if the CT provision city is at the destination,
      // add an Arunk segment at the end.
      ArunkSeg aSeg;

      if (obTvlSeg.front()->boardMultiCity() != ibTvlSeg.back()->offMultiCity())
      {
        aSeg.segmentType() = Arunk;
        aSeg.origin() = ibTvlSeg.back()->destination();
        aSeg.boardMultiCity() = ibTvlSeg.back()->offMultiCity();
        aSeg.origAirport() = ibTvlSeg.back()->destAirport();
        aSeg.destination() = obTvlSeg.front()->origin();
        aSeg.offMultiCity() = obTvlSeg.front()->boardMultiCity();
        aSeg.destAirport() = obTvlSeg.front()->origAirport();
        ibTvlSeg.push_back(&aSeg);

        ibRepriceTrx = TrxUtil::reprice(_trx, ibTvlSeg, FMDirection::UNKNOWN);
      }

      // diag << "OB: " << obTvlSeg << " - IB: " << ibTvlSeg << "\n";

      LOG4CXX_DEBUG(logger,
                    "TVLSEG: OB: " << (*tvlBoard)->origin()->loc()
                                   << (*tvlOff)->destination()->loc()
                                   << ", IB: " << (*(tvlOff + 1))->origin()->loc()
                                   << ibTvlSeg.back()->destination()->loc());

      FMDirection selectfareUsageDirection = FMDirection::UNKNOWN;
      if (_pu.puFareType() == PricingUnit::SP)
        selectfareUsageDirection =
            ctmFare.fareUsage->isOutbound() ? FMDirection::OUTBOUND : FMDirection::INBOUND;

      RoundTripFareSelection rtFareSel(CTM,
                                       &diag,
                                       _trx,
                                       _farePath,
                                       _pu,
                                       obTvlSeg,
                                       ibTvlSeg,
                                       lowestCabin,
                                       _farePath.paxType(),
                                       _travelDate,
                                       fuVec.front()->paxTypeFare(),
                                       fuVec.back()->paxTypeFare(),
                                       ctmFare.appl,
                                       ctmFare.defaultLogic,
                                       obRepriceTrx,
                                       ibRepriceTrx,
                                       ctmFare.selectFareForDirection,
                                       selectfareUsageDirection);

      compareAndSave(rtFareSel, ctmFare);
      /////////////////////////////////////////////////////////////////////
    }
  }

  if (_trx.excTrxType() == PricingTrx::AR_EXC_TRX)
    restorePricingDates(_trx, _trx.ticketingDate());

  const CtmFareCheck* ctmFareCheck = getHighestCtmFare();
  if (ctmFareCheck != nullptr)
    savePlusUp(*ctmFareCheck);

  if (cpmCheck)
  {
    // This is called from CPM, we don't need to print the summary, just return
    // the highest CTM fare amount.
    return getCtmFareAmt(ctmFareCheck);
  }
  else
  {
    return printPlusUpInfo(diag);
  }
}

void
CTMMinimumFare::printThruFare(DiagCollector& diag) const
{
  const std::vector<FareUsage*>& fuVec = _pu.fareUsage();

  for (const auto fu : fuVec)
  {
    MoneyAmount plusUpAndSurcharge =
        fu->paxTypeFare()->mileageSurchargeAmt() + fu->minFarePlusUp().getSum(HIP);
    printFareInfo(*fu->paxTypeFare(), diag, CTM, "* ", false, plusUpAndSurcharge);
  }
}

void
CTMMinimumFare::compareAndSave(const PaxTypeFare* obFare,
                               const PaxTypeFare* ibFare,
                               CtmFareCheck& ctmFareCheck)
{
  const PaxTypeFare* lowerFare = nullptr;
  if (obFare && ibFare)
  {
    if (ibFare->nucFareAmount() < obFare->nucFareAmount())
      lowerFare = ibFare;
    else
      lowerFare = obFare;
  }
  else if (obFare)
  {
    lowerFare = obFare;
  }
  else if (ibFare)
  {
    lowerFare = ibFare;
  }

  if (lowerFare)
  {
    ctmFareCheck.selectedFare = lowerFare;
    updateCtmFareMap(ctmFareCheck);
  }
}

void
CTMMinimumFare::calculateBaseFare()
{
  const std::vector<FareUsage*>& fuVec = _pu.fareUsage();
  std::vector<FareUsage*>::const_iterator fareUsageI;

  _baseFare = 0;

  for (fareUsageI = fuVec.begin(); fareUsageI != fuVec.end(); fareUsageI++)
  {
    const PaxTypeFare* paxTypeFare = (*fareUsageI)->paxTypeFare(); // lint !e530
    _baseFare += paxTypeFare->nucFareAmount() + paxTypeFare->mileageSurchargeAmt();

    MoneyAmount hipAmount = (*fareUsageI)->minFarePlusUp().getSum(HIP);
    _baseFare += hipAmount;
  }
}

void
CTMMinimumFare::compareAndSave(const RoundTripFareSelection& rtFareSel, CtmFareCheck& ctmFareCheck)
{
  const std::vector<const PaxTypeFare*>& rtFares = rtFareSel.roundTripFare();
  if (rtFareSel.constructedFare())
  {
    LOG4CXX_DEBUG(logger, "RT Fare Sel: return constructed fare");
    if (rtFares.size() != 2)
    {
      LOG4CXX_ERROR(logger, "Fare was constructed, but not properly saved!");
      return;
    }
    if (rtFares.size() == 2)
    {
      MoneyAmount constructedFareAmt = rtFareSel.constructedFareAmt();

      ConstructInfo& constructInfo = ctmFareCheck.constructInfo;
      if (constructedFareAmt > constructInfo._highestUnPubAmount)
      {
        const PaxTypeFare* cfare1 = rtFares.front();
        const PaxTypeFare* cfare2 = rtFares.back();

        constructInfo._highestUnPubAmount = constructedFareAmt;
        constructInfo._boardConstructed =
            cfare1->fareMarket()->travelSeg().front()->boardMultiCity();
        constructInfo._constructedPoint = cfare1->fareMarket()->travelSeg().back()->offMultiCity();
        constructInfo._offConstructed = cfare2->fareMarket()->travelSeg().back()->offMultiCity();

        updateCtmFareMap(ctmFareCheck);
      }
    }
  }
  else
  {
    LOG4CXX_DEBUG(logger, "rt fare count: " << rtFares.size());
    if (rtFares.size() > 0)
    {
      compareAndSave(rtFares.front(), rtFares.back(), ctmFareCheck);
    }
  }
}

void
CTMMinimumFare::updateCtmFareMap(CtmFareCheck& ctmFareCheck)
{
  LocCode boardCity = (ctmFareCheck.selectedFare != nullptr)
                          ? ctmFareCheck.selectedFare->fareMarket()->boardMultiCity()
                          : ctmFareCheck.constructInfo._boardConstructed;
  LocCode offCity = (ctmFareCheck.selectedFare != nullptr)
                        ? ctmFareCheck.selectedFare->fareMarket()->offMultiCity()
                        : ctmFareCheck.constructInfo._offConstructed;

  std::map<std::string, CtmFareCheck*>::iterator iter =
      _ctmFareMap.find(std::string(boardCity + offCity));
  if (iter == _ctmFareMap.end())
  {
    iter = _ctmFareMap.find(offCity + boardCity);
  }

  if (iter != _ctmFareMap.end())
  {
    MoneyAmount storedFareAmount = getFareAmount(*iter->second);
    MoneyAmount currentFareAmount = getFareAmount(ctmFareCheck);

    if (storedFareAmount > currentFareAmount)
      iter->second = &ctmFareCheck;
  }
  else
  {
    _ctmFareMap.insert(
        std::pair<std::string, CtmFareCheck*>(std::string(boardCity + offCity), &ctmFareCheck));
  }
}

const CTMMinimumFare::CtmFareCheck*
CTMMinimumFare::getHighestCtmFare() const
{
  const CtmFareCheck* highestCtmFare = nullptr;
  MoneyAmount highestCtmAmount = _baseFare;

  for (const auto& elem : _ctmFareMap)
  {
    CtmFareCheck* ctmFareCheck = elem.second;
    if (ctmFareCheck != nullptr)
    {
      MoneyAmount curCtmAmount = getFareAmount(*ctmFareCheck) * 2;

      if (curCtmAmount > highestCtmAmount)
      {
        highestCtmFare = ctmFareCheck;
        highestCtmAmount = curCtmAmount;
      }
    }
  }

  return highestCtmFare;
}

void
CTMMinimumFare::savePlusUp(const CtmFareCheck& ctmFareCheck)
{
  MinFarePlusUpItem* plusUpInfo = nullptr;

  if (ctmFareCheck.selectedFare != nullptr)
  {
    const PaxTypeFare* ctmFare = ctmFareCheck.selectedFare;
    _trx.dataHandle().get(plusUpInfo);

    if (plusUpInfo == nullptr)
    {
      LOG4CXX_ERROR(logger, "DataHandle failed to allocate memory (plusUpInfo)");
      return;
    }
    // lint --e{413}
    plusUpInfo->plusUpAmount = ((ctmFare->nucFareAmount()) * 2) - _baseFare;
    plusUpInfo->baseAmount = _baseFare;
    const FareMarket& ctmFareMarket = *ctmFare->fareMarket();
    plusUpInfo->boardPoint = _pu.travelSeg().front()->boardMultiCity();
    if (plusUpInfo->boardPoint == ctmFareMarket.boardMultiCity())
    {
      plusUpInfo->offPoint = ctmFareMarket.offMultiCity();
    }
    else
    {
      plusUpInfo->offPoint = ctmFareMarket.boardMultiCity();
    }
    _pu.minFarePlusUp().addItem(CTM, plusUpInfo);
  }
  else
  {
    const ConstructInfo& constructInfo = ctmFareCheck.constructInfo;

    _trx.dataHandle().get(plusUpInfo);

    if (plusUpInfo == nullptr)
    {
      LOG4CXX_ERROR(logger, "DataHandle failed to allocate memory (plusUpInfo)");
      return;
    }
    // lint --e{413}
    plusUpInfo->plusUpAmount = (constructInfo._highestUnPubAmount * 2) - _baseFare;
    plusUpInfo->baseAmount = _baseFare;
    plusUpInfo->constructPoint = constructInfo._constructedPoint;

    plusUpInfo->boardPoint = _pu.travelSeg().front()->boardMultiCity();
    if (plusUpInfo->boardPoint == constructInfo._boardConstructed)
    {
      plusUpInfo->offPoint = constructInfo._offConstructed;
    }
    else
    {
      plusUpInfo->offPoint = constructInfo._boardConstructed;
    }

    _pu.minFarePlusUp().addItem(CTM, plusUpInfo);
  }
}

MoneyAmount
CTMMinimumFare::getCtmFareAmt(const CtmFareCheck* ctmFareCheck) const
{
  MinFarePlusUp::const_iterator ctmPlusUpI;

  ctmPlusUpI = _pu.minFarePlusUp().find(CTM);
  // return the highest CTM fare amount:
  if (ctmPlusUpI != _pu.minFarePlusUp().end())
  {
    return (ctmPlusUpI->second->plusUpAmount + ctmPlusUpI->second->baseAmount);
  }

  if (ctmFareCheck != nullptr)
  {
    return (getFareAmount(*ctmFareCheck) * 2);
  }

  // otherwise, return the base amount:
  return _baseFare;
}

MoneyAmount
CTMMinimumFare::getFareAmount(const CtmFareCheck& ctmFareCheck) const
{
  return ((ctmFareCheck.selectedFare != nullptr) ? ctmFareCheck.selectedFare->nucFareAmount()
                                           : ctmFareCheck.constructInfo._highestUnPubAmount);
}

MoneyAmount
CTMMinimumFare::printPlusUpInfo(DiagCollector& diag)
{
  std::ostringstream os;

  os << "  BASE FARE NUC";
  os.setf(std::ios::right, std::ios::adjustfield);
  os.setf(std::ios::fixed, std::ios::floatfield);
  os.precision(2);
  os << _baseFare << " ";

  MoneyAmount ctmPlusUpAmount = 0.0;
  MinFarePlusUpItem* ctmPlusUpInfo = nullptr;
  MinFarePlusUp::const_iterator ctmPlusUpI;

  ctmPlusUpI = _pu.minFarePlusUp().find(CTM);
  if (ctmPlusUpI != _pu.minFarePlusUp().end())
  {
    ctmPlusUpInfo = ctmPlusUpI->second;
    ctmPlusUpAmount = ctmPlusUpInfo->plusUpAmount;
    os << ctmPlusUpInfo->boardPoint << ctmPlusUpInfo->offPoint << "    PLUS UP ";

    os.setf(std::ios::right, std::ios::adjustfield);
    os.setf(std::ios::fixed, std::ios::floatfield);
    os.precision(2);
    os << std::setw(8);
    os << ctmPlusUpAmount << " \n";
  }
  else
  {
    os << "   PLUS UP 0.00\n";
  }

  if (diag.isActive())
  {
    diag << os.str();
  }

  return ctmPlusUpAmount;
}

bool
CTMMinimumFare::isPuSameCarrier() const
{
  bool sameCarrier = true;

  const std::vector<FareUsage*>& fuVec = _pu.fareUsage();
  CarrierCode carrier = fuVec.front()->paxTypeFare()->fareMarket()->governingCarrier();

  for (std::vector<FareUsage*>::const_iterator fuI = fuVec.begin(), fuEnd = fuVec.end();
       ++fuI != fuEnd;)
  {
    if (carrier != (*fuI)->paxTypeFare()->fareMarket()->governingCarrier())
    {
      sameCarrier = false;
      break;
    }
  }

  return sameCarrier;
}

int
CTMMinimumFare::checkRuleLevelExcl(bool sameCarrier, DiagCollector& diag)
{
  int excl_count = 0;

  for (auto& ctmFare : _ctmFareList)
  {
    const PaxTypeFare* thruMarketFare = nullptr;

    if (ctmFare.fareUsage != nullptr)
      thruMarketFare = ctmFare.fareUsage->paxTypeFare();

    if (thruMarketFare == nullptr)
      continue;

    if (_trx.excTrxType() == PricingTrx::AR_EXC_TRX)
    {
      _travelDate = _farePath.itin()->travelDate();
      adjustRexPricingDates(_trx, thruMarketFare->retrievalDate());
    }

    ctmFare.ruleLevelExcl = MinFareLogic::getRuleLevelExcl(
        _trx, *_farePath.itin(), *thruMarketFare, CTM, _ruleLevelMap, _travelDate); // lint !e530

    if (ctmFare.ruleLevelExcl)
    {
      if (ctmFare.ruleLevelExcl->ctmMinFareAppl() == YES)
      {
        ctmFare.excludedByRuleLevelExcl = true;
        excl_count++;

        // If all fare components have the same carrier, then all fares will
        // be excluded by the same excl rule.  We can stop here.
        if (sameCarrier)
        {
          printExceptionInfo(*thruMarketFare, diag);
          diag << " EXEMPTED BY RULE LEVEL EXCL TABLE - " << ctmFare.ruleLevelExcl->seqNo() << "\n";
          diag << "SAME CARRIER - CTM EXEMPTED FOR PU\n";
          break;
        }
      }
    }
  }

  return excl_count;
}

void
CTMMinimumFare::checkApplication()
{
  for (auto& ctmFare : _ctmFareList)
  {
    const PaxTypeFare* thruMarketFare = nullptr;

    if (ctmFare.fareUsage != nullptr)
      thruMarketFare = ctmFare.fareUsage->paxTypeFare();

    if (thruMarketFare == nullptr)
      continue;

    if (_trx.excTrxType() == PricingTrx::AR_EXC_TRX)
    {
      _travelDate = _farePath.itin()->travelDate();
      adjustRexPricingDates(_trx, thruMarketFare->retrievalDate());
    }

    const PaxTypeFare& paxTypeFare(*thruMarketFare);
    ctmFare.appl = MinFareLogic::getApplication(_trx,
                                                _farePath,
                                                *_farePath.itin(),
                                                paxTypeFare,
                                                CTM,
                                                _applMap,
                                                _defaultLogicMap,
                                                _travelDate);

    if (ctmFare.appl == nullptr)
    {
      LOG4CXX_ERROR(logger, "No matching Application table item found.")
      throw ErrorResponseException(ErrorResponseException::MIN_FARE_MISSING_DATA);
    }

    if (ctmFare.appl->applyDefaultLogic() == YES)
    {

      ctmFare.defaultLogic = MinFareLogic::getDefaultLogic(
          CTM, _trx, *_farePath.itin(), paxTypeFare, _defaultLogicMap);

      if (ctmFare.defaultLogic == nullptr)
      {
        LOG4CXX_ERROR(logger,
                      "Default logic processing required, "
                      "but no Default Logic table item found.");
        throw ErrorResponseException(ErrorResponseException::MIN_FARE_MISSING_DATA);
      }
    }
  }
}

bool
CTMMinimumFare::checkRtwException()
{
  std::vector<TravelSeg*>& tvlSeg = _pu.travelSeg();
  std::string origNation = tvlSeg.front()->origin()->nation();
  std::string destNation = tvlSeg.back()->destination()->nation();

  // check the PU origin country for Australia and New Zealand
  if (origNation != destNation || (origNation != AUSTRALIA && origNation != NEW_ZEALAND))
  {
    return false;
  }

  // check if Around the World (ATW): Travel from the point of origin and
  // return thereto, or travel from the point of origin and return to the
  // same country which involves only one crossing of the Atlantic Ocean,
  // and only one crossing of the Pacific Ocean.

  int paCnt = 0, atCnt = 0;
  for (const auto fu : _pu.fareUsage())
  {
    GlobalDirection gd = fu->paxTypeFare()->fareMarket()->getGlobalDirection();
    if (gd == GlobalDirection::AP || gd == GlobalDirection::NP || gd == GlobalDirection::PA ||
        gd == GlobalDirection::PN)
      paCnt++;
    if (gd == GlobalDirection::AP || gd == GlobalDirection::AT || gd == GlobalDirection::SA ||
        gd == GlobalDirection::SN)
      atCnt++;
  }

  if (paCnt == 1 && atCnt == 1)
  {
    return true;
  }

  return false;
}

bool
CTMMinimumFare::checkInfantException()
{
  // check if this PU contain infant fare 0 adomestic 0 amount
  //     then no CTM check for this PU
  for (const auto fu : _pu.fareUsage())
  {
    if ((fu->paxTypeFare()->fcasPaxType() == INFANT) && (fu->paxTypeFare()->isDomestic()) &&
        (fu->paxTypeFare()->originalFareAmount() == 0))
      return true;
  }

  return false;
}

bool
CTMMinimumFare::qualifyCtmCheck() const
{
  return !(_pu.puFareType() == PricingUnit::SP);
}

bool
CTMMinimumFare::getThruMarketFare()
{
  const std::vector<FareUsage*>& fuVec = _pu.fareUsage();

  if (fuVec.empty())
    return false;

  const PaxTypeFare* paxTypeFare = fuVec.front()->paxTypeFare();

  if (_trx.excTrxType() == PricingTrx::AR_EXC_TRX)
    adjustRexPricingDates(_trx, paxTypeFare->retrievalDate());

  const MinFareAppl* appl = MinFareLogic::getApplication(_trx,
                                                         _farePath,
                                                         *_farePath.itin(),
                                                         *paxTypeFare,
                                                         CTM,
                                                         _applMap,
                                                         _defaultLogicMap,
                                                         _travelDate);

  const MinFareDefaultLogic* defLogic =
      MinFareLogic::getDefaultLogic(CTM, _trx, *_farePath.itin(), *paxTypeFare, _defaultLogicMap);

  for (std::vector<TravelSeg*>::iterator tvlBegin = _pu.travelSeg().begin(),
                                         tvlI = _pu.travelSeg().begin(),
                                         tvlEnd = _pu.travelSeg().end();
       (tvlI + 1) != tvlEnd;
       ++tvlI)
  {
    std::vector<TravelSeg*> obTvlSeg(tvlBegin, tvlI + 1);

    CtmFareCheck ctmFare;
    ctmFare.tvlOff = tvlI;

    // Keep the last segment's fare usage - needed for HIP check.
    for (std::vector<FareUsage*>::const_iterator fuI = fuVec.begin(),
                                                 fuBegin = fuVec.begin(),
                                                 fuEnd = fuVec.end();
         fuI != fuEnd;
         ++fuI)
    {
      std::vector<TravelSeg*>::const_iterator iter =
          std::find((*fuI)->travelSeg().begin(), (*fuI)->travelSeg().end(), *tvlI);

      if (iter != (*fuI)->travelSeg().end())
      {
        if (_pu.puFareType() == PricingUnit::NL)
        {
          ctmFare.fareUsage = *fuI;
          ctmFare.thruMarketFare = (*fuI)->paxTypeFare();
          break;
        }
        else // Special Fare Processing -- new code
        {
          if (fuI == fuBegin || *fuI == fuVec.back())
          {
            ctmFare.fareUsage = *fuI;
            ctmFare.thruMarketFare = (*fuI)->paxTypeFare();
          }
          else if ((*fuI)->isOutbound())
          {
            ctmFare.fareUsage = *(fuBegin);
            ctmFare.thruMarketFare = (*(fuBegin))->paxTypeFare();
          }
          else if ((*fuI)->isInbound())
          {
            ctmFare.fareUsage = *(fuEnd - 1);
            ctmFare.thruMarketFare = (*(fuEnd - 1))->paxTypeFare();
          }
          break;
        }
      }
    }

    const std::vector<TravelSeg*>& puObTvlSegs = paxTypeFare->fareMarket()->travelSeg();
    const PaxTypeFare* thruMrktFare =
        (obTvlSeg.front() == puObTvlSegs.front() && obTvlSeg.back() == puObTvlSegs.back())
            ? paxTypeFare
            : MinFareLogic::selectQualifyFare(CTM,
                                              _trx,
                                              *_farePath.itin(),
                                              *paxTypeFare,
                                              *_farePath.paxType(),
                                              paxTypeFare->cabin(),
                                              paxTypeFare->isNormal(),
                                              MinFareFareSelection::OUTBOUND,
                                              MinFareLogic::eligibleFare(_pu),
                                              obTvlSeg,
                                              _travelDate,
                                              appl,
                                              defLogic,
                                              nullptr,
                                              &_farePath);

    if (thruMrktFare)
    {
      ctmFare.thruMarketFare = thruMrktFare;
    }

    bool isSurfaceMissing = ((*tvlI)->offMultiCity() != (*(tvlI + 1))->boardMultiCity());
    if (isSurfaceMissing)
    {
      CtmFareCheck secondCtmFare;
      secondCtmFare.tvlOff = ctmFare.tvlOff;
      secondCtmFare.fareUsage = ctmFare.fareUsage;
      secondCtmFare.thruMarketFare = ctmFare.thruMarketFare;
      secondCtmFare.selectFareForDirection = FMDirection::INBOUND;

      ctmFare.selectFareForDirection = FMDirection::OUTBOUND;

      _ctmFareList.push_back(ctmFare);
      _ctmFareList.push_back(secondCtmFare);
    }
    else
    {
      _ctmFareList.push_back(ctmFare);
    }
  }

  return (!_ctmFareList.empty());
}
} // namespace tse
