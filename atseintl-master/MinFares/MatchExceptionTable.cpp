//----------------------------------------------------------------------------
//
//
//  File:     MatchExceptionTable.cpp
//  Created:  8/20/2004
//  Authors:  Quan Ta
//
//  Description:
//          Function object to match Minimum Fare Application Table.
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

#include "MinFares/MatchExceptionTable.h"
#include "Common/FallbackUtil.h"
#include "Common/DateTime.h"
#include "Common/DiagMonitor.h"
#include "Common/FallbackUtil.h"
#include "Common/LocUtil.h"
#include "Common/TrxUtil.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"
#include "DBAccess/MinFareAppl.h"
#include "MinFares/MatchDefaultLogicTable.h"
#include "MinFares/MinFareFareSelection.h"
#include "MinFares/MinFareLogic.h"
#include "MinFares/MinimumFare.h"
#include "Rules/RuleUtil.h"

namespace tse
{

MatchExceptionTable::MatchExceptionTable(MinimumFareModule module,
                                         PricingTrx& trx,
                                         const FarePath& farePath,
                                         const Itin& itin,
                                         const PaxTypeFare& paxTypeFare,
                                         const DateTime& tvlDate,
                                         const CarrierCode& govCxr)
  : _module(module),
    _trx(trx),
    _farePath(farePath),
    _itin(itin),
    _paxTypeFare(paxTypeFare),
    _tvlDate(tvlDate),
    _govCxr(govCxr),
    _diag(nullptr),
    _diagEnabled(false),
    _fareForCat17(&paxTypeFare),
    _compCat17ItemOnly(false),
    _matchedApplItem(nullptr),
    _matchedDefaultItem(nullptr)
{
}

MatchExceptionTable::~MatchExceptionTable() {}

bool
MatchExceptionTable::
operator()()
{
  DiagMonitor diagMonitor(_trx, Diagnostic709);
  _diag = &diagMonitor.diag();
  _diagEnabled = (_diag && _diag->isActive());

  int32_t table996No = RuleUtil::getCat17Table996ItemNo(_trx, _paxTypeFare, _fareForCat17);
  if (UNLIKELY(_fareForCat17 == nullptr))
    return false;

  if (_fareForCat17 != &_paxTypeFare) // Cat17 from base fare
  {
    _compCat17ItemOnly = true;

    if (_diagEnabled)
    {
      *_diag << " \n************* " << MinFareFareSelection::MinFareModuleName[_module]
             << " MINIMUM FARE APPLICATION TABLE *************\n"
             << "CAT 17 FROM BASE FARE\n"
             << "VENDOR/" << _fareForCat17->vendor() << " TBL996/" << table996No << " CXR/"
             << _govCxr << " \n";

      if (_trx.excTrxType() == PricingTrx::AR_EXC_TRX)
        MinimumFare::printRexDatesAdjustment(*_diag, _trx.dataHandle().ticketDate());

      MinimumFare::printFareInfo(*_fareForCat17, *_diag, _module);
    }

    const std::vector<MinFareAppl*>& applTable =
        _trx.dataHandle().getMinFareAppl(_fareForCat17->vendor(), table996No, _govCxr, _tvlDate);

    std::vector<MinFareAppl*>::const_iterator applIter =
        find_if(applTable.begin(), applTable.end(), *this);

    if (applIter != applTable.end())
    {
      _matchedApplItem = *applIter;
      if (_matchedApplItem)
      {
        if (MinFareLogic::getMinFareApplIndicator(_module, *_matchedApplItem) == YES)
        {
          if (_matchedApplItem->applyDefaultLogic() == YES)
          {
            MatchDefaultLogicTable matchDefaultLogic(_module, _trx, _itin, *_fareForCat17);
            if (matchDefaultLogic())
            {
              _matchedDefaultItem = matchDefaultLogic.matchedDefaultItem();
              return true;
            }
            else
            {
              // No match default item found - log as error:
              LOG4CXX_ERROR(log4cxx::Logger::getLogger("atseintl.MinFares.MinFareAppl"),
                            "No matched MinFareDefaultLogic table item for "
                                << "VENDOR/" << _fareForCat17->vendor() << " CXR/" << _govCxr);
            }
          }
        }
        return true;
      }
    }
    else
    {
      // Reset table996No
      table996No = 0;
    }
  }

  // for Original fare
  _compCat17ItemOnly = false;

  const std::vector<MinFareAppl*>& applTable =
      _trx.dataHandle().getMinFareAppl(_paxTypeFare.vendor(), table996No, _govCxr, _tvlDate);

  if (UNLIKELY(_diagEnabled))
  {
    *_diag << " \n************* " << MinFareFareSelection::MinFareModuleName[_module]
           << " MINIMUM FARE APPLICATION TABLE *************\n"
           << "VENDOR/" << _paxTypeFare.vendor() << " TBL996/" << table996No << " CXR/" << _govCxr
           << " \n";

    if (_trx.excTrxType() == PricingTrx::AR_EXC_TRX)
      MinimumFare::printRexDatesAdjustment(*_diag, _trx.dataHandle().ticketDate());

    MinimumFare::printFareInfo(_paxTypeFare, *_diag, _module);
  }

  std::vector<MinFareAppl*>::const_iterator applIter =
      find_if(applTable.begin(), applTable.end(), *this);

  if (LIKELY(applIter != applTable.end()))
  {
    _matchedApplItem = *applIter;
    if (LIKELY(_matchedApplItem))
    {
      if (MinFareLogic::getMinFareApplIndicator(_module, *_matchedApplItem) == YES)
      {
        if (_matchedApplItem->applyDefaultLogic() == YES)
        {
          MatchDefaultLogicTable matchDefaultLogic(_module, _trx, _itin, _paxTypeFare);
          if (LIKELY(matchDefaultLogic()))
          {
            _matchedDefaultItem = matchDefaultLogic.matchedDefaultItem();
            return true;
          }
          else
          {
            // No match default item found - log as error:
            LOG4CXX_ERROR(log4cxx::Logger::getLogger("atseintl.MinFares.MinFareAppl"),
                          "No matched MinFareDefaultLogic table item for "
                              << "VENDOR/" << _paxTypeFare.vendor() << " CXR/" << _govCxr);
          }
        }
      }
      return true;
    }
  }
  else
  {
    // No match found - log as error:
    LOG4CXX_ERROR(log4cxx::Logger::getLogger("atseintl.MinFares.MinFareAppl"),
                  "No matched MinFareAppl table item for "
                      << "VENDOR/" << _paxTypeFare.vendor() << " TBL996/" << table996No << " CXR/"
                      << _govCxr);
  }

  return false;
}

bool
MatchExceptionTable::matchFareRule(const RuleNumber& sameFareGroupRN,
                                       const PaxTypeFare& paxTypeFare) const
{
  if (sameFareGroupRN.empty() || sameFareGroupRN == " " ||
      sameFareGroupRN == paxTypeFare.ruleNumber())
    return true;

  return false;
}

bool
MatchExceptionTable::matchFootnote(const Footnote& fn,
                                       const PaxTypeFare& paxTypeFare) const
{
  if (fn.empty() || fn == " " ||
      fn == paxTypeFare.footNote1() || fn == paxTypeFare.footNote2())
    return true;

  return false;
}

bool
MatchExceptionTable::
operator()(const MinFareAppl* appl) const
{
  const PaxTypeFare& paxTypeFare = (_compCat17ItemOnly ? *_fareForCat17 : _paxTypeFare);
  const FareMarket& fareMarket = *paxTypeFare.fareMarket();
  const MinFareAppl& application = *appl;

  if (UNLIKELY(_diagEnabled))
    *_diag << " \nGOVCXR/" << std::setw(2) << application.governingCarrier() << " SEQNO/"
           << application.seqNo() << " TEXTTBLVENDOR/" << application.textTblVendor()
           << " TEXTTBLITEMNO/" << application.textTblItemNo() << "\n";

  // Check if the min fare application indicator for the current module is blank
  if (MinFareLogic::getMinFareApplIndicator(_module, *appl) == MinimumFare::BLANK)
  {
    if (UNLIKELY(_diagEnabled))
      *_diag << MinFareFareSelection::MinFareModuleName[_module] << " APPL INDICATOR IS BLANK\n";
    return false;
  }

  // Check exception carrier
  const std::vector<CarrierCode>& cxrCodes = application.excptCxrs();
  if (UNLIKELY(std::find(cxrCodes.begin(), cxrCodes.end(), fareMarket.governingCarrier()) != cxrCodes.end()))
  {
    if (_diagEnabled)
      *_diag << "FARE - GOVERNING CXR MATCH EXCEPTION CXR " << fareMarket.governingCarrier()
             << '\n';
    return false;
  }

  // Check secondary carrier restriction
  const std::vector<CarrierCode>& secCxrCodes = application.secondaryCxrs();
  if (secCxrCodes.size() != 0)
  {
    std::vector<CarrierCode>::const_iterator cxrIter;
    cxrIter = std::find(secCxrCodes.begin(), secCxrCodes.end(), paxTypeFare.carrier());

    if ((cxrIter != secCxrCodes.end()) && (application.exceptSecondaryCxr() == YES))
    {
      if (_diagEnabled)
        *_diag << "FARE - SECONDARY CXR MATCH AND EXCEPT " << paxTypeFare.carrier() << '\n';
      return false;
    }
    else if ((cxrIter == secCxrCodes.end()) && (application.exceptSecondaryCxr() == NO))
    {
      if (_diagEnabled)
        *_diag << "FARE - SECONDARY CXR NOT MATCH " << paxTypeFare.carrier() << '\n';
      return false;
    }
  }

  // Check ISI code
  switch (_farePath.intlSaleIndicator())
  {
  case Itin::SITI:
    if (UNLIKELY(application.sitiInd() == YES))
    {
      if (_diagEnabled)
      {
        *_diag << "ISI CODE SITI - DO NOT APPLY /"
               << "SITI:" << application.sitiInd() << " SITO:" << application.sitoInd()
               << " SOTI:" << application.sotiInd() << " SOTO:" << application.sotoInd() << "/\n";
      }
      return false;
    }
    break;

  case Itin::SITO:
    if (application.sitoInd() == YES)
    {
      if (_diagEnabled)
      {
        *_diag << "ISI CODE SITO - DO NOT APPLY /"
               << "SITI:" << application.sitiInd() << " SITO:" << application.sitoInd()
               << " SOTI:" << application.sotiInd() << " SOTO:" << application.sotoInd() << "/\n";
      }
      return false;
    }
    break;

  case Itin::SOTI:
    if (application.sotiInd() == YES)
    {
      if (_diagEnabled)
      {
        *_diag << "ISI CODE SOTI - DO NOT APPLY /"
               << "SITI:" << application.sitiInd() << " SITO:" << application.sitoInd()
               << " SOTI:" << application.sotiInd() << " SOTO:" << application.sotoInd() << "/\n";
      }
      return false;
    }
    break;

  case Itin::SOTO:
    if (application.sotoInd() == YES)
    {
      if (_diagEnabled)
      {
        *_diag << "ISI CODE SOTO - DO NOT APPLY /"
               << "SITI:" << application.sitiInd() << " SITO:" << application.sitoInd()
               << " SOTI:" << application.sotiInd() << " SOTO:" << application.sotoInd() << "/\n";
      }
      return false;
    }
    break;

  default:
    break;
  }

  // Check point of sale
  const Loc* saleTicketLoc = TrxUtil::saleLoc(_trx);

  bool isiPoint = LocUtil::isInLoc(*saleTicketLoc,
                                   application.posLoc().locType(),
                                   application.posLoc().loc(),
                                   Vendor::SABRE,
                                   MANUAL,
                                   LocUtil::OTHER,
                                   GeoTravelType::International,
                                   EMPTY_STRING(),
                                   _trx.getRequest()->ticketingDT());

  if (UNLIKELY((isiPoint) && (application.posExceptInd() == YES)))
  {
    if (_diagEnabled)
      *_diag << "POINT OF SALE MATCH BUT EXCEPT" << '\n';
    return false;
  }

  if (!(isiPoint))
  {
    if (_diagEnabled)
      *_diag << "POINT OF SALE NOT MATCH " << '\n';
    return false;
  }

  // Check point of issue
  saleTicketLoc = TrxUtil::ticketingLoc(_trx);
  isiPoint = LocUtil::isInLoc(*saleTicketLoc,
                              application.poiLoc().locType(),
                              application.poiLoc().loc(),
                              Vendor::SABRE,
                              MANUAL,
                              LocUtil::OTHER,
                              GeoTravelType::International,
                              EMPTY_STRING(),
                              _trx.getRequest()->ticketingDT());

  if (UNLIKELY((isiPoint) && (application.poiExceptInd() == YES)))
  {
    if (_diagEnabled)
      *_diag << "POINT OF ISSUE MATCH BUT EXCEPT" << '\n';
    return false;
  }

  if (!(isiPoint))
  {
    if (_diagEnabled)
      *_diag << "POINT OF ISSUE NOT MATCH " << '\n';
    return false;
  }

  // Check Tariff Category
  if (application.tariffCat() != MinimumFare::ANY_TARIFF &&
      paxTypeFare.tcrTariffCat() != application.tariffCat())
  {
    if (_diagEnabled)
      *_diag << "FARE - TARIFF CATEGORY NOT MATCH " << paxTypeFare.tcrTariffCat() << '\n';
    return false;
  }

  // Check Rule Tariff (remove this matching after already match from dbaccess/cache)
  if (application.ruleTariff() != MinimumFare::ANY_TARIFF &&
      paxTypeFare.tcrRuleTariff() != application.ruleTariff())
  {
    if (UNLIKELY(_diagEnabled))
      *_diag << "FARE - RULE TARIFF NUMBER NOT MATCH " << paxTypeFare.tcrRuleTariff() << '\n';
    return false;
  }

  // Check Rule and Footnote
  const std::vector<std::pair<RuleNumber, Footnote> >& ruleFootnotes =
       application.ruleFootnotes();
  if (!(ruleFootnotes.empty()))
  {
    bool everMatch = false;
    for(const auto& element: ruleFootnotes)
    {
      if (matchFareRule(element.first, paxTypeFare) &&
                   matchFootnote(element.second, paxTypeFare))
      {
        everMatch = true;
        break;
      }
    }
    if (!everMatch)
    {
      if (UNLIKELY(_diagEnabled))
      {
        *_diag << "FARE - RULE NUMBER NOT MATCH " << paxTypeFare.ruleNumber();
        if (ruleFootnotes.size() > 0)
        {
          *_diag << " : ";
          for(const auto& elem: ruleFootnotes)
          {
            *_diag << elem.first << "  " << elem.second << std::endl;
          }
        }
        *_diag << '\n';
      }
	  return false;
    }
  }
  // Check Fare Class/Generic
  const std::vector<FareClassCode>& fareClasses = application.fareClasses();
  if (!(fareClasses.empty()))
  {
    std::vector<FareClassCode>::const_iterator fareClassIter = fareClasses.begin();
    for (; fareClassIter != fareClasses.end(); ++fareClassIter)
    {
      if ((*fareClassIter).empty() ||
          RuleUtil::matchFareClass(fareClassIter->c_str(), paxTypeFare.fareClass().c_str()))
        break;
    }
    if (fareClassIter == fareClasses.end())
    {
      if (_diagEnabled)
        *_diag << "FARE - FARE CLASS NOT MATCH " << paxTypeFare.fareClass() << ", REQ:";
      for (fareClassIter = fareClasses.begin(); fareClassIter != fareClasses.end(); ++fareClassIter)
      {
        *_diag << " " << *fareClassIter;
      }
      *_diag << "\n";
      return false;
    }
  }

  // Check Fare Type/Generic
  const std::vector<FareType>& fareTypes = application.fareTypes();
  if (!(fareTypes.empty()))
  {
    std::vector<FareType>::const_iterator fareTypeIter = fareTypes.begin();
    for (; fareTypeIter != fareTypes.end(); ++fareTypeIter)
    {
      if ((*fareTypeIter).empty() ||
          RuleUtil::matchFareType(*fareTypeIter, _paxTypeFare.fcaFareType()))
        break;
    }

    if (fareTypeIter == fareTypes.end())
    {
      if (UNLIKELY(_diagEnabled))
        *_diag << "FARE - FARE TYPE NOT MATCH " << paxTypeFare.fcaFareType() << ".  REQ:";
      for (fareTypeIter = fareTypes.begin(); fareTypeIter != fareTypes.end(); ++fareTypeIter)
      {
        *_diag << " " << *fareTypeIter;
      }
      *_diag << "\n";
      return false;
    }
  }

  bool isRouting =
      paxTypeFare.isRoutingProcessed() ? paxTypeFare.isRouting() : _paxTypeFare.isRouting();

  // Check MPM Indicator
  if (UNLIKELY(application.mpmInd() == YES && // Item only apply to MPM fare
      isRouting))
  {
    if (_diagEnabled)
      *_diag << "FARE IS ROUTING FARE NOT MATCH " << '\n';
    return false;
  }

  // Check Routing Application
  if (application.routingInd() == YES && // Item only apply to Routing fare
      !isRouting)
  {
    if (_diagEnabled)
      *_diag << "FARE IS MILEAGE FARE NOT MATCH " << '\n';
    return false;
  }

  // Check Routing Tariff
  if (((application.routingTariff1() != MinimumFare::ANY_TARIFF) &&
       (application.routingTariff1() != paxTypeFare.tcrRoutingTariff1())) ||
      ((application.routingTariff2() != MinimumFare::ANY_TARIFF) &&
       (application.routingTariff2() != paxTypeFare.tcrRoutingTariff2())))
  {
    if (_diagEnabled)
      *_diag << "FARE - ROUTING TARIFF NOT MATCH " << paxTypeFare.tcrRoutingTariff1() << " "
             << paxTypeFare.tcrRoutingTariff2() << '\n';
    return false;
  }

  // Check Routing
  RoutingNumber rn = paxTypeFare.fcaRoutingNumber().empty()?
                     paxTypeFare.routingNumber() : paxTypeFare.fcaRoutingNumber();

  if (!RuleUtil::compareRoutings(application.routing(), rn))
  {
    if (_diagEnabled)
      *_diag << "FARE - ROUTING NUMBER NOT MATCH " << paxTypeFare.fcaRoutingNumber() << '\n';

    return false;
  }

  // Check Global Direction
  if (application.globalDir() != GlobalDirection::ZZ &&
      application.globalDir() != paxTypeFare.fareMarket()->getGlobalDirection())
  {
    if (UNLIKELY(_diagEnabled))
    {
      *_diag << "FARE - GLOBAL NOT MATCH ";
      std::string gd;
      globalDirectionToStr(gd, paxTypeFare.fareMarket()->getGlobalDirection());
      *_diag << gd << '\n';
    }
    return false;
  }

  bool reverseOrigin = ((paxTypeFare.directionality() == TO) ||
                        (paxTypeFare.fareMarket()->direction() == FMDirection::INBOUND));

  // thru/terminate geo matching
  const std::vector<TravelSeg*>& travelSegVec = fareMarket.travelSeg();
  if (application.loc1().locType() != MinimumFare::BLANK ||
      application.loc2().locType() != MinimumFare::BLANK)
  {
    const Directionality& directionality = application.directionalInd();
    if (!MinimumFare::matchGeo(_trx,
                               directionality,
                               application.loc1(),
                               application.loc2(),
                               travelSegVec,
                               _itin,
                               reverseOrigin))
    {
      if (UNLIKELY(_diagEnabled))
        *_diag << "THROUGH/TERMINATE LOCATION NOT MATCH "
               << (*(travelSegVec.begin()))->origin()->loc() << "-"
               << (*(travelSegVec.end() - 1))->destination()->loc() << '\n';

      return false;
    }
  }

  // check flight restriction
  const std::vector<MinFareCxrFltRestr*>& cxrFltRest = application.cxrFltRestrs();
  if (UNLIKELY(cxrFltRest.size() != 0))
  {
    std::vector<MinFareCxrFltRestr*>::const_iterator cxrFltBeginI = cxrFltRest.begin();
    const std::vector<MinFareCxrFltRestr*>::const_iterator cxrFltEndI = cxrFltRest.end();

    for (; cxrFltBeginI < cxrFltEndI; cxrFltBeginI++)
    {
      const std::vector<TravelSeg*>& travelSegVec = fareMarket.travelSeg();
      std::vector<TravelSeg*>::const_iterator travelBoardIter = travelSegVec.begin();
      std::vector<TravelSeg*>::const_iterator travelEndIter = travelSegVec.end();
      for (; travelBoardIter < travelEndIter; travelBoardIter++)
      {
        AirSeg* airSeg = dynamic_cast<AirSeg*>(*travelBoardIter);
        if (!airSeg)
          continue; // when both are not airseg

        if (airSeg->carrier() != (*cxrFltBeginI)->carrier())
          break;

        if ((airSeg->flightNumber() < (*cxrFltBeginI)->flight1()) ||
            (airSeg->flightNumber() > (*cxrFltBeginI)->flight2()))
          break;

      } // for loop travel segs
      break; // matched found
    } // for loop table item

    if ((cxrFltBeginI != cxrFltEndI) && (application.exceptCxrFltRestr() == YES))
    {
      if (_diagEnabled)
        *_diag << "FARE - FLIGHT RESTRICTION MATCH AND EXCEPT " << '\n';
      return false;
    }
    else if ((cxrFltBeginI == cxrFltEndI) && (application.exceptCxrFltRestr() == NO))
    {
      if (_diagEnabled)
        *_diag << "FARE - FLIGHT RESTRICTION NOT MATCH " << '\n';
      return false;
    }
  }

  // via geo matching
  if (application.viaLoc1().locType() != MinimumFare::BLANK ||
      application.viaLoc2().locType() != MinimumFare::BLANK)
  {
    const Directionality& viaDirectionality = application.viaDirectionalInd();

    // Do not consider the origin and dest of the through market
    bool viaGeo = false;

    if (travelSegVec.size() > 2)
    {
      std::vector<TravelSeg*>::const_iterator viaTvlSegI = travelSegVec.begin() + 1;
      for (; (viaTvlSegI != (travelSegVec.end() - 1)) && !viaGeo; ++viaTvlSegI)
      {
        std::vector<TravelSeg*> travelSegs;
        travelSegs.push_back(*viaTvlSegI);

        viaGeo = MinimumFare::matchGeo(_trx,
                                       viaDirectionality,
                                       application.viaLoc1(),
                                       application.viaLoc2(),
                                       travelSegs,
                                       _itin,
                                       reverseOrigin);
      }
    }

    if (!viaGeo && (application.viaLoc2().locType() == MinimumFare::BLANK) &&
        ((viaDirectionality == FROM) || (viaDirectionality == BETWEEN)))
    {
      // Check last travel segment origin
      if (LocUtil::isInLoc(*travelSegVec.back()->origin(),
                           application.viaLoc1().locType(),
                           application.viaLoc1().loc(),
                           Vendor::SABRE,
                           MANUAL,
                           LocUtil::OTHER,
                           GeoTravelType::International,
                           EMPTY_STRING(),
                           _trx.getRequest()->ticketingDT()))
        viaGeo = true;
    }

    if ((viaGeo) && (application.viaExceptInd() == YES))
    {
      if (_diagEnabled)
        *_diag << "VIA LOCATION MATCH\n"
               << "VIA EXCEPTION INDICATOR - YES\n";
      return false;
    }
    if ((!viaGeo) && (application.viaExceptInd() == NO))
    {
      if (_diagEnabled)
        *_diag << "VIA LOCATION NOT MATCH " << (*(travelSegVec.begin()))->origin()->loc() << "-"
               << (*(travelSegVec.end() - 1))->destination()->loc()
               << "\nVIA EXCEPTION INDICATOR - NO\n";
      return false;
    }
  }

  // Intermediate Location Travel Application
  if (application.betwLoc1().locType() != MinimumFare::BLANK ||
      application.betwLoc2().locType() != MinimumFare::BLANK)
  {
    const Directionality& intermediateDir = application.betwDirectionalInd();
    std::vector<TravelSeg*>::const_iterator travelBoardIter = travelSegVec.begin();
    std::vector<TravelSeg*>::const_iterator travelOffIter;
    std::vector<TravelSeg*>::const_iterator travelEndIter = travelSegVec.end();
    bool interlineFlight = false;
    bool fareBreakPointMatch = false;

    for (; travelBoardIter < travelEndIter; travelBoardIter++)
    {
      for (travelOffIter = travelBoardIter; travelOffIter < travelEndIter; travelOffIter++)
      {
        std::vector<TravelSeg*> travelSegs;
        travelSegs.insert(travelSegs.begin(), travelBoardIter, travelOffIter + 1);

        if ((!(dynamic_cast<AirSeg*>(*travelBoardIter))) &&
            (!(dynamic_cast<AirSeg*>(*travelOffIter))))
        {
          continue; // when both are not airseg
        }

        if (MinimumFare::matchGeo(_trx,
                                  intermediateDir,
                                  application.betwLoc1(),
                                  application.betwLoc2(),
                                  travelSegs,
                                  _itin,
                                  reverseOrigin))
        {
          if (_diagEnabled)
            *_diag << "INTERMEDIATE LOCATION MATCH " << (*travelBoardIter)->origin()->loc() << "-"
                   << (*travelOffIter)->destination()->loc() << "\n";

          if ((_itin.segmentOrder(*travelBoardIter) == _itin.segmentOrder(travelSegVec.front())) ||
              (_itin.segmentOrder(*travelOffIter) == _itin.segmentOrder(travelSegVec.back())))
            fareBreakPointMatch = true;

          std::vector<TravelSeg*>::const_iterator tvlBrdIter = travelSegs.begin();
          std::vector<TravelSeg*>::const_iterator tvlEndIter = travelSegs.end();
          CarrierCode marketingCarrier;

          for (; tvlBrdIter < tvlEndIter; tvlBrdIter++)
          {
            const AirSeg* airSeg = dynamic_cast<AirSeg*>(*tvlBrdIter);
            if (airSeg == nullptr)
              continue;

            if (marketingCarrier.empty())
              marketingCarrier = airSeg->marketingCarrierCode();
            else if (marketingCarrier != airSeg->marketingCarrierCode())
              interlineFlight = true;

          } // for loop within intermediate board and off point

          // Check service restriction
          if ((application.serviceRestr() == 'O') && // must be online
              (interlineFlight == true))
            return false;

          if ((application.serviceRestr() == 'I') && // must be interline
              (interlineFlight == false))
            return false;

          // Check construction points must be within locs
          if ((application.constructPointInd() == YES) && (fareBreakPointMatch == false))
            return false;

          // Check direct flight
          if ((application.directInd() == YES) && (travelSegs.size() > 1))
            return false;

          // Check non stop flight (if both direct and non stop is "YES" , either one pass is fine)
          if ((application.directInd() != YES) && (application.nonStopInd() == YES) &&
              ((travelSegs.size() > 1) || (!(*(travelSegs.begin()))->hiddenStops().empty())))
            return false;

        } // if geo match
      } // for loop off point
    } // for loop board point
  }

  if (UNLIKELY(_diagEnabled))
  {
    *_diag << "MATCHED APPLICATION ITEM " << '\n';
    displayApplication(application);
  }

  return true;
}

void
MatchExceptionTable::displayApplication(const MinFareAppl& application) const
{
  if (_diagEnabled)
  {
    *_diag << "  RULE TARIFF CAT: " << std::setw(2) << application.tariffCat()
           << "  RULE TARIFF NO:  " << std::setw(2) << application.ruleTariff() << "\n";

    const std::vector<RuleNumber>& rules = application.rules();
    std::vector<RuleNumber>::const_iterator ruleIter = rules.begin();
    if (rules.size() > 0)
    {
      *_diag << "  RULE NUMBER:";
      for (; ruleIter != rules.end(); ruleIter++)
      {
        *_diag << "  " << *ruleIter;
      }
      *_diag << "\n";
    }

    const std::vector<FareClassCode>& fareClasses = application.fareClasses();
    std::vector<FareClassCode>::const_iterator fareClassIter = fareClasses.begin();
    if (fareClasses.size() > 0)
    {
      *_diag << "  FARE CLASS:";
      for (; fareClassIter != fareClasses.end(); fareClassIter++)
      {
        *_diag << "  " << *fareClassIter;
      }
      *_diag << "\n";
    }

    const std::vector<FareType>& fareTypes = application.fareTypes();
    std::vector<FareType>::const_iterator fareTypeIter = fareTypes.begin();
    if (fareTypes.size() > 0)
    {
      *_diag << "  FARE TYPE:";
      for (; fareTypeIter != fareTypes.end(); fareTypeIter++)
      {
        *_diag << "  " << *fareTypeIter;
      }
      *_diag << "\n";
    }

    std::string gd;
    globalDirectionToStr(gd, application.globalDir());

    _diag->setf(std::ios::left, std::ios::adjustfield);

    *_diag << "  RULE TARIFF CAT: " << std::setw(2) << application.tariffCat()
           << "  RTG TARIFF1: " << std::setw(6) << application.routingTariff1()
           << "  GLOBAL DIR:      " << std::setw(2) << gd << '\n'
           << "  RULE TARIFF: " << std::setw(6) << application.ruleTariff()
           << "  RTG TARIFF2: " << std::setw(6) << application.routingTariff2()
           << "  USER APPL TYPE:  " << std::setw(2) << application.userApplType() << '\n'
           << "  TARIFF CODE:" << std::setw(7) << application.ruleTariffCode()
           << "  FARE TYPE APPL:  " << std::setw(2) << application.fareTypeAppl()
           << "  USER APPL:     " << std::setw(4) << application.userAppl() << '\n'
           << "MINFARE CHECK  APPLY/DONOTAPPLY  STOPOVER/TICKETPOINTS\n"
           << "  HIP          " << std::setw(18) << application.hipCheckAppl()
           << application.hipStopTktInd() << "\n"
           << "  DMC          " << std::setw(18) << application.dmcCheckAppl()
           << application.dmcStopTktInd() << "\n"
           << "  CTM          " << std::setw(18) << application.ctmCheckAppl()
           << application.ctmStopTktInd() << "\n"
           << "  BHC          " << std::setw(18) << application.backhaulCheckAppl()
           << application.backhaulStopTktInd() << "\n"
           << "  COM          " << std::setw(18) << application.comCheckAppl()
           << application.comStopTktInd() << "\n"
           << "  CPM          " << std::setw(18) << application.cpmCheckAppl()
           << application.cpmStopTktInd() << "\n"
           << "ISI IND - DO NOT APPLY /SITI:" << application.sitiInd()
           << " SITO:" << application.sitoInd() << " SOTI:" << application.sotiInd()
           << " SOTO:" << application.sotoInd() << "/\n"
           << "  APPLY DEF LOGIC: " << std::setw(2) << application.applyDefaultLogic() << '\n';

    if (application.applyDefaultLogic() == NO)
    {
      *_diag << "******************** OVERRIDE LOGIC ************************\n";
      if ((_module == HIP) || (_module == BHC) || (_module == CTM))
      {
        *_diag << "HIP/CTM/BHC DOMESTIC APPLICATION - \n"
               << "  APPL/EXCLUDE ALL DOMESTIC: " << application.domAppl() << "\n"
               << "  EXCEPT: " << application.domExceptInd()
               << "  LOC: " << application.domLoc().locType() << " " << application.domLoc().loc()
               << "\n"
               << "  EXCEPT FARE TYPE: " << application.domFareTypeExcept() << "\n"
               << "  THROUGH FARE TYPE GENERIC: ";

        std::vector<FareTypeAbbrev>::const_iterator domFareTypeI =
            application.domFareTypes().begin();
        for (; domFareTypeI != application.domFareTypes().end(); ++domFareTypeI)
        {
          *_diag << *domFareTypeI;
        }

        *_diag << " \n";
      }

      if (_module == HIP)
      {
        *_diag << "NORMAL HIP - \n"
               << "  DETAILED CHECK POINTS - \n"
               << "    FROM GROUP 1              TO GROUP 2 \n"
               << "    ORIG                 " << application.nmlHipOrigInd()
               << "    DEST            " << application.nmlHipDestInd() << "\n"
               << "    ORIG NATION          " << application.nmlHipOrigNationInd()
               << "    DEST NATION     " << application.nmlHipDestNationInd() << "\n"
               << "    INTERM               " << application.nmlHipFromInterInd()
               << "    INTERM          " << application.nmlHipToInterInd() << "\n"
               << "  ADDITIONAL CHECK POINTS - \n"
               << "    FROM GROUP 1              TO GROUP 2 \n"
               << "    ORIG NATION TKT      " << application.nmlHipOrigNationTktPt()
               << "    TKT             " << application.nmlHipTicketedPt() << "\n"
               << "    ORIG NATION STOPOVER " << application.nmlHipOrigNationStopPt()
               << "    STOPOVER        " << application.nmlHipStopoverPt() << "\n"
               << "  EXEMPT INTERM TO INTERM: " << application.nmlHipExemptInterToInter() << "\n"
               << "SPECIAL HIP - \n"
               << "  SPECIAL ONLY:     " << application.spclHipSpclOnlyInd()
               << "  INTERM LOC BY PASS NML: " << application.spclHipLoc().locType() << " "
               << application.spclHipLoc().loc() << "\n"
               << "  DETAILED CHECK POINTS - \n"
               << "    FROM GROUP 1              TO GROUP 2 \n"
               << "    ORIG                 " << application.spclHipOrigInd()
               << "    DEST            " << application.spclHipDestInd() << "\n"
               << "    ORIG NATION          " << application.spclHipOrigNationInd()
               << "    DEST NATION     " << application.spclHipDestNationInd() << "\n"
               << "    INTERM               " << application.spclHipFromInterInd()
               << "    INTERM          " << application.spclHipToInterInd() << "\n"
               << "  ADDITIONAL CHECK POINTS - \n"
               << "    FROM GROUP 1              TO GROUP 2 \n"
               << "    ORIG NATION TKT      " << application.spclHipOrigNationTktPt()
               << "    TKT             " << application.spclHipTicketedPt() << "\n"
               << "    ORIG NATION STOPOVER " << application.spclHipOrigNationStopPt()
               << "    STOPOVER        " << application.spclHipStopoverPt() << "\n"
               << "  EXEMPT INTERM TO INTERM: " << application.spclHipExemptInterToInter() << "\n";
      }

      *_diag << "SPECIAL FARE TYPE GROUP - \n";
      *_diag << "  PROCESS NAME: " << application.specialProcessName() << " \n";

      if (_module == CTM)
      {
        *_diag << "NORMAL CTM - \n"
               << "  DETAILED CHECK POINTS - \n"
               << "    FROM GROUP 1              TO GROUP 2 \n"
               << "    ORIG                 " << application.nmlCtmOrigInd()
               << "    INTERM          " << application.nmlCtmToInterInd() << "\n"
               << "                          "
               << "    DEST NATION     " << application.nmlCtmDestNationInd() << "\n"
               << "SPECIAL CTM - \n"
               << "  DETAILED CHECK POINTS - \n"
               << "    FROM GROUP 1              TO GROUP 2 \n"
               << "    ORIG                 " << application.spclCtmOrigInd()
               << "    INTERM          " << application.spclCtmToInterInd() << "\n"
               << "                          "
               << "    DEST NATION     " << application.spclCtmDestNationInd() << "\n";
      }

      if (_module == CPM)
      {
        *_diag << "CPM EXCLUSION - " << application.cpmExcl() << "\n";
      }
    }
  }
}
}
