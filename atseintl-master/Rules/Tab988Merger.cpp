//-------------------------------------------------------------------
//
//  File:        Tab988Merger.h
//  Created:     December 10, 2008
//  Authors:     Miroslaw Bartyna
//
//  Updates:
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
#include "Rules/Tab988Merger.h"

#include "Common/Logger.h"
#include "Common/ShoppingRexUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FarePath.h"
#include "DataModel/ProcessTagInfo.h"
#include "DataModel/RexShoppingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/GeoRuleItem.h"
#include "DBAccess/VoluntaryChangesInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Util/BranchPrediction.h"

#include <boost/bind.hpp>

namespace tse
{
FALLBACK_DECL(exsCalendar);
FALLBACK_DECL(changeIndOpt);
FALLBACK_DECL(exscDefaultWholePeriodRange)

namespace
{
class InsertMulticity : public std::insert_iterator<std::set<LocCode> >
{
private:
  struct AssignmentProxy
  {
    void operator=(const TravelSeg* seg) { s->insert(seg->offMultiCity()); }
    std::set<LocCode>* s;
  } proxy;

public:
  InsertMulticity(std::set<LocCode>& s) : std::insert_iterator<std::set<LocCode> >(s, s.begin())
  {
    proxy.s = &s;
  };
  AssignmentProxy& operator*() { return proxy; }
};

class CollectResFareByteCxrAppl
{
public:
  using FareByteCxrAppl = RexShoppingTrx::FareByteCxrAppl;

  CollectResFareByteCxrAppl(FareByteCxrAppl& fareByteCxrAppl) : _fareByteCxrAppl(fareByteCxrAppl)
  {
  }

