//----------------------------------------------------------------------------
//
//  File:     COMMinimumFare.cpp
//  Created:  3/18/2004
//  Authors:
//
//  Description	:This is the class for Country Minimum Fare Check.
//
//  Copyright Sabre 2004
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//
//----------------------------------------------------------------------------
#include "MinFares/COMMinimumFare.h"

#include "Common/DiagMonitor.h"
#include "Common/ErrorResponseException.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/TravelSegUtil.h"
#include "Common/TseUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"
#include "DBAccess/MinFareAppl.h"
#include "DBAccess/MinFareDefaultLogic.h"
#include "DBAccess/MinFareRuleLevelExcl.h"
#include "Diagnostic/DiagCollector.h"
#include "MinFares/HIPMinimumFare.h"
#include "MinFares/MinFareFareSelection.h"
#include "MinFares/MinFareLogic.h"
#include "MinFares/MinFareNormalFareSelection.h"

#include <functional>

namespace tse
{
static Logger
logger("atseintl.MinFares.COMMinimumFare");

namespace ComUtil
{

template <class T>
bool
includes(const std::set<T>& s1, const std::set<T>& s2)
{
  return std::includes(s1.begin(), s1.end(), s2.begin(), s2.end());
}

std::set<int>
getTvlSegOrders(const Itin& aItin, const std::vector<TravelSeg*>& tvl)
{
  std::set<int> segOrders;
  for (std::vector<TravelSeg*>::const_iterator i = tvl.begin(), end = tvl.end(); i != end; ++i)
  {
    segOrders.insert(aItin.segmentOrder(*i));
  }
  return segOrders;
}

std::set<int>
getPuSegOrders(const Itin& aItin, const PricingUnit& pu)
{
  return getTvlSegOrders(aItin, pu.travelSeg());
}

void
getOscPlusUp(const FarePath& farePath, std::map<std::set<int>, FarePath::OscPlusUp*>& oscPlusUpMap)
{
  Loc const* const origin = MinimumFare::originOfPricingUnit(*farePath.pricingUnit().front());

  TravelSegUtil::OriginNationEquals viaCountryOfOrigin(origin->nation());

  // lint -e{530}
  const std::vector<FarePath::OscPlusUp*>& oscPlusUp = farePath.oscPlusUp();

  for (std::vector<FarePath::OscPlusUp*>::const_iterator i = oscPlusUp.begin(),
                                                         end = oscPlusUp.end();
       i != end;
       ++i)
  {
    const std::set<int> k1 =
        getTvlSegOrders(*farePath.itin(), (*i)->thruFare->fareMarket()->travelSeg());
    bool insert = true;
    for (std::map<std::set<int>, FarePath::OscPlusUp*>::iterator mI = oscPlusUpMap.begin(),
                                                                 mEnd = oscPlusUpMap.end();
         mI != mEnd;)
    {
      if (((*mI).first == k1) || includes((*mI).first, k1))
      {
        insert = false;
        break;
      }
      else if (includes(k1, (*mI).first))
      {
        oscPlusUpMap.erase(mI++);
      }
      else
      {
        ++mI;
      }
    }

    if (insert)
    {
      // Check if travelling via the country of origin
      const std::vector<TravelSeg*>& tvlSeg = (*i)->thruFare->fareMarket()->travelSeg();
      if (std::find_if(tvlSeg.begin(), tvlSeg.end(), viaCountryOfOrigin) != tvlSeg.end())
      {
        // ...via Country of Origin, will process as multi pu
        oscPlusUpMap.insert(std::make_pair(k1, *i));
      }
    }
  }
}

// COM's notion of same country:
struct OriginNationEqual : public std::binary_function<TravelSeg*, Loc*, bool>
{
  bool operator()(const TravelSeg* t, const Loc* loc) const
  {
    return (t && loc && LocUtil::isSameISINation(*t->origin(), *loc));
  }
};

} // end of ComUtil

COMMinimumFare::COMMinimumFare(PricingTrx& trx,
                               FarePath& farePath,
                               std::multimap<uint16_t, const MinFareRuleLevelExcl*>& ruleLevelMap,
                               std::multimap<uint16_t, const MinFareAppl*>& applMap,
                               std::map<uint16_t, const MinFareDefaultLogic*>& defaultLogicMap)
  : MinimumFare(farePath.itin()->travelDate()),
    _trx(trx),
    _farePath(farePath),
    _ruleLevelMap(ruleLevelMap),
    _applMap(applMap),
    _defaultLogicMap(defaultLogicMap)
{

  // class was prepared for historical dates database retrieval - member _travelDate was added
  //_travelDate should be adjusted to achieve that
  // now it is set to: farePath.itin()->travelDate() like in regular pricing
}

bool
COMMinimumFare::qualifyComCheck(std::vector<ComProcInfo>& comProcInfoList)
{
  // Check PU travel via previous pu's origin country and return country
  std::vector<TravelSeg*> puOriginTvlSeg;
  if (_farePath.pricingUnit().size() > 1)
  {
    puOriginTvlSeg.push_back(_farePath.pricingUnit().front()->travelSeg().front());

    for (std::vector<PricingUnit*>::iterator puI = _farePath.pricingUnit().begin() + 1,
                                             puEnd = _farePath.pricingUnit().end();
         (_farePath.pricingUnit().size() > 1) && (puI != puEnd);
         ++puI)
    {
      std::vector<TravelSeg*>& puTvlSeg = (*puI)->travelSeg();

      for (std::vector<TravelSeg*>::iterator tvlI = puOriginTvlSeg.begin(),
                                             tvlEnd = puOriginTvlSeg.end();
           tvlI != tvlEnd;
           ++tvlI)
      {
        std::vector<TravelSeg*>::const_iterator comTvlI;
        // To check if we are travelling via one of the origin country in previous pu
        comTvlI = std::find_if(puTvlSeg.begin() + 1,
                               puTvlSeg.end(),
                               bind2nd(ComUtil::OriginNationEqual(), (*tvlI)->origin()));

        if (comTvlI != puTvlSeg.end())
        {
          // To check if we are returning to one of the origin country in previous pu.
          // i.e., if the destination nation of this pu is the same as one of the
          // previous pu's originating nation - COM does not apply in such case.
          std::vector<TravelSeg*>::const_iterator retTvlI;
          retTvlI =
              std::find_if(puOriginTvlSeg.begin(),
                           puOriginTvlSeg.end(),
                           bind2nd(ComUtil::OriginNationEqual(), puTvlSeg.back()->destination()));

          if (retTvlI != puOriginTvlSeg.end())
          {
            // Returning to one of the origin country of previous pu - this pu doesn't
            // qualify for COM check.
            continue;
          }
          else
          {
            // This PU qualifies COM Check
            comProcInfoList.push_back(ComProcInfo(*puI, *comTvlI));
          }
        }
      }

      puOriginTvlSeg.push_back((*puI)->travelSeg().front());
    }
  }

  return (comProcInfoList.size() > 0);
}

MoneyAmount
COMMinimumFare::process()
{
  DiagMonitor diagMonitor(_trx, Diagnostic760);
  DiagCollector& diag = diagMonitor.diag();

  std::vector<ComProcInfo> comProcInfoList;
  if (!qualifyComCheck(comProcInfoList))
  {
    diag << " \n"
         << "COUNTRY OF ORIGIN MINIMUM CHECK\n"
         << "-- COM DOES NOT APPLY\n";
    return 0;
  }

  MoneyAmount plusUp = 0;

  diag << " \n"
       << "COUNTRY OF ORIGIN MINIMUM CHECK\n"
       << "CITY      GOV  CLASS                         DIR FARE  GLOG EXCL\n"
       << "PAIR      CXR           CUR   AMOUNT RTG TAG I/O TYPE  IND  IND \n";

  std::map<std::set<int>, FarePath::OscPlusUp*> oscPlusUpMap;
  ComUtil::getOscPlusUp(_farePath, oscPlusUpMap);

  for (std::vector<PricingUnit*>::iterator puI = _farePath.pricingUnit().begin(),
                                           puEnd = _farePath.pricingUnit().end();
       puI != puEnd;
       ++puI)
  {
    if ((*puI)->exemptMinFare())
      continue;

    const std::set<int> puSegOrders = ComUtil::getPuSegOrders(*_farePath.itin(), **puI);

    bool processMultiPu = false;

    for (std::map<std::set<int>, FarePath::OscPlusUp*>::iterator i = oscPlusUpMap.begin(),
                                                                 iend = oscPlusUpMap.end();
         i != iend;
         ++i)
    {
      const std::set<int> key = (*i).first;
      FarePath::OscPlusUp const* const oscPlusUp = (*i).second;
      if (ComUtil::includes(key, puSegOrders))
      {
        processMultiPu = true;

        if (*key.begin() == *puSegOrders.begin())
        {
          const std::vector<TravelSeg*>& tvlSeg = oscPlusUp->thruFare->fareMarket()->travelSeg();

          std::vector<TravelSeg*>::const_iterator comTvlSegIter;
          const Loc* originNation = _farePath.itin()->travelSeg().front()->origin();
          comTvlSegIter = std::find_if(tvlSeg.begin() + 1,
                                       tvlSeg.end(),
                                       bind2nd(ComUtil::OriginNationEqual(), originNation));

          if ((tvlSeg.size() > 0) && (comTvlSegIter != tvlSeg.end()))
          {
            // Display the PU's thru fares:
            for (std::vector<FareUsage*>::const_iterator fuI = (*puI)->fareUsage().begin(),
                                                         fuEnd = (*puI)->fareUsage().end();
                 fuI != fuEnd;
                 ++fuI)
            {
              printFareInfo(*(*fuI)->paxTypeFare(), diag, COM, "* ");
            }

            MinFarePlusUpItem* comPlusUp = nullptr;
            processIntermediateCityPair(**puI, *oscPlusUp, comTvlSegIter, comPlusUp, true, diag);

            printPlusUpInfo(*oscPlusUp->thruFare, **puI, comPlusUp, diag);
            if (comPlusUp && comPlusUp->plusUpAmount > 0)
            {
              plusUp += comPlusUp->plusUpAmount;
            }
          }
        }
      }
    }

    if (!processMultiPu)
    {
      // Display the PU's thru fares:
      for (std::vector<FareUsage*>::const_iterator fuI = (*puI)->fareUsage().begin(),
                                                   fuEnd = (*puI)->fareUsage().end();
           fuI != fuEnd;
           ++fuI)
      {
        printFareInfo(*(*fuI)->paxTypeFare(), diag, COM, "* ");
      }

      // Check if this is one of the pu we need to do COM check
      const std::vector<ComProcInfo>::iterator comProcInfoIter = std::find_if(
          comProcInfoList.begin(), comProcInfoList.end(), bind2nd(ComProcInfo::PuEqual(), *puI));
      if (comProcInfoIter == comProcInfoList.end())
      {
        continue;
      }

      plusUp += processPricingUnit(*comProcInfoIter, diag);
    }
  }

  return plusUp;
}

MoneyAmount
COMMinimumFare::processPricingUnit(ComProcInfo& comProcInfo, DiagCollector& diag)
{
  MoneyAmount plusUp = 0;

  PricingUnit& pu = *comProcInfo.pu;
  for (std::vector<FareUsage*>::iterator fuI = pu.fareUsage().begin(), fuEnd = pu.fareUsage().end();
       fuI != fuEnd;
       ++fuI)
  {
    MinFarePlusUpItem* comPlusUp = nullptr;

    processIntermediateCityPair(comProcInfo, **fuI, comPlusUp, false, diag);

    printPlusUpInfo(**fuI, comPlusUp, diag);
    if (comPlusUp && comPlusUp->plusUpAmount > 0)
    {
      plusUp += comPlusUp->plusUpAmount;
    }
  }

  return plusUp;
}

void
COMMinimumFare::processIntermediateCityPair(const PricingUnit& pu,
                                            const FarePath::OscPlusUp& oscPlusUp,
                                            std::vector<TravelSeg*>::const_iterator& comPosItr,
                                            MinFarePlusUpItem*& comPlusUp,
                                            bool isCrossPu,
                                            DiagCollector& diag)
{
  FareUsage tmpFu;
  tmpFu.paxTypeFare() = const_cast<PaxTypeFare*>(oscPlusUp.thruFare);

  return processIntermediateCityPair(pu, tmpFu, comPosItr, comPlusUp, isCrossPu, diag);
}

void
COMMinimumFare::processIntermediateCityPair(ComProcInfo& comProcInfo,
                                            FareUsage& fareUsage,
                                            MinFarePlusUpItem*& comPlusUp,
                                            bool isCrossPu,
                                            DiagCollector& diag)
{
  const PaxTypeFare* ptf = fareUsage.paxTypeFare();
  if (!ptf)
    return;

  const Itin& aItin = *_farePath.itin();

  const FareMarket& fareMarket = *(ptf->fareMarket());
  const int16_t startSeg = aItin.segmentOrder(fareMarket.travelSeg().front());
  const int16_t endSeg = aItin.segmentOrder(fareMarket.travelSeg().back());

  // Check if com has already been processed
  std::vector<FarePath::PlusUpInfo*>& plusUpList = _farePath.plusUpInfoList();
  std::vector<FarePath::PlusUpInfo*>::iterator iter;
  if ((iter = std::find_if(plusUpList.begin(),
                           plusUpList.end(),
                           FarePath::PlusUpInfo::KeyEquals(COM, startSeg, endSeg))) !=
      plusUpList.end())
  {
    comPlusUp = (*iter)->minFarePlusUp();
    return;
  }

  // Perform Rule Level Excl, Application and Default Logic table matching:
  if (!checkMinFareTables(*ptf, diag))
    return;

  const std::vector<TravelSeg*>& tvlSeg = fareMarket.travelSeg();

  std::vector<TravelSeg*>::const_iterator comTvlSegIter;
  comTvlSegIter = std::find_if(
      tvlSeg.begin() + 1,
      tvlSeg.end(),
      bind2nd(TravelSegUtil::OriginNationEqual(), comProcInfo.comTvlSeg->origin()->nation()));

  if (comTvlSegIter == fareMarket.travelSeg().end())
    return;

  std::vector<TravelSeg*>::const_iterator aIter;
  std::vector<TravelSeg*>::const_iterator zIter;
  MinFareFareSelection::FareDirectionChoice fareDirection = MinFareFareSelection::OUTBOUND;

  std::vector<TravelSeg*>::const_iterator tvlIter = tvlSeg.end();
  do
  {
    --tvlIter;
    if (fareDirection == MinFareFareSelection::OUTBOUND)
    {
      aIter = comTvlSegIter;
      zIter = tvlIter;
    }
    else
    {
      aIter = tvlIter;
      zIter = comTvlSegIter;
    }

    processCityPair(*comProcInfo.pu, fareUsage, aIter, zIter, fareDirection, comPlusUp, diag);

    if (tvlIter == comTvlSegIter)
    {
      fareDirection = MinFareFareSelection::INBOUND;
    }
  } while (tvlIter != tvlSeg.begin());

  if (comPlusUp)
  {
    FarePath::PlusUpInfo* plusUpInfo = nullptr;
    _trx.dataHandle().get(plusUpInfo);

    if (plusUpInfo)
    {
      plusUpInfo->module() = COM;
      plusUpInfo->startSeg() = startSeg;
      plusUpInfo->endSeg() = endSeg;
      plusUpInfo->minFarePlusUp() = comPlusUp;
      _farePath.plusUpInfoList().push_back(plusUpInfo);
    }
    else
    {
      LOG4CXX_ERROR(logger, "DataHandle failed to allocate memory (plusUpInfo)");
    }
  }
}

void
COMMinimumFare::processIntermediateCityPair(const PricingUnit& pu,
                                            FareUsage& fareUsage,
                                            std::vector<TravelSeg*>::const_iterator& comPosItr,
                                            MinFarePlusUpItem*& comPlusUp,
                                            bool isCrossPu,
                                            DiagCollector& diag)
{
  const PaxTypeFare* ptf = fareUsage.paxTypeFare();
  if (!ptf)
    return;

  const Itin& aItin = *_farePath.itin();

  const FareMarket& fareMarket = *(ptf->fareMarket());
  const int startSeg = aItin.segmentOrder(fareMarket.travelSeg().front());
  const int endSeg = aItin.segmentOrder(fareMarket.travelSeg().back());

  // Check if com has already been processed
  std::vector<FarePath::PlusUpInfo*>& plusUpList = _farePath.plusUpInfoList();
  std::vector<FarePath::PlusUpInfo*>::iterator iter;
  if ((iter = std::find_if(plusUpList.begin(),
                           plusUpList.end(),
                           FarePath::PlusUpInfo::KeyEquals(COM, startSeg, endSeg))) !=
      plusUpList.end())
  {
    comPlusUp = (*iter)->minFarePlusUp();
    return;
  }

  // Perform Rule Level Excl, Application and Default Logic table matching:
  if (!checkMinFareTables(*ptf, diag))
    return;

  const std::vector<TravelSeg*>& tvlSeg = fareMarket.travelSeg();
  std::vector<TravelSeg*>::const_iterator aIter;
  std::vector<TravelSeg*>::const_iterator zIter;
  MinFareFareSelection::FareDirectionChoice fareDirection;

  std::vector<TravelSeg*>::const_iterator tvlIter = tvlSeg.end();
  fareDirection = MinFareFareSelection::OUTBOUND;
  do
  {
    --tvlIter;

    if (fareDirection == MinFareFareSelection::OUTBOUND)
    {
      aIter = comPosItr;
      zIter = tvlIter;
    }
    else
    {
      aIter = tvlIter;
      zIter = comPosItr;
    }

    processCityPair(pu, fareUsage, aIter, zIter, fareDirection, comPlusUp, diag);

    if (aItin.segmentOrder(*tvlIter) == aItin.segmentOrder(*comPosItr))
    {
      fareDirection = MinFareFareSelection::INBOUND;
    }
  } while (tvlIter != tvlSeg.begin());

  if (comPlusUp)
  {
    FarePath::PlusUpInfo* plusUpInfo = nullptr;

    // lint --e{413}
    _trx.dataHandle().get(plusUpInfo);

    if (plusUpInfo)
    {
      plusUpInfo->module() = COM;
      plusUpInfo->startSeg() = startSeg;
      plusUpInfo->endSeg() = endSeg;
      plusUpInfo->minFarePlusUp() = comPlusUp;
      _farePath.plusUpInfoList().push_back(plusUpInfo);
    }
    else
    {
      LOG4CXX_ERROR(logger, "DataHandle failed to allocate memory (plusUpInfo)");
    }
  }
}

void
COMMinimumFare::processCityPair(const PricingUnit& pu,
                                FareUsage& fareUsage,
                                std::vector<TravelSeg*>::const_iterator& aIter,
                                std::vector<TravelSeg*>::const_iterator& zIter,
                                MinFareFareSelection::FareDirectionChoice fareDirection,
                                MinFarePlusUpItem*& comPlusUp,
                                DiagCollector& diag)
{
  const PaxTypeFare& ptf = *fareUsage.paxTypeFare();
  const FareMarket& fareMarket = *(ptf.fareMarket());
  const std::vector<TravelSeg*>& tvlSeg = fareMarket.travelSeg();

  if (_matchedApplItem != nullptr)
  {
    std::vector<TravelSeg*> tmpTvlSeg(aIter, zIter + 1);
    if (MinFareLogic::checkDomesticExclusion(
            _trx, *_farePath.itin(), ptf, _matchedApplItem, _matchedDefaultItem, tmpTvlSeg))
    {
      printExceptionInfo(diag, tmpTvlSeg, fareDirection, "EXEMPT BY DOMESTIC\n");
    }

    if (checkIntermediateExclusion(_trx,
                                   COM,
                                   *_matchedApplItem,
                                   _matchedDefaultItem,
                                   ptf.isNormal(),
                                   tvlSeg,
                                   *(_farePath.itin()),
                                   &pu,
                                   &fareUsage,
                                   fareDirection,
                                   aIter,
                                   zIter))
    {
      printExceptionInfo(diag, tmpTvlSeg, fareDirection);
      diag << "EXEMPT BY INTERM. LOC. EXCL. APPL. TABLE " << (*_matchedApplItem).seqNo() << '\n';
      return;
    }
  }

  std::vector<TravelSeg*> travelSegVec;
  if (fareDirection == MinFareFareSelection::OUTBOUND)
    travelSegVec.insert(travelSegVec.begin(), aIter, zIter + 1);
  else
    travelSegVec.insert(travelSegVec.begin(), aIter, zIter);

  PtfPair ptfPair = MinFareLogic::selectQualifyConstFare(COM,
                                                         _trx,
                                                         *_farePath.itin(),
                                                         ptf,
                                                         *_farePath.paxType(),
                                                         ptf.cabin(),
                                                         ptf.isNormal(),
                                                         fareDirection,
                                                         MinFareFareSelection::ONE_WAY,
                                                         travelSegVec,
                                                         _travelDate,
                                                         _matchedApplItem,
                                                         _matchedDefaultItem,
                                                         nullptr,
                                                         &_farePath,
                                                         &pu);

  if (ptfPair.first && ptfPair.second)
  {
    // Save constructed fare info:
    printFareInfo(ptfPair, fareDirection, (diag << "  "), COM);
    MoneyAmount unPubAmount = ptfPair.first->nucFareAmount() + ptfPair.second->nucFareAmount();
    calculatePlusUp(0.0 /* baseFare */,
                    ptf,
                    unPubAmount,
                    ptfPair.first->fareMarket()->offMultiCity(),
                    (*aIter)->boardMultiCity(),
                    (*zIter)->offMultiCity(),
                    pu,
                    comPlusUp);
  }
  else if (ptfPair.first)
  {
    if (ptfPair.first->isForeignDomestic())
    {
      printExceptionInfo(diag, travelSegVec, fareDirection, "EXEMPT BY DOMESTIC\n");
      return;
    }
    printFareInfo(*ptfPair.first, (diag << "  "), COM);
    calculatePlusUp(0.0 /* baseFare */, ptf, *ptfPair.first, pu, comPlusUp);
  }
  else
  {
    printExceptionInfo(diag, travelSegVec, fareDirection, "NO FARE FOUND\n");
  }
}

/**
 * Perform check on Min Fare tables:
 *    RuleLevelExclusion,
 *    Application,
 *    Default Logic
 *
 * return true - continue process processing
 *        false (or exception) return from current fare component processing
 */
bool
COMMinimumFare::checkMinFareTables(const PaxTypeFare& ptf, DiagCollector& diag)
{
  const Itin& aItin = *_farePath.itin();
  //--- Check Rule Level Exclusion Table
  _matchedRuleItem =
      MinFareLogic::getRuleLevelExcl(_trx, aItin, ptf, COM, _ruleLevelMap, _travelDate);

  if (_matchedRuleItem)
  {
    if (_matchedRuleItem->comMinFareAppl() == YES)
    {
      printExceptionInfo(ptf, diag);
      diag << "EXEMPT BY RULE LEVEL EXCL. TABLE - " << _matchedRuleItem->seqNo() << '\n';

      return false;
    }
  }

  //--- Check Application Table
  _matchedDefaultItem = nullptr;

  _matchedApplItem = MinFareLogic::getApplication(
      _trx, _farePath, aItin, ptf, COM, _applMap, _defaultLogicMap, _travelDate);

  if (_matchedApplItem == nullptr)
  {
    printExceptionInfo(ptf, diag);
    diag << "FAILED MATCHING IN APPLICATION TABLE - RETURN" << '\n';
    // throw ErrorResponseException(ErrorResponseException::MIN_FARE_MISSING_DATA);
    return false;
  }

  if (_matchedApplItem->applyDefaultLogic() == YES)
  {
    _matchedDefaultItem = MinFareLogic::getDefaultLogic(COM, _trx, aItin, ptf, _defaultLogicMap);
    if (_matchedDefaultItem == nullptr)
    {
      printExceptionInfo(ptf, diag);
      diag << "FAILED MATCHING DEFAULT LOGIC TABLE - RETURN" << '\n';
      // throw ErrorResponseException(ErrorResponseException::MIN_FARE_MISSING_DATA);
      return false;
    }
  }

  return true;
}

MoneyAmount
COMMinimumFare::accumulateBaseFare(const FareUsage& fareUsage)
{
  const PaxTypeFare& ptf = *fareUsage.paxTypeFare();

  MoneyAmount baseFare =
      ptf.nucFareAmount() + ptf.mileageSurchargeAmt() + fareUsage.minFarePlusUp().getSum(HIP);

  return baseFare;
}

MoneyAmount
COMMinimumFare::accumulateBaseFare(const PaxTypeFare& ptf, const PricingUnit& prcUnit)
{
  FareUsage tmpFareUsage;
  tmpFareUsage.paxTypeFare() = const_cast<PaxTypeFare*>(&ptf);

  HIPMinimumFare hip(_trx);

  MoneyAmount baseFare =
      ptf.nucFareAmount() + ptf.mileageSurchargeAmt() +
      hip.process(tmpFareUsage, prcUnit, _farePath, _ruleLevelMap, _applMap, _defaultLogicMap);

  return baseFare;
}

MoneyAmount
COMMinimumFare::calculatePlusUp(const MoneyAmount& abaseFare,
                                const PaxTypeFare& ptf,
                                const PaxTypeFare& coPaxTypeFare,
                                const PricingUnit& prcUnit,
                                MinFarePlusUpItem*& comPlusUp)
{
  MoneyAmount baseFare = abaseFare;
  if (baseFare == 0.0)
  {
    baseFare = accumulateBaseFare(ptf, prcUnit);
  }

  MoneyAmount coFareAmt = coPaxTypeFare.nucFareAmount();
  if (!coPaxTypeFare.isRouting() && ptf.mileageSurchargePctg() > 0)
  {
    coFareAmt += (coFareAmt * ptf.mileageSurchargePctg() / 100.0);
  }

  MoneyAmount plusUp = coFareAmt - baseFare;

  if (plusUp > 0)
  {
    if (!comPlusUp)
    {
      _trx.dataHandle().get(comPlusUp);
    }
    if (comPlusUp && plusUp > comPlusUp->plusUpAmount)
    {
      comPlusUp->plusUpAmount = plusUp;
      comPlusUp->baseAmount = baseFare;
      if (coPaxTypeFare.directionality() != TO)
      {
        comPlusUp->boardPoint = coPaxTypeFare.fareMarket()->boardMultiCity();
        comPlusUp->offPoint = coPaxTypeFare.fareMarket()->offMultiCity();
      }
      else
      {
        comPlusUp->boardPoint = coPaxTypeFare.fareMarket()->offMultiCity();
        comPlusUp->offPoint = coPaxTypeFare.fareMarket()->boardMultiCity();
      }
    }

    return plusUp;
  }

  return 0;
}

MoneyAmount
COMMinimumFare::calculatePlusUp(const MoneyAmount& abaseFare,
                                const PaxTypeFare& ptf,
                                const MoneyAmount& unPubAmount,
                                const LocCode& constructPoint,
                                const LocCode& boardPoint,
                                const LocCode& offPoint,
                                const PricingUnit& prcUnit,
                                MinFarePlusUpItem*& comPlusUp)
{
  MoneyAmount baseFare = abaseFare;
  if (baseFare == 0.0)
  {
    baseFare = accumulateBaseFare(ptf, prcUnit);
  }

  MoneyAmount coFareAmt = unPubAmount;
  if (!ptf.isRouting() && ptf.mileageSurchargePctg() > 0)
  {
    coFareAmt += (unPubAmount * ptf.mileageSurchargePctg() / 100.0);
  }

  MoneyAmount plusUp = coFareAmt - baseFare;

  if (plusUp > 0)
  {
    if (!comPlusUp)
    {
      _trx.dataHandle().get(comPlusUp);
    }

    if (comPlusUp && plusUp > comPlusUp->plusUpAmount)
    {
      comPlusUp->plusUpAmount = plusUp;
      comPlusUp->baseAmount = baseFare;
      comPlusUp->constructPoint = constructPoint;
      if (ptf.directionality() != TO)
      {
        comPlusUp->boardPoint = boardPoint;
        comPlusUp->offPoint = offPoint;
      }
      else
      {
        comPlusUp->boardPoint = offPoint;
        comPlusUp->offPoint = boardPoint;
      }
    }

    return plusUp;
  }

  return 0;
}

void
COMMinimumFare::printPlusUpInfo(const FareUsage& fareUsage,
                                const MinFarePlusUpItem* comPlusUp,
                                DiagCollector& diag)
{
  MoneyAmount baseFare = accumulateBaseFare(fareUsage);
  printPlusUpInfo(baseFare, comPlusUp, diag);
}

void
COMMinimumFare::printPlusUpInfo(const PaxTypeFare& ptf,
                                const PricingUnit& prcUnit,
                                const MinFarePlusUpItem* comPlusUp,
                                DiagCollector& diag)
{
  MoneyAmount baseFare = accumulateBaseFare(ptf, prcUnit);
  printPlusUpInfo(baseFare, comPlusUp, diag);
}

void
COMMinimumFare::printPlusUpInfo(const MoneyAmount& baseFare,
                                const MinFarePlusUpItem* comPlusUp,
                                DiagCollector& diag)
{
  diag << "  BASE FARE NUC ";
  diag.setf(std::ios::right, std::ios::adjustfield);
  diag.setf(std::ios::fixed, std::ios::floatfield);
  diag.precision(2);
  if (comPlusUp)
    diag << comPlusUp->baseAmount << " ";
  else
    diag << baseFare << " ";

  if (comPlusUp && comPlusUp->plusUpAmount > 0)
  {
    diag << comPlusUp->boardPoint << comPlusUp->offPoint << "  PLUS UP ";
    diag.setf(std::ios::right, std::ios::adjustfield);
    diag.setf(std::ios::fixed, std::ios::floatfield);
    diag.precision(2);
    diag << std::setw(8);
    diag << comPlusUp->plusUpAmount << "\n";
  }
  else
  {
    diag << " NO PLUS UP\n \n";
  }
}
} // namespace tse
