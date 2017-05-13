
#include "FareCalc/FcDispFareUsage.h"

#include "Common/CurrencyConverter.h"
#include "Common/FallbackUtil.h"
#include "Common/FareMarketUtil.h"
#include "Common/GlobalDirectionFinderV2Adapter.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/Money.h"
#include "Common/TravelSegUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/MileageTypeData.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/NetRemitFarePath.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/SurchargeData.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/Loc.h"
#include "FareCalc/CalcTotals.h"
#include "FareCalc/FareCalcCollector.h"
#include "FareCalc/FareCalcConsts.h"
#include "FareCalc/FareCalcHelper.h"
#include "FareCalc/FareCalculation.h"
#include "FareCalc/FareUsageIter.h"
#include "FareCalc/FcStream.h"
#include "FareCalc/FcUtil.h"
#include "Routing/MileageInfo.h"
#include "Rules/NegotiatedFareRuleUtil.h"
#include "Rules/RuleConst.h"


namespace tse
{
FALLBACK_DECL(fallbackForINFINITvlCommencementDate);

namespace
{
Logger logger("atseintl.FareCalc.FcDispFareUsage");
}

FcDispFareUsage::FcDispFareUsage(PricingTrx& trx,
                                 const FarePath& fp,
                                 const CalcTotals* calcTotals,
                                 const FareCalcConfig& fcConfig,
                                 FareCalcCollector& fcCollector,
                                 FareCalc::FcStream& os,
                                 const FareCalc::FareAmountAdjustment& fareAmountAdjustment,
                                 bool useNetPubFbc)
  : FcDispItem(trx, fcConfig, fcCollector),
    _fp(fp),
    _os(os),
    _calcTotals(calcTotals),
    _matchCurrencyCode(false),
    _noDec(0),
    _surchargeCount(0),
    _prevFareUsage(nullptr),
    _prevDest(""),
    _inSideTrip(false),
    _fareAmountAdjustment(fareAmountAdjustment),
    _originalCalcTotals(nullptr),
    _prevDestConnection(false),
    _useNetPubFbc(useNetPubFbc)
{
  if (LIKELY(_calcTotals != nullptr))
  {
    if (_fp.calculationCurrency() == _calcTotals->equivCurrencyCode)
      _matchCurrencyCode = true;

    if (_fp.calculationCurrency() == NUC)
    {
      _noDec = 2; // NUC has 2 decimal
    }
    else if (trx.excTrxType() == PricingTrx::PORT_EXC_TRX && _fp.calculationCurrency() != NUC &&
             _fp.calculationCurrency() != _fp.baseFareCurrency())
    {
      _noDec = _calcTotals->calcCurrencyNoDec;
    }
    else
    {
      _noDec = _calcTotals->convertedBaseFareNoDec;
    }
  }

  const NetRemitFarePath* netRemitFp = dynamic_cast<const NetRemitFarePath*>(&_fp);
  if (netRemitFp != nullptr)
  {
    _originalCalcTotals = fcCollector.findCalcTotals(netRemitFp->originalFarePath());
  }
  else if (trx.getRequest()->ticketingAgent()->axessUser() &&
           trx.getRequest()->isWpNettRequested() && _fp.originalFarePathAxess() != nullptr)
  {
    _originalCalcTotals = fcCollector.findCalcTotals(_fp.originalFarePathAxess());
  }
}

void
FcDispFareUsage::
operator()(const FareUsage* fu)
{
  if (LIKELY(fu == nullptr || _calcTotals == nullptr || fu == _prevFareUsage))
    return;

  if (!_inSideTrip && _prevFareUsage &&
      _fp.itin()->segmentOrder(fu->travelSeg().front()) <
          _fp.itin()->segmentOrder(_prevFareUsage->travelSeg().back()))
    return;

  displayFareUsage(fu);

  _prevFareUsage = fu;
}

void
FcDispFareUsage::displayTravelCommenceDate()
{
  if (fallback::fallbackForINFINITvlCommencementDate(&_trx) && _trx.getOptions()->isRecordQuote())
  {
    return;
  }

  if (_trx.getOptions()->returnAllData() == GDS ||
      _trx.getOptions()->returnAllData() == WD || _trx.getOptions()->returnAllData() == FP)
  {
    return;
  }

  if (!fallback::fallbackForINFINITvlCommencementDate(&_trx))
  {
    TravelSeg* tvlS = _trx.travelSeg().front();
    if (tvlS->isAir())
    {
      DateTime& depDate (tvlS->departureDT()),
                tckDate (_trx.getRequest()->ticketingDT());

      if (!depDate.isEmptyDate())
      {
        _os << depDate.dateToString(DDMMMYY, "");
      }
      else if (!tckDate.isEmptyDate())
      {
        _os << tckDate.dateToString(DDMMMYY, "");
      }

    }
  }
  else
  {
    TravelSeg* tvlS = _trx.travelSeg().front();
    AirSeg* airS = dynamic_cast<AirSeg*>(tvlS);
    if (airS != nullptr && tvlS != nullptr)
      _os << tvlS->departureDT().dateToString(DDMMMYY, "");
  }
}

const std::vector<TravelSeg*>&
FcDispFareUsage::getTravelSeg(const FareUsage* netOriginalFu, const FareUsage* fu) const
{
  if (UNLIKELY(netOriginalFu))
  {
    // Do not use origTravelSeg if netOriginalFu has TFDPSC
    return (netOriginalFu->netRemitPscResults().size() /*has TFDPSC*/) ? fu->travelSeg()
                                                                       : netOriginalFu->travelSeg();
  }
  return fu->travelSeg();
}

void
FcDispFareUsage::displayFareUsage(const FareUsage* fu)
{
  bool isWqTrx = (dynamic_cast<NoPNRPricingTrx*>(&_trx) != nullptr);

  const FareUsage* netOriginalFu = nullptr;
  if (UNLIKELY(_originalCalcTotals != nullptr))
  {
    const NetRemitFarePath* netRemitFp = dynamic_cast<const NetRemitFarePath*>(&_fp);
    if (netRemitFp != nullptr)
    {
      netOriginalFu = netRemitFp->getOriginalFareUsage(fu);
      if ((netOriginalFu != nullptr) && (netOriginalFu->paxTypeFare()->fareMarket() ==
                                   fu->paxTypeFare()->fareMarket())) // No fare market change
      {
        netOriginalFu = nullptr;
      }
    }
  }

  // For Net Remit retrieved published fare, need to display
  // from original travel segments but with surcharges from new travel segments
  // in the retrieved published fare market.
  const std::vector<TravelSeg*>& tvlSeg = getTravelSeg(netOriginalFu, fu);

  DataHandle dataHandle(_trx.ticketingDate());

  bool sideTripProcessed = false;
  bool fareBasisSpace = (_prevFareUsage != nullptr);

  LocCode orig, dest;

  uint16_t pos = 0;

  CalcTotals& calcTotals = const_cast<CalcTotals&>(*_calcTotals);

  const TravelSeg* lastTvlSeg = tvlSeg.back();

  FareCalc::Group fcGroup(_os);
  bool cnxWithCityGroup = false;
  for (std::vector<TravelSeg*>::const_iterator ibegin = tvlSeg.begin(),
                                               i = ibegin,
                                               iend = tvlSeg.end();
       i != iend;
       _prevDest = dest, ++i)
  {
    const TravelSeg* ts = *i;

    const TravelSeg* netRemitTs = nullptr;
    if (UNLIKELY(netOriginalFu != nullptr))
      netRemitTs = netOriginalFu->getTktNetRemitTravelSeg(ts);

    orig = getDisplayLoc(*_fcConfig.fcc(),
                         _fp.itin()->geoTravelType(),
                         ts->boardMultiCity(),
                         ts->origAirport(),
                         fu->paxTypeFare()->carrier(),
                         ts->departureDT(),
                         isWqTrx);

    dest = getDisplayLoc(*_fcConfig.fcc(),
                         _fp.itin()->geoTravelType(),
                         ts->offMultiCity(),
                         ts->destAirport(),
                         fu->paxTypeFare()->carrier(),
                         ts->departureDT(),
                         isWqTrx);

    if ((orig == dest) && (dynamic_cast<const AirSeg*>(ts) == nullptr))
    {
      processSideTrip(fu, ts, sideTripProcessed, cnxWithCityGroup, fcGroup);
      continue;
    }

    bool firstCity = (i == ibegin && _prevFareUsage == nullptr && _prevDest.empty());

    bool fareBreak = ts->isForcedFareBrk() || ts == lastTvlSeg;
    bool prevFareBreak = (i == ibegin && !firstCity);

    if (firstCity && _fcConfig.tvlCommencementDate() == FareCalcConsts::FC_YES)
    {
      displayTravelCommenceDate();
    }

    LOG4CXX_DEBUG(logger,
                  "\n" << fu << ":" << ts->origin()->loc() << "-" << ts->destination()->loc()
                       << ", firstCity: " << firstCity << ", fareBreak: " << fareBreak
                       << ", arunk: " << (_prevDest != orig));

    const TravelSeg* nextSeg = ts;
    if (i + 1 != iend)
    {
      nextSeg = *(i + 1);
    }

    if (fu->hasSideTrip() && !sideTripProcessed && isNotArunkSegBeforeSideTrip(fu, nextSeg))
    {
      processSideTrip(fu, ts, sideTripProcessed, cnxWithCityGroup, fcGroup);
      cnxWithCityGroup =
          displayOrigAirport(ts, orig, firstCity, prevFareBreak, cnxWithCityGroup, &fcGroup);
    }
    else
    {
      cnxWithCityGroup =
          displayOrigAirport(ts, orig, firstCity, prevFareBreak, cnxWithCityGroup, &fcGroup);
      processSideTrip(fu, ts, sideTripProcessed, cnxWithCityGroup, fcGroup);
    }

    bool lastFuTvlSeg = (ts == lastTvlSeg);
    if (lastFuTvlSeg)
    {
      pos = _os.ufstr().length();
    }

    cnxWithCityGroup = displayCarrier(fu, ts, fareBasisSpace, cnxWithCityGroup, &fcGroup);

    // netRemitTs == 0 check for non TFDPSC. Why?
    bool isNetRemitTsNull =
        netRemitTs == nullptr && netOriginalFu && !netOriginalFu->netRemitPscResults().size();

    if (UNLIKELY(_originalCalcTotals != nullptr && netOriginalFu != nullptr &&
        netOriginalFu->travelSeg().size() > fu->travelSeg().size() &&
        isNetRemitTsNull)) // Original travel seg not in the alternate fare market
    {
      cnxWithCityGroup = displayDestAirport(
          fu, ts, dest, fareBreak, lastFuTvlSeg, _originalCalcTotals, cnxWithCityGroup, &fcGroup);
    }
    else
    {
      cnxWithCityGroup = displayDestAirport(fu,
                                            (netRemitTs != nullptr ? netRemitTs : ts),
                                            dest,
                                            fareBreak,
                                            lastFuTvlSeg,
                                            _calcTotals,
                                            cnxWithCityGroup,
                                            &fcGroup);
    }

    // NET::S66
    if (UNLIKELY(_useNetPubFbc && (i + 1) != iend))
    {
      FareUsage::TktNetRemitPscResultVec::const_iterator nrResult =
          FareCalculation::findNetRemitPscResults(*fu, ts);
      if (nrResult != fu->netRemitPscResults().end() && nrResult->_resultFare)
        setFbcFromTFDPSC(nrResult->_resultFare->createFareBasis(_trx));
    }

    cnxWithCityGroup =
        displayCat12Surcharge(netRemitTs != nullptr ? netRemitTs : ts, cnxWithCityGroup, &fcGroup);

    if (lastFuTvlSeg)
    {
      cnxWithCityGroup = displayCat12Surcharge(tvlSeg, cnxWithCityGroup, &fcGroup);
      cnxWithCityGroup = displayBreakOffMileageSurcharge(
          fu, (netRemitTs != nullptr ? netRemitTs : ts), cnxWithCityGroup, &fcGroup);
      cnxWithCityGroup = displayHipPlusUp(fu, cnxWithCityGroup, &fcGroup);
    }

    cnxWithCityGroup = displaySingleStopOverSurcharge(ts, cnxWithCityGroup, &fcGroup);

    cnxWithCityGroup = displaySingleTransferSurcharge(ts, cnxWithCityGroup, &fcGroup);

    calcTotals.ticketOrigin.push_back(orig);
    calcTotals.ticketDestination.push_back(dest);

    fareBasisSpace = false;
  }
  cnxWithCityGroup = displayFareAmount(fu, cnxWithCityGroup, &fcGroup, lastTvlSeg);

  if (UNLIKELY(cnxWithCityGroup))
    fcGroup.endGroup();

  if (LIKELY(pos > 0))
  {
    calcTotals.ticketFareInfo.push_back(_os.ufstr().substr(pos));
  }
}

bool
FcDispFareUsage::isNotArunkSegBeforeSideTrip(const FareUsage* fu, const TravelSeg* tvlSeg)
{
  if (tvlSeg->segmentType() == Arunk &&
      _fp.itin()->segmentOrder(tvlSeg) <
          _fp.itin()->segmentOrder(fu->sideTripPUs().front()->travelSeg().front()))
    return false;

  return true;
}

bool
FcDispFareUsage::displayOrigAirport(const TravelSeg* ts,
                                    const LocCode& orig,
                                    bool firstCity,
                                    bool fareBreak,
                                    bool cnxWithCityGroup,
                                    FareCalc::Group* fcGroup)
{

  if (firstCity)
  {
    _os << ' ' << orig << ' ';
  }
  else
  {
    const ArunkSeg* arunkSeg = dynamic_cast<const ArunkSeg*>(ts);

    if (UNLIKELY(_trx.getRequest()->ticketingAgent()->axessUser()))
    {
      if (arunkSeg)
      {
        if (cnxWithCityGroup)
        {
          restartFcGroup(fcGroup);
        }
        _os << "//";
      }
      else
      {
        if (_prevDest != orig)
        {
          displaySurfaceSegment(orig, fareBreak, cnxWithCityGroup, fcGroup);
        }
      }
    }
    else
    {
      if (arunkSeg)
      {
        _os << "//";
      }
      else
      {
        if (_prevDest != orig)
          _os << "/-" << orig;
      }
    }
  }
  return (cnxWithCityGroup);
}

bool
FcDispFareUsage::displayDestAirport(const FareUsage* fu,
                                    const TravelSeg* ts,
                                    const LocCode& dest,
                                    bool fareBreak,
                                    bool lastFuTvlSeg,
                                    const CalcTotals* calcTotals,
                                    bool cnxWithCityGroup,
                                    FareCalc::Group* fcGroup)
{
  if (UNLIKELY(!_os.lastCharSpace() && isalnum(_os.lastChar())))
  {
    if (cnxWithCityGroup)
    {
      cnxWithCityGroup = false;
      fcGroup->endGroup();
    }
    _os << ' ';
  }

  if (UNLIKELY(_trx.getRequest()->ticketingAgent()->axessUser() && !cnxWithCityGroup))
  {
    cnxWithCityGroup = true;
    fcGroup->startGroup();
  }

  if (calcTotals->getDispConnectionInd(_trx, ts, _fcConfig.fcConnectionInd()))
  {
    _prevDestConnection = true;
    _os << "X/";
  }
  else
  {
    _prevDestConnection = false;
  }

  if (!fareBreak && !lastFuTvlSeg)
    cnxWithCityGroup = displayNonBreakOffMileageSurcharge(fu, ts, cnxWithCityGroup, fcGroup, dest);

  _os << dest;

  return (cnxWithCityGroup);
}

bool
FcDispFareUsage::displayCarrier(const FareUsage* fu,
                                const TravelSeg* ts,
                                bool fareBasisSpace,
                                bool cnxWithCityGroup,
                                FareCalc::Group* fcGroup)
{
  const AirSeg* as = dynamic_cast<const AirSeg*>(ts);
  if (as == nullptr)
    return (cnxWithCityGroup);

  if (UNLIKELY(cnxWithCityGroup))
  {
    cnxWithCityGroup = false;
    fcGroup->endGroup();
  }

  CarrierCode carrier = MCPCarrierUtil::swapToPseudo(&_trx, as->carrier());
  if (UNLIKELY(carrier.empty() || carrier.equalToConst("  ")))
  {
    carrier = "YY";
  }

  if (!_os.lastCharSpace() && !_os.lastCharSpecial())
  {
    if (fareBasisSpace && ((_fcConfig.fareBasisDisplayOption() == FareCalcConsts::FC_YES) ||
                           (TrxUtil::getValidatingCxrFbcDisplayPref(_trx, _fp))))
    {
      _os << ' ';
    }
    else if ((isalpha(carrier[0]) && _os.lastCharAlpha()) ||
             (isdigit(carrier[0]) && _os.lastCharDigit()) ||
             (isdigit(carrier[0]) && _os.lastCharAlpha()))
    {
      _os << ' ';
    }
  }

  _os << carrier;

  displayGlobalDirection(fu, ts);
  return (cnxWithCityGroup);
}

bool
FcDispFareUsage::displaySideTrip(const FareUsage* fu, const TravelSeg* ts)
{
  if (!fu->hasSideTrip())
    return false;

  if (_fp.itin()->segmentOrder(ts) <
      _fp.itin()->segmentOrder(fu->sideTripPUs().front()->travelSeg().front()))
    return false;

  bool isWqTrx = (dynamic_cast<NoPNRPricingTrx*>(&_trx) != nullptr);

  // Side Trip:
  {
    StartSideTrip startSideTrip(this);

    FareUsageIter fuIter(_fp, fu->sideTripPUs());
    const std::vector<FareUsage*>& fus = fuIter.fareUsages();
    std::for_each(fus.begin(), fus.end(), *this);

    if (!fu->paxTypeFare()->fareMarket()->sideTripTravelSeg().empty())
    {
      TravelSeg* lastTvlSegPu = nullptr;
      if (!fu->paxTypeFare()->fareMarket()->sideTripTravelSeg()[0].empty())
        lastTvlSegPu = fu->paxTypeFare()->fareMarket()->sideTripTravelSeg()[0].back();
      if (lastTvlSegPu != nullptr)
      {
        AirSeg* lastAirSegPu = dynamic_cast<AirSeg*>(lastTvlSegPu);
        if (lastAirSegPu == nullptr) // only if the last segment of PU is ARUNK
        {
          for (FareUsage* fu : fuIter.fareUsages())
          {
            for (TravelSeg* ts : fu->travelSeg())
            {
              if (ts == lastTvlSegPu)
                return true;
            }
          }

          LocCode destLoc = getDisplayLoc(*_fcConfig.fcc(),
                                          _fp.itin()->geoTravelType(),
                                          lastTvlSegPu->offMultiCity(),
                                          lastTvlSegPu->destAirport(),
                                          fu->paxTypeFare()->carrier(),
                                          lastTvlSegPu->departureDT(),
                                          isWqTrx);
          if (!destLoc.empty())
            _os << " "
                << "/-" << destLoc;
        }
      }
    }
  }

  return true;
}

void
FcDispFareUsage::displaySideTripMarker()
{
  if (_os.lastCharSpecial())
    return;

  if (_trx.getOptions()->isRecordQuote() || (_trx.getRequest()->isLowFareRequested() &&
                                             !(_trx.getRequest()->ticketingAgent()->axessUser())) ||
      _trx.getOptions()->returnAllData() == GDS || _trx.getOptions()->returnAllData() == WD ||
      _trx.getOptions()->returnAllData() == FP)
  {
    _os << '*';
    return;
  }

  if (_fcConfig.globalSidetripInd() == FareCalcConsts::FC_ONE)
    _os << '(';
  else
    // default with FareCalcConsts::FC_TWO
    _os << '*';
}

void
FcDispFareUsage::displayEndSideTripMarker()
{
  if (_os.lastCharSpecial())
    return;

  if (_trx.getOptions()->isRecordQuote() || (_trx.getRequest()->isLowFareRequested() &&
                                             !(_trx.getRequest()->ticketingAgent()->axessUser())) ||
      _trx.getOptions()->returnAllData() == GDS || _trx.getOptions()->returnAllData() == WD ||
      _trx.getOptions()->returnAllData() == FP)
  {
    _os << '*';
    return;
  }

  if (_fcConfig.globalSidetripInd() == FareCalcConsts::FC_ONE)
  {
    if (_trx.getRequest()->ticketingAgent()->axessUser())
      _os << '*';
    else
      _os << ')';
  }
  else
    // default with FareCalcConsts::FC_TWO
    _os << '*';
}

void
FcDispFareUsage::displayGlobalDirection(const FareUsage* fu, const TravelSeg* ts)
{
  std::string tempGlobalDir;
  tempGlobalDir.clear();
  GlobalDirection gd = GlobalDirection::XX;

  const PaxTypeFare* paxTypeFare = fu->paxTypeFare(); // lint !e530
  NoPNRPricingTrx* noPNRTrx = dynamic_cast<NoPNRPricingTrx*>(&_trx);

  if (UNLIKELY(noPNRTrx != nullptr))
  {
    gd = (*paxTypeFare->fareMarket()).getGlobalDirection();
    globalDirectionToStr(tempGlobalDir, gd);
  }
  else
  {
    if (UNLIKELY(paxTypeFare->isDummyFare()))
    {
      if (!ts->globalDirectionOverride().empty())
        tempGlobalDir = ts->globalDirectionOverride();
    }
    else
    {
      std::vector<TravelSeg*> partialTravelSegs;
      partialTravelSegs.push_back(const_cast<TravelSeg*>(ts));
      GlobalDirectionFinderV2Adapter::getGlobalDirection(
          &_trx, _fp.itin()->travelDate(), partialTravelSegs, gd);

      if (LIKELY(gd != GlobalDirection::XX))
        globalDirectionToStr(tempGlobalDir, gd);
    }
  }

  if (LIKELY(noDisplayGlobalDirection(fu, ts, tempGlobalDir)))
  {
    _os << ' ';
  }
  else if (_trx.getOptions()->isRecordQuote() || _trx.getOptions()->returnAllData() == WD ||
           _trx.getOptions()->returnAllData() == FP)
  {
    _os << '*';
  }
  else
  {
    _fcConfig.globalSidetripInd() == FareCalcConsts::FC_ONE
        ? _os << '('
        : _os << '*'; // default with FareCalcConsts::FC_TWO

    _os << tempGlobalDir;

    if (_fcConfig.globalSidetripInd() == FareCalcConsts::FC_ONE)
    {
      if (_trx.getRequest()->ticketingAgent()->axessUser())
        _os << '*';
      else
        _os << ')';
    }
    else
      // default with FareCalcConsts::FC_TWO
      _os << '*';
  }
}

bool
FcDispFareUsage::noDisplayGlobalDirection(const FareUsage* fu,
                                          const TravelSeg* ts,
                                          const std::string& tempGlobalDir)
{
  if (UNLIKELY(tempGlobalDir.empty()))
    return true;

  const PaxTypeFare* paxTypeFare = fu->paxTypeFare();
  if (UNLIKELY(paxTypeFare->isDummyFare()))
  {
    if (!ts->globalDirectionOverride().empty())
      return false;
    else
      return true;
  }
  const LocCode& boardMultiCity =
      FareMarketUtil::getBoardMultiCity(*paxTypeFare->fareMarket(), *ts);
  const LocCode& offMultiCity = FareMarketUtil::getOffMultiCity(*paxTypeFare->fareMarket(), *ts);
  if (LIKELY(!(isMultiGlobalDir(boardMultiCity, offMultiCity, _trx.ticketingDate()) ||
        isMultiGlobalDir(offMultiCity, boardMultiCity, _trx.ticketingDate()))))
    return true;

  return false;
}

bool
FcDispFareUsage::displayNonBreakOffMileageSurcharge(const FareUsage* fu,
                                                    const TravelSeg* ts,
                                                    bool cnxWithCityGroup,
                                                    FareCalc::Group* fcGroup,
                                                    const LocCode& loc)
{
  if (fu == nullptr || ts == nullptr)
    return (cnxWithCityGroup);

  const PaxTypeFare* ptf = fu->paxTypeFare();
  if (!ptf)
    return (cnxWithCityGroup);

  const FareMarket* fm = ptf->fareMarket();
  if (!fm)
    return (cnxWithCityGroup);

  const MileageInfo* mi = ptf->mileageInfo();
  if (!mi)
    return (cnxWithCityGroup);

  const LocCode& boardMultiCity = FareMarketUtil::getBoardMultiCity(*fm, *ts);
  const LocCode& offMultiCity = FareMarketUtil::getOffMultiCity(*fm, *ts);

  // Section 7.2
  if (_calcTotals->extraMileageTravelSegs.find(ts) != _calcTotals->extraMileageTravelSegs.end())
  {
    if (_os.lastCharAlpha() || _os.lastCharSpecial())
    {
      if (cnxWithCityGroup)
      {
        cnxWithCityGroup = false;
        fcGroup->endGroup();
      }
      _os << ' ';
    }
    if (_trx.getRequest()->ticketingAgent()->axessUser() && !cnxWithCityGroup)
    {
      cnxWithCityGroup = true;
      fcGroup->startGroup();
    }
    _os << "E/";
  }

  // Section 7.3
  if (mi->southAtlanticTPMExclusion() && mi->southAtlanticTPDCities().size() > 1)
  {
    for (std::vector<Market>::const_iterator i = mi->southAtlanticTPDCities().begin(),
                                             iend = mi->southAtlanticTPDCities().end();
         std::distance(i, iend) > 1;
         ++i)
    {
      if (boardMultiCity == i->first && offMultiCity == i->second)
      {
        if (_os.lastCharAlpha() || _os.lastCharSpecial())
        {
          if (cnxWithCityGroup)
          {
            cnxWithCityGroup = false;
            fcGroup->endGroup();
          }
          _os << ' ';
        }
        if (_trx.getRequest()->ticketingAgent()->axessUser() && !cnxWithCityGroup)
        {
          cnxWithCityGroup = true;
          fcGroup->startGroup();
        }
        _os << "T/";
        saveMileageTypeData(ts, MileageTypeData::TICKETEDPOINT, loc);
        break;
      }
    }
  }

  return (cnxWithCityGroup);
}

bool
FcDispFareUsage::displayBreakOffMileageSurcharge(const FareUsage* fu,
                                                 const TravelSeg* ts,
                                                 bool cnxWithCityGroup,
                                                 FareCalc::Group* fcGroup)
{
  if (UNLIKELY(fu == nullptr || ts == nullptr))
    return (cnxWithCityGroup);

  // Section 13 ... 4
  const PaxTypeFare* ptf = fu->paxTypeFare();
  if (UNLIKELY(!ptf))
  {
    return (cnxWithCityGroup);
  }
  const FareMarket* fm = ptf->fareMarket();
  if (UNLIKELY(!fm))
  {
    return (cnxWithCityGroup);
  }
  const MileageInfo* mi = ptf->mileageInfo();
  if (!mi)
  {
    return (cnxWithCityGroup);
  }

  // Display Mileage Equalization Indicator
  if (mi->mileageEqualizationApplies() && !ptf->isRouting())
  {
    if (_os.lastCharAlpha() || _os.lastCharSpecial())
    {
      if (cnxWithCityGroup)
      {
        cnxWithCityGroup = false;
        fcGroup->endGroup();
      }
      _os << ' ';
    }

    const LocCode& boardCity = fu->travelSeg().front()->boardMultiCity();
    const LocCode& offCity = fu->travelSeg().back()->offMultiCity();

    if (_trx.excTrxType() == PricingTrx::PORT_EXC_TRX && (ptf->isDummyFare()))
    {
      LocCode destination;
      excMileageEqulalizationDisplay(ts, destination);
      _os << "B/" << destination;
    }
    else if ((boardCity == RIO_DE_JANEIRO) || (offCity == RIO_DE_JANEIRO))
    {
      if (_trx.getRequest()->ticketingAgent()->axessUser() && !cnxWithCityGroup)
      {
        cnxWithCityGroup = true;
        fcGroup->startGroup();
      }
      _os << "B/SAO";
      LocCode destination = "SAO";
      saveMileageTypeData(ts, MileageTypeData::EQUALIZATION, destination);
    }
    else
    {
      if (_trx.getRequest()->ticketingAgent()->axessUser() && !cnxWithCityGroup)
      {
        cnxWithCityGroup = true;
        fcGroup->startGroup();
      }
      _os << "B/RIO";
      LocCode destination = "RIO";
      saveMileageTypeData(ts, MileageTypeData::EQUALIZATION, destination);
    }
  }

  const LocCode& offMultiCity = FareMarketUtil::getOffMultiCity(*fm, *ts);
  std::map<LocCode, TpdViaGeoLocMatching>::const_iterator iter;
  iter = mi->tpdViaGeoLocs().find(offMultiCity);

  // Section 13 ... 5
  if (!ptf->isRouting() && // to mileage fare
      _calcTotals->extraMileageFareUsages.find(fu) != _calcTotals->extraMileageFareUsages.end())
  {
    if (_os.lastCharAlpha() || _os.lastCharSpecial())
    {
      if (cnxWithCityGroup)
      {
        cnxWithCityGroup = false;
        fcGroup->endGroup();
      }
      _os << ' ';
    }
    if (_trx.getRequest()->ticketingAgent()->axessUser() && !cnxWithCityGroup)
    {
      cnxWithCityGroup = true;
      fcGroup->startGroup();
    }
    _os << "E/XXX";
  }

  // Section 13 ... 6:  Process mileage fare
  if (ptf->isRouting() || ptf->isPSRApplied())
  {
    return (cnxWithCityGroup);
  }
  if (ptf->mileageSurchargePctg())
  {
    if (_os.lastCharDigit())
    {
      if (cnxWithCityGroup)
      {
        cnxWithCityGroup = false;
        fcGroup->endGroup();
      }
      _os << ' ';
    }
    // ensure there is no break between mileage% and M
    std::ostringstream str;
    str << ptf->mileageSurchargePctg() << 'M';
    _os << str.str();
  }
  else if (!fu->travelSeg().empty())
  {
    std::vector<TravelSeg*>::const_iterator i = fu->travelSeg().begin();
    for (uint16_t tsn = 0; i != fu->travelSeg().end(); i++)
    {
      if ((*i)->segmentType() != Surface)
        tsn++;
      if (tsn > 1)
      {
        if (_os.lastCharAlpha())
        {
          if (cnxWithCityGroup)
          {
            cnxWithCityGroup = false;
            fcGroup->endGroup();
          }
          _os << ' ';
        }
        if (_trx.getRequest()->ticketingAgent()->axessUser() && !cnxWithCityGroup)
        {
          cnxWithCityGroup = true;
          fcGroup->startGroup();
        }
        _os << 'M';
        break;
      }
    }
  }
  return (cnxWithCityGroup);
}

bool
FcDispFareUsage::displayHipPlusUp(const FareUsage* fu,
                                  bool cnxWithCityGroup,
                                  FareCalc::Group* fcGroup)
{
  const MinFarePlusUp& mfpuV = fu->minFarePlusUp();
  MinFarePlusUp::const_iterator mfpuI = mfpuV.find(HIP);

  if (mfpuI != mfpuV.end())
  {
    MinFarePlusUpItem& mfpuR = *mfpuI->second;
    if (cnxWithCityGroup)
    {
      cnxWithCityGroup = false;
      fcGroup->endGroup();
    }

    _os << ' ';

    if (_trx.getRequest()->ticketingAgent()->axessUser())
    {
      // ensure there is no break between C/ and constructed point
      std::ostringstream str;
      if (!mfpuR.constructPoint.empty())
      {
        str << "C/" << mfpuR.constructPoint << ' ';
        _os << str.str();
      }
      cnxWithCityGroup = true;
      fcGroup->startGroup();
      _os << (mfpuR.boardPoint + mfpuR.offPoint);
    }
    else
    {
      if (!mfpuR.constructPoint.empty())
      {
        _os << ("C/" + mfpuR.constructPoint) << ' ';
      }
      _os << (mfpuR.boardPoint + mfpuR.offPoint);
    }
  }
  return (cnxWithCityGroup);
}

bool
FcDispFareUsage::displayCat12Surcharge(const TravelSeg* ts,
                                       bool cnxWithCityGroup,
                                       FareCalc::Group* fcGroup)
{
  if (UNLIKELY(_calcTotals == nullptr))
    return (cnxWithCityGroup);

  std::map<const TravelSeg*, std::vector<SurchargeData*> >::const_iterator iter;
  iter = _calcTotals->surcharges.find(ts);

  if (iter == _calcTotals->surcharges.end())
    return (cnxWithCityGroup);

  for (const auto elem : iter->second)
  {
    if (elem->amountNuc() != 0 && elem->selectedTkt() && elem->singleSector() && !elem->fcFpLevel())
    {
      if (cnxWithCityGroup)
      {
        cnxWithCityGroup = false;
        fcGroup->endGroup();
      }

      MoneyAmount surcharge = elem->amountNuc() * elem->itinItemCount();
      if (_os.lastCharAlpha() || (_surchargeCount == 0 && _os.lastCharDigit()) ||
          (_surchargeCount > 0 && _fcConfig.multiSurchargeSpacing() == FareCalcConsts::FC_YES))
        _os << ' ';

      FareCalc::Group fcGroup(_os, true);
      _os << 'Q' << std::fixed << std::setprecision(_noDec) << surcharge;
      _surchargeCount++;
    }
  }
  return (cnxWithCityGroup);
}

bool
FcDispFareUsage::displayCat12Surcharge(const std::vector<TravelSeg*>& tvlSeg,
                                       bool cnxWithCityGroup,
                                       FareCalc::Group* fcGroup)
{
  if (UNLIKELY(_calcTotals == nullptr))
    return (cnxWithCityGroup);

  std::vector<TravelSeg*>::const_iterator it = tvlSeg.begin();
  std::vector<TravelSeg*>::const_iterator ie = tvlSeg.end();
  for (; it != ie; ++it)
  {
    std::map<const TravelSeg*, std::vector<SurchargeData*> >::const_iterator iter;
    iter = _calcTotals->surcharges.find(*it);

    if (iter == _calcTotals->surcharges.end())
      continue;

    for (const auto elem : iter->second)
    {
      if (elem->amountNuc() != 0 && elem->selectedTkt() && !elem->singleSector() &&
          !elem->fcFpLevel())
      {
        if (cnxWithCityGroup)
        {
          cnxWithCityGroup = false;
          fcGroup->endGroup();
        }

        MoneyAmount surcharge = elem->amountNuc() * elem->itinItemCount();
        if (_os.lastCharAlpha() || (_surchargeCount == 0 && _os.lastCharDigit()) ||
            (_surchargeCount > 0 && _fcConfig.multiSurchargeSpacing() == FareCalcConsts::FC_YES))
          _os << ' ';

        FareCalc::Group fcGroup(_os, true);
        _os << "Q " << elem->fcBrdCity() << elem->fcOffCity() << std::fixed
            << std::setprecision(_noDec) << surcharge;
        _surchargeCount++;
      }
    }
  }
  return (cnxWithCityGroup);
}

bool
FcDispFareUsage::displaySingleStopOverSurcharge(const TravelSeg* ts,
                                                bool cnxWithCityGroup,
                                                FareCalc::Group* fcGroup)
{
  if (UNLIKELY(_calcTotals == nullptr))
    return (cnxWithCityGroup);

  std::map<const TravelSeg*, std::vector<const FareUsage::StopoverSurcharge*> >::const_iterator
  stopoverSurcharges = _calcTotals->stopoverSurcharges.find(ts);

  if (stopoverSurcharges == _calcTotals->stopoverSurcharges.end())
    return (cnxWithCityGroup);

  std::vector<const FareUsage::StopoverSurcharge*>::const_iterator stopoverSurchargeIter =
      stopoverSurcharges->second.begin();

  std::vector<const FareUsage::StopoverSurcharge*>::const_iterator stopoverSurchargeIterEnd =
      stopoverSurcharges->second.end();

  for (; stopoverSurchargeIter != stopoverSurchargeIterEnd; stopoverSurchargeIter++)
  {
    if ((*stopoverSurchargeIter)->amount() > EPSILON)
    {
      if ((*stopoverSurchargeIter)->isSegmentSpecific())
      {
        if (cnxWithCityGroup)
        {
          fcGroup->endGroup();
          cnxWithCityGroup = false;
        }
        if (_os.lastCharAlpha() ||
            (_surchargeCount > 0 && _fcConfig.multiSurchargeSpacing() == FareCalcConsts::FC_YES))
          _os << ' ';

        if (_fcConfig.wrapAround() == FareCalcConsts::FC_TWO)
        {
          FareCalc::Group fcGroup(_os, true);
          _os << 'S' << std::fixed << std::setprecision(_noDec)
              << (*stopoverSurchargeIter)->amount();
        }
        else
        {
          _os << 'S' << std::fixed << std::setprecision(_noDec)
              << (*stopoverSurchargeIter)->amount();
        }
        _surchargeCount++;
      }
    }
  }
  return (cnxWithCityGroup);
}

bool
FcDispFareUsage::displaySingleTransferSurcharge(const TravelSeg* ts,
                                                bool cnxWithCityGroup,
                                                FareCalc::Group* fcGroup)
{
  if (UNLIKELY(_calcTotals == nullptr))
    return (cnxWithCityGroup);

  std::map<const TravelSeg*, std::vector<const FareUsage::TransferSurcharge*> >::const_iterator
  transferSurcharges = _calcTotals->transferSurcharges.find(ts);

  if (transferSurcharges == _calcTotals->transferSurcharges.end())
    return (cnxWithCityGroup);

  std::vector<const FareUsage::TransferSurcharge*>::const_iterator transferSurchargeIter =
      transferSurcharges->second.begin();

  std::vector<const FareUsage::TransferSurcharge*>::const_iterator transferSurchargeIterEnd =
      transferSurcharges->second.end();

  for (; transferSurchargeIter != transferSurchargeIterEnd; transferSurchargeIter++)
  {
    if ((*transferSurchargeIter)->amount() > EPSILON)
    {
      if ((*transferSurchargeIter)->isSegmentSpecific())
      {
        if (cnxWithCityGroup)
        {
          fcGroup->endGroup();
          cnxWithCityGroup = false;
        }

        if (_os.lastCharAlpha() ||
            (_surchargeCount > 0 && _fcConfig.multiSurchargeSpacing() == FareCalcConsts::FC_YES))
          _os << ' ';

        if (_fcConfig.wrapAround() == FareCalcConsts::FC_TWO)
        {
          FareCalc::Group fcGroup(_os, true);
          _os << 'S' << std::fixed << std::setprecision(_noDec)
              << (*transferSurchargeIter)->amount();
        }
        else
        {
          _os << 'S' << std::fixed << std::setprecision(_noDec)
              << (*transferSurchargeIter)->amount();
        }
        _surchargeCount++;
      }
    }
  }
  return (cnxWithCityGroup);
}

bool
FcDispFareUsage::displayFareAmount(const FareUsage* fu,
                                   bool cnxWithCityGroup,
                                   FareCalc::Group* fcGroup,
                                   const TravelSeg* ts)
{
  const FareBreakPointInfo* fbpInfo = _calcTotals->getFareBreakPointInfo(fu);
  if (UNLIKELY(fbpInfo == nullptr))
    return (cnxWithCityGroup);

  if (_os.lastCharDigit())
    _os << ' ';

  if (UNLIKELY(_trx.getRequest()->ticketingAgent()->axessUser() && !cnxWithCityGroup))
  {
    cnxWithCityGroup = true;
    fcGroup->startGroup();
  }

  _os << std::fixed << std::setprecision(_noDec) << fbpInfo->fareAmount;

  std::string fareBasis = fbpInfo->fareBasisCode;

  if ((_fcConfig.fareBasisDisplayOption() == FareCalcConsts::FC_YES) ||
      (TrxUtil::getValidatingCxrFbcDisplayPref(_trx, _fp)))
  {
    const NetRemitFarePath* netRemitFp = dynamic_cast<const NetRemitFarePath*>(&_fp);
    setFareBasis((netRemitFp ? netRemitFp->getOriginalFareUsage(fu) : fu),
                 *fbpInfo,
                 ts,
                 fareBasis,
                 netRemitFp);
    _os << fareBasis;
  }

  if (LIKELY(!_useNetPubFbc))
  {
    CalcTotals& calcTotals = const_cast<CalcTotals&>(*_calcTotals);
    calcTotals.ticketFareBasisCode.push_back(fareBasis);
  }

  return (cnxWithCityGroup);
}

LocCode
FcDispFareUsage::getDisplayLoc(const FareCalcConfig& fcConfig,
                               const GeoTravelType& geoTravelType,
                               const LocCode& multiCity,
                               const LocCode& airport,
                               const CarrierCode& carrier,
                               const DateTime& date,
                               bool alwaysCheckMultiTable)
{
  LocCode displayLoc;
  if (LIKELY(!getFccDisplayLoc(fcConfig, airport, displayLoc)))
  {
    if (geoTravelType == GeoTravelType::Domestic && !alwaysCheckMultiTable)
    {
      displayLoc = multiCity;
    }
    else
    {
      displayLoc = LocUtil::getMultiTransportCity(airport, carrier, geoTravelType, date);
      if (UNLIKELY(displayLoc.empty()))
        displayLoc = airport;
    }
  }
  else if (displayLoc.empty())
  {
    displayLoc = multiCity;
  }
  return displayLoc;
}

bool
FcDispFareUsage::getFccDisplayLoc(const FareCalcConfig& fcConfig,
                                  const LocCode& loc,
                                  LocCode& displayLoc)
{
  const std::vector<FareCalcConfigSeg*>& fcSeg = fcConfig.segs();

  for (const auto elem : fcSeg)
  {
    if (elem->marketLoc() == loc)
    {
      displayLoc = elem->displayLoc();
      return true;
    }
  }
  return false;
}

bool
FcDispFareUsage::isMultiGlobalDir(const LocCode& origin, const LocCode& dest, const DateTime& date)
{
  DataHandle dataHandle(_trx.ticketingDate());
  const std::vector<Mileage*>& mileVector = dataHandle.getMileage(origin, dest, date, TPM);
  if (mileVector.size() > 1)
    return true;

  return false;
}

void
FcDispFareUsage::processSideTrip(const FareUsage* fu,
                                 const TravelSeg* ts,
                                 bool& sideTripProcessed,
                                 bool& cnxWithCityGroup,
                                 FareCalc::Group& fcGroup)
{
  if (fu->hasSideTrip() && !sideTripProcessed)
  {
    if (cnxWithCityGroup)
    {
      cnxWithCityGroup = false;
      fcGroup.endGroup();
    }
    sideTripProcessed = displaySideTrip(fu, ts);
  }
}

void
FcDispFareUsage::saveMileageTypeData(const TravelSeg* ts,
                                     MileageTypeData::MileageType mtype,
                                     const LocCode& loc)
{

  MileageTypeData* mileageTypeData = nullptr;
  _trx.dataHandle().get(mileageTypeData);
  if (mileageTypeData == nullptr)
    return;

  mileageTypeData->type() = mtype;
  mileageTypeData->city() = loc;

  TravelSeg* tSeg = const_cast<TravelSeg*>(ts);
  mileageTypeData->travelSeg() = tSeg;

  (const_cast<CalcTotals&>(*_calcTotals)).mileageTypeData().push_back(mileageTypeData);
}

void
FcDispFareUsage::excMileageEqulalizationDisplay(const TravelSeg* ts, LocCode& destination)
{
  const ExchangePricingTrx& excTrx = static_cast<const ExchangePricingTrx&>(_trx);
  std::vector<MileageTypeData*>::const_iterator mileTypeDataI =
      excTrx.exchangeOverrides().mileageTypeData().begin();
  std::vector<MileageTypeData*>::const_iterator mileTypeDataIE =
      excTrx.exchangeOverrides().mileageTypeData().end();
  for (; mileTypeDataI != mileTypeDataIE; ++mileTypeDataI)
  {
    if ((*mileTypeDataI)->travelSeg() == ts &&
        (*mileTypeDataI)->type() == MileageTypeData::EQUALIZATION)
    {
      destination = (*mileTypeDataI)->city();
      break;
    }
  }
}

void
FcDispFareUsage::displaySurfaceSegment(const LocCode& orig,
                                       bool fareBreak,
                                       bool cnxWithCityGroup,
                                       FareCalc::Group* fcGroup)
{
  if (!fareBreak)
  {
    if (cnxWithCityGroup)
    {
      restartFcGroup(fcGroup);
    }
    if (_prevDestConnection)
      _os << "//X/" << orig; // previous destination was a connection
    else
      _os << "//" << orig; // previous destination was a stopover
  }
  else
  {
    FareCalc::Group fcGroup(_os, true); // execute for Axess only

    _os << "/-" << orig;
  }
}

void
FcDispFareUsage::restartFcGroup(FareCalc::Group* fcGroup)
{
  fcGroup->endGroup();
  fcGroup->startGroup();
}

void
FcDispFareUsage::setFbcFromTFDPSC(const std::string& fbc)
{
  if (!_os.lastCharSpace() && !isdigit(_os.lastChar()))
    _os << ' ';
  _os << fbc;
  _os << ' ';
}

void
FcDispFareUsage::setFareBasis(const FareUsage* fu,
                              const FareBreakPointInfo& fbpInfo,
                              const TravelSeg* ts,
                              std::string& fareBasis,
                              const NetRemitFarePath* netRemitFp) const
{
  if (!fu || !ts)
    return;

  FareUsage::TktNetRemitPscResultVec::const_iterator nrResults =
      FareCalculation::findNetRemitPscResults(*fu, ts);

  if (_useNetPubFbc) // Ind F, NET::S66
  {
    if (fbpInfo.netPubFbc.size()) // TFD
      fareBasis = fbpInfo.netPubFbc;
    else if (nrResults != fu->netRemitPscResults().end() && nrResults->_resultFare) // TFDPSC
      fareBasis = nrResults->_resultFare->createFareBasis(_trx);
  }
  else if (netRemitFp) // NET::S66
  {
    Indicator fareBasisAmtInd = NegotiatedFareRuleUtil::getFareAmountIndicator(_trx, fu);
    if ((fareBasisAmtInd == RuleConst::NR_VALUE_B || fareBasisAmtInd == RuleConst::NR_VALUE_N) &&
        fu->paxTypeFare())
    {
      fareBasis = fu->paxTypeFare()->createFareBasis(_trx);
    }
  }
}
} // tse