  void operator()(CarrierApplicationInfo* in)
  {
    constexpr const Indicator CARRIER_ALLOWED = ' ';
    if (in->applInd() == CARRIER_ALLOWED)
      _fareByteCxrAppl.applCxr.insert(in->carrier());
    else
      _fareByteCxrAppl.restCxr.insert(in->carrier());
  }

private:
  FareByteCxrAppl& _fareByteCxrAppl;
};

Logger
logger("atseintl.Rules.Tab988Merger");
}

Tab988Merger::Tab988Merger(RexPricingTrx& trx, const Itin* itin, DiagCollector* out)
  : ReissueTable(trx, itin, out)
{
}

Tab988Merger::Tab988Merger(RexPricingTrx& trx, const Itin* itin, const PricingUnit* pu)
  : ReissueTable(trx, itin, pu)
{
  if (UNLIKELY(_trx.diagnostic().diagnosticType() == Diagnostic331))
  {
    DCFactory* factory = DCFactory::instance();
    _dc = factory->create(_trx);

    if (_dc != nullptr)
    {
      _dc->enable(Diagnostic331);

      if (!_dc->isActive())
      {
        _dc = nullptr;
      }
    }
  }
}

Tab988Merger::~Tab988Merger()
{
  if (UNLIKELY(_dc != nullptr && !_isExternalOutput))
  {
    _dc->flushMsg();
    _dc = nullptr;
  }
}

bool
Tab988Merger::merge(const PaxTypeFare& ptf,
                    const VoluntaryChangesInfo& vcRec3,
                    bool overridenApplied,
                    const DateTime& applDate,
                    const ExchShopCalendar::R3ValidationResult* r3ValidationResults)
{
  T988Seqs* matchedT988Seqs;
  _trx.dataHandle().get(matchedT988Seqs);

  ExchShopCalendar::DateRange dateRange;
  if (ExchShopCalendar::isEXSCalendar(_trx))
  {
     dateRange = r3ValidationResults->getDateRange();
  }

  if (!vcRec3.reissueTblItemNo())
  {
    if (UNLIKELY(_dc && _trx.diagnostic().diagParamMapItem("DD") == "PORTION"))
      diagBanner(*ptf.fareMarket(), vcRec3, true);

    _trx.reissueOptions().insertOption(&ptf, &vcRec3, dateRange);

    if (_trx.getTrxType() == PricingTrx::RESHOP_TRX)
    {
      RexShoppingTrx& rsTrx = static_cast<RexShoppingTrx&>(_trx);
      auto& dataItemVec = rsTrx.oadDataItem(ptf.fareMarket(), vcRec3.itemNo());
      RexShoppingTrx::R3SeqsConstraint dataItem;
      dataItem.calendarAppl = ExchShopCalendar::WHOLE_PERIOD;
      dataItem.calendarRange = _dateIndRange[ExchShopCalendar::WHOLE_PERIOD];

      if (!fallback::exscDefaultWholePeriodRange(&_trx) && !dataItem.calendarRange.isValid())
      {
        const auto ondIndex =
            r3ValidationResults->getOndIndexForSeg(*ptf.fareMarket()->travelSeg().front());
        if (ondIndex != ExchShopCalendar::INVALID_OND_INDEX)
        {
          dataItem.calendarRange = r3ValidationResults->getDateRangeForOnd(ondIndex);
          if (ExchShopCalendar::isEXSCalendar(_trx))
          {
            auto& dateRange = dataItem.calendarRange;
            const auto& ond = _trx.orgDest[ondIndex];

            dateRange.firstDate = dateRange.firstDate.subtractDays(ond.calDaysBefore);
            dateRange.lastDate = dateRange.lastDate.addDays(ond.calDaysAfter);
          }
        }
      }

      if (!fallback::changeIndOpt(&_trx))
        dataItem.changeInd = vcRec3.changeInd();

      if (fallback::exscDefaultWholePeriodRange(&_trx) || dataItem.calendarRange.isValid())
        dataItemVec.push_back(std::move(dataItem));
    }

    return true;
  }

  const T988Seqs& t988Seqs = getReissue(vcRec3.vendor(), vcRec3.reissueTblItemNo(), applDate);

  if (UNLIKELY(_dc && _trx.diagnostic().diagParamMapItem("DD") == "PORTION"))
    diagBanner(*ptf.fareMarket(), vcRec3, t988Seqs.empty());

  if (t988Seqs.empty())
  {
    LOG4CXX_ERROR(logger, "Reissue table item No. " << vcRec3.reissueTblItemNo() << " not found");
    return false;
  }

  matchTbl988Seqs(matchedT988Seqs, *ptf.fareMarket(), t988Seqs, r3ValidationResults);

  if (!matchedT988Seqs->empty())
  {
    _trx.reissueOptions().insertOption(&ptf, &vcRec3, dateRange);

    for (const auto* seq : *matchedT988Seqs)
    {
      _trx.reissueOptions().insertOption(&ptf, &vcRec3, seq, _dateIndRange[seq->dateInd()]);
    }

    processT988Sequences(*ptf.fareMarket(), vcRec3, matchedT988Seqs);
  }

  if (UNLIKELY(_dc))
  {
    *_dc << "***************************************************" << std::endl;
    _dc->flushMsg();
  }

  return !matchedT988Seqs->empty();
}

void Tab988Merger::matchTbl988Seqs(T988Seqs* matchedT988Seqs,
                                   const FareMarket& fareMarket,
                                   const T988Seqs& t988Seqs,
                                   const ExchShopCalendar::R3ValidationResult* r3ValidationResults)
{
  if (UNLIKELY(_dc && _trx.diagnostic().diagParamMapItem("DD") != "988"))
  {
    _dc->deActivate();
  }

  RexShoppingTrx& rsTrx = dynamic_cast<RexShoppingTrx&>(_trx);

  if (UNLIKELY(_dc))
  {
    if (rsTrx.cxrListFromPSS().cxrList.empty())
    {
      *_dc << " CXR FROM PSS IS EMPTY\n";
    }
    else
    {
      *_dc << " CXR FROM PSS: ";
      std::set<CarrierCode>::const_iterator cxrIter = rsTrx.cxrListFromPSS().cxrList.begin();

      for (; cxrIter != rsTrx.cxrListFromPSS().cxrList.end(); ++cxrIter)
      {
        *_dc << *cxrIter << " ";
      }

      *_dc << " ARE " << (rsTrx.cxrListFromPSS().excluded ? "EXCLUDED" : "APPLICABLE") << "\n";
    }

    *_dc << "GOVERNING CRX: " << fareMarket.governingCarrier() << "\n\n";
  }

  for (auto seqElem = t988Seqs.begin(); seqElem != t988Seqs.end(); ++seqElem)
  {
    bool result = false;
    result = matchCancelAndStartOver(**seqElem) && matchAgencyRestrictions(**seqElem) &&
             matchCarrierRestrictions(fareMarket, **seqElem) &&
             matchCxrsFromPSS(rsTrx, fareMarket, **seqElem);

    if (result)
    {
      result = matchDepartureDate(fareMarket, **seqElem, r3ValidationResults) &&
               matchOriginallyScheduledFlight(fareMarket, **seqElem);
    }

    if (result)
      matchedT988Seqs->push_back(*seqElem);

    if (UNLIKELY(_dc))
      *_dc << "  SEQ " << (*seqElem)->seqNo() << (result ? ": PASS\n\n" : ": FAIL\n\n");
  }

  if (UNLIKELY(_dc && _trx.diagnostic().diagParamMapItem("DD") != "988"))
  {
    _dc->activate();
  }
}

void Tab988Merger::mergePortion(const T988Seqs& t988Seqs,
                                RexShoppingTrx::R3SeqsConstraint& dataItem)
{
  PortionMergeTvlVectType portionMergeTvlVect;
  bool canMerge = false;
  bool doNOTMerge = false;
  bool collectDiag = false;

  if (UNLIKELY(_dc && _trx.diagnostic().diagParamMapItem("DD") == "PORTION"))
  {
    collectDiag = true;
    *_dc << " ***** PORTION MERGE  *****";
  }

  for (const auto t988Seq : t988Seqs)
  {
    bool matched = true;
    canMerge = false;
    RuleUtil::TravelSegWrapperVector applTravelSegmentFrom;
    RuleUtil::TravelSegWrapperVector applTravelSegmentTo;
    std::vector<TravelSeg*> validTvlSegs;
    TvlVectType tvlVect;
    const int& itemNo = t988Seq->itemNo();
    const VendorCode& vendor = t988Seq->vendor();
    const Indicator& portionInd = t988Seq->portionInd();
    const int& geoTblItemNoFrom = t988Seq->tvlGeoTblItemNoFrom();
    const int& geoTblItemNoTo = t988Seq->tvlGeoTblItemNoTo();

    if (UNLIKELY(collectDiag))
    {
      *_dc << "\n   PORTION CHECK: " << portionInd;
      *_dc << "\n    FROM GEO SPEC TBL ITEM: " << geoTblItemNoFrom;
      *_dc << "\n    TO GEO SPEC TBL ITEM: " << geoTblItemNoTo;
      *_dc << "\n";
    }

    bool checkOrigFrom = false;
    bool checkDestFrom = false;
    TSICode tsiFrom = 0;
    LocKey locKey1From;
    LocKey locKey2From;
    RuleConst::TSIScopeType scopeFrom = RuleConst::TSI_SCOPE_JOURNEY;
    bool checkOrigTo = false;
    bool checkDestTo = false;
    TSICode tsiTo = 0;
    LocKey locKey1To;
    LocKey locKey2To;
    RuleConst::TSIScopeType scopeTo = RuleConst::TSI_SCOPE_JOURNEY;

    if (portionInd == NOT_APPLY)
    {
      if (UNLIKELY(collectDiag))
        *_dc << "\n     NO MERGED DATA\n";

      if (UNLIKELY(collectDiag && _trx.diagnostic().diagParamMapItem("PO") == "SHOWALL"))
      {
        doNOTMerge = true;
        continue;
      }
      else
      {
        canMerge = false;
        break;
      }
    }

    if (matched && geoTblItemNoFrom)
    {
      if (!getGeoRuleItem(geoTblItemNoFrom,
                          vendor,
                          _trx,
                          checkOrigFrom,
                          checkDestFrom,
                          tsiFrom,
                          locKey1From,
                          locKey2From))
      {
        if (UNLIKELY(collectDiag))
          *_dc << "    FAILED GET FROM GEO RULE ITEM: " << geoTblItemNoFrom << "\n";

        matched = false;
      }
      else
      {
        if (UNLIKELY(collectDiag))
        {
          *_dc << "    FROM GEO SPECIFICATION: ";
          diagGeoTblItem(geoTblItemNoFrom, vendor, _trx);
          *_dc << "\n";
        }
      }
    }

    if (matched && geoTblItemNoFrom && tsiFrom)
    {
      const TSIInfo* tsiInfo = getTSI(tsiFrom);

      if (tsiInfo)
        scopeFrom = RuleConst::TSIScopeType(tsiInfo->scope());
    }

    if (matched && geoTblItemNoTo)
    {
      if (!getGeoRuleItem(
              geoTblItemNoTo, vendor, _trx, checkOrigTo, checkDestTo, tsiTo, locKey1To, locKey2To))
      {
        if (UNLIKELY(collectDiag))
          *_dc << "    FAILED GET TO GEO RULE ITEM: " << geoTblItemNoFrom << "\n";

        matched = false;
      }
      else
      {
        if (UNLIKELY(collectDiag))
        {
          *_dc << "    TO GEO SPECIFICATION: ";
          diagGeoTblItem(geoTblItemNoTo, vendor, _trx);
          *_dc << "\n";
        }
      }
    }

    if (matched && geoTblItemNoTo && tsiTo)
    {
      const TSIInfo* tsiInfo = getTSI(tsiTo);

      if (tsiInfo)
        scopeTo = RuleConst::TSIScopeType(tsiInfo->scope());
    }

    if (matched && geoTblItemNoFrom != 0 && geoTblItemNoTo != 0 &&
        !checkPortionBadData(
            itemNo, vendor, checkOrigFrom, checkDestFrom, checkOrigTo, checkDestTo))
    {
      matched = false;
    }

    if (geoTblItemNoFrom != 0 && geoTblItemNoTo != 0)
    {
      if (matched && tsiFrom != 0 && tsiTo != 0 && scopeFrom != scopeTo)
      {
        if (UNLIKELY(collectDiag))
          *_dc << "    BOTH TSI MUST HAVE THE SAME TYPE OF APPLICATION\n";

        matched = false;
      }

      if (matched)
        matched = validateGeoRule(geoTblItemNoFrom,
                                  geoTblItemNoTo,
                                  vendor,
                                  tsiFrom,
                                  tsiTo,
                                  tsiFrom ? scopeFrom : scopeTo,
                                  validTvlSegs);

      if (matched)
        canMerge = determineUnchangableSegs(portionInd, validTvlSegs, tvlVect, collectDiag);
    }
    else
    {
      if (matched && geoTblItemNoFrom)
      {
        if (!tsiFrom)
          checkOrigFrom = true;

        matched = validateGeoRule(geoTblItemNoFrom,
                                  vendor,
                                  checkOrigFrom,
                                  checkDestFrom,
                                  scopeFrom,
                                  applTravelSegmentFrom);
      }

      if (matched && geoTblItemNoTo)
      {
        if (!tsiTo)
          checkDestTo = true;

        matched = validateGeoRule(
            geoTblItemNoTo, vendor, checkOrigTo, checkDestTo, scopeTo, applTravelSegmentTo);
      }

      if (matched)
      {
        canMerge = determineUnchangableSegs(portionInd,
                                            checkOrigFrom,
                                            checkDestFrom,
                                            checkOrigTo,
                                            checkDestTo,
                                            applTravelSegmentFrom,
                                            applTravelSegmentTo,
                                            tvlVect,
                                            collectDiag);
      }
    }

    if (UNLIKELY(!matched && collectDiag))
    {
      *_dc << "  SEQ " << t988Seq->seqNo() << ": PORTION MATCH FAILED\n";
    }

    if (UNLIKELY(matched && collectDiag))
    {
      if (canMerge)
      {
        *_dc << "\n     BEFORE MERGE:";
        *_dc << "\n      SEQ NO " << t988Seq->seqNo() << "\n";
        *_dc << "      UNCHANGABLE SEGS POS: ";
        TvlVectType::const_iterator tvlVectIter = tvlVect.begin();

        for (; tvlVectIter != tvlVect.end(); ++tvlVectIter)
        {
          *_dc << *tvlVectIter << " ";
        }

        *_dc << std::endl;
      }
      else
        *_dc << "\n     NO MERGED DATA";
    }

    if (matched)
    {
      if (UNLIKELY(_dc && _trx.diagnostic().diagParamMapItem("PO") == "SHOWALL"))
      {
        if (!canMerge)
        {
          doNOTMerge = true;
          continue;
        }

        portionMergeTvlVect.push_back(tvlVect);
      }
      else
      {
        if (!canMerge)
          break;

        portionMergeTvlVect.push_back(tvlVect);
      }
    }
    else
      canMerge = true;
  }

  RexShoppingTrx::PortionMergeTvlVectType portionMergePerR3;

  if (canMerge && !doNOTMerge)
    ShoppingRexUtil::mergePortion(portionMergeTvlVect, portionMergePerR3, collectDiag ? _dc : nullptr);
  else if (UNLIKELY(collectDiag))
    *_dc << "\n     NO MERGED PORTION RESTRICTIONS\n";

  // for no constrains for R3 add empty vector of merged sequences
  dataItem.portionMerge = portionMergePerR3;
}

bool
Tab988Merger::determineUnchangableSegs(const Indicator& portionInd,
                                       std::vector<TravelSeg*>& validTvlSegs,
                                       TvlVectType& tvlVect,
                                       bool collectDiag)
{
  const FareUsage* fu = nullptr;
  std::vector<TravelSeg*>::const_iterator tsi;
  std::vector<TravelSeg*>::const_iterator tsie;

  if (portionInd == NOT_APPLY || !setupFirstFlight(portionInd, fu, tsi, tsie))
    return false;

  std::vector<TravelSeg*>::const_iterator fts = validTvlSegs.begin();
  std::vector<TravelSeg*>::const_iterator ftse = validTvlSegs.end();

  if (UNLIKELY(collectDiag))
  {
    showSegsFromExchangeItin(tsi, tsie);
    *_dc << "    UNCHANGABLE SEG: ";
    fts = validTvlSegs.begin();
    ftse = validTvlSegs.end();

    if (fts != ftse)
    {
      const TravelSeg& ts = **fts;
      *_dc << ts.origAirport();
      *_dc << "-" << ts.destAirport();
      ++fts;
    }

    for (; fts != ftse; ++fts)
    {
      const TravelSeg& ts = **fts;
      *_dc << "," << ts.origAirport();
      *_dc << "-" << ts.destAirport();
    }

    *_dc << "\n";
  }

  if (validTvlSegs.size() > 0)
  {
    fts = validTvlSegs.begin();
    ftse = validTvlSegs.end();
    std::vector<TravelSeg*>::const_iterator foundTvl = tsi;

    for (; fts != ftse; ++fts)
    {
      const TravelSeg& ts = **fts;

      if (portionInd != NOT_APPLY && (foundTvl = find(tsi, tsie, &ts)) == tsie)
        continue;

      tvlVect.push_back(_itin->segmentOrder(*foundTvl));

      if (UNLIKELY(collectDiag))
        *_dc << "    SEGMENT CANNOT CHANGE: " << ts.origAirport() << "-" << ts.destAirport()
             << "\n";
    }
  }

  return !tvlVect.empty();
}

bool
Tab988Merger::determineUnchangableSegs(
    const Indicator& portionInd,
    const bool& checkOrigFrom,
    const bool& checkDestFrom,
    const bool& checkOrigTo,
    const bool& checkDestTo,
    const RuleUtil::TravelSegWrapperVector& applTravelSegmentFrom,
    const RuleUtil::TravelSegWrapperVector& applTravelSegmentTo,
    TvlVectType& tvlVect,
    bool collectDiag)
{
  bool checkOrig;
  bool checkDest;
  std::vector<const RuleUtil::TravelSegWrapperVector*> applTravelSegments;
  const FareUsage* fu = nullptr;
  std::vector<TravelSeg*>::const_iterator tsi;
  std::vector<TravelSeg*>::const_iterator tsie;

  if (applTravelSegmentFrom.size() > 0 && applTravelSegmentTo.size() > 0)
  {
    applTravelSegments.push_back(&applTravelSegmentFrom);
    applTravelSegments.push_back(&applTravelSegmentTo);
  }
  else if (applTravelSegmentFrom.size() > 0)
    applTravelSegments.push_back(&applTravelSegmentFrom);
  else if (applTravelSegmentTo.size() > 0)
    applTravelSegments.push_back(&applTravelSegmentTo);

  if (portionInd == NOT_APPLY || !setupFirstFlight(portionInd, fu, tsi, tsie))
    return false;

  if (collectDiag)
  {
    showSegsFromExchangeItin(tsi, tsie);
  }

  std::vector<const RuleUtil::TravelSegWrapperVector*>::const_iterator atsv =
      applTravelSegments.begin();
  std::vector<const RuleUtil::TravelSegWrapperVector*>::const_iterator atsve =
      applTravelSegments.end();

  for (; collectDiag && atsv != atsve; ++atsv)
  {
    const RuleUtil::TravelSegWrapperVector& applTravelSegment = **atsv;
    RuleUtil::TravelSegWrapperVector::const_iterator ats = applTravelSegment.begin();
    RuleUtil::TravelSegWrapperVector::const_iterator atse = applTravelSegment.end();

    if (&applTravelSegment == &applTravelSegmentFrom)
    {
      checkOrig = checkOrigFrom;
      checkDest = checkDestFrom;
      *_dc << "    FROM GEO UNCHANGABLE SEG: ";
    }
    else
    {
      checkOrig = checkOrigTo;
      checkDest = checkDestTo;
      *_dc << "    TO GEO UNCHANGABLE SEG: ";
    }

    bool firstTime = true;

    for (; ats != atse; ++ats)
    {
      const RuleUtil::TravelSegWrapper& tsw = **ats;

      if (checkOrig && !tsw.origMatch())
        continue;

      if (checkDest && !tsw.destMatch())
        continue;

      if (!firstTime)
        *_dc << ", ";

      *_dc << tsw.travelSeg()->origAirport();
      *_dc << "-" << tsw.travelSeg()->destAirport();
      firstTime = false;
    }

    *_dc << "\n";
  }

  atsv = applTravelSegments.begin();
  atsve = applTravelSegments.end();
  std::vector<TravelSeg*>::const_iterator foundTvl = tsi;

  for (; atsv != atsve; ++atsv)
  {
    const RuleUtil::TravelSegWrapperVector& applTravelSegment = **atsv;
    RuleUtil::TravelSegWrapperVector::const_iterator ats = applTravelSegment.begin();
    RuleUtil::TravelSegWrapperVector::const_iterator atse = applTravelSegment.end();

    for (; ats != atse; ++ats)
    {
      const RuleUtil::TravelSegWrapper& tsw = **ats;

      if (portionInd != NOT_APPLY && (foundTvl = find(tsi, tsie, tsw.travelSeg())) == tsie)
        continue;

      if (&applTravelSegment == &applTravelSegmentFrom)
      {
        checkOrig = checkOrigFrom;
        checkDest = checkDestFrom;
      }
      else
      {
        checkOrig = checkOrigTo;
        checkDest = checkDestTo;
      }

      if (checkOrig && !tsw.origMatch())
        continue;

      if (checkDest && !tsw.destMatch())
        continue;

      tvlVect.push_back(_itin->segmentOrder(*foundTvl));

      if (UNLIKELY(collectDiag))
        *_dc << "    SEGMENT CANNOT CHANGE: " << tsw.travelSeg()->origAirport() << "-"
             << tsw.travelSeg()->destAirport() << "\n";
    }
  }

  return !tvlVect.empty();
}

void
Tab988Merger::diagBanner(const FareMarket& fareMarket,
                         const VoluntaryChangesInfo& vcRec3,
                         const bool table988SequenceEmpty)
{
  *_dc << "***************************************************" << std::endl;
  *_dc << "Diagnostic 331 : MERGE STATUS for FARE MARKET: " << std::endl;
  *_dc << fareMarket.origin()->description() << '-';
  *_dc << fareMarket.destination()->description() << '\n';
  *_dc << "F - FIRST FLIGHT COUPON\n";
  *_dc << "S - FIRST FARE COMPONENT\n";
  *_dc << "BLANK - ALLOW ALL TO CHANGE\n";
  *_dc << "***************************************************" << std::endl;

  if (table988SequenceEmpty && !vcRec3.reissueTblItemNo())
  {
    *_dc << "\nERROR: TABLE 988 ITEMNO " << vcRec3.reissueTblItemNo() << " NOT FOUND\n";
  }
  else
  {
    *_dc << "\nTABLE 988 ITEMNO: " << vcRec3.reissueTblItemNo() << std::endl;
  }
}

void
Tab988Merger::showSegsFromExchangeItin(std::vector<TravelSeg*>::const_iterator& tsi,
                                       std::vector<TravelSeg*>::const_iterator& tsie)
{
  *_dc << "    SEGS FROM EXCHANGE ITIN: ";
  std::vector<TravelSeg*>::const_iterator fts = tsi;
  std::vector<TravelSeg*>::const_iterator ftse = tsie;

  if (fts != ftse)
  {
    const TravelSeg& ts = **fts;
    *_dc << ts.origAirport();
    *_dc << "-" << ts.destAirport();
    ++fts;
  }

  for (; fts != ftse; ++fts)
  {
    const TravelSeg& ts = **fts;
    *_dc << ", " << ts.origAirport();
    *_dc << "-" << ts.destAirport();
  }

  *_dc << "\n";
}

void Tab988Merger::collectForcedConnections(const T988Seqs& t988Seqs,
                                            RexShoppingTrx::R3SeqsConstraint& dataItem)
{
  static const Indicator MATCHSTOPOVERS = 'S';
  static const Indicator BLANK = ' ';
  bool collectDiag = false;

  if (UNLIKELY(_dc && _trx.diagnostic().diagParamMapItem("DD") == "CONNECTION"))
  {
    collectDiag = true;
    *_dc << "\n TAB988 FORCED CONX";
  }

  std::set<LocCode> forcedConnections;
  const std::vector<TravelSeg*>& segsExcItin = _trx.exchangeItin().front()->travelSeg();
  remove_copy_if(segsExcItin.begin(),
                 segsExcItin.end(),
                 InsertMulticity(forcedConnections),
                 std::not1(IsConnection()));

  for (const auto seqElem : t988Seqs)
  {
    ReissueSequence& t988Seq = *seqElem;
    Indicator stpCnxInd = t988Seq.stopoverConnectInd();

    if (UNLIKELY(collectDiag))
    {
      *_dc << "\n      SEQ NO " << t988Seq.seqNo();
      *_dc << "\n      STOPOVER CONN IND: " << stpCnxInd;
    }

    if (stpCnxInd == BLANK || stpCnxInd == MATCHSTOPOVERS)
    {
      forcedConnections.clear();
      break;
    }
  }

  if (UNLIKELY(collectDiag))
  {
    *_dc << "\n\n MERGED SEQS FORCED CONX: ";
    std::copy(forcedConnections.begin(),
              forcedConnections.end(),
              std::ostream_iterator<LocCode>(*_dc, " "));
    *_dc << "\n";
  }

  dataItem.forcedConnection = forcedConnections;
}

void Tab988Merger::collectFirstBreakRest(const T988Seqs& t988Seqs,
                                         RexShoppingTrx::R3SeqsConstraint& dataItem)
{
  constexpr Indicator RESTR_1ST_BREAK = 'Y';
  bool collectDiag = false;

  if (UNLIKELY(_dc && _trx.diagnostic().diagParamMapItem("DD") == "FIRSTBREAK"))
  {
    collectDiag = true;
    *_dc << "\n TAB988 FIRST BREAK RESTRICTION";

    for (const ReissueSequence* t988Seq : t988Seqs)
    {
      *_dc << "\n    SEQ NO: " << t988Seq->seqNo();
      *_dc << "\n    SAME FARE IND: " << t988Seq->firstBreakInd();
    }
  }

  bool firstFareChangeRestr = t988Seqs.empty() ? false :
                              !std::any_of(t988Seqs.begin(), t988Seqs.end(),
                              [](const ReissueSequence* seq)
                              {
                                return seq->firstBreakInd() != RESTR_1ST_BREAK;
                              });

  if (UNLIKELY(collectDiag))
  {
    *_dc << "\n FIRST BREAK RESTRICTED: " << (firstFareChangeRestr ? "Y" : "N") << "\n";
  }

  dataItem.firstBreakStatus = firstFareChangeRestr;
}

void Tab988Merger::mergeFareByteCxrAppl(const T988Seqs& t988Seqs,
                                        const CarrierCode& governingCarrier,
                                        RexShoppingTrx::R3SeqsConstraint& dataItem)
{
  RexShoppingTrx::FareByteCxrApplVect fareByteCxrApplVect;
  bool collectDiag = false;

  if (UNLIKELY(_dc && _trx.diagnostic().diagParamMapItem("DD") == "FAREBYTE"))
  {
    collectDiag = true;
    *_dc << " ***** FARE BYTE CXR APPLICABLE MERGE  *****";
  }

  for (const ReissueSequence* t988Seq : t988Seqs)
  {
    RexShoppingTrx::FareByteCxrAppl fareByteCxrAppl;

    if (t988Seq->fareCxrApplTblItemNo())
    {
      const std::vector<CarrierApplicationInfo*>& cxrAppl =
          getCarrierApplication(t988Seq->vendor(), t988Seq->fareCxrApplTblItemNo());
      std::for_each(cxrAppl.begin(),
                    cxrAppl.end(),
                    CollectResFareByteCxrAppl(fareByteCxrAppl));
    }
    else
    {
      fareByteCxrAppl.govCxrPref = true;
      // add governing carrier to applicable carriers
      fareByteCxrAppl.applCxr.insert(governingCarrier);
    }

    fareByteCxrApplVect.push_back(fareByteCxrAppl);

    if (UNLIKELY(collectDiag))
      showCxrPreMergeData(fareByteCxrAppl, t988Seq->seqNo());
  }

  RexShoppingTrx::FareByteCxrAppl fareByteCxrAppl;
  ShoppingRexUtil::mergeFareByteCxrApplRestrictions(
      fareByteCxrApplVect, fareByteCxrAppl, collectDiag ? _dc : nullptr);
  dataItem.fareByteCxrAppl = fareByteCxrAppl;
}

void
Tab988Merger::showCxrPreMergeData(const RexShoppingTrx::FareByteCxrAppl& fareByteCxrAppl,
                                  const int seqNo)
{
  *_dc << "\n\n      SEQ NO: " << seqNo;
  const std::set<CarrierCode>& restrictedCxrAppl = fareByteCxrAppl.restCxr;
  const std::set<CarrierCode>& applicableCxrAppl = fareByteCxrAppl.applCxr;
  *_dc << "\n       RESTRICTED CXR:";
  std::set<CarrierCode>::const_iterator resIter = restrictedCxrAppl.begin();

  for (; resIter != restrictedCxrAppl.end(); ++resIter)
  {
    *_dc << " " << *resIter;
  }

  *_dc << "\n       APPLICABLE CXR:";
  std::set<CarrierCode>::const_iterator appIter = applicableCxrAppl.begin();

  for (; appIter != applicableCxrAppl.end(); ++appIter)
  {
    *_dc << " " << *appIter;
  }

  if (fareByteCxrAppl.govCxrPref)
    *_dc << "\n       GOVERNING CARRIER IS PREFERRED";
}

void Tab988Merger::mergeFlightNumber(const T988Seqs& t988Seqs,
                                     RexShoppingTrx::R3SeqsConstraint& dataItem)
{
  if (UNLIKELY(_dc && _trx.diagnostic().diagParamMapItem("DD") == "FLIGHTNO"))
  {
    *_dc << " ***** FLIGHT NUMBER RESTRICTIONS MERGE  *****";
    for (const ReissueSequence* seq : t988Seqs)
    {
      *_dc << "\n      SEQ NO: " << seq->seqNo();

      if (seq->flightNoInd() == 'X')
        *_dc << "\n       FLIGHT NUMBER IS RESTRICTED";
      else
        *_dc << "\n       FLIGHT NUMBER IS NOT RESTRICTED";
      *_dc << "\n";
    }
  }

  dataItem.flightNumberRestriction = t988Seqs.empty() ? false :
                                     !std::any_of(t988Seqs.begin(), t988Seqs.end(),
                                     [](const ReissueSequence* seq)
                                     {
                                       return seq->flightNoInd() != 'X';
                                     });
}

void Tab988Merger::collectOutboundPortionRest(const T988Seqs& t988Seqs,
                                              const FareMarket& fareMarket,
                                              RexShoppingTrx::R3SeqsConstraint& dataItem)
{
  const std::vector<TravelSeg*>& outboundFCTravel = fareMarket.travelSeg();
  static const Indicator NO_RESTRICTION = ' ';
  static const Indicator FIRST_FC = 'F';
  static const Indicator ORIG_TO_STOPOVER = 'O';
  bool foundRestr(false);
  bool upToStopover(false);
  bool collectDiag(false);

  if (UNLIKELY(_dc && _trx.diagnostic().diagParamMapItem("DD") == "OUTPORTION"))
  {
    collectDiag = true;
    *_dc << "OUTBOUND PORTION RESTRICTIONS MERGE:";

    for (const auto seqElem : t988Seqs)
    {
      ReissueSequence& t988Seq = *seqElem;
      *_dc << "\n    SEQ NO: " << t988Seq.seqNo();
      *_dc << "\n    OUTBOUND PORTION IND: " << t988Seq.outboundInd();
    }
  }

  T988Seqs::const_iterator seqElem;

  for (seqElem = t988Seqs.begin(); seqElem != t988Seqs.end(); ++seqElem)
  {
    ReissueSequence& t988Seq = **seqElem;

    if (NO_RESTRICTION == t988Seq.outboundInd())
    {
      foundRestr = false;
      break;
    }
    else if (ORIG_TO_STOPOVER == t988Seq.outboundInd())
    {
      foundRestr = true;
      upToStopover = true;
    }
    else if (FIRST_FC == t988Seq.outboundInd())
    {
      if (upToStopover)
        continue;

      foundRestr = true;
    }
  }

  if (UNLIKELY(collectDiag))
    *_dc << "\n\n MERGED SEQS OUTBOUND PORTION: ";

  if (foundRestr)
  {
    std::vector<TravelSeg*>::const_iterator segRangeEnd = outboundFCTravel.end();

    if (upToStopover)
    {
      segRangeEnd = std::find_if(outboundFCTravel.begin(), outboundFCTravel.end(), IsStopOver());

      if (segRangeEnd != outboundFCTravel.end())
        segRangeEnd++;
    }

    std::vector<TravelSeg*>::const_iterator segRangeIter = outboundFCTravel.begin();

    for (; segRangeIter != segRangeEnd; ++segRangeIter)
    {
      dataItem.outboundPortion.push_back(_itin->segmentOrder(*segRangeIter));
    }

    if (UNLIKELY(collectDiag))
    {
      std::copy(dataItem.outboundPortion.begin(),
                dataItem.outboundPortion.end(),
                std::ostream_iterator<int>(*_dc, " "));
    }
  }

  if (UNLIKELY(collectDiag))
    *_dc << "\n";
}

bool
Tab988Merger::matchCxrsFromPSS(RexShoppingTrx& rsTrx,
                               const FareMarket& fareMarket,
                               const ReissueSequence& t988Seq)
{
  if (rsTrx.cxrListFromPSS().cxrList.empty())
  {
    if (UNLIKELY(_dc))
      *_dc << "  SEQ " << t988Seq.seqNo() << ": CXR FROM PSS CHECK PASSED\n";

    return true;
  }

  bool match = true;
  const uint32_t cxrApplTblItemNo = t988Seq.fareCxrApplTblItemNo();
  RexShoppingTrx::FareByteCxrAppl fareByteCxrAppl;

  if (UNLIKELY(_dc))
    *_dc << " CXRAPPLTBLITEMNO: " << cxrApplTblItemNo << "\n";

  if (cxrApplTblItemNo)
  {
    const std::vector<CarrierApplicationInfo*>& cxrAppl =
        getCarrierApplication(t988Seq.vendor(), cxrApplTblItemNo);

    if (UNLIKELY(_dc))
    {
      if (cxrAppl.empty())
      {
        *_dc << " CXR LIST IS EMPTY\n";
      }
      else
      {
        std::vector<CarrierApplicationInfo*>::const_iterator cxrApplInfoIter = cxrAppl.begin();

        for (; cxrApplInfoIter != cxrAppl.end(); ++cxrApplInfoIter)
        {
          if ((*cxrApplInfoIter)->applInd() == 'X')
            *_dc << "  REST CXR : ";
          else
            *_dc << "  APPL CXR : ";

          *_dc << (*cxrApplInfoIter)->carrier() << "\n";
        }
      }
    }

    std::for_each(cxrAppl.begin(),
                  cxrAppl.end(),
                  CollectResFareByteCxrAppl(fareByteCxrAppl));
  }

  if (rsTrx.cxrListFromPSS().excluded) // list of restricted CXRs
  {
    if (!(fareByteCxrAppl.restCxr.empty())) // if any restricted - TRUE
    {
      if (UNLIKELY(_dc))
        *_dc << "  SEQ " << t988Seq.seqNo() << ": CXR FROM PSS CHECK PASSED\n";

      return true;
    }

    if (cxrApplTblItemNo)
    {
      std::set<CarrierCode> diff;
      std::set_difference(fareByteCxrAppl.applCxr.begin(),
                          fareByteCxrAppl.applCxr.end(),
                          rsTrx.cxrListFromPSS().cxrList.begin(),
                          rsTrx.cxrListFromPSS().cxrList.end(),
                          std::inserter(diff, diff.begin()));

      if (diff.empty())
        match = false;
    }
  }
  else // list of applicable CXRs
  {
    if (cxrApplTblItemNo)
    {
      if (!(fareByteCxrAppl.restCxr.empty())) // restricted
      {
        std::set<CarrierCode> inter;
        std::set_intersection(fareByteCxrAppl.restCxr.begin(),
                              fareByteCxrAppl.restCxr.end(),
                              rsTrx.cxrListFromPSS().cxrList.begin(),
                              rsTrx.cxrListFromPSS().cxrList.end(),
                              std::inserter(inter, inter.begin()));

        if (inter.size() ==
            rsTrx.cxrListFromPSS().cxrList.size()) // all from cxr list are restricted
          match = false;
      }
      else // applicable
      {
        if (fareByteCxrAppl.applCxr.count(DOLLAR_CARRIER) == 0) // if $$ all are applicable
        {
          std::set<CarrierCode> inter;
          std::set_intersection(fareByteCxrAppl.applCxr.begin(),
                                fareByteCxrAppl.applCxr.end(),
                                rsTrx.cxrListFromPSS().cxrList.begin(),
                                rsTrx.cxrListFromPSS().cxrList.end(),
                                std::inserter(inter, inter.begin()));

          if (inter.empty()) // all from cxr list are not in applicable
            match = false;
        }
      }
    }
  }

  if (UNLIKELY(_dc))
  {
    *_dc << "  SEQ " << t988Seq.seqNo() << ": CXR FROM PSS CHECK";

    if (!match)
      *_dc << " FAILED\n";
    else
      *_dc << " PASSED\n";
  }

  return match;
}

void
Tab988Merger::createFMToFirstFMinPU(const FareMarket& currFareMarket, RexShoppingTrx& rsTrx)
{
  const FareMarket& firstFareMarket = *_pu->fareUsage().front()->paxTypeFare()->fareMarket();
  rsTrx.fMToFirstFMinPU()[&firstFareMarket].push_back(&currFareMarket);
}

namespace
{
Tab988Merger::T988Seqs
choose(const Indicator ind, const Tab988Merger::T988Seqs& matchedT988Seqs)
{
  Tab988Merger::T988Seqs result;
  result.reserve(matchedT988Seqs.size());

  for (auto t988Seq : matchedT988Seqs)
    if (t988Seq->dateInd() == ind)
      result.emplace_back(t988Seq);

  return result;
}
} // namespace

std::vector<Tab988Merger::T988Seqs>
Tab988Merger::mergeDateRestrictions(const T988Seqs& seqs)
{
  std::vector<T988Seqs> result;

  auto insertIfExists = [&seqs, &result](auto ind)
  {
    auto hasIndicator = [&](const ReissueSequence* seq)
    {
      return seq->dateInd() == ind;
    };
    if (std::any_of(seqs.begin(), seqs.end(), hasIndicator))
      result.push_back(choose(ind, seqs));
  };

  insertIfExists(NOT_APPLY);
  insertIfExists(MATCH_SAME_DEPARTURE_DATE);
  insertIfExists(MATCH_LATER_DEPARTURE_DATE);

  return result;
}

void
Tab988Merger::processT988Sequences(const FareMarket& fareMarket,
                                   const VoluntaryChangesInfo& vcRec3,
                                   const T988Seqs* matchedT988Seqs)
{
  RexShoppingTrx& rsTrx = static_cast<RexShoppingTrx&>(_trx);
  auto& dataItemVec = rsTrx.oadDataItem(&fareMarket, vcRec3.itemNo());
  createFMToFirstFMinPU(fareMarket, rsTrx);

  auto mergeDataItem = [&](const T988Seqs& t988Seqs)
  {
    RexShoppingTrx::R3SeqsConstraint dataItem;

    mergePortion(t988Seqs, dataItem);
    collectForcedConnections(t988Seqs, dataItem);
    collectFirstBreakRest(t988Seqs, dataItem);
    mergeFareByteCxrAppl(t988Seqs, fareMarket.governingCarrier(), dataItem);
    mergeFlightNumber(t988Seqs, dataItem);
    const FareMarket& firstFareMarket = *_pu->fareUsage().front()->paxTypeFare()->fareMarket();
    collectOutboundPortionRest(t988Seqs, firstFareMarket, dataItem);
    if (!fallback::changeIndOpt(&_trx) && _trx.getTrxType() == PricingTrx::RESHOP_TRX)
      dataItem.changeInd = vcRec3.changeInd();

    return dataItem;
  };

  if (!fallback::exsCalendar(&_trx) && _trx.getTrxType() == PricingTrx::RESHOP_TRX)
  {
    auto mergedRestrictions = mergeDateRestrictions(*matchedT988Seqs);
    for (const auto& seqs : mergedRestrictions)
    {
      auto dataItem = mergeDataItem(seqs);
      auto ind = seqs.front()->dateInd();
      dataItem.calendarAppl = ExchShopCalendar::convertToDateApplication(ind);
      dataItem.calendarRange = _dateIndRange[ind];
      dataItemVec.push_back(std::move(dataItem));
    }
  }
  else
  {
    dataItemVec.emplace_back(mergeDataItem(*matchedT988Seqs));
  }
}

} // namespace tse
