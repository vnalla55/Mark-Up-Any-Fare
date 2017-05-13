//-------------------------------------------------------------------
//
//  Copyright Sabre 2009
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

#include "Shopping/RexConstrainsConsolidator.h"

#include "Common/FallbackUtil.h"
#include "Common/LocUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareCompInfo.h"
#include "Diagnostic/DCFactory.h"

#include <boost/range/adaptor/indexed.hpp>
#include <boost/range/adaptor/map.hpp>

namespace tse
{
FALLBACK_DECL(exscSetEmptyDateRangeAsWholePeriod)

RexConstrainsConsolidator::RexConstrainsConsolidator(RexShoppingTrx& trx)
  : _dc(nullptr),
    _trx(trx),
    _ondInfWithSplittedFMUnflown(nullptr),
    _ondInfWithSplittedFMFlown(nullptr),
    _addCxrRestToUnmappedONDInfos(true)
{
  if (_trx.diagnostic().diagnosticType() == Diagnostic987)
  {
    DCFactory* factory = DCFactory::instance();
    _dc = factory->create(_trx);
    if (_dc != nullptr)
    {
      _dc->enable(Diagnostic987);
      if (!_dc->isActive())
      {
        _dc = nullptr;
      }
    }
  }
}

RexConstrainsConsolidator::~RexConstrainsConsolidator()
{
  if (_dc != nullptr)
  {
    _dc->flushMsg();
    _dc = nullptr;
  }
}

void
RexConstrainsConsolidator::process()
{
  if (_dc)
  {
    *_dc << "***************** START DIAG 987 **************************\n";
    *_dc << "********** EXCHANGE CONSTRAINS CONSOLIDATOR ***************\n";
    *_dc << "***********************************************************\n";

    *_dc << "EXCHANGE ITIN:\n";
    printItin(*_trx.exchangeItin().front());
    *_dc << "NEW ITIN:\n";
    printItin(*_trx.newItin().front());
  }

  if (!(_trx.originDest().empty()))
  {
    copySegWithoutARUNKs();
    calculatePlusMinusDateShift();
    detachFlownToNewLeg();
    matchONDInfotoExcOND();
    consolideConstrainsForONDInfo();
    createSODForONDInfo();
  }

  if (_dc)
  {
    printConsolidatedConstrains();
    printConsolidatedConstrainsWithSOD();
    *_dc << "\n****************** END DIAG 987 ***************************\n";
  }
}

void
RexConstrainsConsolidator::detachFlownToNewLeg()
{
  std::vector<TravelSeg*>& newTravelSeg = _newItinSeg;
  std::vector<TravelSeg*>::iterator frontSeg;
  std::vector<TravelSeg*>::iterator backSeg;
  if (_dc && _trx.originDest().empty())
  {
    *_dc << "\nOND IS EMPTY\n";
  }

  std::vector<PricingTrx::OriginDestination>::const_iterator fmIter = _trx.originDest().begin();
  for (; fmIter != _trx.originDest().end(); ++fmIter)
  {
    getSegForOND(*fmIter, newTravelSeg, frontSeg, backSeg);
    if (frontSeg != newTravelSeg.end() && backSeg != newTravelSeg.end())
    {
      bool statusFlown = (*frontSeg)->unflown();
      bool splittedFM = false;
      std::vector<TravelSeg*>::iterator newSegIter = frontSeg + 1;
      for (; newSegIter != backSeg + 1 && newSegIter != newTravelSeg.end(); ++newSegIter)
      {
        if (statusFlown != (*newSegIter)->unflown())
        {
          splittedFM = true;
          addONDInfo(frontSeg, newSegIter - 1, *fmIter, splittedFM);
          frontSeg = newSegIter;
          break;
        }
      }
      addONDInfo(frontSeg, backSeg, *fmIter, splittedFM);
    }
  }
}

void
RexConstrainsConsolidator::addONDInfo(const std::vector<TravelSeg*>::iterator& frontSeg,
                                      const std::vector<TravelSeg*>::iterator& backSeg,
                                      const PricingTrx::OriginDestination& odThruFM,
                                      bool splittedFM)
{
  if (_dc)
  {
    *_dc << "\n            ADD OND: ";
    if (isMutliAirportCityTheSame((*frontSeg)->boardMultiCity(), odThruFM.boardMultiCity))
      *_dc << odThruFM.boardMultiCity;
    else
      *_dc << (*frontSeg)->origAirport();
    if (isMutliAirportCityTheSame((*backSeg)->offMultiCity(), odThruFM.offMultiCity))
      *_dc << "-" << odThruFM.offMultiCity;
    else
      *_dc << "-" << (*backSeg)->destAirport();
    if ((isMutliAirportCityTheSame((*frontSeg)->boardMultiCity(), odThruFM.boardMultiCity)) ||
        ((*frontSeg)->origAirport() == odThruFM.boardMultiCity))
      *_dc << " " << odThruFM.travelDate;
    else
      *_dc << " " << (*frontSeg)->departureDT();
    *_dc << " - " << ((*frontSeg)->unflown() ? "UNFLOWN" : "FLOWN");
  }

  ONDInf* ondInf;
  _trx.dataHandle().get(ondInf);

  if (isMutliAirportCityTheSame((*frontSeg)->boardMultiCity(), odThruFM.boardMultiCity))
  {
    ondInf->origAirport = odThruFM.boardMultiCity;
  }
  else
  {
    ondInf->origAirport = (*frontSeg)->origAirport();
  }

  if (isMutliAirportCityTheSame((*backSeg)->offMultiCity(), odThruFM.offMultiCity))
  {
    ondInf->destAirport = odThruFM.offMultiCity;
  }
  else
  {
    ondInf->destAirport = (*backSeg)->destAirport();
  }

  if ((isMutliAirportCityTheSame((*frontSeg)->boardMultiCity(), odThruFM.boardMultiCity)) ||
      ((*frontSeg)->origAirport() == odThruFM.boardMultiCity))
  {
    ondInf->travelDate = odThruFM.travelDate.date();
  }
  else
  {
    ondInf->travelDate = (*frontSeg)->departureDT().date();
  }

  ondInf->unflown = (*frontSeg)->unflown();
  ondInf->firstONDInfoNewSegPos = segmentOrder(_newItinSeg, *frontSeg);
  ondInf->lastONDInfoNewSegPos = segmentOrder(_newItinSeg, *backSeg);

  if (splittedFM)
  {
    if (ondInf->unflown)
      _ondInfWithSplittedFMUnflown = ondInf;
    else
      _ondInfWithSplittedFMFlown = ondInf;
  }

  markUnshoppedFlights(frontSeg, backSeg, ondInf);
  setExcSegPosToNewSegPos(frontSeg, backSeg, ondInf);

  _ondInfo.push_back(ondInf);
}

void
RexConstrainsConsolidator::getSegForOND(const PricingTrx::OriginDestination& odThruFM,
                                        std::vector<TravelSeg*>& newTravelSeg,
                                        std::vector<TravelSeg*>::iterator& frontSeg,
                                        std::vector<TravelSeg*>::iterator& backSeg)
{
  if (_dc)
  {
    *_dc << "\nLOOKING FOR NEW OND: " << odThruFM.boardMultiCity << "-" << odThruFM.offMultiCity
         << " " << odThruFM.travelDate;
  }
  frontSeg = newTravelSeg.end();
  backSeg = newTravelSeg.end();
  std::vector<TravelSeg*>::iterator ondOrigAirport;
  std::vector<TravelSeg*>::iterator ondDestAirport;
  for (ondDestAirport = newTravelSeg.begin(); ondDestAirport != newTravelSeg.end();
       ++ondDestAirport)
  {
    if ((*ondDestAirport)->destAirport() != odThruFM.offMultiCity &&
        !isMutliAirportCityTheSame((*ondDestAirport)->offMultiCity(), odThruFM.offMultiCity))
      continue;
    for (ondOrigAirport = newTravelSeg.begin(); ondOrigAirport != newTravelSeg.end();
         ++ondOrigAirport)
    {
      if ((*ondOrigAirport)->origAirport() != odThruFM.boardMultiCity &&
          !isMutliAirportCityTheSame((*ondOrigAirport)->boardMultiCity(), odThruFM.boardMultiCity))
        continue;
      if (ondDestAirport - ondOrigAirport >= 0)
      {
        if (isTravelDateTheSame(*ondOrigAirport, odThruFM))
        {
          frontSeg = ondOrigAirport;
          backSeg = ondDestAirport;
          if (_dc)
          {
            *_dc << "\n      FOUND NEW OND: " << (*frontSeg)->origAirport() << "-"
                 << (*backSeg)->destAirport() << " " << (*frontSeg)->departureDT();
          }
          return;
        }
      }
    }
  }
  if (_dc)
  {
    *_dc << "\nNEW OND NOT FOUND";
  }
}

void
RexConstrainsConsolidator::markUnshoppedFlights(const std::vector<TravelSeg*>::iterator& frontSeg,
                                                const std::vector<TravelSeg*>::iterator& backSeg,
                                                ONDInf* ondInf)
{
  std::vector<TravelSeg*>::iterator segIter = frontSeg;
  for (; segIter != backSeg + 1; ++segIter)
  {
    if (!((*segIter)->isShopped()))
    {
      ondInf->unshoppedFlights.push_back(segmentOrder(_newItinSeg, *segIter));
    }
    if (_dc)
    {
      *_dc << " " << segmentOrder(_newItinSeg, *segIter);
      if ((*segIter)->isShopped())
      {
        *_dc << "S";
      }
      else
      {
        *_dc << "U";
      }
    }
  }
}

void
RexConstrainsConsolidator::matchONDInfotoExcOND()
{
  ExcSegPosToFMMap excSegPosToFM;
  createExcSegPosToFMMap(excSegPosToFM);

  matchONDInfoToExcSegPos();

  matchFMsToONDInfo(excSegPosToFM);

  calculateUnmappedFC(excSegPosToFM);

  if (_dc && _trx.diagnostic().diagParamMapItem("DD") == "DETAILS")
  {
    printONDInfoToFM(excSegPosToFM);
    printONDInfoNotMatched();
    printFMNotMatched();
    *_dc << (_addCxrRestToUnmappedONDInfos ? "" : "\n\nDONT ADD CXR REST FOR UNMAPPED OAD");
  }
}

void
RexConstrainsConsolidator::createExcSegPosToFMMap(ExcSegPosToFMMap& excSegPosToFM)
{
  for (const FareCompInfo* fci : _trx.exchangeItin().front()->fareComponent())
  {
    for (const TravelSeg* tvlSeg : fci->fareMarket()->travelSeg())
    {
      int segmentPos = segmentOrder(_excItinSeg, tvlSeg);
      if (segmentPos)
        excSegPosToFM[segmentPos] = fci->fareMarket();
    }
  }
}

void
RexConstrainsConsolidator::matchONDInfoToExcSegPos()
{
  std::vector<TravelSeg*>::const_iterator tvlSegIter = _excItinSeg.begin();

  std::vector<ONDInf*>::const_iterator ondInfoIter = _ondInfo.begin();
  for (; ondInfoIter != _ondInfo.end(); ++ondInfoIter)
  {
    bool found = false;
    std::vector<TravelSeg*>::const_iterator ondOrigAirport;
    std::vector<TravelSeg*>::const_iterator ondDestAirport;

    for (ondOrigAirport = tvlSegIter; ondOrigAirport < _excItinSeg.end();
         ++ondOrigAirport, ++tvlSegIter)
    {
      if ((*ondOrigAirport)->origAirport() != (*ondInfoIter)->origAirport &&
          !isMutliAirportCityTheSame((*ondOrigAirport)->boardMultiCity(),
                                     (*ondInfoIter)->origAirport))
        continue;

      for (ondDestAirport = tvlSegIter; ondDestAirport < _excItinSeg.end();
           ++ondDestAirport, ++tvlSegIter)
      {
        if ((*ondDestAirport)->destAirport() == (*ondInfoIter)->destAirport ||
            isMutliAirportCityTheSame((*ondDestAirport)->offMultiCity(),
                                      (*ondInfoIter)->destAirport))
        {
          found = true;
          int segmentPos = -1;
          std::vector<TravelSeg*>::const_iterator segIter = ondOrigAirport;
          for (; segIter != ondDestAirport + 1; ++segIter, segmentPos = -1)
          {
            segmentPos = segmentOrder(_excItinSeg, *segIter);
            _ondInfoToExcSegPosSet[*ondInfoIter].push_back(segmentPos);
            _ondInfoToExcSegPosWithoutNoMatchedSet[*ondInfoIter].push_back(segmentPos);
          }
          break;
        }
      }
      if (found)
        break;
    }

    if (!found)
    {
      _ondInfoToExcSegPosSet[*ondInfoIter];
    }
  }
}

void
RexConstrainsConsolidator::matchFMsToONDInfo(ExcSegPosToFMMap& excSegPosToFM)
{
  ONDInfoToExcSegPosSetMap::const_iterator ondInfoToExcSegPosSetIter =
      _ondInfoToExcSegPosSet.begin();
  for (; ondInfoToExcSegPosSetIter != _ondInfoToExcSegPosSet.end(); ++ondInfoToExcSegPosSetIter)
  {
    std::vector<int>::const_iterator sopPosIter = ondInfoToExcSegPosSetIter->second.begin();
    const ONDInf* ondInfo = ondInfoToExcSegPosSetIter->first;

    for (; sopPosIter != ondInfoToExcSegPosSetIter->second.end(); ++sopPosIter)
    {
      if (excSegPosToFM.count(*sopPosIter) != 0)
      {
        _ondInfoToFM[ondInfo].push_back(excSegPosToFM[*sopPosIter]);
      }
    }
  }

  removeDuplicates(_ondInfoToFM);
}

void
RexConstrainsConsolidator::printONDInfoToFM(const ExcSegPosToFMMap& excSegPosToFM)
{
  const std::vector<TravelSeg*>& excTvlSegV = _excItinSeg;
  if (excSegPosToFM.empty())
  {
    *_dc << "\n\nEXC SEG POS TO FM IS EMPTY";
  }
  else
  {
    *_dc << "\n\nEXC SEG POS TO FARE MARKET:";
    ExcSegPosToFMMap::const_iterator excSegPosToFMIter = excSegPosToFM.begin();
    for (; excSegPosToFMIter != excSegPosToFM.end(); ++excSegPosToFMIter)
    {
      if (excSegPosToFMIter->first != -1)
      {
        const TravelSeg* currSeg = excTvlSegV[(excSegPosToFMIter->first) - 1];
        const FareMarket* currFM = excSegPosToFMIter->second;
        *_dc << "\n EXC SEG POS: " << excSegPosToFMIter->first
             << " - SEG INFO: " << currSeg->origAirport() << "-" << currSeg->destAirport() << " "
             << currSeg->departureDT();
        *_dc << "\n  FARE MARKET: " << currFM->origin()->loc() << "-"
             << currFM->destination()->loc();
      }
    }
  }

  if (_ondInfoToExcSegPosSet.empty())
  {
    *_dc << "\n\nOND INFO TO EXC SEG POS IS EMPTY";
  }
  else
  {
    *_dc << "\n\nOND INFO TO EXC SEG POS:";
    ONDInfoToExcSegPosSetMap::const_iterator ondInfoToExcSegPosSetIter =
        _ondInfoToExcSegPosSet.begin();
    for (; ondInfoToExcSegPosSetIter != _ondInfoToExcSegPosSet.end(); ++ondInfoToExcSegPosSetIter)
    {
      const ONDInf* ondInf = ondInfoToExcSegPosSetIter->first;
      *_dc << "\n OND INFO: " << ondInf->origAirport << "-" << ondInf->destAirport << " "
           << ondInf->travelDate;
      std::vector<int>::const_iterator segPosIter = ondInfoToExcSegPosSetIter->second.begin();
      for (; segPosIter != ondInfoToExcSegPosSetIter->second.end(); ++segPosIter)
      {
        if (*segPosIter != -1)
        {
          const TravelSeg* currSeg = excTvlSegV[(*segPosIter) - 1];
          *_dc << "\n  EXC SEG POS: " << *segPosIter << " - SEG INFO: " << currSeg->origAirport()
               << "-" << currSeg->destAirport() << " " << currSeg->departureDT();
        }
      }
    }
  }

  if (_ondInfoToFM.empty())
  {
    *_dc << "\n\nOND INFO TO FM IS EMPTY";
  }
  else
  {
    *_dc << "\n\nOND INFO TO FARE MARKET:";
    ONDInfoToFM::const_iterator ondInfoToFMIter = _ondInfoToFM.begin();
    for (; ondInfoToFMIter != _ondInfoToFM.end(); ++ondInfoToFMIter)
    {
      const ONDInf* ondInf = ondInfoToFMIter->first;
      *_dc << "\n OND INFO: " << ondInf->origAirport << "-" << ondInf->destAirport << " "
           << ondInf->travelDate;
      std::vector<const FareMarket*>::const_iterator fmIter = ondInfoToFMIter->second.begin();
      for (; fmIter != ondInfoToFMIter->second.end(); ++fmIter)
      {
        *_dc << "\n  FARE MARKET: " << (*fmIter)->origin()->loc() << "-"
             << (*fmIter)->destination()->loc();
      }
    }
  }

  if (_ondInfWithSplittedFMFlown)
  {
    *_dc << "\n\nSPLITTED FLOWN OND INFO: " << _ondInfWithSplittedFMFlown->origAirport << "-"
         << _ondInfWithSplittedFMFlown->destAirport << " "
         << _ondInfWithSplittedFMFlown->travelDate;
  }
  if (_ondInfWithSplittedFMUnflown)
  {
    *_dc << "\n\nSPLITTED UNFLOWN OND INFO: " << _ondInfWithSplittedFMUnflown->origAirport << "-"
         << _ondInfWithSplittedFMUnflown->destAirport << " "
         << _ondInfWithSplittedFMUnflown->travelDate;
  }
}

void
RexConstrainsConsolidator::consolideConstrainsForONDInfo()
{
  if (_dc && _ondInfo.empty())
  {
    *_dc << "\nOND INFO IS EMPTY\n";
  }

  matchFMToExcSegPos();
  createFMToSkipConsRest();

  if (_dc && (_trx.diagnostic().diagParamMapItem("DD") == "DETAILS"))
  {
    printFMToExcSegPos();
    printFMToFirstFMinPU();
    printFMToSkipConsRest();
    printPSSCxrList();
  }

  for (const ONDInf* ondInfo : _ondInfo)
  {
    RexShoppingTrx::OADConsolidatedConstrains* oadConsRest = nullptr;
    _trx.dataHandle().get(oadConsRest);
    initOADConsRest(*oadConsRest, ondInfo);

    addUnshoppedFlightsToOADConsRest(*oadConsRest, ondInfo); // unshopped flights

    consolidateRestFlightsWithBkg(*oadConsRest, ondInfo); // portion + outbound portion
    consolidateRestFlights(*oadConsRest, ondInfo); // flight no
    checkRestrictionNeeded(*oadConsRest, ondInfo);

    consolidateRestCxrs(*oadConsRest, ondInfo); // cxr appl tbl 990 + first break

    consolidateCalendarRanges(*oadConsRest, ondInfo);

    _trx.oadConsRest().push_back(oadConsRest);
  }
}

void
RexConstrainsConsolidator::initOADConsRest(RexShoppingTrx::OADConsolidatedConstrains& oadConsRest,
                                           const ONDInf* ondInfo)
{
  oadConsRest.firstONDInfoSegPos = ondInfo->firstONDInfoNewSegPos;
  oadConsRest.lastONDInfoSegPos = ondInfo->lastONDInfoNewSegPos;
  oadConsRest.origAirport = ondInfo->origAirport;
  oadConsRest.destAirport = ondInfo->destAirport;
  oadConsRest.travelDate = ondInfo->travelDate;
  oadConsRest.unflown = ondInfo->unflown;
  oadConsRest.sodConstraints[0];
  _constToONDInfo[&oadConsRest] = ondInfo;

  if (_dc && (_trx.diagnostic().diagParamMapItem("DD") == "DETAILS"))
  {
    *_dc << "\n\nOND INFO: " << ondInfo->origAirport << "-" << ondInfo->destAirport << " "
         << ondInfo->travelDate << (ondInfo->unflown ? " UNFLOWN" : " FLOWN");

    if (_ondInfoToFM.count(ondInfo) != 0)
    {
      *_dc << "\n FIRST SEG IN OND INFO: " << _ondInfoToExcSegPosSet[ondInfo].front()
           << "\n LAST SEG IN OND INFO: " << _ondInfoToExcSegPosSet[ondInfo].back();
    }
  }
}

void
RexConstrainsConsolidator::matchFMToExcSegPos()
{
  for (const FareCompInfo* fci : _trx.exchangeItin().front()->fareComponent())
  {
    for (const TravelSeg* seg : fci->fareMarket()->travelSeg())
    {
      int segmentPos = segmentOrder(_excItinSeg, seg);
      if (segmentPos)
        _fmToExcSegPosSet[fci->fareMarket()].push_back(segmentPos);
    }
  }
}

void
RexConstrainsConsolidator::printFMToExcSegPos()
{
  if (_fmToExcSegPosSet.empty())
  {
    *_dc << "\n\nFM TO EXC SEG POS IS EMPTY\n";
  }
  else
  {
    *_dc << "\n\nFM TO EXC SEG POS:";
    const std::vector<TravelSeg*>& excTvlSegV = _excItinSeg;
    FMToExcSegPosVectMap::const_iterator fmToExcSegPosIter = _fmToExcSegPosSet.begin();
    for (; fmToExcSegPosIter != _fmToExcSegPosSet.end(); ++fmToExcSegPosIter)
    {
      *_dc << "\n FARE MARKET: " << fmToExcSegPosIter->first->origin()->loc() << "-"
           << fmToExcSegPosIter->first->destination()->loc();

      std::vector<int>::const_iterator segPosIter = fmToExcSegPosIter->second.begin();
      for (; segPosIter != fmToExcSegPosIter->second.end(); ++segPosIter)
      {
        const TravelSeg* currSeg = excTvlSegV[*segPosIter - 1];
        *_dc << "\n  EXC SEG POS: " << *segPosIter << " - SEG INFO: " << currSeg->origAirport()
             << "-" << currSeg->destAirport() << " " << currSeg->departureDT();
      }
    }
  }
}

void
RexConstrainsConsolidator::consolidateRestFlightsWithBkg(
    RexShoppingTrx::OADConsolidatedConstrains& oadConsRest, const ONDInf* ondInfo)
{
  bool viewDetails = (_trx.diagnostic().diagParamMapItem("DD") == "DETAILS");
  if (_dc && viewDetails)
  {
    *_dc << "\n\n * PORTION * OUTBOUND PORTION *";
  }

  if (_ondInfoToFM.count(ondInfo) != 0)
  {
    std::vector<const FareMarket*>::const_iterator ondInfoToFmIter = _ondInfoToFM[ondInfo].begin();
    for (; ondInfoToFmIter != _ondInfoToFM[ondInfo].end(); ++ondInfoToFmIter)
    {
      const FareMarket* fareMarket = *ondInfoToFmIter;
      const int firstSegPos = _ondInfoToExcSegPosSet[ondInfo].front();
      const int lastSegPos = _ondInfoToExcSegPosSet[ondInfo].back();

      if (_dc && viewDetails)
      {
        *_dc << "\n\n  FARE MARKET: " << fareMarket->origin()->loc() << "-"
             << fareMarket->destination()->loc();
      }

      if (std::find(_fmToSkipConsRest.begin(), _fmToSkipConsRest.end(), fareMarket) ==
          _fmToSkipConsRest.end())
      {
        if (_trx.oadResponse().count(fareMarket) != 0)
        {
          const auto& oadResponseData = _trx.oadResponse()[fareMarket];

          for (std::size_t i = 0; i < oadResponseData.size(); ++i)
          {
            if (_dc && viewDetails)
            {
              *_dc << "\n   PORTION REST SEGMENTS: ";
            }
            addRestFlightsWithBkgToOADConsRest(oadConsRest.sodConstraints[i],
                                               oadResponseData[i].portion, firstSegPos, lastSegPos,
                                               ondInfo);

            if (_dc && viewDetails)
            {
              *_dc << "\n   OUTBOUND PORTION REST SEGMENTS: ";
            }
            addRestFlightsWithBkgToOADConsRest(oadConsRest.sodConstraints[i],
                                               oadResponseData[i].outboundPortion, firstSegPos,
                                               lastSegPos, ondInfo);
          }
        }
        else if (_dc && viewDetails)
        {
          *_dc << "\n   FM DOES NOT HAVE ANY MERGED REST";
        }
      }
      else
      {
        if (_dc && viewDetails)
        {
          *_dc << "\n   SKIPPED DURING REST CONSOLIDATION";
        }
      }
    }
  }
  else if (_dc && viewDetails)
  {
    *_dc << "\n  OND INFO DOES NOT HAVE ANY FARE MARKETS";
  }

  if (_dc && viewDetails)
  {
    for (const auto& sodConstraint : oadConsRest.sodConstraints | boost::adaptors::map_values)
    {
      *_dc << "\n\n   MERGED REST SEGMENTS: ";
      if (!sodConstraint.restFlightsWithBkg.empty())
      {
        copy(sodConstraint.restFlightsWithBkg.begin(), sodConstraint.restFlightsWithBkg.end(),
             std::ostream_iterator<int>(*_dc, " "));
      }
    }
  }
}

void
RexConstrainsConsolidator::addRestFlightsWithBkgToOADConsRest(
    RexShoppingTrx::SODConstraint& sodConstraint,
    const RexShoppingTrx::PortionMergeTvlVectType& portion,
    const int firstSegPos,
    const int lastSegPos,
    const ONDInf* ondInfo)
{
  if (_dc && _trx.diagnostic().diagParamMapItem("DD") == "DETAILS")
  {
    if (!portion.empty())
    {
      copy(portion.begin(), portion.end(), std::ostream_iterator<int>(*_dc, " "));
    }
  }

  RexShoppingTrx::PortionMergeTvlVectType::const_iterator portIter = portion.begin();
  for (; portIter != portion.end(); ++portIter)
  {
    if (*portIter >= firstSegPos && *portIter <= lastSegPos)
    {
      if (_excSegPosToNewSegPos.count(ondInfo) &&
          _excSegPosToNewSegPos[ondInfo].count(*portIter) != 0)
        sodConstraint.restFlightsWithBkg.insert(_excSegPosToNewSegPos[ondInfo][*portIter]);
      else
        throw ErrorResponseException(ErrorResponseException::REISSUE_RULES_FAIL);
    }
  }
}

void
RexConstrainsConsolidator::addUnshoppedFlightsToOADConsRest(
    RexShoppingTrx::OADConsolidatedConstrains& oadConsRest, const ONDInf* ondInfo)
{
  if (_dc && _trx.diagnostic().diagParamMapItem("DD") == "DETAILS")
  {
    *_dc << "\n UNSHOPPED SEGMENTS: ";
    if (!ondInfo->unshoppedFlights.empty())
    {
      copy(ondInfo->unshoppedFlights.begin(),
           ondInfo->unshoppedFlights.end(),
           std::ostream_iterator<int>(*_dc, " "));
    }
  }

  oadConsRest.unshoppedFlights.insert(ondInfo->unshoppedFlights.begin(),
                                      ondInfo->unshoppedFlights.end());
}

void
RexConstrainsConsolidator::consolidateRestFlights(
    RexShoppingTrx::OADConsolidatedConstrains& oadConsRest, const ONDInf* ondInfo)
{
  bool viewDetails = (_trx.diagnostic().diagParamMapItem("DD") == "DETAILS");
  if (_dc && viewDetails)
  {
    *_dc << "\n\n * FLIGHT NUMBER * UNSHOPPED FLIGHTS*";
  }

  if (_ondInfoToFM.count(ondInfo) != 0)
  {
    std::vector<const FareMarket*>::const_iterator ondInfoToFmIter = _ondInfoToFM[ondInfo].begin();
    for (; ondInfoToFmIter != _ondInfoToFM[ondInfo].end(); ++ondInfoToFmIter)
    {
      const FareMarket* fareMarket = *ondInfoToFmIter;
      const int firstSegPos = _ondInfoToExcSegPosSet[ondInfo].front();
      const int lastSegPos = _ondInfoToExcSegPosSet[ondInfo].back();

      if (_dc && viewDetails)
      {
        *_dc << "\n\n  FARE MARKET: " << fareMarket->origin()->loc() << "-"
             << fareMarket->destination()->loc();
      }

      if (std::find(_fmToSkipConsRest.begin(), _fmToSkipConsRest.end(), fareMarket) ==
          _fmToSkipConsRest.end())
      {
        if (_trx.oadResponse().count(fareMarket) != 0)
        {
          const auto& oadResponseData = _trx.oadResponse()[fareMarket];

          for (std::size_t i = 0; i < oadResponseData.size(); ++i)
          {
            if (oadResponseData[i].flightNumberRestriction == true)
            {
              addRestFlightsToOADConsRest(oadConsRest.sodConstraints[i], fareMarket, firstSegPos,
                                          lastSegPos, ondInfo);
              if (_dc && viewDetails)
              {
                *_dc << "\n   FLIGHT NO IS RESTRICTED";
              }
            }
            else if (_dc && viewDetails)
            {
              *_dc << "\n   FLIGHT NO IS NOT RESTRICTED";
            }
          }
        }
        else if (_dc && viewDetails)
        {
          *_dc << "\n   FM DOES NOT HAVE ANY MERGED REST";
        }
      }
      else
      {
        if (_dc && viewDetails)
        {
          *_dc << "\n   SKIPPED DURING REST CONSOLIDATION";
        }
      }
    }
  }
  else if (_dc && viewDetails)
  {
    *_dc << "\n  OND INFO DOES NOT HAVE ANY FARE MARKETS";
  }

  addUnshoppedFlightsToOADConsRest(oadConsRest);

  if (_dc && viewDetails)
  {
    for (const auto& sodConstraint : oadConsRest.sodConstraints | boost::adaptors::map_values)
    {
      *_dc << "\n\n   MERGED REST SEGMENTS: ";
      if (!sodConstraint.restFlights.empty())
      {
        copy(sodConstraint.restFlights.begin(), sodConstraint.restFlights.end(),
             std::ostream_iterator<int>(*_dc, " "));
      }
    }
  }
}

void
RexConstrainsConsolidator::addRestFlightsToOADConsRest(
    RexShoppingTrx::SODConstraint& sodConstraint,
    const FareMarket* fareMarket,
    const int firstSegPos,
    const int lastSegPos,
    const ONDInf* ondInfo)
{
  std::vector<int>::const_iterator fmToExcSegPosIter = _fmToExcSegPosSet[fareMarket].begin();
  for (; fmToExcSegPosIter != _fmToExcSegPosSet[fareMarket].end(); ++fmToExcSegPosIter)
  {
    if (*fmToExcSegPosIter >= firstSegPos && *fmToExcSegPosIter <= lastSegPos)
    {
      if (_excSegPosToNewSegPos.count(ondInfo) &&
          _excSegPosToNewSegPos[ondInfo].count(*fmToExcSegPosIter) != 0)
        sodConstraint.restFlights.insert(_excSegPosToNewSegPos[ondInfo][*fmToExcSegPosIter]);
      else
        throw ErrorResponseException(ErrorResponseException::REISSUE_RULES_FAIL);
    }
  }
}

void
RexConstrainsConsolidator::consolidateRestCxrs(
    RexShoppingTrx::OADConsolidatedConstrains& oadConsRest, const ONDInf* ondInfo)
{
  bool viewDetails = (_trx.diagnostic().diagParamMapItem("DD") == "DETAILS");
  if (_dc && viewDetails)
  {
    *_dc << "\n\n * CXR APPL TBL 990 * FIRST BREAK *";
  }

  if (_ondInfoToFM.count(ondInfo) != 0)
  {
    std::vector<const FareMarket*>::const_iterator ondInfoToFmIter = _ondInfoToFM[ondInfo].begin();
    for (; ondInfoToFmIter != _ondInfoToFM[ondInfo].end(); ++ondInfoToFmIter)
    {
      const FareMarket* fareMarket = *ondInfoToFmIter;

      if (_dc && viewDetails)
      {
        *_dc << "\n\n  FARE MARKET: " << fareMarket->origin()->loc() << "-"
             << fareMarket->destination()->loc();
      }

      if (std::find(_fmToSkipConsRest.begin(), _fmToSkipConsRest.end(), fareMarket) ==
          _fmToSkipConsRest.end())
      {
        if (_trx.oadResponse().count(fareMarket) != 0)
        {
          const auto& oadResponseData = _trx.oadResponse()[fareMarket];

          for (std::size_t i = 0; i < oadResponseData.size(); ++i)
          {
            addCarrierRestToOADConsRest(oadConsRest.sodConstraints[i], fareMarket,
                                        oadResponseData[i], ondInfo);
          }
        }
        else if (_dc && viewDetails)
        {
          *_dc << "\n   FM DOES NOT HAVE ANY MERGED REST";
        }
      }
      else
      {
        if (_dc && viewDetails)
        {
          *_dc << "\n   SKIPPED DURING REST CONSOLIDATION";
        }
      }
    }
  }
  else if (_dc && viewDetails)
  {
    *_dc << "\n  OND INFO DOES NOT HAVE ANY FARE MARKETS";
  }

  for (auto& sodConstraint : oadConsRest.sodConstraints | boost::adaptors::map_values)
    mergeOADCrxListWithPSSCrxList(sodConstraint);

  if (_dc && viewDetails)
  {
    for (const auto& sodConstraint : oadConsRest.sodConstraints | boost::adaptors::map_values)
    {
      *_dc << "\n\n   MERGED GOV CARRIER REST: ";
      if (sodConstraint.fareByteCxrAppl)
      {
        *_dc << "\n    "
             << (sodConstraint.fareByteCxrAppl->govCxr.excluded ? "RESTRICTED" : "APPLICABLE")
             << " CXR: ";
        if (!sodConstraint.fareByteCxrAppl->govCxr.cxrList.empty())
        {
          copy(sodConstraint.fareByteCxrAppl->govCxr.cxrList.begin(),
               sodConstraint.fareByteCxrAppl->govCxr.cxrList.end(),
               std::ostream_iterator<CarrierCode>(*_dc, " "));
        }
      }
      *_dc << "\n\n   MERGED USER CARRIER REST: ";
      if (sodConstraint.fareByteCxrAppl)
      {
        *_dc << "\n    "
             << (sodConstraint.fareByteCxrAppl->usrCxr.excluded ? "RESTRICTED" : "APPLICABLE")
             << " CXR: ";
        if (!sodConstraint.fareByteCxrAppl->usrCxr.cxrList.empty())
        {
          copy(sodConstraint.fareByteCxrAppl->usrCxr.cxrList.begin(),
               sodConstraint.fareByteCxrAppl->usrCxr.cxrList.end(),
               std::ostream_iterator<CarrierCode>(*_dc, " "));
        }
      }
    }
  }
}

void
RexConstrainsConsolidator::checkRestrictionNeeded(
    RexShoppingTrx::OADConsolidatedConstrains& oadConsRest, const ONDInf* ondInfo)
{
  std::vector<const FareMarket*> excFMs = _ondInfoToFM[ondInfo];

  if (excFMs.empty())
    return;

  if (excFMs.front()->boardMultiCity() == oadConsRest.origAirport ||
      isMutliAirportCityTheSame(excFMs.front()->boardMultiCity(), oadConsRest.origAirport))
  {
    if (excFMs.back()->offMultiCity() == oadConsRest.destAirport ||
        isMutliAirportCityTheSame(excFMs.back()->offMultiCity(), oadConsRest.destAirport))
    {
      oadConsRest.cxrRestrictionNeeded = true;

      return;
    }
  }

  for (const TravelSeg* excSeg : excFMs.front()->travelSeg())
  {
    if (excSeg->boardMultiCity() == oadConsRest.origAirport ||
        isMutliAirportCityTheSame(excSeg->boardMultiCity(), oadConsRest.origAirport))
    {
      if (excSeg->offMultiCity() == oadConsRest.destAirport ||
          isMutliAirportCityTheSame(excSeg->offMultiCity(), oadConsRest.destAirport))
      {
        if (excSeg == excFMs.front()->primarySector())
        {
          oadConsRest.cxrRestrictionNeeded = true;
        }
        else
        {
          oadConsRest.cxrRestrictionNeeded = false;
        }
        return;
      }
    }
  }
}

void
RexConstrainsConsolidator::addCarrierRestToOADConsRest(
    RexShoppingTrx::SODConstraint& sodConstraint,
    const FareMarket* fareMarket,
    const RexShoppingTrx::OADResponseData& oadResponseData,
    const ONDInf* ondInfo)
{
  if (_dc && _trx.diagnostic().diagParamMapItem("DD") == "DETAILS")
  {
    *_dc << "\n     " << (oadResponseData.fareByteCxrAppl.excluded ? "RESTRICTED" : "APPLICABLE")
         << " CXR: ";
    if (!oadResponseData.fareByteCxrAppl.cxrList.empty())
    {
      copy(oadResponseData.fareByteCxrAppl.cxrList.begin(),
           oadResponseData.fareByteCxrAppl.cxrList.end(),
           std::ostream_iterator<CarrierCode>(*_dc, " "));
    }
    if (_trx.fMToFirstFMinPU().count(fareMarket) != 0)
    {
      *_dc << "\n     THIS IS FIRST FARE MARKET IN PRICING UNIT";
      std::vector<const FareMarket*>::const_iterator fmIter =
          _trx.fMToFirstFMinPU()[fareMarket].begin();
      for (; fmIter != _trx.fMToFirstFMinPU()[fareMarket].end(); ++fmIter)
      {
        if (_trx.oadResponse().count(*fmIter) != 0)
        {
          *_dc << "\n      FARE MARKET: " << (*fmIter)->origin()->loc() << "-"
               << (*fmIter)->destination()->loc()
               << " FIRST BREAK: " << (oadResponseData.firstBreakRest ? "Y" : "N");
        }
      }
    }
  }

  if (sodConstraint.fareByteCxrAppl == nullptr)
  {
    _trx.dataHandle().get(sodConstraint.fareByteCxrAppl);

    sodConstraint.fareByteCxrAppl->govCxr.cxrList.insert(
        oadResponseData.fareByteCxrAppl.cxrList.begin(),
        oadResponseData.fareByteCxrAppl.cxrList.end());
    sodConstraint.fareByteCxrAppl->govCxr.excluded = oadResponseData.fareByteCxrAppl.excluded;
    if (isPrefGovCxrForcedByFirstBreakRest(fareMarket, oadResponseData))
    {
      addGovCxr(sodConstraint.fareByteCxrAppl->govCxr, fareMarket->governingCarrier());
    }
  }
  else
  {
    mergeCxrRest(sodConstraint.fareByteCxrAppl->govCxr, oadResponseData, fareMarket);
  }
}

bool
RexConstrainsConsolidator::isPrefGovCxrForcedByFirstBreakRest(
    const FareMarket* fareMarket, const RexShoppingTrx::OADResponseData& oadResponseData)
{
  if (_trx.fMToFirstFMinPU().count(fareMarket) != 0)
  {
    std::vector<const FareMarket*>::const_iterator fmIter =
        _trx.fMToFirstFMinPU()[fareMarket].begin();
    for (; fmIter != _trx.fMToFirstFMinPU()[fareMarket].end(); ++fmIter)
    {
      if (_trx.oadResponse().count(*fmIter) != 0)
      {
        if (oadResponseData.firstBreakRest == true)
        {
          return true;
        }
      }
    }
  }
  return false;
}

void
RexConstrainsConsolidator::consolidateCalendarRanges(
    RexShoppingTrx::OADConsolidatedConstrains& oadConsRest, const ONDInf* ondInfo)
{
  if (_ondInfoToFM.count(ondInfo) != 0)
  {
    for (const auto* fareMarket : _ondInfoToFM[ondInfo])
    {
      const auto& oadResponseData = _trx.oadResponse()[fareMarket];
      for (std::size_t i = 0; i < oadResponseData.size(); ++i)
      {
        oadConsRest.sodConstraints[i].calendarRange = oadResponseData[i].calendarRange;
      }
    }
  }
}

void
RexConstrainsConsolidator::printFMToFirstFMinPU()
{
  if (_trx.fMToFirstFMinPU().empty())
  {
    *_dc << "\n\nFM VECT IN PU IS EMPTY\n";
  }
  else
  {
    *_dc << "\n\nFM VECT IN PU:";
    RexShoppingTrx::FMToFirstFMMap::const_iterator fMToFirstFMIter = _trx.fMToFirstFMinPU().begin();
    for (; fMToFirstFMIter != _trx.fMToFirstFMinPU().end(); ++fMToFirstFMIter)
    {
      const FareMarket* firstFareMarket = fMToFirstFMIter->first;
      const std::vector<const FareMarket*>& fareMarketVect = fMToFirstFMIter->second;
      *_dc << "\n FIRST FARE MARKET IN PU: " << firstFareMarket->origin()->loc() << "-"
           << firstFareMarket->destination()->loc();
      std::vector<const FareMarket*>::const_iterator fmIter = fareMarketVect.begin();
      for (; fmIter != fareMarketVect.end(); ++fmIter)
      {
        *_dc << "\n FARE MARKET IN PU: " << (*fmIter)->origin()->loc() << "-"
             << (*fmIter)->destination()->loc() << " - " << (*fmIter)->governingCarrier();
      }
    }
  }
}

void
RexConstrainsConsolidator::printConsolidatedConstrains()
{
  *_dc << "\n\n*** CONSOLIDATED CONSTRAINS ***";
  RexShoppingTrx::OADConsolidatedConstrainsVect::const_iterator oadConsRest =
      _trx.oadConsRest().begin();
  for (; oadConsRest != _trx.oadConsRest().end(); ++oadConsRest)
  {
    RexShoppingTrx::OADConsolidatedConstrains& oadConsolConstr = **oadConsRest;
    *_dc << "\nOAD: " << oadConsolConstr.origAirport << "-" << oadConsolConstr.destAirport << " "
         << oadConsolConstr.travelDate << " " << (oadConsolConstr.unflown ? "UNFLOWN" : "FLOWN");

    *_dc << "\n UNSHOPPED FLIGHTS: ";
    const std::vector<TravelSeg*>& newTvlSegV = _newItinSeg;
    std::set<int>::const_iterator segUnFltIter = oadConsolConstr.unshoppedFlights.begin();
    for (; segUnFltIter != oadConsolConstr.unshoppedFlights.end(); ++segUnFltIter)
    {
      const TravelSeg* currSeg = newTvlSegV[(*segUnFltIter) - 1];
      *_dc << "\n  NEW SEG POS: " << *segUnFltIter << " - SEG INFO: " << currSeg->origAirport()
           << "-" << currSeg->destAirport() << " " << currSeg->departureDT();
    }

    for (const auto& sodConstraint : oadConsolConstr.sodConstraints | boost::adaptors::map_values)
    {
      if (sodConstraint.calendarRange.isValid())
        *_dc << "\n RESTRICTIONS VALID FROM "
             << sodConstraint.calendarRange.firstDate.dateToString(DateFormat::YYYYMMDD, "-")
             << " TO "
             << sodConstraint.calendarRange.lastDate.dateToString(DateFormat::YYYYMMDD, "-");

      *_dc << "\n REST SEGMENTS (BKG CANNOT CHANGE): ";
      std::set<int>::const_iterator segBkgIter = sodConstraint.restFlightsWithBkg.begin();
      for (; segBkgIter != sodConstraint.restFlightsWithBkg.end(); ++segBkgIter)
      {
        const TravelSeg* currSeg = newTvlSegV[(*segBkgIter) - 1];
        *_dc << "\n  NEW SEG POS: " << *segBkgIter << " - SEG INFO: " << currSeg->origAirport()
             << "-" << currSeg->destAirport() << " " << currSeg->departureDT();
      }
      *_dc << "\n REST SEGMENTS (BKG CAN CHANGE): ";
      std::set<int>::const_iterator segIter = sodConstraint.restFlights.begin();
      for (; segIter != sodConstraint.restFlights.end(); ++segIter)
      {
        const TravelSeg* currSeg = newTvlSegV[(*segIter) - 1];
        *_dc << "\n  NEW SEG POS: " << *segIter << " - SEG INFO: " << currSeg->origAirport() << "-"
             << currSeg->destAirport() << " " << currSeg->departureDT();
      }
      if (sodConstraint.fareByteCxrAppl && !sodConstraint.fareByteCxrAppl->govCxr.cxrList.empty())
      {
        *_dc << "\n CARRIER REST: ";
        *_dc << "\n   "
             << (sodConstraint.fareByteCxrAppl->govCxr.excluded ? "RESTRICTED" : "APPLICABLE")
             << " CXR: ";

        copy(sodConstraint.fareByteCxrAppl->govCxr.cxrList.begin(),
             sodConstraint.fareByteCxrAppl->govCxr.cxrList.end(),
             std::ostream_iterator<CarrierCode>(*_dc, " "));
      }
    }
  }
  if (_trx.oadResponse().empty())
  {
    return;
  }
  const auto& oadResponseData = _trx.oadResponse().begin()->second;
  for (const auto& oadResponse : oadResponseData)
  {
    const RexShoppingTrx::ForcedConnectionSet& frcConxRest = oadResponse.forcedConnections;
    *_dc << "\nFORCED CONNECTIONS: ";
    if (!frcConxRest.empty())
    {
      copy(frcConxRest.begin(), frcConxRest.end(), std::ostream_iterator<LocCode>(*_dc, " "));
    }
  }
}

void
RexConstrainsConsolidator::createFMToSkipConsRest()
{
  const ExcItin* excItin = _trx.exchangeItin().front();
  bool exchangeInternationalTicket = (excItin->geoTravelType() == GeoTravelType::International);

  FMToExcSegPosVectMap::const_iterator fmToExcSegPosIter = _fmToExcSegPosSet.begin();
  for (; fmToExcSegPosIter != _fmToExcSegPosSet.end(); ++fmToExcSegPosIter)
  {
    const FareMarket* fareMarket = fmToExcSegPosIter->first;
    if (exchangeInternationalTicket && fareMarket->geoTravelType() == GeoTravelType::Domestic)
    {
      _fmToSkipConsRest.push_back(fareMarket);
    }
  }
}

void
RexConstrainsConsolidator::printFMToSkipConsRest()
{
  if (_fmToSkipConsRest.empty())
  {
    *_dc << "\n\nFM TO SKIP RESTRICTION CONSOLIDATION IS EMPTY\n";
  }
  else
  {
    *_dc << "\n\nFM TO SKIP RESTRICTION CONSOLIDATION:";
    FMToSkipConsRest::const_iterator fmToSkipConsRestIter = _fmToSkipConsRest.begin();
    for (; fmToSkipConsRestIter != _fmToSkipConsRest.end(); ++fmToSkipConsRestIter)
    {
      *_dc << "\n FARE MARKET: " << (*fmToSkipConsRestIter)->origin()->loc() << "-"
           << (*fmToSkipConsRestIter)->destination()->loc();
    }
  }
}

void
RexConstrainsConsolidator::createSODForONDInfo()
{
  std::vector<const FareMarket*> fmWithFlownGov;

  createFmWithFlownGovSet(fmWithFlownGov);
  if (_dc && _trx.diagnostic().diagParamMapItem("DD") == "DETAILS")
  {
    printFmWithFlownGovSet(fmWithFlownGov);
  }

  bool frontOADwasUnflownAndwithoutCxrRest = false;
  for (const auto& element : _trx.oadConsRest() | boost::adaptors::indexed())
  {
    RexShoppingTrx::OADConsolidatedConstrains& oadConsolConstr = *element.value();
    const auto ondIndex = element.index();

    for (const auto& sodConstraint : oadConsolConstr.sodConstraints | boost::adaptors::map_values)
    {
      bool addCxrRest = true;

      addCxrRest = !isONDinExcFMWithFlownGov(fmWithFlownGov, &oadConsolConstr);
      if (!addCxrRest && _dc && _trx.diagnostic().diagParamMapItem("DD") == "DETAILS")
      {
        printOADConsRest(oadConsolConstr, "GOV ALREADY FLOWN");
      }

      if (!_addCxrRestToUnmappedONDInfos &&
          _ondInfoNotMatched.count(_constToONDInfo[&oadConsolConstr]) != 0)
      {
        if (!frontOADwasUnflownAndwithoutCxrRest)
        {
          addCxrRest = false;
          if (_dc && _trx.diagnostic().diagParamMapItem("DD") == "DETAILS")
          {
            printOADConsRest(oadConsolConstr, "UNMAPPED OAD ALREADY FLOWN");
          }
        }
      }
      if (!_addCxrRestToUnmappedONDInfos &&
          _ondInfoToFMWithoutNoMatched.count(_constToONDInfo[&oadConsolConstr]) != 0)
      {
        if (oadConsolConstr.unflown && sodConstraint.fareByteCxrAppl &&
            sodConstraint.fareByteCxrAppl->govCxr.cxrList.empty())
        {
          frontOADwasUnflownAndwithoutCxrRest = true;
          addCxrRest = false;
          if (_dc && _trx.diagnostic().diagParamMapItem("DD") == "DETAILS")
          {
            printOADConsRest(oadConsolConstr, "UNMAPPED OAD ALREADY FLOWN-UNFLOWN");
          }
        }
      }

      int firstONDInfoSegPos = oadConsolConstr.firstONDInfoSegPos;
      const int lastONDInfoSegPos = oadConsolConstr.lastONDInfoSegPos;

      bool prevBkcRestricted = (sodConstraint.restFlightsWithBkg.count(firstONDInfoSegPos) != 0);
      bool prevFlightRestricted = (sodConstraint.restFlights.count(firstONDInfoSegPos) != 0);

      int firstSODexcSegPos = firstONDInfoSegPos;
      if (firstONDInfoSegPos < lastONDInfoSegPos)
      {
        for (int segPos = firstONDInfoSegPos + 1; segPos <= lastONDInfoSegPos; ++segPos)
        {
          bool nextFlightRestricted = sodConstraint.restFlights.count(segPos) != 0;
          bool nextBkcRestricted = sodConstraint.restFlightsWithBkg.count(segPos) != 0;

          if ((!prevBkcRestricted && !nextBkcRestricted &&
               prevFlightRestricted != nextFlightRestricted) ||
              (!prevFlightRestricted && !nextFlightRestricted &&
               prevBkcRestricted != nextBkcRestricted))
          {
            addSOD(firstSODexcSegPos, segPos - 1, oadConsolConstr, sodConstraint,
                   prevBkcRestricted || prevFlightRestricted, addCxrRest, ondIndex);

            firstSODexcSegPos = segPos;
            prevBkcRestricted = (sodConstraint.restFlightsWithBkg.count(segPos) != 0);
            prevFlightRestricted = (sodConstraint.restFlights.count(segPos) != 0);
          }
        }
      }
      addSOD(firstSODexcSegPos, lastONDInfoSegPos, oadConsolConstr, sodConstraint,
             prevBkcRestricted || prevFlightRestricted, addCxrRest, ondIndex);
    }
  }
}

void RexConstrainsConsolidator::addSOD(const int firstSODnewSegPos,
                                       const int lastSODnewSegPos,
                                       RexShoppingTrx::OADConsolidatedConstrains& oadConsolConstr,
                                       const RexShoppingTrx::SODConstraint& sodConstraint,
                                       const int restricted,
                                       const bool addCxrRest,
                                       const std::size_t ondIndex)
{
  RexShoppingTrx::SubOriginDestination* sod = nullptr;
  _trx.dataHandle().get(sod);
  const std::vector<TravelSeg*>& newTvlSegV = _newItinSeg;
  const TravelSeg* firstSODSeg = newTvlSegV[firstSODnewSegPos - 1];
  const TravelSeg* lastSODSeg = newTvlSegV[lastSODnewSegPos - 1];
  if (isMutliAirportCityTheSame(firstSODSeg->boardMultiCity(), oadConsolConstr.origAirport))
    sod->origAirport = oadConsolConstr.origAirport;
  else
    sod->origAirport = firstSODSeg->origAirport();
  if (isMutliAirportCityTheSame(lastSODSeg->offMultiCity(), oadConsolConstr.destAirport))
    sod->destAirport = oadConsolConstr.destAirport;
  else
    sod->destAirport = lastSODSeg->destAirport();
  if ((isMutliAirportCityTheSame(firstSODSeg->boardMultiCity(), oadConsolConstr.origAirport)) ||
      (firstSODSeg->origAirport() == oadConsolConstr.origAirport))
    sod->travelDate = oadConsolConstr.travelDate.date();
  else
    sod->travelDate = firstSODSeg->departureDT().date();
  for (int segPos = firstSODnewSegPos; segPos <= lastSODnewSegPos; ++segPos)
  {
    if (restricted || !oadConsolConstr.unflown)
    {
      sod->flights.insert(segPos);
      bool flightHasNoPortopnRest = (sodConstraint.restFlightsWithBkg.count(segPos) == 0);
      if (flightHasNoPortopnRest && oadConsolConstr.unflown)
      {
        sod->bkcCanChange.insert(segPos);
      }
    }
  }

  bool addGovCxr = false;

  addGovCxr = addCxrRest && oadConsolConstr.unflown && oadConsolConstr.cxrRestrictionNeeded;

  if (addGovCxr)
  {
    setGovCxr(sod->carriers, sodConstraint.fareByteCxrAppl);
    if (sod->carriers && !sod->carriers->govCxr.excluded)
    {
      sod->preferred_cxr = true;
    }
    else
    {
      sod->preferred_cxr = false;
    }
  }

  setUsrCxr(sod->carriers, sodConstraint.fareByteCxrAppl);
  if (sod->carriers && !sod->carriers->usrCxr.excluded)
  {
    sod->exact_cxr = true;
  }
  else
  {
    sod->exact_cxr = false;
  }

  if (restricted || !oadConsolConstr.unflown)
  {
    sod->change = false;
  }
  else
  {
    sod->change = true;
  }

  auto calendarRange = sodConstraint.calendarRange;
  if (!fallback::exscSetEmptyDateRangeAsWholePeriod(&_trx) && !calendarRange.isValid())
  {
    const auto& ond = _trx.orgDest[ondIndex];
    calendarRange = ExchShopCalendar::getDateRangeForOnd(&ond, ond.travelDate);
  }
  sod->calendarRange = calendarRange;

  oadConsolConstr.sod.push_back(sod);
}

void
RexConstrainsConsolidator::printConsolidatedConstrainsWithSOD()
{
  *_dc << "\n\n*** CONSOLIDATED CONSTRAINS WITH SOD ***";
  RexShoppingTrx::OADConsolidatedConstrainsVect::const_iterator oadConsRest =
      _trx.oadConsRest().begin();
  for (; oadConsRest != _trx.oadConsRest().end(); ++oadConsRest)
  {
    RexShoppingTrx::OADConsolidatedConstrains& oadConsolConstr = **oadConsRest;
    *_dc << "\nOAD: " << oadConsolConstr.origAirport << "-" << oadConsolConstr.destAirport << " "
         << oadConsolConstr.travelDate << " " << (oadConsolConstr.unflown ? "UNFLOWN" : "FLOWN");

    std::vector<RexShoppingTrx::SubOriginDestination*>::const_iterator sodIter =
        oadConsolConstr.sod.begin();
    for (; sodIter != oadConsolConstr.sod.end(); ++sodIter)
    {
      RexShoppingTrx::SubOriginDestination& sod = **sodIter;
      *_dc << "\n SOD: " << sod.origAirport << "-" << sod.destAirport << " " << sod.travelDate
           << " "
           << "CHANGE=" << (sod.change ? "Y" : "N") << " EXACT_CXR=" << (sod.exact_cxr ? "Y" : "N")
           << " PREFERRED_CXR=" << (sod.preferred_cxr ? "Y" : "N");

      if (!sod.flights.empty())
      {
        *_dc << "\n  FLIGHT LIST: ";
        const std::vector<TravelSeg*>& newTvlSegV = _newItinSeg;
        std::set<int>::const_iterator fltIter = sod.flights.begin();
        for (; fltIter != sod.flights.end(); ++fltIter)
        {
          const TravelSeg* currSeg = newTvlSegV[(*fltIter) - 1];
          *_dc << "\n   NEW SEG POS: " << *fltIter << " - SEG INFO: " << currSeg->origAirport()
               << "-" << currSeg->destAirport() << " " << currSeg->departureDT();
          if (sod.bkcCanChange.count(*fltIter) != 0)
          {
            *_dc << " - BKC_CHANGE=Y";
          }
        }
      }
      if (sod.carriers && !sod.carriers->govCxr.cxrList.empty())
      {
        *_dc << "\n  GOVERNING CARRIER LIST: ";
        copy(sod.carriers->govCxr.cxrList.begin(),
             sod.carriers->govCxr.cxrList.end(),
             std::ostream_iterator<CarrierCode>(*_dc, " "));
      }

      if (sod.carriers && !sod.carriers->usrCxr.cxrList.empty())
      {
        *_dc << "\n  USER CARRIER LIST: ";
        copy(sod.carriers->usrCxr.cxrList.begin(),
             sod.carriers->usrCxr.cxrList.end(),
             std::ostream_iterator<CarrierCode>(*_dc, " "));
      }
    }
  }
  if (_trx.oadResponse().empty())
  {
    return;
  }
  const auto& oadResponseData = _trx.oadResponse().begin()->second;
  for (const auto& oadResponse : oadResponseData)
  {
    const RexShoppingTrx::ForcedConnectionSet& frcConxRest = oadResponse.forcedConnections;
    if (!frcConxRest.empty())
    {
      *_dc << "\nFORCED CONNECTIONS: ";
      copy(frcConxRest.begin(), frcConxRest.end(), std::ostream_iterator<LocCode>(*_dc, " "));
    }
  }
}

void
RexConstrainsConsolidator::addUnshoppedFlightsToOADConsRest(
    RexShoppingTrx::OADConsolidatedConstrains& oadConsRest)
{
  if (_dc && _trx.diagnostic().diagParamMapItem("DD") == "DETAILS")
  {
    *_dc << "\n  UNSHOPPED FLIGHTS: ";
    if (!oadConsRest.unshoppedFlights.empty())
    {
      copy(oadConsRest.unshoppedFlights.begin(),
           oadConsRest.unshoppedFlights.end(),
           std::ostream_iterator<int>(*_dc, " "));
    }
  }

  std::set<int>::const_iterator unshpFltIter = oadConsRest.unshoppedFlights.begin();
  for (; unshpFltIter != oadConsRest.unshoppedFlights.end(); ++unshpFltIter)
  {
    for (auto& sodConstraint : oadConsRest.sodConstraints | boost::adaptors::map_values)
      sodConstraint.restFlights.insert(*unshpFltIter);
  }
}

void
RexConstrainsConsolidator::setExcSegPosToNewSegPos(
    const std::vector<TravelSeg*>::iterator& frontSeg,
    const std::vector<TravelSeg*>::iterator& backSeg,
    ONDInf* ondInf)
{
  std::vector<TravelSeg*>::iterator segIter = frontSeg;
  int segPosInNewItin = -1;
  for (; segIter != backSeg + 1; ++segIter)
  {
    segPosInNewItin = segmentOrder(_newItinSeg, *segIter);
    const std::vector<TravelSeg*>& excTvlSegV = _excItinSeg;
    std::vector<TravelSeg*>::const_iterator excSegIter = excTvlSegV.begin();
    for (; excSegIter != excTvlSegV.end(); ++excSegIter)
    {
      if (((*segIter)->destAirport() == (*excSegIter)->destAirport()) &&
          ((*segIter)->origAirport() == (*excSegIter)->origAirport()) &&
          ((*segIter)->departureDT().get64BitRepDateOnly() ==
           (*excSegIter)->departureDT().get64BitRepDateOnly()))
      {
        int segPosInExcItin = segmentOrder(_excItinSeg, *excSegIter);
        _excSegPosToNewSegPos[ondInf][segPosInExcItin] = segPosInNewItin;
        break;
      }
    }
  }
}

void
RexConstrainsConsolidator::mergeCxrRest(
    RexShoppingTrx::FareByteCxrApplData& fareByteCxrApplDataRest,
    const RexShoppingTrx::OADResponseData& oadResponseData,
    const FareMarket* fareMarket)
{
  std::set<CarrierCode> mergedRestrictedCxrAppl;
  if (fareByteCxrApplDataRest.excluded == oadResponseData.fareByteCxrAppl.excluded)
  {
    std::set_intersection(fareByteCxrApplDataRest.cxrList.begin(),
                          fareByteCxrApplDataRest.cxrList.end(),
                          oadResponseData.fareByteCxrAppl.cxrList.begin(),
                          oadResponseData.fareByteCxrAppl.cxrList.end(),
                          std::inserter(mergedRestrictedCxrAppl, mergedRestrictedCxrAppl.begin()));
  }
  else
  {
    if (fareByteCxrApplDataRest.excluded)
    {
      std::set_difference(oadResponseData.fareByteCxrAppl.cxrList.begin(),
                          oadResponseData.fareByteCxrAppl.cxrList.end(),
                          fareByteCxrApplDataRest.cxrList.begin(),
                          fareByteCxrApplDataRest.cxrList.end(),
                          std::inserter(mergedRestrictedCxrAppl, mergedRestrictedCxrAppl.begin()));
    }
    else
    {
      std::set_difference(fareByteCxrApplDataRest.cxrList.begin(),
                          fareByteCxrApplDataRest.cxrList.end(),
                          oadResponseData.fareByteCxrAppl.cxrList.begin(),
                          oadResponseData.fareByteCxrAppl.cxrList.end(),
                          std::inserter(mergedRestrictedCxrAppl, mergedRestrictedCxrAppl.begin()));
    }
    fareByteCxrApplDataRest.excluded = false;
  }
  fareByteCxrApplDataRest.cxrList.swap(mergedRestrictedCxrAppl);

  if (isPrefGovCxrForcedByFirstBreakRest(fareMarket, oadResponseData))
  {
    addGovCxr(fareByteCxrApplDataRest, fareMarket->governingCarrier());
  }
}

void
RexConstrainsConsolidator::printItin(const Itin& itin)
{
  LocCode prevDestAirport;
  std::vector<TravelSeg*>::const_iterator tvlIter = itin.travelSeg().begin();
  for (; tvlIter != itin.travelSeg().end(); ++tvlIter)
  {
    TravelSeg* tvlS = *tvlIter;

    if (prevDestAirport != tvlS->origAirport())
    {
      *_dc << "                     " << tvlS->origAirport() << " " << tvlS->boardMultiCity()
           << std::endl;
    }

    ArunkSeg* arunkS = dynamic_cast<ArunkSeg*>(tvlS);
    if (!arunkS)
    {
      if (tvlS->stopOver() || tvlS->isForcedStopOver())
        *_dc << "O ";
      else
        *_dc << "X ";
    }
    else
    {
      *_dc << "  ";
    }

    AirSeg* airS = dynamic_cast<AirSeg*>(tvlS);
    if (airS)
    {
      *_dc << std::setw(4) << airS->carrier() << " " << std::setw(4) << airS->flightNumber() << " ";
    }
    else
    {
      *_dc << "          ";
    }

    if (arunkS)
      *_dc << "      ";
    else
      *_dc << tvlS->departureDT().dateToString(DDMMM, "") << " ";

    *_dc << std::setw(3) << tvlS->equipmentType() << " ";

    *_dc << tvlS->destAirport() << " " << tvlS->offMultiCity() << " ";

    *_dc << tvlS->getBookingCode() << std::endl;

    prevDestAirport = tvlS->destAirport();
  }
  *_dc << std::endl;
}

void
RexConstrainsConsolidator::mergeOADCrxListWithPSSCrxList(
    RexShoppingTrx::SODConstraint& sodConstraint)
{
  if (sodConstraint.fareByteCxrAppl == nullptr)
  {
    _trx.dataHandle().get(sodConstraint.fareByteCxrAppl);
  }
  RexShoppingTrx::FareByteCxrApplData& cxrListFromPSS = _trx.cxrListFromPSS();
  RexShoppingTrx::FareByteCxrApplData& fareByteGovCxrAppl = sodConstraint.fareByteCxrAppl->govCxr;
  RexShoppingTrx::FareByteCxrApplData& fareByteUsrCxrAppl = sodConstraint.fareByteCxrAppl->usrCxr;

  if (!fareByteGovCxrAppl.cxrList.empty() && fareByteGovCxrAppl.excluded)
  {
    if (cxrListFromPSS.excluded || cxrListFromPSS.cxrList.empty())
    {
      std::set<CarrierCode> diff;
      std::set_difference(fareByteGovCxrAppl.cxrList.begin(),
                          fareByteGovCxrAppl.cxrList.end(),
                          cxrListFromPSS.cxrList.begin(),
                          cxrListFromPSS.cxrList.end(),
                          std::inserter(diff, diff.begin()));
      fareByteGovCxrAppl.cxrList.swap(diff);
      fareByteGovCxrAppl.excluded = true;
    }
  }
  fareByteUsrCxrAppl.cxrList.insert(cxrListFromPSS.cxrList.begin(), cxrListFromPSS.cxrList.end());
  fareByteUsrCxrAppl.excluded = cxrListFromPSS.excluded;
}

void
RexConstrainsConsolidator::printPSSCxrList()
{
  const RexShoppingTrx::FareByteCxrApplData& cxrListFromPSS = _trx.cxrListFromPSS();
  if (cxrListFromPSS.cxrList.empty())
  {
    *_dc << "\n\nPSS CARRIER LIST IS EMPTY\n";
  }
  else
  {
    *_dc << "\n\nPSS CARRIER LIST:";
    *_dc << "\n " << (cxrListFromPSS.excluded ? "RESTRICTED" : "APPLICABLE") << " CXR: ";
    copy(cxrListFromPSS.cxrList.begin(),
         cxrListFromPSS.cxrList.end(),
         std::ostream_iterator<CarrierCode>(*_dc, " "));
  }
}

void
RexConstrainsConsolidator::addGovCxr(RexShoppingTrx::FareByteCxrApplData& fareByteCxrApplDataRest,
                                     const CarrierCode& governingCarrier)
{
  if (fareByteCxrApplDataRest.cxrList.empty())
  {
    fareByteCxrApplDataRest.cxrList.insert(governingCarrier);
    fareByteCxrApplDataRest.excluded = false;
  }
  else
  {
    if (!fareByteCxrApplDataRest.excluded)
    {
      fareByteCxrApplDataRest.cxrList.insert(governingCarrier);
    }
  }
}

void
RexConstrainsConsolidator::calculatePlusMinusDateShift()
{
  std::vector<TravelSeg*>& newTravelSeg = _newItinSeg;
  std::vector<TravelSeg*>::iterator frontSeg;
  std::vector<TravelSeg*>::iterator backSeg;
  std::vector<TravelSeg*>::iterator lastSeg = _newItinSeg.begin();

  if (_dc)
    *_dc << "\nPLUS/MINUS DAY SHIFT FOR NEW OND:\n";

  std::vector<PricingTrx::OriginDestination>::const_iterator fmIter = _trx.originDest().begin();
  for (; fmIter != _trx.originDest().end(); ++fmIter)
  {
    getNextSegForOND(*fmIter, lastSeg, newTravelSeg, frontSeg, backSeg);
    if (frontSeg != newTravelSeg.end() && backSeg != newTravelSeg.end())
    {
      if ((*frontSeg)->unflown())
      {
        _plusMinusDateShift[&(*fmIter)] =
            fmIter->travelDate.date() - (*frontSeg)->departureDT().date();
        if (_dc)
        {
          *_dc << " " << fmIter->boardMultiCity << "-" << fmIter->offMultiCity << " "
               << fmIter->travelDate.date() << " WITH OFFSET "
               << _plusMinusDateShift[&(*fmIter)].days() << "\n";
        }
      }
      lastSeg = backSeg + 1;
    }
    else
    {
      return;
    }
  }
}

void
RexConstrainsConsolidator::getNextSegForOND(const PricingTrx::OriginDestination& odThruFM,
                                            std::vector<TravelSeg*>::iterator& lastSeg,
                                            std::vector<TravelSeg*>& newTravelSeg,
                                            std::vector<TravelSeg*>::iterator& frontSeg,
                                            std::vector<TravelSeg*>::iterator& backSeg)
{
  frontSeg = newTravelSeg.end();
  backSeg = newTravelSeg.end();
  std::vector<TravelSeg*>::iterator ondOrigAirport;
  std::vector<TravelSeg*>::iterator ondDestAirport;
  for (ondDestAirport = lastSeg; ondDestAirport != newTravelSeg.end(); ++ondDestAirport)
  {
    if ((*ondDestAirport)->destAirport() != odThruFM.offMultiCity &&
        !isMutliAirportCityTheSame((*ondDestAirport)->offMultiCity(), odThruFM.offMultiCity))
      continue;
    for (ondOrigAirport = lastSeg; ondOrigAirport != newTravelSeg.end(); ++ondOrigAirport)
    {
      if ((*ondOrigAirport)->origAirport() != odThruFM.boardMultiCity &&
          !isMutliAirportCityTheSame((*ondOrigAirport)->boardMultiCity(), odThruFM.boardMultiCity))
        continue;
      if (ondDestAirport - ondOrigAirport >= 0)
      {
        frontSeg = ondOrigAirport;
        backSeg = ondDestAirport;
        return;
      }
    }
  }
}

bool
RexConstrainsConsolidator::isTravelDateTheSame(const TravelSeg* tvlSeg,
                                               const PricingTrx::OriginDestination& odThruFM)
{
  if (tvlSeg->unflown() && _plusMinusDateShift.count(&odThruFM) != 0 &&
      _plusMinusDateShift[&odThruFM].days() != 0)
  {
    boost::gregorian::date_duration plusMinusDateShift = _plusMinusDateShift[&odThruFM];
    DateTime tvlSegDT = tvlSeg->departureDT();
    DateTime tvlSegDTWithDateShift;
    if (plusMinusDateShift.days() > 0)
    {
      tvlSegDTWithDateShift = tvlSegDT.addDays(plusMinusDateShift.days());
    }
    else
    {
      tvlSegDTWithDateShift = tvlSegDT.subtractDays(-plusMinusDateShift.days());
    }
    return tvlSegDTWithDateShift.get64BitRepDateOnly() == odThruFM.travelDate.get64BitRepDateOnly();
  }
  else
  {
    return tvlSeg->departureDT().get64BitRepDateOnly() == odThruFM.travelDate.get64BitRepDateOnly();
  }
}

int
RexConstrainsConsolidator::segmentOrder(const std::vector<TravelSeg*>& tvlSegVect,
                                        const TravelSeg* segment) const
{
  std::vector<TravelSeg*>::const_iterator iter = tvlSegVect.begin();
  std::vector<TravelSeg*>::const_iterator iterEnd = tvlSegVect.end();

  int segOrd = 1;
  for (; iter != iterEnd; iter++, ++segOrd)
  {
    if (*iter == segment)
    {
      return segOrd;
    }
  }
  return 0;
}

bool
RexConstrainsConsolidator::isMutliAirportCityTheSame(const LocCode& multiAirportCity,
                                                     const LocCode& ondMultiAirportCity) const
{
  return LocUtil::isSameCity(multiAirportCity, ondMultiAirportCity, _trx.dataHandle());
}

void
RexConstrainsConsolidator::copySegWithoutARUNKs()
{
  std::vector<ExcItin*>& vExcItin = _trx.exchangeItin();
  ExcItin* excItin = vExcItin.front();
  std::vector<TravelSeg*>& tSegs = excItin->travelSeg();

  std::vector<TravelSeg*>::const_iterator excTravelSeg = tSegs.begin();
  for (; excTravelSeg != _trx.exchangeItin().front()->travelSeg().end(); ++excTravelSeg)
  {
    if ((*excTravelSeg)->segmentType() != Arunk)
    {
      _excItinSeg.push_back(*excTravelSeg);
    }
  }

  std::vector<TravelSeg*>::const_iterator newTravelSeg =
      _trx.newItin().front()->travelSeg().begin();
  for (; newTravelSeg != _trx.newItin().front()->travelSeg().end(); ++newTravelSeg)
  {
    if ((*newTravelSeg)->segmentType() != Arunk)
    {
      _newItinSeg.push_back(*newTravelSeg);
    }
  }
}

void
RexConstrainsConsolidator::createFmWithFlownGovSet(std::vector<const FareMarket*>& fmWithFlownGov)
{
  RexShoppingTrx::OADConsolidatedConstrainsVect::const_iterator oadConsRest =
      _trx.oadConsRest().begin();
  for (; oadConsRest != _trx.oadConsRest().end(); ++oadConsRest)
  {
    RexShoppingTrx::OADConsolidatedConstrains& oadConsolConstr = **oadConsRest;

    int firstONDInfoSegPos = oadConsolConstr.firstONDInfoSegPos;
    const int lastONDInfoSegPos = oadConsolConstr.lastONDInfoSegPos;

    if (!oadConsolConstr.unflown)
    {
      int firstONDExcTvlSegPos = 0;
      int lastONDExcTvlSegPos = 0;

      // match flown newTvlSeg with flown excTvlSeg;
      if (getFlownExcONDTvlSegPos(
              firstONDInfoSegPos, lastONDInfoSegPos, firstONDExcTvlSegPos, lastONDExcTvlSegPos))
      {
        const FareMarket* fareMarket = findFMforOND(firstONDExcTvlSegPos, lastONDExcTvlSegPos);
        if (fareMarket && ondHasPrimarySectorInExcFlownPart(
                              fareMarket, firstONDExcTvlSegPos, lastONDExcTvlSegPos))
        {
          fmWithFlownGov.push_back(fareMarket);
        }
      }
    }
  }
}

bool
RexConstrainsConsolidator::getFlownExcONDTvlSegPos(const int firstONDNewTvlSegPos,
                                                   const int lastONDNewTvlSegPos,
                                                   int& firstONDExcTvlSegPos,
                                                   int& lastONDExcTvlSegPos)
{
  const std::vector<TravelSeg*>& newTvlSegV = _newItinSeg;
  const std::vector<TravelSeg*>& excTvlSegV = _excItinSeg;

  if (!newTvlSegV.empty() && !excTvlSegV.empty() && firstONDNewTvlSegPos > 0 &&
      firstONDNewTvlSegPos <= int(newTvlSegV.size()) && lastONDNewTvlSegPos > 0 &&
      lastONDNewTvlSegPos <= int(newTvlSegV.size()))
  {
    const TravelSeg* firstONDNewTvlSeg = newTvlSegV[firstONDNewTvlSegPos - 1];
    const TravelSeg* lastONDNewTvlSeg = newTvlSegV[lastONDNewTvlSegPos - 1];
    std::vector<TravelSeg*>::const_iterator ondOrigAirport;
    std::vector<TravelSeg*>::const_iterator ondDestAirport;
    for (lastONDExcTvlSegPos = 0, ondDestAirport = excTvlSegV.begin();
         ondDestAirport != excTvlSegV.end();
         ++ondDestAirport, ++lastONDExcTvlSegPos)
    {
      if ((*ondDestAirport)->destAirport() != lastONDNewTvlSeg->destAirport() &&
          !isMutliAirportCityTheSame((*ondDestAirport)->offMultiCity(),
                                     lastONDNewTvlSeg->offMultiCity()))
        continue;
      for (firstONDExcTvlSegPos = 0, ondOrigAirport = excTvlSegV.begin();
           ondOrigAirport != excTvlSegV.end();
           ++ondOrigAirport, ++firstONDExcTvlSegPos)
      {
        if ((*ondOrigAirport)->origAirport() != firstONDNewTvlSeg->origAirport() &&
            !isMutliAirportCityTheSame((*ondOrigAirport)->boardMultiCity(),
                                       firstONDNewTvlSeg->boardMultiCity()))
          continue;
        if (ondDestAirport - ondOrigAirport >= 0)
        {
          if ((*ondOrigAirport)->departureDT().get64BitRepDateOnly() ==
              firstONDNewTvlSeg->departureDT().get64BitRepDateOnly())
          {
            return true;
          }
        }
      }
    }
  }

  return false;
}

bool
RexConstrainsConsolidator::ondHasPrimarySectorInExcFlownPart(const FareMarket* fareMarket,
                                                             const int firstONDExcTvlSegPos,
                                                             const int lastONDExcTvlSegPos)
{
  if (fareMarket)
  {
    const std::vector<TravelSeg*>& excTvlSegV = _excItinSeg;
    for (int curONDexcSegPos = firstONDExcTvlSegPos; curONDexcSegPos <= lastONDExcTvlSegPos;
         curONDexcSegPos++)
    {
      if (fareMarket->primarySector() == excTvlSegV[curONDexcSegPos])
      {
        return true;
      }
    }
  }
  return false;
}

const FareMarket*
RexConstrainsConsolidator::findFMforOND(const int firstONDExcTvlSegPos,
                                        const int lastONDExcTvlSegPos)
{
  const std::vector<TravelSeg*>& excTvlSegV = _excItinSeg;
  if (!excTvlSegV.empty() && firstONDExcTvlSegPos >= 0 &&
      firstONDExcTvlSegPos < int(excTvlSegV.size()) && lastONDExcTvlSegPos >= 0 &&
      lastONDExcTvlSegPos < int(excTvlSegV.size()))
  {
    for (int curONDexcSegPos = firstONDExcTvlSegPos; curONDexcSegPos <= lastONDExcTvlSegPos;
         curONDexcSegPos++)
    {
      const FareMarket* fareMarket = findFMforSegment(excTvlSegV[curONDexcSegPos]);
      if (fareMarket)
        return fareMarket;
    }
  }
  return nullptr;
}

const FareMarket*
RexConstrainsConsolidator::findFMforSegment(const TravelSeg* tvlSeg)
{
  std::vector<FareCompInfo*>::const_iterator fareCompInfoIter;
  for (fareCompInfoIter = _trx.exchangeItin().front()->fareComponent().begin();
       fareCompInfoIter != _trx.exchangeItin().front()->fareComponent().end();
       fareCompInfoIter++)
  {
    const FareMarket* fm = (*fareCompInfoIter)->fareMarket();
    std::vector<TravelSeg*>::const_iterator segIter = fm->travelSeg().begin();
    for (; segIter != fm->travelSeg().end(); ++segIter)
    {
      if (*segIter == tvlSeg)
        return fm;
    }
  }

  return nullptr;
}

bool
RexConstrainsConsolidator::isONDinExcFMWithFlownGov(
    std::vector<const FareMarket*>& fmWithFlownGov,
    const RexShoppingTrx::OADConsolidatedConstrains* oadConsRest)
{
  if (_constToONDInfo.count(oadConsRest) != 0)
  {
    const ONDInf* ondInfo = _constToONDInfo[oadConsRest];
    if (_ondInfWithSplittedFMUnflown && _ondInfWithSplittedFMFlown && ondInfo->unflown &&
        _ondInfWithSplittedFMUnflown == ondInfo)
    {
      if (_ondInfoToFM.count(_ondInfWithSplittedFMFlown) != 0)
      {
        std::vector<const FareMarket*>& fmVect = _ondInfoToFM[_ondInfWithSplittedFMFlown];
        std::vector<const FareMarket*>::const_iterator fmIter = fmVect.begin();
        for (; fmIter != fmVect.end(); ++fmIter)
        {
          if (std::find(fmWithFlownGov.begin(), fmWithFlownGov.end(), *fmIter) !=
              fmWithFlownGov.end())
          {
            return true;
          }
        }
      }
    }
    else if (_trx.exchangeItin().front()->fareComponent().size() == fmWithFlownGov.size() &&
             ondInfo->unflown)
      return true;
  }
  return false;
}

void
RexConstrainsConsolidator::printFmWithFlownGovSet(std::vector<const FareMarket*>& fmWithFlownGov)
{
  if (fmWithFlownGov.empty())
  {
    *_dc << "\n\nFLOWN FM WITH GOVERNING CARRIER IS EMPTY\n";
  }
  else
  {
    *_dc << "\n\nFLOWN FM WITH GOVERNING CARRIER:";
    std::vector<const FareMarket*>::const_iterator fmWithFlownGovIter = fmWithFlownGov.begin();
    for (; fmWithFlownGovIter != fmWithFlownGov.end(); ++fmWithFlownGovIter)
    {
      *_dc << "\n FARE MARKET: " << (*fmWithFlownGovIter)->origin()->loc() << "-"
           << (*fmWithFlownGovIter)->destination()->loc();
    }
  }
}

void
RexConstrainsConsolidator::setGovCxr(RexShoppingTrx::FareByteCxrData*& sodCarriers,
                                     const RexShoppingTrx::FareByteCxrData* ondCarriers)
{
  if (ondCarriers)
  {
    const RexShoppingTrx::FareByteCxrApplData& fareByteGovCxrAppl = ondCarriers->govCxr;
    const RexShoppingTrx::FareByteCxrApplData& fareByteUsrCxrAppl = ondCarriers->usrCxr;

    _trx.dataHandle().get(sodCarriers);

    if (fareByteGovCxrAppl.excluded == false && !fareByteGovCxrAppl.cxrList.empty())
    {
      if (fareByteUsrCxrAppl.excluded == false)
      {
        std::set<CarrierCode> inter;
        std::set_intersection(fareByteUsrCxrAppl.cxrList.begin(),
                              fareByteUsrCxrAppl.cxrList.end(),
                              fareByteGovCxrAppl.cxrList.begin(),
                              fareByteGovCxrAppl.cxrList.end(),
                              std::inserter(inter, inter.begin()));
        if (inter.empty())
        {
          _trx.setShopOnlyCurrentItin(true);
        }
      }
      else if (fareByteUsrCxrAppl.excluded == true)
      {
        std::set<CarrierCode> diff;
        std::set_difference(fareByteGovCxrAppl.cxrList.begin(),
                            fareByteGovCxrAppl.cxrList.end(),
                            fareByteUsrCxrAppl.cxrList.begin(),
                            fareByteUsrCxrAppl.cxrList.end(),
                            std::inserter(diff, diff.begin()));
        if (diff.empty())
        {
          _trx.setShopOnlyCurrentItin(true);
        }
      }
    }

    sodCarriers->govCxr = ondCarriers->govCxr;
  }
}

void
RexConstrainsConsolidator::setUsrCxr(RexShoppingTrx::FareByteCxrData*& sodCarriers,
                                     const RexShoppingTrx::FareByteCxrData* ondCarriers)
{
  if (ondCarriers)
  {
    if (sodCarriers == nullptr)
      _trx.dataHandle().get(sodCarriers);

    sodCarriers->usrCxr = ondCarriers->usrCxr;
  }
}

void
RexConstrainsConsolidator::calculateUnmappedFC(ExcSegPosToFMMap& excSegPosToFM)
{
  if (routingHasChanged())
  {
    matchFMsToONDInfoWithoutNoMatched(excSegPosToFM);

    createOndInfoNotMatched();

    createFmNotMatched();

    _addCxrRestToUnmappedONDInfos = !doesAnyFlownFCHasSameCxrAsUnmappedFC();
  }
}

bool
RexConstrainsConsolidator::routingHasChanged()
{
  std::vector<TravelSeg*>::const_iterator excItinSeg = _excItinSeg.begin();
  std::vector<TravelSeg*>::const_iterator newItinSeg = _newItinSeg.begin();

  bool routinHasChanged = false;
  for (; excItinSeg != _excItinSeg.end() && newItinSeg != _newItinSeg.end();
       ++excItinSeg, ++newItinSeg)
  {
    if ((*excItinSeg)->destAirport() != (*newItinSeg)->destAirport() ||
        !isMutliAirportCityTheSame((*excItinSeg)->offMultiCity(), (*newItinSeg)->offMultiCity()) ||
        (*excItinSeg)->origAirport() != (*newItinSeg)->origAirport() ||
        !isMutliAirportCityTheSame((*excItinSeg)->boardMultiCity(),
                                   (*newItinSeg)->boardMultiCity()))
    {
      routinHasChanged = true;
      break;
    }
  }

  return routinHasChanged;
}

void
RexConstrainsConsolidator::matchFMsToONDInfoWithoutNoMatched(ExcSegPosToFMMap& excSegPosToFM)
{
  ONDInfoToExcSegPosSetMap::const_iterator ondInfoToExcSegPosSetIter =
      _ondInfoToExcSegPosWithoutNoMatchedSet.begin();
  for (; ondInfoToExcSegPosSetIter != _ondInfoToExcSegPosWithoutNoMatchedSet.end();
       ++ondInfoToExcSegPosSetIter)
  {
    const ONDInf* ondInfo = ondInfoToExcSegPosSetIter->first;
    std::vector<int>::const_iterator sopPosIter = ondInfoToExcSegPosSetIter->second.begin();

    for (; sopPosIter != ondInfoToExcSegPosSetIter->second.end(); ++sopPosIter)
    {
      if (excSegPosToFM.count(*sopPosIter) != 0)
      {
        _ondInfoToFMWithoutNoMatched[ondInfo].push_back(excSegPosToFM[*sopPosIter]);
      }
    }
  }

  removeDuplicates(_ondInfoToFMWithoutNoMatched);

  if (_dc && _trx.diagnostic().diagParamMapItem("DD") == "DETAILS")
  {
    if (_ondInfoToFMWithoutNoMatched.empty())
    {
      *_dc << "\n\nOND INFO TO FM WITHOUT NO MATCHED IS EMPTY";
    }
    else
    {
      *_dc << "\n\nOND INFO TO FM WITHOUT NO MATCHED:";
      ONDInfoToFM::const_iterator ondInfoToFMIter = _ondInfoToFMWithoutNoMatched.begin();
      for (; ondInfoToFMIter != _ondInfoToFMWithoutNoMatched.end(); ++ondInfoToFMIter)
      {
        const ONDInf* ondInf = ondInfoToFMIter->first;
        *_dc << "\n OND INFO: " << ondInf->origAirport << "-" << ondInf->destAirport << " "
             << ondInf->travelDate;
        std::vector<const FareMarket*>::const_iterator fmIter = ondInfoToFMIter->second.begin();
        for (; fmIter != ondInfoToFMIter->second.end(); ++fmIter)
        {
          *_dc << "\n  FARE MARKET: " << (*fmIter)->origin()->loc() << "-"
               << (*fmIter)->destination()->loc();
        }
      }
    }
  }
}

void
RexConstrainsConsolidator::createOndInfoNotMatched()
{
  std::vector<ONDInf*>::const_iterator ondInfoIter = _ondInfo.begin();
  for (; ondInfoIter != _ondInfo.end(); ++ondInfoIter)
  {
    const ONDInf* ondInfMain = *ondInfoIter;
    bool found = false;
    ONDInfoToFM::const_iterator ondInfoToFMIter = _ondInfoToFMWithoutNoMatched.begin();
    for (; ondInfoToFMIter != _ondInfoToFMWithoutNoMatched.end(); ++ondInfoToFMIter)
    {
      const ONDInf* ondInf = ondInfoToFMIter->first;
      if (ondInf == ondInfMain)
      {
        found = true;
        break;
      }
    }

    if (!found)
      _ondInfoNotMatched.insert(ondInfMain);

    if (!ondInfMain->unflown)
    {
      if (!_ondInfoToFMWithoutNoMatched[ondInfMain].empty())
      {
        const FareMarket* fm = *(_ondInfoToFMWithoutNoMatched[ondInfMain].begin());
        if (!isMutliAirportCityTheSame(fm->boardMultiCity(), ondInfMain->origAirport) ||
            !isMutliAirportCityTheSame(fm->offMultiCity(), ondInfMain->destAirport))
        {
          _fmNotMatched.push_back(fm);
        }
      }
    }
  }
}

void
RexConstrainsConsolidator::createFmNotMatched()
{
  std::vector<FareCompInfo*>::const_iterator fareCompInfoIter;
  for (fareCompInfoIter = _trx.exchangeItin().front()->fareComponent().begin();
       fareCompInfoIter != _trx.exchangeItin().front()->fareComponent().end();
       fareCompInfoIter++)
  {
    const FareMarket* fm = (*fareCompInfoIter)->fareMarket();

    bool found = false;
    ONDInfoToFM::const_iterator ondInfoToFMIter = _ondInfoToFMWithoutNoMatched.begin();
    for (; ondInfoToFMIter != _ondInfoToFMWithoutNoMatched.end(); ++ondInfoToFMIter)
    {
      std::vector<const FareMarket*> fmVec = ondInfoToFMIter->second;
      if (std::find(fmVec.begin(), fmVec.end(), fm) != fmVec.end())
      {
        found = true;
        break;
      }
    }

    if (!found)
      _fmNotMatched.push_back(fm);
  }
}

bool
RexConstrainsConsolidator::doesAnyFlownFCHasSameCxrAsUnmappedFC()
{
  ONDInfoToFM::const_iterator ondInfoToFMIter = _ondInfoToFMWithoutNoMatched.begin();
  for (; ondInfoToFMIter != _ondInfoToFMWithoutNoMatched.end(); ++ondInfoToFMIter)
  {
    std::vector<const FareMarket*> fmSet = ondInfoToFMIter->second;
    std::vector<const FareMarket*>::const_iterator fmSetIter = fmSet.begin();
    for (; fmSetIter != fmSet.end(); ++fmSetIter)
    {
      std::vector<const FareMarket*>::const_iterator fmIter = _fmNotMatched.begin();
      for (; fmIter != _fmNotMatched.end(); ++fmIter)
      {
        if ((*fmIter)->governingCarrier() == (*fmSetIter)->governingCarrier())
          return true;
      }
    }
  }

  return false;
}

void
RexConstrainsConsolidator::printONDInfoNotMatched()
{
  if (_ondInfoNotMatched.empty())
  {
    *_dc << "\n\nOND INFO NOT MATCHED IS EMPTY";
  }
  else
  {
    *_dc << "\n\nOND INFO NOT MATCHED:";
    std::set<const ONDInf*>::const_iterator ondInfoIter = _ondInfoNotMatched.begin();
    for (; ondInfoIter != _ondInfoNotMatched.end(); ++ondInfoIter)
    {
      const ONDInf* ondInf = *ondInfoIter;
      *_dc << "\n OND INFO: " << ondInf->origAirport << "-" << ondInf->destAirport << " "
           << ondInf->travelDate;
    }
  }
}

void
RexConstrainsConsolidator::printFMNotMatched()
{
  if (_fmNotMatched.empty())
  {
    *_dc << "\n\nFM NOT MATCHED IS EMPTY";
  }
  else
  {
    *_dc << "\n\nFM NOT MATCHED:";
    std::vector<const FareMarket*>::const_iterator fmIter = _fmNotMatched.begin();
    for (; fmIter != _fmNotMatched.end(); ++fmIter)
    {
      *_dc << "\n  FARE MARKET: " << (*fmIter)->origin()->loc() << "-"
           << (*fmIter)->destination()->loc();
    }
  }
}

void
RexConstrainsConsolidator::printOADConsRest(
    RexShoppingTrx::OADConsolidatedConstrains& oadConsolConstr, std::string reason)
{
  *_dc << "\n\nOAD WITHOUT CXR REST: " << oadConsolConstr.origAirport << "-"
       << oadConsolConstr.destAirport << " " << oadConsolConstr.travelDate << " - " << reason;
}

void
RexConstrainsConsolidator::removeDuplicates(
    std::map<const ONDInf*, std::vector<const FareMarket*> >& ondMap)
{
  typedef std::map<const ONDInf*, std::vector<const FareMarket*> >::value_type Iterator;

  for (Iterator& iter : ondMap)
  {
    std::vector<const FareMarket*>::iterator newEnd;
    newEnd = std::unique(iter.second.begin(), iter.second.end());
    iter.second.erase(newEnd, iter.second.end());
  }
}
}
