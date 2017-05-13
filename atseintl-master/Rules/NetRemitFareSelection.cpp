//----------------------------------------------------------------------------
//  Copyright Sabre 2006
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
#include "Rules/NetRemitFareSelection.h"

#include "Common/CurrencyConversionFacade.h"
#include "Common/FareMarketUtil.h"
#include "Common/LocUtil.h"
#include "Common/Money.h"
#include "Common/PaxTypeUtil.h"
#include "Common/Thread/TseRunnableExecutor.h"
#include "Common/TrxUtil.h"
#include "Common/Vendor.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/IndustryFare.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RepricingTrx.h"
#include "DBAccess/Loc.h"
#include "DBAccess/MultiTransport.h"
#include "DBAccess/NegFareRest.h"
#include "DBAccess/NegFareRestExt.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag450Collector.h"
#include "Diagnostic/DiagCollector.h"
#include "Diagnostic/DiagManager.h"
#include "Fares/RoutingController.h"
#include "Pricing/PricingOrchestrator.h"
#include "Routing/RoutingInfo.h"
#include "Routing/TravelRoute.h"
#include "Rules/NetFareMarketThreadTask.h"
#include "Rules/NetRemitPscFareSelection.h"
#include "Rules/RuleControllerWithChancelor.h"
#include "Rules/RuleUtil.h"
#include "Util/BranchPrediction.h"

#include <iomanip>

namespace tse
{
boost::mutex ruleRtgMutex;

Logger
NetRemitFareSelection::_logger("atseintl.Rules.NetRemitFareSelection");

bool
NetRemitFareSelection::processNetRemitFareSelection(PricingTrx& trx,
                                                    const FarePath& farePath,
                                                    PricingUnit& pricingUnit,
                                                    FareUsage& fareUsage,
                                                    const NegFareRest& negFareRest)
{
  if (TrxUtil::optimusNetRemitEnabled(trx) && (negFareRest.tktFareDataInd1() == RuleConst::BLANK))
  {
    const NegPaxTypeFareRuleData* ruleData = fareUsage.paxTypeFare()->getNegRuleData();

    TSE_ASSERT(ruleData);
    const NegFareRestExt* negFareRestExt = ruleData->negFareRestExt();

    if (negFareRestExt && negFareRestExt->fareBasisAmtInd() != RuleConst::BLANK)
    {
      NetRemitPscFareSelection nrPscFareSel(trx,
                                            farePath,
                                            pricingUnit,
                                            fareUsage,
                                            negFareRest,
                                            *negFareRestExt,
                                            ruleData->negFareRestExtSeq());

      nrPscFareSel.process();

      return (!fareUsage.netRemitPscResults().empty() &&
              fareUsage.netRemitPscResults().begin()->_resultFare);
    }
  }

  NetRemitFareSelection nrFareSel(trx, farePath, pricingUnit, fareUsage, negFareRest);
  nrFareSel.process();

  return fareUsage.tktNetRemitFare() != nullptr;
}

NetRemitFareSelection::NetRemitFareSelection(PricingTrx& trx,
                                             const FarePath& fPath,
                                             PricingUnit& pu,
                                             FareUsage& fu,
                                             const NegFareRest& negFareRest)
  : _trx(trx),
    _fp(fPath),
    _pu(pu),
    _fu(fu),
    _negFareRest(negFareRest),
    _fm(*(fu.paxTypeFare()->fareMarket())),
    _alterCxrFm(nullptr),
    _seg1OrigBetw(nullptr),
    _seg1BetwAnd(nullptr),
    _seg1BetwDest(nullptr),
    _seg1BetwOrig(nullptr),
    _seg1DestBetw(nullptr),
    _seg2BetwAnd(nullptr),
    _diag(nullptr),
    _dispAllFares(false),
    _dispMatchFare(false),
    _selSeg1(true),
    _axessUser(false),
    _isCmdPricing(false),
    _isDirFlippedInNewFmForJCB(false),
    _rpCalculationCurrency("")
{
  if (_trx.getRequest()->ticketingAgent()->axessUser())
    _axessUser = true;

  if (UNLIKELY(_trx.diagnostic().diagnosticType() == Diagnostic692))
  {
    std::map<std::string, std::string>::const_iterator i =
        trx.diagnostic().diagParamMap().find("DD");

    if (i != trx.diagnostic().diagParamMap().end())
    {
      if (i->second == "ALL")
        _dispAllFares = true;
      else if (i->second == "MATCH")
        _dispMatchFare = true;
    }

    DCFactory* factory = DCFactory::instance();
    _diag = factory->create(_trx);
    if (_diag != nullptr)
    {
      _diag->enable(Diagnostic692);
      if (!_diag->isActive())
      {
        _diag = nullptr;
      }
    }
  }

  if (fu.paxTypeFare()->isCmdPricing())
    _isCmdPricing = true;
}

void
NetRemitFareSelection::printHeader()
{
  if (UNLIKELY(_diag != nullptr))
  {
    if (_axessUser)
      (*_diag) << " \n\n***************** JAL AXESS FARE SELECTION *******************\n";
    else
      (*_diag) << " \n\n***************** NET REMIT FARE SELECTION *******************\n";
    (*_diag) << "------------------- CAT35 FARE -------------------------------\n"
             << FareMarketUtil::getFullDisplayString(_fm) << " REQPAX-" << _fp.paxType()->paxType()
             << "\n";

    displayFare(*(_fu.paxTypeFare()));
    displayTicketedFareData();
  }
}

void
NetRemitFareSelection::process()
{
  printHeader();

  if (_axessUser)
  {
    // 1.JAL/AXESS is using the same FM.
    //   Do not build a new FM when tkt_carrier=YY because
    //   YY fare selection for JAL/AXESS and YY fare pricing are based
    //   on the same logic (carrierPref table).
    // 2. PricingOrchestrator calls this function within the single thread.
    const PaxTypeFare* sPTF = nullptr;

    sPTF = selectTktPblFare(_fm);

    _fu.tktNetRemitFare() = sPTF;
    if (UNLIKELY(_diag != nullptr))
    {
      _diag->flushMsg();
    }
    return;
  }
  else
  {
    buildPblFareMarkets();

    // Select fare from these fare markets
    if (_alterCxrFm != nullptr)
    {
      _fu.tktNetRemitFare() = selectTktPblFare(*_alterCxrFm);
    }
    else if (_seg1BetwAnd != nullptr)
    {
      if (_seg2BetwAnd != nullptr)
      {
        if (_fu.isInbound())
        {
          _fu.tktNetRemitFare2() = selectTktPblFare(*_seg1BetwAnd);
          _selSeg1 = false;
          _fu.tktNetRemitFare() = selectTktPblFare(*_seg2BetwAnd);
        }
        else
        {
          _fu.tktNetRemitFare() = selectTktPblFare(*_seg1BetwAnd);
          _selSeg1 = false;
          _fu.tktNetRemitFare2() = selectTktPblFare(*_seg2BetwAnd);
        }
      }
      else
      {
        _fu.tktNetRemitFare() = selectTktPblFare(*_seg1BetwAnd);
      }
    }
    else if (_seg1OrigBetw != nullptr)
    {
      _fu.tktNetRemitFare() =
          selectTktPblFare(*_seg1OrigBetw, _seg1BetwDest, _seg1BetwOrig, _seg1DestBetw);
    }
    else
    {
      _fu.tktNetRemitFare() = selectTktPblFare(_fm);
      // Process Routing for selected fare only when multiple travel segments involved
      if ((_fu.tktNetRemitFare() != nullptr) && (_fm.travelSeg().size() > 1))
      {
        processRouting(_fu.tktNetRemitFare());
        checkMileageRouting();
      }
    }

    if (UNLIKELY(_diag != nullptr))
    {
      _diag->flushMsg();
    }
  }
}

void
NetRemitFareSelection::checkMileageRouting()
{
  if (!_fu.tktNetRemitFare()->isRoutingValid() && !_fu.tktNetRemitFare()->isRouting())
  {
    // bad fare selection when failed Mileage
    _fu.tktNetRemitFare() = nullptr;
    if (UNLIKELY(_diag != nullptr))
      (*_diag) << "FAILED BY MILEAGE\n";
  }
}

void
NetRemitFareSelection::processRouting(const PaxTypeFare* paxTypeFare)
{
  RoutingController routingController(_trx);
  TravelRoute travelRoute;
  TravelRoute travelRouteTktOnly;
  travelRoute.travelRouteTktOnly() = &travelRouteTktOnly;
  RoutingInfos routingInfos;

  routingController.process(*(const_cast<PaxTypeFare*>(paxTypeFare)), travelRoute, routingInfos);

  routingController.processRoutingDiagnostic(
      travelRoute, routingInfos, *(const_cast<FareMarket*>(paxTypeFare->fareMarket())));

  if (TrxUtil::isFullMapRoutingActivated(_trx) && travelRoute.travelRouteTktOnly())
  {
    routingController.processRoutingDiagnostic(
        *travelRoute.travelRouteTktOnly(),
        routingInfos,
        *(const_cast<FareMarket*>(paxTypeFare->fareMarket())));
  }
}

void
NetRemitFareSelection::buildPblFareMarkets()
{
  TravelSeg* firstTvlSeg = _fm.travelSeg().front();
  TravelSeg* lastTvlSeg = _fm.travelSeg().back();

  const DateTime& departureDT = firstTvlSeg->departureDT();
  const DateTime& arrivalDT = lastTvlSeg->arrivalDT();

  if (!_negFareRest.betwCity1().empty())
  {
    const Loc* betwLoc1 = _trx.dataHandle().getLoc(_negFareRest.betwCity1(), departureDT);
    if (betwLoc1 == nullptr)
      return;

    if (_negFareRest.andCity1().empty())
    {
      createFareMarkets(
          firstTvlSeg->origin(), lastTvlSeg->destination(), arrivalDT, departureDT, betwLoc1);
    }
    else
    {
      const Loc* andLoc1 = _trx.dataHandle().getLoc(_negFareRest.andCity1(), departureDT);
      if (andLoc1 == nullptr)
        return;

      if (!_negFareRest.betwCity2().empty() &&
          !_negFareRest.andCity2().empty()) // Recurring segments
      {
        // Build fare market for betwCity1 - andCity1
        _seg1BetwAnd = getPblFareMarket(
            betwLoc1,
            andLoc1,
            (_negFareRest.carrier11().empty() ? _fm.governingCarrier() : _negFareRest.carrier11()),
            departureDT,
            firstTvlSeg->arrivalDT());

        // Build fare market for betwCity2 - and City2
        const Loc* betwLoc2 =
            _trx.dataHandle().getLoc(_negFareRest.betwCity2(), lastTvlSeg->departureDT());
        if (betwLoc2 == nullptr)
          return;

        const Loc* andLoc2 =
            _trx.dataHandle().getLoc(_negFareRest.andCity2(), lastTvlSeg->departureDT());
        if (andLoc2 == nullptr)
          return;

        _seg2BetwAnd = getPblFareMarket(
            betwLoc2,
            andLoc2,
            (_negFareRest.carrier21().empty() ? _fm.governingCarrier() : _negFareRest.carrier21()),
            lastTvlSeg->departureDT(),
            arrivalDT);
      }
      else // Alternate market
      {
        // Build fare market for betwCity1 - andCity1
        _seg1BetwAnd = getPblFareMarket(
            betwLoc1,
            andLoc1,
            (_negFareRest.carrier11().empty() ? _fm.governingCarrier() : _negFareRest.carrier11()),
            departureDT,
            arrivalDT);
      }
    }
  }
  else if (!(_negFareRest.carrier11().empty()) &&
           _negFareRest.carrier11() != _fm.governingCarrier()) // Alternate Cxr
  {
    if (_negFareRest.carrier11() != INDUSTRY_CARRIER) // For YY reuse the current fare market
    {
      // Build fare market with different carrier
      _alterCxrFm = getPblFareMarket(firstTvlSeg->origin(),
                                     lastTvlSeg->destination(),
                                     _negFareRest.carrier11(),
                                     departureDT,
                                     arrivalDT);
    }
  }
  else if (_fp.paxType()->paxType() == JCB)
  {
    // Since JCB does not default retrieve ADT fare, always build fare market for JCB
    _alterCxrFm = getPblFareMarket(firstTvlSeg->origin(),
                                   lastTvlSeg->destination(),
                                   _fm.governingCarrier(),
                                   departureDT,
                                   arrivalDT);

    // Direction in new market can differ from original market. This is seen in PTC JCB
    // The problem may be in how we create FareMarkets.
    _isDirFlippedInNewFmForJCB = _fu.isInbound() && _alterCxrFm &&
                                 _alterCxrFm->origin() == _fm.destination() &&
                                 _alterCxrFm->destination() == _fm.origin();
  }
}

void
NetRemitFareSelection::createFareMarkets(const Loc* originLoc,
                                         const Loc* destLoc,
                                         const DateTime& arrivalDT,
                                         const DateTime& departureDT,
                                         const Loc* betwLoc1)
{
  NetFareMarketThreadTask netFmTask1(
      *this, _trx, originLoc, betwLoc1, arrivalDT, departureDT, _seg1OrigBetw);
  NetFareMarketThreadTask netFmTask2(
      *this, _trx, betwLoc1, destLoc, arrivalDT, departureDT, _seg1BetwDest);
  NetFareMarketThreadTask netFmTask3(
      *this, _trx, betwLoc1, originLoc, arrivalDT, departureDT, _seg1BetwOrig);
  NetFareMarketThreadTask netFmTask4(
      *this, _trx, destLoc, betwLoc1, arrivalDT, departureDT, _seg1DestBetw);
  TseRunnableExecutor netRemitFareMarketExecutor(TseThreadingConst::NETFAREMARKETS_TASK);
  netRemitFareMarketExecutor.execute(netFmTask1);
  netRemitFareMarketExecutor.execute(netFmTask2);
  netRemitFareMarketExecutor.execute(netFmTask3);
  netRemitFareMarketExecutor.execute(netFmTask4);

  netRemitFareMarketExecutor.wait();
}

const FareMarket*
NetRemitFareSelection::getPblFareMarket(const Loc* origLoc,
                                        const Loc* destLoc,
                                        const CarrierCode& cxr,
                                        const DateTime& deptDT,
                                        const DateTime& arrDT)
{
  if (origLoc == nullptr || destLoc == nullptr)
    return nullptr;

  const Loc* orig = getOrig(origLoc, destLoc);
  const Loc* dest = getDest(origLoc, destLoc);

  const FareMarket* pblFm = nullptr;

  // Create Published Travel Seg
  AirSeg* pblTvlSeg = nullptr;
  _trx.dataHandle().get(pblTvlSeg);
  if (pblTvlSeg == nullptr)
    return pblFm;

  pblTvlSeg->origin() = orig;
  pblTvlSeg->destination() = dest;
  pblTvlSeg->segmentOrder() = 0;
  pblTvlSeg->setMarketingCarrierCode(cxr);
  pblTvlSeg->departureDT() = deptDT;
  pblTvlSeg->arrivalDT() = arrDT;

  // Set GEO travel type
  if (LocUtil::isDomestic(*orig, *dest))
    pblTvlSeg->geoTravelType() = GeoTravelType::Domestic;
  else if (LocUtil::isInternational(*orig, *dest))
    pblTvlSeg->geoTravelType() = GeoTravelType::International;
  else if (LocUtil::isTransBorder(*orig, *dest))
    pblTvlSeg->geoTravelType() = GeoTravelType::Transborder;
  else if (LocUtil::isForeignDomestic(*orig, *dest))
    pblTvlSeg->geoTravelType() = GeoTravelType::ForeignDomestic;

  // Set offMultiCity and boardMultiCity
  const std::vector<tse::MultiTransport*>& boardMACList =
      _trx.dataHandle().getMultiTransportCity(orig->loc(), cxr, pblTvlSeg->geoTravelType(), deptDT);
  std::vector<tse::MultiTransport*>::const_iterator boardListIter = boardMACList.begin();

  pblTvlSeg->boardMultiCity() =
      ((boardListIter != boardMACList.end()) ? (**boardListIter).multitranscity() : orig->loc());

  const std::vector<tse::MultiTransport*>& offMACList =
      _trx.dataHandle().getMultiTransportCity(dest->loc(), cxr, pblTvlSeg->geoTravelType(), deptDT);
  std::vector<tse::MultiTransport*>::const_iterator offListIter = offMACList.begin();
  pblTvlSeg->offMultiCity() =
      ((offListIter != offMACList.end()) ? (**offListIter).multitranscity() : dest->loc());

  pblTvlSeg->origAirport() = pblTvlSeg->boardMultiCity();
  pblTvlSeg->destAirport() = pblTvlSeg->offMultiCity();

  std::vector<TravelSeg*> tvlSegs;
  tvlSegs.push_back(pblTvlSeg);

  pblFm = getPblFareMarket(tvlSegs, cxr);

  return pblFm;
}

const Loc*
NetRemitFareSelection::getOrig(const Loc* origLoc, const Loc* destLoc) const
{
  return _fu.isInbound() ? destLoc : origLoc;
}

const Loc*
NetRemitFareSelection::getDest(const Loc* origLoc, const Loc* destLoc) const
{
  return _fu.isInbound() ? origLoc : destLoc;
}

RepricingTrx*
NetRemitFareSelection::runRepriceTrx(std::vector<TravelSeg*>& tvlSegs, const CarrierCode& cxr)
{
  return TrxUtil::reprice(
      _trx,
      tvlSegs,
      FMDirection::UNKNOWN,
      false,
      &cxr,
      (_negFareRest.globalDir1() != GlobalDirection::ZZ ? &_negFareRest.globalDir1() : nullptr),
      ((_fp.paxType()->paxType() == JCB) ? ADULT : ""),
      false,
      TrxUtil::cat35LtypeEnabled(_trx));
}

const FareMarket*
NetRemitFareSelection::getFareMarket(RepricingTrx* rpTrx,
                                     std::vector<TravelSeg*>& tvlSegs,
                                     const CarrierCode& cxr)
{
  if (rpTrx)
  {
    if (rpTrx->itin().front())
    {
      _rpCalculationCurrency = rpTrx->itin().front()->calculationCurrency();
    }
    return TrxUtil::getFareMarket(
        *rpTrx, cxr, tvlSegs, _fu.paxTypeFare()->retrievalDate(), _fp.itin());
  }
  return nullptr;
}

const FareMarket*
NetRemitFareSelection::getPblFareMarket(std::vector<TravelSeg*>& tvlSegs, const CarrierCode& cxr)
{
  RepricingTrx* rpTrx = nullptr;

  try { rpTrx = runRepriceTrx(tvlSegs, cxr); }
  catch (const ErrorResponseException& ex)
  {
    LOG4CXX_ERROR(_logger,
                  "Exception during repricing for [" << tvlSegs.front()->boardMultiCity() << " - "
                                                     << tvlSegs.back()->offMultiCity() << " "
                                                     << ex.code() << " - " << ex.message());
    throw;
  }
  catch (...)
  {
    LOG4CXX_ERROR(_logger,
                  "Unknown exception during repricing for [" << tvlSegs.front()->boardMultiCity()
                                                             << " - "
                                                             << tvlSegs.back()->offMultiCity());
    throw;
  }

  return getFareMarket(rpTrx, tvlSegs, cxr);
}

void
NetRemitFareSelection::displayFareMarket(const FareMarket& fareMarket)
{
  if (LIKELY(_diag == nullptr))
    return;

  DiagCollector& dc = *_diag;

  dc << "------------------ SELECTED FARE MARKET ---------------------\n"
     << FareMarketUtil::getFullDisplayString(fareMarket) << " REQPAX-" << _fp.paxType()->paxType()
     << "\n";

  displayFareHeader();

  if (_dispAllFares && !_axessUser)
  {
    const PaxTypeBucket* paxTypeCortege = fareMarket.paxTypeCortege(_fp.paxType());
    if (paxTypeCortege == nullptr)
      return;

    const std::vector<PaxTypeFare*>& fares = (_fp.paxType()->paxType() == JCB)
                                                 ? fareMarket.allPaxTypeFare()
                                                 : paxTypeCortege->paxTypeFare();

    std::vector<PaxTypeFare*>::const_iterator iter = fares.begin();
    for (; iter != fares.end(); iter++)
    {
      displayFare(**iter);
    }
  }
}

void
NetRemitFareSelection::displayFareHeader()
{
  DiagCollector& dc = *_diag;
  dc << "  CXR GI V RULE RUL FARE CLASS  O O FAR PAX SEA DOW BASE    CNV\n"
     << "                TRF             R I TPE TPE TPE TPE CUR     AMT\n";
}

void
NetRemitFareSelection::displayFare(const PaxTypeFare& paxFare) const
{
  DiagCollector& dc = *_diag;

  dc.setf(std::ios::left, std::ios::adjustfield);

  dc << std::setw(2) << dc.cnvFlags(paxFare) << std::setw(4) << paxFare.carrier();

  std::string gd;
  globalDirectionToStr(gd, paxFare.fare()->globalDirection());

  dc << std::setw(3) << gd << std::setw(2) << Vendor::displayChar(paxFare.vendor()) << std::setw(5)
     << paxFare.ruleNumber() << std::setw(4) << paxFare.tcrRuleTariff() << std::setw(12)
     << paxFare.fareClass();

  if (paxFare.owrt() == ONE_WAY_MAY_BE_DOUBLED)
    dc << std::setw(2) << "X";
  else if (paxFare.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
    dc << std::setw(2) << "R";
  else if (paxFare.owrt() == ONE_WAY_MAYNOT_BE_DOUBLED)
    dc << std::setw(2) << "O";
  else
    dc << std::setw(2) << " ";

  if (paxFare.directionality() == FROM)
    dc << std::setw(2) << "O";

  else if (paxFare.directionality() == TO)
    dc << std::setw(2) << "I";

  else
    dc << "  ";

  if (!paxFare.isFareClassAppMissing())
  {
    dc << std::setw(4) << paxFare.fcaFareType();
  }
  else
  {
    dc << "UNK";
  }

  if (!paxFare.isFareClassAppSegMissing())
  {
    if (paxFare.fcasPaxType().empty())
      dc << "*** ";
    else
      dc << std::setw(4) << paxFare.fcasPaxType();
  }
  else
  {
    dc << "UNK";
  }

  dc << std::setw(4) << paxFare.fcaSeasonType() << std::setw(4) << paxFare.fcaDowType()
     << std::setw(4) << paxFare.currency();

  dc.setf(std::ios::right, std::ios::adjustfield);
  dc.setf(std::ios::fixed, std::ios::floatfield);
  dc.precision(2);
  dc << std::setw(8) << paxFare.nucFareAmount() << '\n';
}

void
NetRemitFareSelection::displayTicketedFareData()
{
  DiagCollector& dc = *_diag;

  dc << "--------------- REC 3 TICKETED FARE DATA ---------------------\n"
     << "O GI RUL RULE FARE CLASS  FAR SEA DOW CXR BWT AND \n"
     << "R    TRF                  TYP TYP TYP     CTY CTY \n";

  if (_axessUser)
    dc << "              *X          *X  *X  *X      *X  *X  \n";

  dc.setf(std::ios::left, std::ios::adjustfield);

  dc << std::setw(2) << _negFareRest.owrt1();

  std::string gd;
  globalDirectionToStr(gd, _negFareRest.globalDir1());

  dc << std::setw(3) << gd;

  dc << std::setw(4) << _negFareRest.ruleTariff1() << std::setw(5) << _negFareRest.rule1()
     << std::setw(12) << _negFareRest.fareClass1() << std::setw(4) << _negFareRest.fareType1()
     << std::setw(4) << _negFareRest.seasonType1() << std::setw(4) << _negFareRest.dowType1()
     << std::setw(4) << _negFareRest.carrier11() << std::setw(4) << _negFareRest.betwCity1()
     << std::setw(4) << _negFareRest.andCity1() << "\n";

  if (!_negFareRest.betwCity2().empty())
  {
    dc << std::setw(2) << _negFareRest.owrt1();

    std::string gd2;
    globalDirectionToStr(gd2, _negFareRest.globalDir2());

    dc << std::setw(3) << gd2;

    dc << std::setw(4) << _negFareRest.ruleTariff2() << std::setw(5) << _negFareRest.rule2()
       << std::setw(12) << _negFareRest.fareClass2() << std::setw(4) << _negFareRest.fareType2()
       << std::setw(4) << _negFareRest.seasonType2() << std::setw(4) << _negFareRest.dowType2()
       << std::setw(4) << _negFareRest.carrier21() << std::setw(4) << _negFareRest.betwCity2()
       << std::setw(4) << _negFareRest.andCity2() << "\n";
  }
  if (_axessUser)
  {
    dc << " "
       << "\n";
    dc << "NOTE: *X - IS IGNORED FOR AXESS USER"
       << "\n";
  }
}

void
NetRemitFareSelection::findFaresForPaxType(std::vector<PaxTypeFare*>& ptFares,
                                           const PaxTypeCode& pCode,
                                           const FareMarket* fm) const
{
  if (!fm)
    return;

  if (pCode == JCB || _axessUser)
  {
    ptFares.insert(ptFares.end(), fm->allPaxTypeFare().begin(), fm->allPaxTypeFare().end());
    return;
  }
  std::vector<PaxTypeBucket>::const_iterator ptcIt = fm->paxTypeCortege().begin();
  std::vector<PaxTypeBucket>::const_iterator ptcEnd = fm->paxTypeCortege().end();
  for (; ptcIt != ptcEnd; ++ptcIt)
  {
    if (ptcIt->requestedPaxType()->paxType() == pCode)
    {
      ptFares.insert(ptFares.end(), ptcIt->paxTypeFare().begin(), ptcIt->paxTypeFare().end());
      break;
    }
  }
}

std::vector<PaxTypeFare*>
NetRemitFareSelection::collectFaresFromFareMarkets(const FareMarket* fm1,
                                                   const FareMarket* fm2,
                                                   const FareMarket* fm3,
                                                   const FareMarket* fm4)
{
  const PaxTypeCode& paxCode = _fp.paxType()->paxType();
  std::vector<PaxTypeFare*> ptFares;
  findFaresForPaxType(ptFares, paxCode, fm1);
  findFaresForPaxType(ptFares, paxCode, fm2);
  findFaresForPaxType(ptFares, paxCode, fm3);
  findFaresForPaxType(ptFares, paxCode, fm4);
  std::sort(ptFares.begin(), ptFares.end(), PaxTypeFare::fareSort);
  return ptFares;
}

const std::vector<PaxTypeFare*>&
NetRemitFareSelection::collectFaresFromFareMarket(const FareMarket* fm) const
{
  const PaxTypeCode& pCode = _fp.paxType()->paxType();
  if (pCode == JCB || _axessUser)
    return fm->allPaxTypeFare();
  std::vector<PaxTypeBucket>::const_iterator ptcIt = fm->paxTypeCortege().begin();
  std::vector<PaxTypeBucket>::const_iterator ptcEnd = fm->paxTypeCortege().end();
  for (; ptcIt != ptcEnd; ++ptcIt)
  {
    if (ptcIt->requestedPaxType()->paxType() == pCode)
    {
      return ptcIt->paxTypeFare();
    }
  }
  static const std::vector<PaxTypeFare*> ptfv;
  return ptfv;
}

void
NetRemitFareSelection::buildFareMarketsTask(const Loc* loc1,
                                            const Loc* loc2,
                                            const DateTime& arrivalDT,
                                            const DateTime& departureDT,
                                            const FareMarket*& fm)
{
  fm = getPblFareMarket(
      loc1,
      loc2,
      (_negFareRest.carrier11().empty() ? _fm.governingCarrier() : _negFareRest.carrier11()),
      departureDT,
      arrivalDT);
}

const PaxTypeFare*
NetRemitFareSelection::selectTktPblFare(const FareMarket& fm,
                                        const FareMarket* fm2,
                                        const FareMarket* fm3,
                                        const FareMarket* fm4)
{
  const PaxTypeFare* sPTF = nullptr;
  const std::vector<PaxTypeFare*>& ptFares =
      ((fm2 == nullptr && fm3 == nullptr && fm3 == nullptr)
           ? collectFaresFromFareMarket(&fm)
           : collectFaresFromFareMarkets(&fm, fm2, fm3, fm4));

  if (ptFares.empty())
    return nullptr;

  displayPotentialFares(&fm, fm2, fm3, fm4);
  // turn off diag
  DiagCollector* tmpDiag = _diag;
  if (!_axessUser)
    _diag = nullptr;

  sPTF = selectTktPblFare(ptFares);
  _diag = tmpDiag;
  if (sPTF != nullptr)
  {
    if (UNLIKELY(_diag != nullptr && !_axessUser))
    {
      (*_diag) << " \nFINAL FARE SELECTED:\n";
      displayFare(*sPTF);
    }
    adjustFare(fm, *sPTF);
    return sPTF;
  }

  if (UNLIKELY(_diag != nullptr))
  {
    (*_diag) << " \nNO FARE SELECTED\n";
  }

  return sPTF;
}

void
NetRemitFareSelection::displayPotentialFares(const FareMarket* fm,
                                             const FareMarket* fm2,
                                             const FareMarket* fm3,
                                             const FareMarket* fm4)
{
  if (LIKELY(_diag == nullptr))
    return;
  displayFareMarket(*fm);
  displayFares(fm);
  if (fm2)
  {
    displayFareMarket(*fm2);
    displayFares(fm2);
  }
  if (fm3)
  {
    displayFareMarket(*fm3);
    displayFares(fm3);
  }
  if (fm4)
  {
    displayFareMarket(*fm4);
    displayFares(fm4);
  }
}

void
NetRemitFareSelection::displayFares(const FareMarket* fm) const
{
  displayFares(fm, Fare::FS_PublishedFare);
  displayFares(fm, Fare::FS_ConstructedFare);
  displayFares(fm, Fare::FS_IndustryFare);
}

void
NetRemitFareSelection::displayFares(const FareMarket* fm, Fare::FareState fs) const
{
  if (_axessUser)
    return;
  if (fs == Fare::FS_PublishedFare)
  {
    (*_diag) << " \nSELECT PUBLISHED FARE\n";
  }
  else if (fs == Fare::FS_ConstructedFare)
  {
    (*_diag) << " \nSELECT CONSTRUCTED FARE\n";
  }
  else // fs == Fare::Fare::FS_IndustryFare
  {
    (*_diag) << " \nSELECT YY FARE\n";
  }

  const PaxTypeCode& paxCode = _fp.paxType()->paxType();
  std::vector<PaxTypeFare*> ptFares;
  findFaresForPaxType(ptFares, paxCode, fm);

  if (ptFares.empty())
    (*_diag) << "\nNO MATCHING FARES FOUND\n";

  bool isDisplayed = false;
  std::vector<PaxTypeFare*>::const_iterator itB = ptFares.begin(); // published fares on Market
  std::vector<PaxTypeFare*>::const_iterator itEnd = ptFares.end();
  for (; itB != itEnd; ++itB)
  {
    PaxTypeFare& paxFare = **itB;
    Fare* fare = paxFare.fare();

    if (filterFare(paxFare, fare, fs))
      continue;

    if (!_dispMatchFare && !_dispAllFares && !_axessUser)
    {
      isDisplayed = true;
      displayFare(paxFare);
    }

    if (!isMatchedFare(paxFare))
      continue;

    if (_dispMatchFare && !_dispAllFares)
    {
      isDisplayed = true;
      displayFare(paxFare);
    }
  }
  if (!isDisplayed)
    (*_diag) << "\nNO MATCHING FARES FOUND\n";
}

void
NetRemitFareSelection::adjustFare(const FareMarket& repricedFareMarket,
                                  const PaxTypeFare& paxTypeFare)
{
  if (_axessUser)
    return;

  if ((&repricedFareMarket == &_fm) || _fu.isOutbound())
    return;

  // Only tag 2 fare in inbound repriced fare market may have 1 cent extra problem
  if ((paxTypeFare.fare()->fareInfo()->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED) &&
      (_fp.itin()->calculationCurrency() != paxTypeFare.fare()->currency()))
  {
    if (_fp.itin()->calculationCurrency() != _rpCalculationCurrency)
    {
      Money fareCurrency(paxTypeFare.fare()->originalFareAmount(), paxTypeFare.fare()->currency());
      Money calcCurrency(_fp.itin()->calculationCurrency());

      MoneyAmount convertedAmount = 0.0;
      CurrencyConversionFacade ccFacade;
      if (ccFacade.convert(calcCurrency,
                           fareCurrency,
                           _trx,
                           _fp.itin()->calculationCurrency(),
                           convertedAmount,
                           _fp.itin()->useInternationalRounding()))
      {
        adjustNucFareAmount(paxTypeFare, convertedAmount);
      }
    }
    else
    {
      adjustNucFareAmount(paxTypeFare, paxTypeFare.fare()->nucOriginalFareAmount());
    }
  }
}

void
NetRemitFareSelection::adjustNucFareAmount(const PaxTypeFare& paxTypeFare, MoneyAmount fareAmount)
{
  if (paxTypeFare.fare()->nucFareAmount() > (fareAmount / 2))
  {
    // Make inbound fare to have less amount than outbound to make it even
    (const_cast<PaxTypeFare&>(paxTypeFare)).fare()->nucFareAmount() =
        (fareAmount - paxTypeFare.fare()->nucFareAmount());
  }
}

const PaxTypeFare*
NetRemitFareSelection::selectTktPblFare(const std::vector<PaxTypeFare*>& ptFares,
                                        Fare::FareState fs)
{
  const PaxTypeFare* sPTF = nullptr;
  std::vector<PaxTypeFare*> selFares; // selected fares

  // Select from same currency fares
  getTktPblFares(ptFares, selFares, fs, true);

  if (_axessUser)
    sPTF = selectValidTktPblFareForAxess(ptFares, selFares);
  else
    sPTF = selectValidTktPblFare(selFares);

  if (sPTF != nullptr)
  {
    if (!_axessUser)
    {
      if (UNLIKELY(_diag != nullptr))
      {
        (*_diag) << " \nFARE SELECTED:\n";
        displayFare(*sPTF);
      }
    }
    else // axessUser
    {
      _fu.selectedPTFs() = selFares;
    }

    return sPTF;
  }

  // Select from different currency fares
  selFares.clear();
  getTktPblFares(ptFares, selFares, fs, false);

  if (_axessUser)
    sPTF = selectValidTktPblFareForAxess(ptFares, selFares);
  else
    sPTF = selectValidTktPblFare(selFares);
  if (UNLIKELY(sPTF && _diag && !_axessUser))
  {
    (*_diag) << " \nFARE SELECTED:\n";
    displayFare(*sPTF);
  }

  if (_axessUser && sPTF != nullptr)
  {
    _fu.selectedPTFs() = selFares;
  }

  return sPTF;
}

const PaxTypeFare*
NetRemitFareSelection::selectTktPblFare(const std::vector<PaxTypeFare*>& ptFares)
{
  const PaxTypeFare* sPTF = nullptr;
  sPTF = selectTktPblFare(ptFares, true); // select from same currency
  if (sPTF == nullptr)
    sPTF = selectTktPblFare(ptFares, false); // select from diffrent currency
  return sPTF;
}

const PaxTypeFare*
NetRemitFareSelection::selectTktPblFare(const std::vector<PaxTypeFare*>& ptFares,
                                        bool isSameCurrency)
{
  const PaxTypeFare* sPTF = nullptr;
  sPTF = selectTktPblFare(ptFares, Fare::FS_PublishedFare, isSameCurrency);
  if (sPTF == nullptr)
    sPTF = selectTktPblFare(ptFares, Fare::FS_ConstructedFare, isSameCurrency);
  if (sPTF == nullptr)
    sPTF = selectTktPblFare(ptFares, Fare::FS_IndustryFare, isSameCurrency);
  return sPTF;
}

const PaxTypeFare*
NetRemitFareSelection::selectTktPblFare(const std::vector<PaxTypeFare*>& ptFares,
                                        Fare::FareState fs,
                                        bool isSameCurrency)
{
  const PaxTypeFare* sPTF = nullptr;
  std::vector<PaxTypeFare*> selFares; // selected fares

  getTktPblFares(ptFares, selFares, fs, isSameCurrency);

  if (_axessUser)
  {
    sPTF = selectValidTktPblFareForAxess(ptFares, selFares);
    if (sPTF)
      _fu.selectedPTFs() = selFares;
  }
  else
    sPTF = selectValidTktPblFare(selFares);
  return sPTF;
}

const PaxTypeFare*
NetRemitFareSelection::selectValidTktPblFareForAxess(const std::vector<PaxTypeFare*>& ptFares,
                                                     std::vector<PaxTypeFare*>& selFares)
{
  // check footnotes cat14, cat15
  checkFootnotesC14C15(selFares);
  if (selFares.empty())
    return nullptr; // bad selection

  // check routing number
  checkRoutingNo(selFares);
  if (selFares.empty())
    return nullptr;

  // reject hard_failed fares
  checkRuleStatusForJalAxess(ptFares, selFares);
  if (selFares.empty())
    return nullptr;

  if (UNLIKELY(_diag != nullptr))
  {
    (*_diag) << " \nFARES SELECTED:\n";
    std::vector<PaxTypeFare*>::iterator itSFB = selFares.begin();
    std::vector<PaxTypeFare*>::iterator itSFEnd = selFares.end();

    for (; itSFB != itSFEnd; ++itSFB)
    {
      PaxTypeFare& pFare = **itSFB;
      displayFare(pFare);
    }
  }

  return selFares.front();
}

const PaxTypeFare*
NetRemitFareSelection::selectValidTktPblFare(std::vector<PaxTypeFare*>& selFares)
{
  bool users = _trx.getRequest()->ticketingAgent()->abacusUser() ||
               _trx.getRequest()->ticketingAgent()->infiniUser() ||
               TrxUtil::isNetRemitEnabled(_trx);

  if (!checkAndBetw(selFares, users))
    return nullptr; // bad selection

  if (selFares.size() == 1)
    return selFares.front(); // done

  // check footnotes cat14, cat15
  checkFootnotesC14C15(selFares);
  if (selFares.empty())
    return nullptr; // bad selection

  if (selFares.size() == 1) // Only one applicable fare
    return selFares.front();

  // check routing number
  checkRoutingNo(selFares);
  if (selFares.empty())
    return nullptr;

  if (selFares.size() == 1)
    return selFares.front();

  // JAL/AXESS:
  // skip fareFamilyMatch selection (cat3)
  if (!_axessUser)
  {
    if (isFareFamilySelection())
    {
      // Fare_family processing
      // check seasons rule
      // send back the 1st lowest fare
      checkSeasons(selFares);
      if (selFares.empty())
        return nullptr;

      return selFares.front(); // Select the lowest fare
    }
  }

  // check seasons and blackouts rule
  // exit with the 1st passed fare (this fare has a lowest amount)
  checkSeasonsBlackouts(selFares);
  if (selFares.empty())
    return nullptr;

  if ((TrxUtil::cat35LtypeEnabled(_trx) && _trx.getRequest()->ticketingAgent()->abacusUser()) ||
      TrxUtil::isNetRemitEnabled(_trx))
  {
    return selectPublicAgainstPrivateFare(selFares);
  }

  return selFares.front();
}

bool
NetRemitFareSelection::isFareFamilySelection()
{
  return isFareFamilySelection(_negFareRest.fareClass1());
}

bool
NetRemitFareSelection::isFareFamilySelection(const FareClassCode& fareClass)
{
  if (fareClass.empty())
    return false;

  size_t posH = fareClass.find("-");

  if (posH == std::string::npos)
    return false;
  else
    return true; // fare family request
}

void
NetRemitFareSelection::getTktPblFares(const std::vector<PaxTypeFare*>& ptFares,
                                      std::vector<PaxTypeFare*>& selFares,
                                      Fare::FareState fs,
                                      bool sameCurrency)
{
  const PaxTypeFare& ptf = *(_fu.paxTypeFare()); // Cat35 NetRemit fare
  if (UNLIKELY(_diag != nullptr && _axessUser))
  {
    if (fs == Fare::FS_PublishedFare)
    {
      (*_diag) << " \nSELECT PUBLISHED FARE WITH ";
    }
    else if (fs == Fare::FS_ConstructedFare)
    {
      (*_diag) << " \nSELECT CONSTRUCTED FARE WITH ";
    }
    else // fs == Fare::Fare::FS_IndustryFare
    {
      (*_diag) << " \nSELECT YY FARE WITH ";
    }

    (*_diag) << (sameCurrency ? "SAME" : "DIFF") << " CURRENCY\n";
  }

  std::vector<PaxTypeFare*>::const_iterator itB = ptFares.begin(); // published fares on Market
  std::vector<PaxTypeFare*>::const_iterator itEnd = ptFares.end();
  for (; itB != itEnd; ++itB)
  {
    PaxTypeFare& paxFare = **itB;
    Fare* fare = paxFare.fare();

    if ((sameCurrency && paxFare.currency() != ptf.currency()) ||
        (!sameCurrency && paxFare.currency() == ptf.currency()))
      continue;

    if (filterFare(paxFare, fare, fs))
      continue;

    if (UNLIKELY(_diag && !_dispMatchFare && !_dispAllFares && !_axessUser))
    {
      displayFare(paxFare);
    }

    if (!isMatchedFare(paxFare))
      continue;

    if (UNLIKELY(_diag && _dispMatchFare && !_dispAllFares))
    {
      displayFare(paxFare);
    }

    selFares.push_back(&paxFare);
  }
}

bool
NetRemitFareSelection::filterFare(PaxTypeFare& paxFare, Fare* fare, Fare::FareState fs) const
{
  if ((TrxUtil::cat35LtypeEnabled(_trx) && _trx.getRequest()->ticketingAgent()->abacusUser()) ||
      TrxUtil::isNetRemitEnabled(_trx))
  {
    if (paxFare.isFareByRule())
      return true;
    if (paxFare.fcaDisplayCatType() == RuleConst::SELLING_FARE ||
        paxFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE ||
        paxFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE_UPD)
    {
      if (!paxFare.isNegotiated())
        return true;
    }
    else if (paxFare.tcrTariffCat() == RuleConst::PRIVATE_TARIFF && !cat15Valid(paxFare))
      return true;
  }
  else
  {
    if (paxFare.tcrTariffCat() == RuleConst::PRIVATE_TARIFF || paxFare.isNegotiated() ||
        paxFare.isFareByRule())
      return true;
  }

  if (fs == Fare::FS_PublishedFare)
  {
    if (!fare->isPublished() || fare->isIndustry())
      return true;
  }
  else if (fs == Fare::FS_ConstructedFare)
  {
    if (!fare->isConstructed() || fare->isIndustry())
      return true;
  }
  else // fs == Fare::Fare::FS_IndustryFare
  {
    if (!fare->isIndustry())
      return true;
    IndustryFare* indf = static_cast<IndustryFare*>(fare);
    if (indf != nullptr && !indf->validForPricing())
      return true;
  }

  if (_isCmdPricing && !_axessUser && !paxFare.fcasPaxType().empty() &&
      paxFare.fcasPaxType() != ADULT &&
      PaxTypeUtil::isAnActualPaxInTrx(_trx, paxFare.carrier(), paxFare.fcasPaxType()) == nullptr)
    return true;

  return false;
}

bool
NetRemitFareSelection::isMatchedFare(const PaxTypeFare& paxTypeFare) const
{
  if (_selSeg1)
  {
    if (_negFareRest.owrt1() != ' ' &&
        !RuleUtil::matchOneWayRoundTrip(_negFareRest.owrt1(), paxTypeFare.owrt()))
      return false;

    if (_negFareRest.globalDir1() != GlobalDirection::ZZ &&
        _negFareRest.globalDir1() != paxTypeFare.globalDirection())
      return false;

    if (_negFareRest.ruleTariff1() != 0 &&
        _negFareRest.ruleTariff1() != paxTypeFare.tcrRuleTariff())
      return false;

    if (!_negFareRest.carrier11().empty() && paxTypeFare.carrier() != INDUSTRY_CARRIER &&
        _negFareRest.carrier11() != paxTypeFare.carrier())
      return false;

    if (!_negFareRest.rule1().empty() && _negFareRest.rule1() != paxTypeFare.ruleNumber())
      return false;

    // JAL/AXESS:
    // skip match on: fareClass, fareType, seasonsCode, DOW.
    if (_axessUser)
      return true;
    //

    if (!_negFareRest.fareClass1().empty() &&
        !RuleUtil::matchFareClass(_negFareRest.fareClass1().c_str(),
                                  paxTypeFare.fareClass().c_str()))
      return false;

    if (!_negFareRest.fareType1().empty() &&
        _negFareRest.fareType1() != paxTypeFare.fcaFareType()) // exactly match
      return false;

    if (_negFareRest.seasonType1() != ' ' &&
        _negFareRest.seasonType1() != paxTypeFare.fcaSeasonType())
      return false;

    if (_negFareRest.dowType1() != ' ' && _negFareRest.dowType1() != paxTypeFare.fcaDowType())
      return false;
  }
  else
  {
    if (_negFareRest.owrt2() != ' ' &&
        !RuleUtil::matchOneWayRoundTrip(_negFareRest.owrt2(), paxTypeFare.owrt()))
      return false;

    if (_negFareRest.globalDir2() != GlobalDirection::ZZ &&
        _negFareRest.globalDir2() != paxTypeFare.globalDirection())
      return false;

    if (_negFareRest.ruleTariff2() != 0 &&
        _negFareRest.ruleTariff2() != paxTypeFare.tcrRuleTariff())
      return false;

    if (!_negFareRest.carrier21().empty() && _negFareRest.carrier21() != paxTypeFare.carrier())
      return false;

    if (!_negFareRest.rule2().empty() && _negFareRest.rule2() != paxTypeFare.ruleNumber())
      return false;

    // JAL/AXESS:
    // skip match on: fareClass, fareType, seasonsCode, DOW.
    if (_axessUser)
      return true;
    //

    if (!_negFareRest.fareClass2().empty() &&
        !RuleUtil::matchFareClass(_negFareRest.fareClass2().c_str(),
                                  paxTypeFare.fareClass().c_str()))
      return false;

    if (!_negFareRest.fareType2().empty() &&
        _negFareRest.fareType2() != paxTypeFare.fcaFareType()) // exactly match
      return false;

    if (_negFareRest.seasonType2() != ' ' &&
        _negFareRest.seasonType2() != paxTypeFare.fcaSeasonType())
      return false;

    if (_negFareRest.dowType2() != ' ' && _negFareRest.dowType2() != paxTypeFare.fcaDowType())
      return false;
  }
  return true;
}

void
NetRemitFareSelection::checkSeasons(std::vector<PaxTypeFare*>& selFares)
{
  std::vector<PaxTypeFare*> newSelFares; // new selected fares

  std::vector<uint16_t> categories;
  categories.push_back(RuleConst::SEASONAL_RULE);

  std::vector<PaxTypeFare*>::iterator itSFB = selFares.begin();
  std::vector<PaxTypeFare*>::iterator itSFEnd = selFares.end();

  for (; itSFB != itSFEnd; ++itSFB)
  {
    PaxTypeFare& pFare = **itSFB;
    pFare.setSelectedFareNetRemit();

    FareUsage fu;
    fu.paxTypeFare() = &pFare;
    fu.inbound() = _fu.inbound();
    RuleControllerWithChancelor<PricingUnitRuleController> ruleController(
        DynamicValidation, categories, &_trx);
    if (ruleController.validate(_trx, *(const_cast<FarePath*>(&_fp)), _pu, fu))
      newSelFares.push_back(&pFare);
    pFare.setSelectedFareNetRemit(false);
  }

  if (newSelFares.empty())
    return; // all fares are eligible.

  // Update vector with fares that passed seasons
  selFares.clear();
  selFares.insert(selFares.begin(), newSelFares.begin(), newSelFares.end());
}

void
NetRemitFareSelection::checkSeasonsBlackouts(std::vector<PaxTypeFare*>& selFares)
{
  std::vector<PaxTypeFare*> newSelFares; // new selected fares

  std::vector<uint16_t> categories;
  categories.push_back(RuleConst::SEASONAL_RULE);
  categories.push_back(RuleConst::BLACKOUTS_RULE);

  std::vector<PaxTypeFare*>::iterator itSFB = selFares.begin();
  std::vector<PaxTypeFare*>::iterator itSFEnd = selFares.end();

  for (; itSFB != itSFEnd; ++itSFB)
  {
    PaxTypeFare& pFare = **itSFB;
    pFare.setSelectedFareNetRemit();

    FareUsage fu;
    fu.paxTypeFare() = &pFare;
    fu.inbound() = _fu.inbound();
    RuleControllerWithChancelor<PricingUnitRuleController> ruleController(
        DynamicValidation, categories, &_trx);
    if (ruleController.validate(_trx, *(const_cast<FarePath*>(&_fp)), _pu, fu))
      newSelFares.push_back(&pFare);
    pFare.setSelectedFareNetRemit(false);
  }

  if (newSelFares.empty())
    return; // all fares are eligible.

  // Update vector with fares that passed seasons and blackouts
  selFares.clear();
  selFares.insert(selFares.begin(), newSelFares.begin(), newSelFares.end());
}

void
NetRemitFareSelection::checkRoutingNo(std::vector<PaxTypeFare*>& selFares)
{
  std::vector<PaxTypeFare*> newSelFares; // new selected fares

  const PaxTypeFare& ptf35 = *(_fu.paxTypeFare()); // Cat35 NetRemit fare

  const RoutingNumber& rtn35 = ptf35.routingNumber(); // Routing from Cat35 fare

  std::vector<PaxTypeFare*>::iterator itSFB = selFares.begin();
  std::vector<PaxTypeFare*>::iterator itSFEnd = selFares.end();

  for (; itSFB != itSFEnd; ++itSFB)
  {
    PaxTypeFare& pFare = **itSFB;

    if (pFare.routingNumber() == rtn35)
    {
      newSelFares.push_back(&pFare);
    }
  }
  if (newSelFares.empty())
    return; // all fares are eligible.

  // Update vector with fares that passed routing selection
  selFares.clear();
  selFares.insert(selFares.begin(), newSelFares.begin(), newSelFares.end());
}

namespace
{

struct FareAmountComparator
{
  bool operator()(const PaxTypeFare* ptf1, const PaxTypeFare* ptf2) const
  {
    MoneyAmount diff = fabs(ptf1->nucFareAmount() - ptf2->nucFareAmount());
    // if amounts are equal
    if (diff <= EPSILON)
      return ptf1->tcrTariffCat() < ptf2->tcrTariffCat();
    return ptf1->nucFareAmount() < ptf2->nucFareAmount();
  }
};
}

const PaxTypeFare*
NetRemitFareSelection::selectPublicAgainstPrivateFare(const std::vector<PaxTypeFare*>& selFares)
    const
{
  if (selFares.size() == 0)
    return nullptr;

  return *std::min_element(selFares.begin(), selFares.end(), FareAmountComparator());
}

void
NetRemitFareSelection::checkFootnotesC14C15(std::vector<PaxTypeFare*>& selFares)
{
  std::vector<PaxTypeFare*> newSelFares; // new selected fares

  const Itin* itin = _fp.itin();

  std::vector<uint16_t> cat14;
  cat14.push_back(RuleConst::TRAVEL_RESTRICTIONS_RULE);

  std::vector<uint16_t> cat15;
  cat15.push_back(RuleConst::SALE_RESTRICTIONS_RULE);

  std::vector<PaxTypeFare*>::iterator itSFB = selFares.begin();
  std::vector<PaxTypeFare*>::iterator itSFEnd = selFares.end();

  for (; itSFB != itSFEnd; ++itSFB)
  {
    PaxTypeFare& pFare = **itSFB;
    pFare.setSelectedFareNetRemit();

    RuleControllerWithChancelor<FareMarketRuleController> ruleContr14(DynamicValidation, cat14);
    if (ruleContr14.validate(_trx, *(const_cast<Itin*>(itin)), pFare))
    {
      RuleControllerWithChancelor<FareMarketRuleController> ruleContr15(DynamicValidation, cat15);
      if (ruleContr15.validate(_trx, *(const_cast<Itin*>(itin)), pFare))
      {
        pFare.setSelectedFareNetRemit(false);
        newSelFares.push_back(&pFare);
        continue;
      }
    }
    pFare.setSelectedFareNetRemit(false);
  }
  if (newSelFares.empty())
    return; // all fares are eligible.

  // Update vector with fares that passed Cat14 and Cat15 footnotes
  selFares.clear();
  selFares.insert(selFares.begin(), newSelFares.begin(), newSelFares.end());
}

bool
NetRemitFareSelection::checkAndBetw(std::vector<PaxTypeFare*>& selFares, bool isAbacusEnabled)
{
  if (isAbacusEnabled)
  {
    checkAndBetw(selFares);
    if (selFares.empty())
      return false;
  }
  return true;
}

void
NetRemitFareSelection::checkAndBetw(std::vector<PaxTypeFare*>& selFares)
{
  if (!_negFareRest.betwCity1().empty() || !_negFareRest.andCity1().empty())
    return;

  std::vector<PaxTypeFare*> newSelFares; // new selected fares
  std::vector<PaxTypeFare*>::iterator itSFB = selFares.begin();
  std::vector<PaxTypeFare*>::iterator itSFEnd = selFares.end();

  bool isSelectNewFare;
  for (; itSFB != itSFEnd; ++itSFB)
  {
    isSelectNewFare = false;
    PaxTypeFare& pFare = **itSFB;

    if (_isDirFlippedInNewFmForJCB)
    {
      if (pFare.directionality() == BOTH || (_fu.isInbound() && pFare.directionality() == FROM) ||
          (_fu.isOutbound() && pFare.directionality() == TO))
        isSelectNewFare = true;
    }
    else if (pFare.directionality() == BOTH || (_fu.isInbound() && pFare.directionality() == TO) ||
             (_fu.isOutbound() && pFare.directionality() == FROM))
    {
      isSelectNewFare = true;
    }

    if (isSelectNewFare)
    {
      newSelFares.push_back(&pFare);
    }
  }

  selFares.clear();
  selFares.insert(selFares.begin(), newSelFares.begin(), newSelFares.end());
}

void
NetRemitFareSelection::checkRuleStatusForJalAxess(const std::vector<PaxTypeFare*>& ptFares,
                                                  std::vector<PaxTypeFare*>& selFares)
{
  std::vector<PaxTypeFare*> newSelFares; // new selected fares

  const Itin* itin = _fp.itin();

  const PaxTypeFare& ptf35 = *(_fu.paxTypeFare()); // Cat35 NetRemit fare
  const PaxType* paxtype = _fp.paxType();
  const VendorCode vendor = ptf35.vendor();

  bool isInfant = false;
  bool isChild = false;
  bool isAdult = false;

  isInfant = PaxTypeUtil::isInfant(_trx, paxtype->paxType(), vendor);
  if (!isInfant)
  {
    isChild = PaxTypeUtil::isChild(_trx, paxtype->paxType(), vendor);
    if (!isChild)
    {
      isAdult = true;
    }
  }

  std::vector<PaxTypeFare*>::const_iterator itPTFEnd = ptFares.end();

  std::vector<PaxTypeFare*>::iterator itSFB = selFares.begin();
  std::vector<PaxTypeFare*>::iterator itSFEnd = selFares.end();

  std::vector<uint16_t> fcCategories;

  for (; itSFB != itSFEnd; ++itSFB)
  {
    fcCategories.clear();
    PaxTypeFare& pFare = **itSFB;

    if (pFare.paxType() == nullptr)
    {
      pFare.paxType() =
          const_cast<PaxType*>(PaxTypeUtil::isAnActualPaxInTrx(_trx, pFare.carrier(), ADULT));
      if (pFare.paxType() == nullptr)
      {
        continue;
      }
    }

    if (pFare.isFareClassAppMissing())
    {
      continue;
    }

    if (isAdult && !PaxTypeUtil::isAdult(_trx, pFare.fcasPaxType(), pFare.vendor()))
    {
      continue;
    }
    else if (isChild && PaxTypeUtil::isInfant(_trx, pFare.fcasPaxType(), pFare.vendor()))
    {
      continue;
    }

    if (pFare.isCategoryProcessed(RuleConst::DAY_TIME_RULE) &&
        !pFare.isCategoryValid(RuleConst::DAY_TIME_RULE))
      continue;
    fcCategories.push_back(RuleConst::DAY_TIME_RULE);

    if (pFare.isCategoryProcessed(RuleConst::SEASONAL_RULE) &&
        !pFare.isCategoryValid(RuleConst::SEASONAL_RULE))
      continue;
    fcCategories.push_back(RuleConst::SEASONAL_RULE);

    if (pFare.isCategoryProcessed(RuleConst::FLIGHT_APPLICATION_RULE) &&
        !pFare.isCategoryValid(RuleConst::FLIGHT_APPLICATION_RULE))
      continue;
    fcCategories.push_back(RuleConst::FLIGHT_APPLICATION_RULE);

    if (pFare.isCategoryProcessed(RuleConst::BLACKOUTS_RULE) &&
        !pFare.isCategoryValid(RuleConst::BLACKOUTS_RULE))
      continue;
    fcCategories.push_back(RuleConst::BLACKOUTS_RULE);

    if (pFare.isCategoryProcessed(RuleConst::MISC_FARE_TAG) &&
        !pFare.isCategoryValid(RuleConst::MISC_FARE_TAG))
      continue;
    fcCategories.push_back(RuleConst::MISC_FARE_TAG);

    if (pFare.isCategoryProcessed(RuleConst::ADVANCE_RESERVATION_RULE) &&
        !pFare.isCategoryValid(RuleConst::ADVANCE_RESERVATION_RULE))
      continue;
    fcCategories.push_back(RuleConst::ADVANCE_RESERVATION_RULE);

    if (pFare.isCategoryProcessed(RuleConst::TRAVEL_RESTRICTIONS_RULE) &&
        !pFare.isCategoryValid(RuleConst::TRAVEL_RESTRICTIONS_RULE))
      continue;
    fcCategories.push_back(RuleConst::TRAVEL_RESTRICTIONS_RULE);

    if (pFare.isCategoryProcessed(RuleConst::SALE_RESTRICTIONS_RULE) &&
        !pFare.isCategoryValid(RuleConst::SALE_RESTRICTIONS_RULE))
      continue;
    fcCategories.push_back(RuleConst::SALE_RESTRICTIONS_RULE);

    if (pFare.isCategoryProcessed(RuleConst::MINIMUM_STAY_RULE) &&
        !pFare.isCategoryValid(RuleConst::MINIMUM_STAY_RULE))
      continue;
    fcCategories.push_back(RuleConst::MINIMUM_STAY_RULE);

    if (pFare.isCategoryProcessed(RuleConst::MAXIMUM_STAY_RULE) &&
        !pFare.isCategoryValid(RuleConst::MAXIMUM_STAY_RULE))
      continue;
    fcCategories.push_back(RuleConst::MAXIMUM_STAY_RULE);

    if (pFare.isCategoryProcessed(RuleConst::STOPOVER_RULE) &&
        !pFare.isCategoryValid(RuleConst::STOPOVER_RULE))
      continue;
    fcCategories.push_back(RuleConst::STOPOVER_RULE);

    if (pFare.isCategoryProcessed(RuleConst::TRANSFER_RULE) &&
        !pFare.isCategoryValid(RuleConst::TRANSFER_RULE))
      continue;
    fcCategories.push_back(RuleConst::TRANSFER_RULE);

    if (pFare.isCategoryProcessed(RuleConst::CHILDREN_DISCOUNT_RULE) &&
        !pFare.isCategoryValid(RuleConst::CHILDREN_DISCOUNT_RULE))
      continue;
    fcCategories.push_back(RuleConst::CHILDREN_DISCOUNT_RULE);

    if (pFare.isCategoryProcessed(RuleConst::TOUR_DISCOUNT_RULE) &&
        !pFare.isCategoryValid(RuleConst::TOUR_DISCOUNT_RULE))
      continue;
    fcCategories.push_back(RuleConst::TOUR_DISCOUNT_RULE);

    if (pFare.isCategoryProcessed(RuleConst::AGENTS_DISCOUNT_RULE) &&
        !pFare.isCategoryValid(RuleConst::AGENTS_DISCOUNT_RULE))
      continue;
    fcCategories.push_back(RuleConst::AGENTS_DISCOUNT_RULE);

    if (pFare.isCategoryProcessed(RuleConst::OTHER_DISCOUNT_RULE) &&
        !pFare.isCategoryValid(RuleConst::OTHER_DISCOUNT_RULE))
      continue;
    fcCategories.push_back(RuleConst::OTHER_DISCOUNT_RULE);

    if (pFare.isCategoryProcessed(RuleConst::ACCOMPANIED_PSG_RULE) &&
        !pFare.isCategoryValid(RuleConst::ACCOMPANIED_PSG_RULE))
      continue;
    fcCategories.push_back(RuleConst::ACCOMPANIED_PSG_RULE);

    {
      boost::lock_guard<boost::mutex> g(ruleRtgMutex);

      // Axess WPNETT (normal pricing)    --> single thread for the final FPath for each PAX.
      // Axess WPNETT ('no match' pricing)--> multi_threading...
      RuleControllerWithChancelor<FareMarketRuleController> ruleValFC(
          NormalValidation, fcCategories, &_trx);
      //       FareMarketRuleController ruleValFC(DynamicValidation, fcCategories);
      if (ruleValFC.validate(_trx, *(const_cast<Itin*>(itin)), pFare))
      {
        if (pFare.isCategoryValid(RuleConst::DAY_TIME_RULE) &&
            pFare.isCategoryValid(RuleConst::SEASONAL_RULE) &&
            pFare.isCategoryValid(RuleConst::FLIGHT_APPLICATION_RULE) &&
            pFare.isCategoryValid(RuleConst::BLACKOUTS_RULE) &&
            pFare.isCategoryValid(RuleConst::MISC_FARE_TAG) &&
            pFare.isCategoryValid(RuleConst::ADVANCE_RESERVATION_RULE) &&
            pFare.isCategoryValid(RuleConst::TRAVEL_RESTRICTIONS_RULE) &&
            pFare.isCategoryValid(RuleConst::SALE_RESTRICTIONS_RULE) &&
            pFare.isCategoryValid(RuleConst::MINIMUM_STAY_RULE) &&
            pFare.isCategoryValid(RuleConst::MAXIMUM_STAY_RULE) &&
            pFare.isCategoryValid(RuleConst::STOPOVER_RULE) &&
            pFare.isCategoryValid(RuleConst::TRANSFER_RULE) &&
            pFare.isCategoryValid(RuleConst::CHILDREN_DISCOUNT_RULE) &&
            pFare.isCategoryValid(RuleConst::TOUR_DISCOUNT_RULE) &&
            pFare.isCategoryValid(RuleConst::AGENTS_DISCOUNT_RULE) &&
            pFare.isCategoryValid(RuleConst::OTHER_DISCOUNT_RULE) &&
            pFare.isCategoryValid(RuleConst::ACCOMPANIED_PSG_RULE))
        {
          processRouting(&pFare);

          if (pFare.isRoutingValid())
          {
            std::vector<PaxTypeFare*>::const_iterator itPTFB = ptFares.begin();
            for (; itPTFB != itPTFEnd; ++itPTFB)
            {
              if (*itPTFB == *itSFB)
              {
                pFare.ticketedFareForAxess() = true;
                newSelFares.push_back(&pFare);
              }
            }
          }
        }
      }
    }
  }
  selFares.clear();

  if (!newSelFares.empty())
    selFares.insert(selFares.begin(), newSelFares.begin(), newSelFares.end());

  return;
}
bool
NetRemitFareSelection::cat15Valid(const PaxTypeFare& paxTypeFare) const
{
  std::vector<uint16_t> categories;
  categories.push_back(RuleConst::SALE_RESTRICTIONS_RULE);

  RuleControllerWithChancelor<FareMarketRuleController> ruleController(
      DynamicValidation, categories, &_trx);
  return ruleController.validate(
      _trx, *(const_cast<Itin*>(_fp.itin())), const_cast<PaxTypeFare&>(paxTypeFare));
}
}

