//-------------------------------------------------------------------
//
//  File:        ReissueTable.cpp
//  Created:     May 22, 2007
//  Authors:     Grzegorz Cholewiak
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
#include "Rules/ReissueTable.h"

#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FarePath.h"
#include "DataModel/ProcessTagInfo.h"
#include "DataModel/RexBaseTrx.h"
#include "DataModel/RexPricingRequest.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CarrierApplicationInfo.h"
#include "DBAccess/DST.h"
#include "DBAccess/GeoRuleItem.h"
#include "DBAccess/ReissueSequence.h"
#include "DBAccess/VoluntaryChangesInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "Rules/OriginallyScheduledFlightValidator.h"
#include "Util/BranchPrediction.h"

#include <algorithm>
#include <functional>
#include <set>
#include <tuple>
#include <utility>

namespace tse
{
FALLBACK_DECL(exsCalendar)
FALLBACK_DECL(exscGetSpecificOnd)

namespace
{
struct IsChanged : public std::unary_function<const TravelSeg*, bool>
{
  bool operator()(const TravelSeg* seg) const
  {
    return (seg->changeStatus() == TravelSeg::CHANGED ||
            seg->changeStatus() == TravelSeg::INVENTORYCHANGED);
  }
};

class InsertMulticity : public std::insert_iterator<std::multiset<LocCode> >
{
private:
  struct AssignmentProxy
  {
    void operator=(const TravelSeg* seg) { s->insert(seg->offMultiCity()); }
    std::multiset<LocCode>* s;
  } proxy;

public:
  InsertMulticity(std::multiset<LocCode>& s)
    : std::insert_iterator<std::multiset<LocCode> >(s, s.begin())
  {
    proxy.s = &s;
  }

  AssignmentProxy& operator*() { return proxy; }
};

DateTime shiftDate(const DateTime& dt, const int shift)
{
  if (shift >= 0)
    return dt.addDays(shift);
  else
    return dt.subtractDays(std::abs(shift));
}

template <typename Pred>
struct CheckDepartureDate : private Pred
{
  using Date = boost::gregorian::date;

  CheckDepartureDate(const Date& date, DiagCollector* dc) : _date(date), _dc(dc) {}
  bool operator()(const TravelSeg* ts, const int shift)
  {
    if (ts->hasEmptyDate())
      return false;

    auto newSegmentDT = shiftDate(ts->departureDT(), shift);
    bool status = Pred::operator()(_date, newSegmentDT.date());
    if (UNLIKELY(_dc))
    {
      *_dc << "    SEG " << ts->pnrSegment() << " " << newSegmentDT.toSimpleString() << " "
          << (status ? "DEPARTURE DATE MATCHED" : "DEPARTURE DATE NOT MATCHED") << "\n";
    }
    return status;
  }

private:
  const Date _date;
  DiagCollector* _dc;
};

using HasSameDepartureDate = CheckDepartureDate<std::equal_to<DateTime>>;
using HasLaterDepartureDate = CheckDepartureDate<std::less<DateTime>>;
} // namespace

Logger
ReissueTable::_logger("atseintl.Rules.Reissue.T988");

constexpr Indicator ReissueTable::MATCH_SAME_DEPARTURE_DATE;
constexpr Indicator ReissueTable::MATCH_LATER_DEPARTURE_DATE;

constexpr Indicator ReissueTable::NOT_APPLY;
constexpr Indicator ReissueTable::FIRST_FLIGHT_COUPON;
constexpr Indicator ReissueTable::FIRST_FLIGHT_COMPONENT;

constexpr Indicator ReissueTable::MATCHSTOPOVERS;
constexpr Indicator ReissueTable::MATCHCONNECTIONS;
constexpr Indicator ReissueTable::MATCHBOTH;
constexpr Indicator ReissueTable::BLANK;

const CarrierCode errorCode("X");

ReissueTable::ReissueTable(RexPricingTrx& trx, const Itin* itin, DiagCollector* out)
  : _dc(out),
    _trx(trx),
    _itin(itin),
    _pu(nullptr),
    _isExternalOutput(true)
{
}

ReissueTable::ReissueTable(RexPricingTrx& trx, const Itin* itin, const PricingUnit* pu)
  : _dc(nullptr),
    _trx(trx),
    _itin(itin),
    _pu(pu),
    _isExternalOutput(false)
{
  if (UNLIKELY(_trx.diagnostic().diagnosticType() == Diagnostic331 &&
               _trx.diagnostic().diagParamMapItem("DD") == "988"))
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

ReissueTable::~ReissueTable()
{
  if (UNLIKELY(_dc != nullptr && !_isExternalOutput))
  {
    _dc->flushMsg();
    _dc = nullptr;
  }
}

const std::vector<ReissueSequence*>&
ReissueTable::getReissue(const VendorCode& vendor, int itemNo, const DateTime& applDate)
{
  return _trx.dataHandle().getReissue(vendor, itemNo, _trx.dataHandle().ticketDate(), applDate);
}

const std::vector<ReissueSequence*>&
ReissueTable::getMatchedT988Seqs(const FareMarket& fareMarket,
                                 const VoluntaryChangesInfo& vcRec3,
                                 bool overridenApplied,
                                 FareCompInfo::SkippedValidationsSet* skippedValidations,
                                 const DateTime& applDate,
                                 const Cat31Info* prevalidatedCat31Info)
{
  std::vector<ReissueSequence*>* matchedT988Seqs;
  _trx.dataHandle().get(matchedT988Seqs);

  if (!vcRec3.reissueTblItemNo())
  {
    if (UNLIKELY(_dc))
      *_dc << "\nTABLE 988 ITEMNO: 0\n";

    matchedT988Seqs->push_back(nullptr);
    return *matchedT988Seqs;
  }

  const std::vector<ReissueSequence*>& t988Seqs =
      getReissue(vcRec3.vendor(), vcRec3.reissueTblItemNo(), applDate);

  if (t988Seqs.empty())
  {
    LOG4CXX_ERROR(_logger, "Reissue table item No. " << vcRec3.reissueTblItemNo() << " not found");
    if (UNLIKELY(_dc))
      *_dc << "\nERROR: TABLE 988 ITEMNO " << vcRec3.reissueTblItemNo() << " NOT FOUND\n";

    return *matchedT988Seqs;
  }

  if (UNLIKELY(_dc))
  {
    *_dc << "\nTABLE 988 ITEMNO: " << vcRec3.reissueTblItemNo() << std::endl;
  }

  for (ReissueSequence* sq : t988Seqs)
  {
    if (_trx.getTrxType() == PricingTrx::MIP_TRX &&
        _trx.excTrxType() == PricingTrx::AR_EXC_TRX &&
        !_trx.isTestRequest())
    {
      if (!matchPrevalidatedSeqTab988(prevalidatedCat31Info, sq->seqNo()))
        continue;
    }

    bool result = matchCancelAndStartOver(*sq) && matchTag7Definition(*sq) &&
                  matchFlightNo(fareMarket, *sq) && matchPortion(*sq) &&
                  matchDepartureDate(fareMarket, *sq);

    if (result)
    {
      if (overridenApplied)
      {
        if (skippedValidations)
        {
          skippedValidations->set(svCarrierRestrictions, true);
          skippedValidations->set(svCoupon, true);
          skippedValidations->set(svAgencyRestrictions, true);

          if (!matchOriginallyScheduledFlight(fareMarket, *sq))
            skippedValidations->set(svOriginallyScheduledFlight, true);
        }
      }
      else
      {
        result = matchOriginallyScheduledFlight(fareMarket, *sq) &&
                 matchCarrierRestrictions(fareMarket, *sq) && matchCoupon(*sq) &&
                 matchAgencyRestrictions(*sq);
      }

      if (result)
        result = matchOutboundPortionOfTravel(fareMarket, *sq);

      if (UNLIKELY(_dc))
      {
        *_dc << " TERM: " << sq->terminalPointInd() << '\n'
             << " FIRST BREAK: " << sq->firstBreakInd() << '\n';
      }

      if (result)
      {
        if (UNLIKELY(_dc))
          *_dc << "  SEQ " << sq->seqNo() << ": PASS\n";
        matchedT988Seqs->push_back(sq);
      }
    }
  }

  if (UNLIKELY(_dc))
    _dc->flushMsg();

  return *matchedT988Seqs;
}

bool
ReissueTable::matchPrevalidatedSeqTab988(const Cat31Info* cat31Info, const int& seqNo)
{
  const std::set<int>* prevalidatedSeqTab988 = nullptr;
  if (cat31Info)
    prevalidatedSeqTab988 = &cat31Info->tab988SeqNo;

  if (!prevalidatedSeqTab988 || prevalidatedSeqTab988->count(seqNo) == 0)
  {
    if (UNLIKELY(_dc))
      *_dc << "  SEQ " << seqNo << ": NOT IN PREVALIDATED DATA\n";
    return false;
  }
  else if (!fallback::exsCalendar(&_trx) && ExchShopCalendar::isEXSCalendar(_trx))
  {
    const ExchShopCalendar::DateRange& calendarRange{
        cat31Info->tab988SeqNoToDateRangeMap.at(seqNo)};

    if (std::none_of(_trx.orgDest.cbegin(), _trx.orgDest.cend(),
                     [&](const PricingTrx::OriginDestination& ond)
                     {
                       return calendarRange.isInDateRange(ond.travelDate);
                     }))
    {
      if (UNLIKELY(_dc))
      {
        *_dc << "  SEQ " << seqNo << ": WITH RANGE " << calendarRange
             << " DOES NOT MATCH PREVALIDATED DATA\n";
      }
      return false;
    }
  }

  return true;
}

bool
ReissueTable::matchCancelAndStartOver(const ReissueSequence& t988Seq)
{
  if (CANCEL_AND_START_OVER == t988Seq.processingInd())
  {
    if (UNLIKELY(_dc))
      *_dc << "  SEQ " << t988Seq.seqNo() << ": CANCEL AND START OVER CHECK FAILED\n";
    return false;
  }
  return true;
}

void
ReissueTable::findPotentialFCBeginsInNewItin(const LocCode& boardMultiCity,
                                             const LocCode& offMultiCity,
                                             TravelSeg::ChangeStatus changeStatus,
                                             const std::vector<TravelSeg*>& newItinSegs,
                                             std::vector<TravelSeg*>& result) const
{
  typedef std::vector<TravelSeg*>::const_iterator ItSeg;

  for (ItSeg c = newItinSegs.begin(); c != newItinSegs.end(); ++c)
    if ((*c)->boardMultiCity() == boardMultiCity && (*c)->changeStatus() == changeStatus)
      for (ItSeg b = c; b != newItinSegs.end(); ++b)
        if ((*b)->offMultiCity() == offMultiCity)
        {
          result.push_back(*c);
          break;
        }
}

bool
ReissueTable::matchFlightNo(const FareMarket& fareMarket, const ReissueSequence& t988Seq)
{
  Indicator flNoInd = t988Seq.flightNoInd();

  if (UNLIKELY(_dc))
    *_dc << " FLIGHT NUMBER CHECK: " << flNoInd << "\n";

  switch (flNoInd)
  {
  case ' ':
    return true;
  case 'X':
    break;
  default:
    LOG4CXX_ERROR(_logger,
                  "T988 item No. " << t988Seq.itemNo() << " seq No. " << t988Seq.seqNo()
                                   << " incorrect byte 23");
    if (UNLIKELY(_dc))
      *_dc << "\nERROR: TABLE 988 SEQ NO " << t988Seq.seqNo() << " INCORRECT BYTE 23\n";
    return false;
  }

  const std::vector<TravelSeg*>& segsFC = fareMarket.travelSeg();
  const std::vector<TravelSeg*>& segsNewItin = _trx.curNewItin()->travelSeg();

  std::vector<TravelSeg*>::const_iterator iFC = segsFC.begin();
  std::vector<TravelSeg*>::const_iterator iNewItin = segsNewItin.begin();

  for (; iFC != segsFC.end(); iFC++)
    if (dynamic_cast<AirSeg*>(*iFC))
      break;
  if (iFC == segsFC.end())
    return true;

  for (; iNewItin != segsNewItin.end(); iNewItin++)
  {
    AirSeg* seg = dynamic_cast<AirSeg*>(*iNewItin);
    if (!seg)
      continue;
    if (seg->flightNumber() == ((AirSeg*)*iFC)->flightNumber() &&
        seg->carrier() == ((AirSeg*)*iFC)->carrier())
      break;
  }

  while (iFC != segsFC.end() && iNewItin != segsNewItin.end())
  {
    AirSeg* oldSeg = dynamic_cast<AirSeg*>(*iFC);
    if (!oldSeg)
    {
      iFC++;
      continue;
    }
    AirSeg* newSeg = dynamic_cast<AirSeg*>(*iNewItin);
    if (!newSeg)
    {
      iNewItin++;
      continue;
    }
    if (oldSeg->flightNumber() != newSeg->flightNumber() || oldSeg->carrier() != newSeg->carrier())
      break;
    iFC++;
    iNewItin++;
  }

  if (iFC == segsFC.end())
    return true;

  if (UNLIKELY(_dc))
  {
    *_dc << "  SEQ " << t988Seq.seqNo() << ": BYTE 23 CHECK FAILED\n"
         << "    UNMATCHED FLIGHT NO: " << ((AirSeg*)*iFC)->carrier()
         << ((AirSeg*)*iFC)->flightNumber() << "\n";
  }
  return false;
}

bool
ReissueTable::updateValidSegments(const TSICode& tsiFrom,
                                  const TSICode& tsiTo,
                                  const std::vector<TravelSeg*>& allTvlSegs,
                                  const TravelSeg* atsFROMFrontSeg,
                                  const TravelSeg* atsFROMBackSeg,
                                  const std::vector<TravelSeg*>& validTOSegs,
                                  std::vector<TravelSeg*>& validTvlSegs)
{
  std::vector<TravelSeg*>::const_iterator beginOfvalid = allTvlSegs.end();
  std::vector<TravelSeg*>::const_iterator endOfvalid = allTvlSegs.end();
  if (tsiFrom)
    beginOfvalid = std::find(allTvlSegs.begin(), allTvlSegs.end(), atsFROMFrontSeg);
  else
    beginOfvalid = std::find(allTvlSegs.begin(), allTvlSegs.end(), atsFROMBackSeg);
  if (beginOfvalid == allTvlSegs.end())
    return false;
  if (tsiTo)
  {
    std::vector<TravelSeg*>::const_reverse_iterator rEndOfvalid = std::find_first_of(
        allTvlSegs.rbegin(), allTvlSegs.rend(), validTOSegs.begin(), validTOSegs.end());
    if (rEndOfvalid == allTvlSegs.rend())
      return false;
    endOfvalid = rEndOfvalid.base();
    endOfvalid--;
    if (beginOfvalid > endOfvalid)
      return false;
  }
  else
  {
    endOfvalid =
        std::find_first_of(beginOfvalid, allTvlSegs.end(), validTOSegs.begin(), validTOSegs.end());
    if (endOfvalid == allTvlSegs.end())
      return false;
  }
  endOfvalid++;
  std::copy(beginOfvalid, endOfvalid, back_inserter(validTvlSegs));

  return true;
}

bool
ReissueTable::matchOriginallyScheduledFlight(const FareMarket& fareMarket,
                                             const ReissueSequence& t988Seq)
{
  OriginallyScheduledFlightValidator val(_trx, _dc, _logger);
  return val.validate(t988Seq.itemNo(),
                      t988Seq.seqNo(),
                      fareMarket.travelSeg(),
                      t988Seq.unusedFlightInd(),
                      t988Seq.origSchedFltPeriod(),
                      t988Seq.origSchedFltUnit());
}

bool
ReissueTable::validateGeoRuleItem(const int& geoTblItemNo,
                                  const VendorCode& vendor,
                                  const RuleConst::TSIScopeParamType defaultScope,
                                  const FarePath* fp,
                                  const PricingUnit* pu,
                                  const FareMarket* fm,
                                  RuleUtil::TravelSegWrapperVector& applTravelSegment,
                                  const bool checkOrig,
                                  const bool checkDest)
{
  const DateTime ticketingDate = _trx.ticketingDate();
  bool fltStopCheck = false;
  TSICode tsiReturn = 0;
  LocKey loc1Return;
  LocKey loc2Return;

  return RuleUtil::validateGeoRuleItem(geoTblItemNo,
                                       vendor,
                                       defaultScope,
                                       true,
                                       true,
                                       true,
                                       _trx,
                                       fp,
                                       nullptr,
                                       pu,
                                       fm,
                                       ticketingDate,
                                       applTravelSegment,
                                       checkOrig,
                                       checkDest,
                                       fltStopCheck,
                                       tsiReturn,
                                       loc1Return,
                                       loc2Return,
                                       Diagnostic331);
}

bool
ReissueTable::validateGeoRule(const int& geoTblItemNoFrom,
                              const int& geoTblItemNoTo,
                              const VendorCode& vendor,
                              const RuleConst::TSIScopeParamType defaultScope,
                              const FarePath* fp,
                              const PricingUnit* pu,
                              const FareMarket* fm,
                              const TSICode& tsiFrom,
                              const TSICode& tsiTo,
                              std::vector<TravelSeg*>& validTvlSegs)
{
  const std::vector<TravelSeg*>& allTvlSegs = _itin->travelSeg();
  if (allTvlSegs.empty())
    return false;
  RuleUtil::TravelSegWrapperVector applTravelSegmentFrom;
  RuleUtil::TravelSegWrapperVector applTravelSegmentTo;
  RuleUtil::TravelSegWrapperVector::const_iterator atsToIter;
  RuleUtil::TravelSegWrapperVector::const_iterator atsToIterEnd;
  std::vector<TravelSeg*> validTOSegs;

  bool matched = validateGeoRuleItem(
      geoTblItemNoFrom, vendor, defaultScope, fp, pu, fm, applTravelSegmentFrom, true, false);

  if (!matched || applTravelSegmentFrom.size() == 0)
    return false;

  matched = validateGeoRuleItem(
      geoTblItemNoTo, vendor, defaultScope, fp, pu, fm, applTravelSegmentTo, false, true);

  if (!matched || applTravelSegmentTo.size() == 0)
    return false;

  atsToIter = applTravelSegmentTo.begin();
  atsToIterEnd = applTravelSegmentTo.end();

  for (; atsToIter != atsToIterEnd; atsToIter++)
    validTOSegs.push_back((*atsToIter)->travelSeg());

  return updateValidSegments(tsiFrom,
                             tsiTo,
                             allTvlSegs,
                             applTravelSegmentFrom.front()->travelSeg(),
                             applTravelSegmentFrom.back()->travelSeg(),
                             validTOSegs,
                             validTvlSegs);
}

bool
ReissueTable::validateGeoRule(const int& geoTblItemNoFrom,
                              const int& geoTblItemNoTo,
                              const VendorCode& vendor,
                              const TSICode& tsiFrom,
                              const TSICode& tsiTo,
                              const RuleConst::TSIScopeType& scope,
                              std::vector<TravelSeg*>& validTvlSegs)
{
  switch (scope)
  {
  case RuleConst::TSI_SCOPE_JOURNEY:
  {
    const FarePath& fp = *(_itin->farePath().front());
    validateGeoRule(geoTblItemNoFrom,
                    geoTblItemNoTo,
                    vendor,
                    RuleConst::TSI_SCOPE_PARAM_JOURNEY,
                    &fp,
                    nullptr,
                    nullptr,
                    tsiFrom,
                    tsiTo,
                    validTvlSegs);
  }
  break;
  case RuleConst::TSI_SCOPE_SJ_AND_FC:
  case RuleConst::TSI_SCOPE_SUB_JOURNEY:
  {
    const FarePath& fp = *(_itin->farePath().front());
    std::vector<PricingUnit*>::const_iterator pu = fp.pricingUnit().begin();
    std::vector<PricingUnit*>::const_iterator pue = fp.pricingUnit().end();
    for (; pu != pue; ++pu)
    {
      if (!validateGeoRule(geoTblItemNoFrom,
                           geoTblItemNoTo,
                           vendor,
                           RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                           nullptr,
                           *pu,
                           nullptr,
                           tsiFrom,
                           tsiTo,
                           validTvlSegs))
        continue;
    }
  }
  break;
  case RuleConst::TSI_SCOPE_FARE_COMPONENT:
  {
    std::vector<FareMarket*>::const_iterator fm = _itin->fareMarket().begin();
    std::vector<FareMarket*>::const_iterator fme = _itin->fareMarket().end();
    for (; fm != fme; ++fm)
    {
      if (!validateGeoRule(geoTblItemNoFrom,
                           geoTblItemNoTo,
                           vendor,
                           RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                           nullptr,
                           nullptr,
                           *fm,
                           tsiFrom,
                           tsiTo,
                           validTvlSegs))
        continue;
    }
  }
  break;
  }

  return validTvlSegs.size();
}

bool
ReissueTable::validateGeoRule(const int& geoTblItemNo,
                              const VendorCode& vendor,
                              const bool& checkOrig,
                              const bool& checkDest,
                              const RuleConst::TSIScopeType& scope,
                              RuleUtil::TravelSegWrapperVector& applTravelSegment)
{
  switch (scope)
  {
  case RuleConst::TSI_SCOPE_JOURNEY:
  {
    const FarePath& fp = *(_itin->farePath().front());
    return validateGeoRuleItem(geoTblItemNo,
                               vendor,
                               RuleConst::TSI_SCOPE_PARAM_JOURNEY,
                               &fp,
                               nullptr,
                               nullptr,
                               applTravelSegment,
                               checkOrig,
                               checkDest);
  }
  break;
  case RuleConst::TSI_SCOPE_SJ_AND_FC:
  case RuleConst::TSI_SCOPE_SUB_JOURNEY:
  {
    const FarePath& fp = *(_itin->farePath().front());
    std::vector<PricingUnit*>::const_iterator pu = fp.pricingUnit().begin();
    std::vector<PricingUnit*>::const_iterator pue = fp.pricingUnit().end();
    for (; pu != pue; ++pu)
    {
      RuleUtil::TravelSegWrapperVector ats;
      if (validateGeoRuleItem(geoTblItemNo,
                              vendor,
                              RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                              nullptr,
                              *pu,
                              nullptr,
                              ats,
                              checkOrig,
                              checkDest))
        copy(ats.begin(), ats.end(), back_inserter(applTravelSegment));
    }
  }
  break;
  case RuleConst::TSI_SCOPE_FARE_COMPONENT:
  {
    std::vector<FareMarket*>::const_iterator fm = _itin->fareMarket().begin();
    std::vector<FareMarket*>::const_iterator fme = _itin->fareMarket().end();
    for (; fm != fme; ++fm)
    {
      RuleUtil::TravelSegWrapperVector ats;
      if (validateGeoRuleItem(geoTblItemNo,
                              vendor,
                              RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                              nullptr,
                              nullptr,
                              *fm,
                              ats,
                              checkOrig,
                              checkDest))
        copy(ats.begin(), ats.end(), back_inserter(applTravelSegment));
    }
  }
  break;
  }

  return applTravelSegment.size();
}

const TSIInfo*
ReissueTable::getTSI(const TSICode& tsi)
{
  return _trx.dataHandle().getTSI(tsi);
}

bool
ReissueTable::checkPortionBadData(const int& itemNo,
                                  const VendorCode& vendor,
                                  bool& checkOrigFrom,
                                  bool& checkDestFrom,
                                  bool& checkOrigTo,
                                  bool& checkDestTo)
{
  if (checkDestFrom == true || checkOrigTo == true)
  {
    if (UNLIKELY(_dc))
      *_dc << "    BAD DATA - FROM/TO GEO TSI DOES NOT SPECIFY DEPARTURE/ARRIVIAL";
    return false;
  }
  return true;
}

bool
ReissueTable::getGeoRuleItem(const uint32_t geoTblItemNo,
                             const VendorCode& vendor,
                             RexPricingTrx& trx,
                             bool& checkOrig,
                             bool& checkDest,
                             TSICode& tsi,
                             LocKey& locKey1,
                             LocKey& locKey2)
{
  return RuleUtil::getOrigDestLocFromGeoRuleItem(
      geoTblItemNo, vendor, trx, checkOrig, checkDest, tsi, locKey1, locKey2);
}

void
ReissueTable::diagGeoTblItem(const uint32_t geoTblItemNo,
                             const tse::VendorCode& vendor,
                             RexPricingTrx& trx)
{
  RuleUtil::diagGeoTblItem(geoTblItemNo, vendor, trx, *_dc);
}

bool
ReissueTable::matchPortion(const ReissueSequence& t988Seq)
{
  bool matched = true;
  RuleUtil::TravelSegWrapperVector applTravelSegmentFrom;
  RuleUtil::TravelSegWrapperVector applTravelSegmentTo;
  std::vector<TravelSeg*> validTvlSegs;

  const int& itemNo = t988Seq.itemNo();
  const VendorCode& vendor = t988Seq.vendor();
  const Indicator& portionInd = t988Seq.portionInd();
  const int& geoTblItemNoFrom = t988Seq.tvlGeoTblItemNoFrom();
  const int& geoTblItemNoTo = t988Seq.tvlGeoTblItemNoTo();

  if (UNLIKELY(_dc))
  {
    *_dc << " PORTION CHECK: " << portionInd << "\n";
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

  if (portionInd == NOT_APPLY && geoTblItemNoFrom == 0 && geoTblItemNoTo == 0)
    return true;
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
      if (UNLIKELY(_dc))
        *_dc << "    FAILED GET FROM GEO RULE ITEM: " << geoTblItemNoFrom << "\n";
      matched = false;
    }
    else
    {
      if (UNLIKELY(_dc))
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
      if (UNLIKELY(_dc))
        *_dc << "    FAILED GET TO GEO RULE ITEM: " << geoTblItemNoFrom << "\n";
      matched = false;
    }
    else
    {
      if (UNLIKELY(_dc))
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
      !checkPortionBadData(itemNo, vendor, checkOrigFrom, checkDestFrom, checkOrigTo, checkDestTo))
  {
    matched = false;
  }
  if (geoTblItemNoFrom != 0 && geoTblItemNoTo != 0)
  {
    if (matched && tsiFrom != 0 && tsiTo != 0 && scopeFrom != scopeTo)
    {
      if (UNLIKELY(_dc))
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
      matched = validateChangeStatus(itemNo, portionInd, validTvlSegs);
  }
  else
  {
    if (matched && geoTblItemNoFrom)
    {
      if (!tsiFrom)
        checkOrigFrom = true;
      matched = validateGeoRule(
          geoTblItemNoFrom, vendor, checkOrigFrom, checkDestFrom, scopeFrom, applTravelSegmentFrom);
    }
    if (matched && geoTblItemNoTo)
    {
      if (!tsiTo)
        checkDestTo = true;
      matched = validateGeoRule(
          geoTblItemNoTo, vendor, checkOrigTo, checkDestTo, scopeTo, applTravelSegmentTo);
    }
    if (matched)
      matched = validateChangeStatus(itemNo,
                                     portionInd,
                                     checkOrigFrom,
                                     checkDestFrom,
                                     checkOrigTo,
                                     checkDestTo,
                                     applTravelSegmentFrom,
                                     applTravelSegmentTo);
  }

  if (UNLIKELY(!matched && _dc))
  {
    *_dc << "  SEQ " << t988Seq.seqNo() << ": PORTION CHECK FAILED\n";
  }

  return matched;
}

bool
ReissueTable::setupFirstFlight(const Indicator& portionInd,
                               const FareUsage*& fu,
                               std::vector<TravelSeg*>::const_iterator& tsi,
                               std::vector<TravelSeg*>::const_iterator& tsie)
{
  if (!_itin)
  {
    if (UNLIKELY(_dc))
      *_dc << "    EMPTY ITINERARY STRUCTURE\n";
  }
  else if (_itin->farePath().size() == 0 || (_itin->farePath().size() > 0 && !_itin->farePath()[0]))
  {
    if (UNLIKELY(_dc))
      *_dc << "    EMPTY FARE PATH STRUCTURE\n";
  }
  else if (_itin->farePath()[0]->pricingUnit().size() == 0 ||
           (_itin->farePath()[0]->pricingUnit().size() > 0 &&
            !_itin->farePath()[0]->pricingUnit()[0]))
  {
    if (UNLIKELY(_dc))
      *_dc << "    EMPTY PRICING UNIT STRUCTURE\n";
  }
  else if (_itin->farePath()[0]->pricingUnit()[0]->fareUsage().size() == 0 ||
           (_itin->farePath()[0]->pricingUnit()[0]->fareUsage().size() > 0 &&
            !_itin->farePath()[0]->pricingUnit()[0]->fareUsage()[0]))
  {
    if (UNLIKELY(_dc))
      *_dc << "    EMPTY FARE USAGE STRUCTURE\n";
  }
  else
  {
    fu = _itin->farePath()[0]->pricingUnit()[0]->fareUsage()[0];
    if (fu->travelSeg().size() == 0 || (fu->travelSeg().size() > 0 && !fu->travelSeg()[0]))
    {
      if (UNLIKELY(_dc))
        *_dc << "    EMPTY TRAVEL SEGMENT STRUCTURE\n";
      return false;
    }
    if (portionInd == FIRST_FLIGHT_COUPON)
    {
      tsi = fu->travelSeg().begin();
      tsie = fu->travelSeg().begin() + 1;
    }
    else if (portionInd == FIRST_FLIGHT_COMPONENT)
    {
      tsi = fu->travelSeg().begin();
      tsie = fu->travelSeg().end();
    }
    return true;
  }
  return false;
}

bool
ReissueTable::checkFirstFlight(const Indicator& portionInd, const FareUsage* fu)
{
  if (portionInd == FIRST_FLIGHT_COUPON)
  {
    const TravelSeg* firstFlightCouponExcItin = fu->travelSeg().front();

    if(IsChanged()(firstFlightCouponExcItin))
    {
      if (UNLIKELY(_dc))
        *_dc << "    CHANGED SEG: " << firstFlightCouponExcItin->origAirport() << "-"
             << firstFlightCouponExcItin->destAirport() << "\n";
      return false;
    }

    const Itin* newItin = _trx.curNewItin();
    if(newItin)
    {
      const TravelSeg* firstFlightCouponNewItin = newItin->travelSeg().front();
      if(IsChanged()(firstFlightCouponNewItin))
      {
        if (UNLIKELY(_dc))
          *_dc << "    CHANGED SEG: " << firstFlightCouponNewItin->origAirport() << "-"
               << firstFlightCouponNewItin->destAirport() << "\n";
        return false;
      }
    }

    return true;
  }
  else if (portionInd == FIRST_FLIGHT_COMPONENT)
  {
    const std::vector<TravelSeg*>::const_iterator& tsi = fu->travelSeg().begin();
    const std::vector<TravelSeg*>::const_iterator& tsie = fu->travelSeg().end();

    std::vector<TravelSeg*>::const_iterator firstChanged = std::find_if(tsi, tsie, IsChanged());

    if (firstChanged == tsie)
      return true;
    if (UNLIKELY(_dc))
      *_dc << "    CHANGED SEG: " << (*firstChanged)->origAirport() << "-"
           << (*firstChanged)->destAirport() << "\n";
    return false;
  }
  return false;
}

bool
ReissueTable::validateChangeStatus(const int& itemNo,
                                   const Indicator& portionInd,
                                   std::vector<TravelSeg*>& validTvlSegs)
{
  const FareUsage* fu = nullptr;
  std::vector<TravelSeg*>::const_iterator tsi;
  std::vector<TravelSeg*>::const_iterator tsie;

  if (portionInd != NOT_APPLY && !setupFirstFlight(portionInd, fu, tsi, tsie))
    return false;
  if (UNLIKELY(_dc))
  {
    *_dc << "    VALID SEGS: ";
    std::vector<TravelSeg*>::const_iterator fts = validTvlSegs.begin();
    std::vector<TravelSeg*>::const_iterator ftse = validTvlSegs.end();

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
    std::vector<TravelSeg*>::const_iterator fts = validTvlSegs.begin();
    std::vector<TravelSeg*>::const_iterator ftse = validTvlSegs.end();
    bool matched = false;

    for (; fts != ftse; ++fts)
    {
      const TravelSeg& ts = **fts;

      if (portionInd != NOT_APPLY && find(tsi, tsie, &ts) == tsie)
        continue;
      matched = true;
      if (IsChanged()(&ts))
      {
        if (UNLIKELY(_dc))
          *_dc << "    CHANGED SEG: " << ts.origAirport() << "-" << ts.destAirport() << "\n";
        matched = false;
        break;
      }
    }
    return matched;
  }
  return false;
}

bool
ReissueTable::validateChangeStatus(const int& itemNo,
                                   const Indicator& portionInd,
                                   const bool& checkOrigFrom,
                                   const bool& checkDestFrom,
                                   const bool& checkOrigTo,
                                   const bool& checkDestTo,
                                   const RuleUtil::TravelSegWrapperVector& applTravelSegmentFrom,
                                   const RuleUtil::TravelSegWrapperVector& applTravelSegmentTo)
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

  if (portionInd != NOT_APPLY && !setupFirstFlight(portionInd, fu, tsi, tsie))
    return false;

  std::vector<const RuleUtil::TravelSegWrapperVector*>::const_iterator atsv = applTravelSegments
                                                                                  .begin(),
                                                                       atsve =
                                                                           applTravelSegments.end();

  for (; _dc && atsv != atsve; ++atsv)
  {
    const RuleUtil::TravelSegWrapperVector& applTravelSegment = **atsv;
    RuleUtil::TravelSegWrapperVector::const_iterator ats = applTravelSegment.begin();
    RuleUtil::TravelSegWrapperVector::const_iterator atse = applTravelSegment.end();

    if (&applTravelSegment == &applTravelSegmentFrom)
    {
      checkOrig = checkOrigFrom;
      checkDest = checkDestFrom;
      *_dc << "    FROM GEO VALID SEG: ";
    }
    else
    {
      checkOrig = checkOrigTo;
      checkDest = checkDestTo;
      *_dc << "    TO GEO VALID SEG: ";
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
  for (; atsv != atsve; ++atsv)
  {
    const RuleUtil::TravelSegWrapperVector& applTravelSegment = **atsv;
    RuleUtil::TravelSegWrapperVector::const_iterator ats = applTravelSegment.begin();
    RuleUtil::TravelSegWrapperVector::const_iterator atse = applTravelSegment.end();

    bool matched = false;
    for (; ats != atse; ++ats)
    {
      const RuleUtil::TravelSegWrapper& tsw = **ats;

      if (portionInd != NOT_APPLY && find(tsi, tsie, tsw.travelSeg()) == tsie)
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
      matched = true;
      if (IsChanged()(tsw.travelSeg()))
      {
        if (UNLIKELY(_dc))
          *_dc << "    CHANGED SEG: " << tsw.travelSeg()->origAirport() << "-"
               << tsw.travelSeg()->destAirport() << "\n";
        matched = false;
        break;
      }
    }
    return matched;
  }

  return checkFirstFlight(portionInd, fu);
}

template <typename Predicate>
bool ReissueTable::validateDepDateInd(
    Predicate predicate,
    const Indicator dtInd,
    const std::vector<TravelSeg*>& newItinDepartures,
    const ExchShopCalendar::R3ValidationResult* r3ValidationResults,
    const int seqNo)
{
  bool result = false;
  int shiftBefore = 0;
  int shiftAfter = 0;

  if (!fallback::exsCalendar(&_trx) && !_trx.orgDest.empty())
  {
    shiftBefore = _trx.orgDest.front().calDaysBefore;
    shiftAfter = _trx.orgDest.front().calDaysAfter;
  }

  auto matchNewSeg = [shiftBefore, shiftAfter, &newItinDepartures, &predicate]()
  {
    TravelSeg* tvlSeg = nullptr;
    int matchedTvlSegShift = 0;
    for (int shift = -shiftBefore; shift <= shiftAfter; ++shift)
    {
      auto it = std::find_if(newItinDepartures.begin(), newItinDepartures.end(),
                             [shift, &predicate](const TravelSeg* seg)
                             {
                               return predicate(seg, shift);
                             });
      if (it != newItinDepartures.end())
      {
        tvlSeg = *it;
        matchedTvlSegShift = shift;
        break;
      }
    }
    return std::make_tuple(tvlSeg, matchedTvlSegShift);
  };

  auto validateAndInsert = [this, &r3ValidationResults, dtInd, seqNo](ExchShopCalendar::DateRange range,
                                                              int32_t ondIndex)
  {
    range = r3ValidationResults->intersection(range.getInclusiveRange(), ondIndex);
    bool status = range.isValid();
    if (status)
    {
      _dateIndRange[dtInd] = range;
      if (UNLIKELY(_dc))
      {
        *_dc << "  SEQ " << seqNo << ": DEPARTURE DATE CHECK PASSED. VALID FROM " << range << "\n";
      }
    }
    return status;
  };

  TravelSeg* matchedSeg;
  int matchedTvlSegShift = 0;
  std::tie(matchedSeg, matchedTvlSegShift) = matchNewSeg();
  if (matchedSeg)
  {
    result = true;
    if (_trx.getTrxType() == PricingTrx::RESHOP_TRX)
    {
      const int32_t ondIndex = r3ValidationResults->getOndIndexForSeg(*matchedSeg);
      if (ondIndex != ExchShopCalendar::INVALID_OND_INDEX)
      {
        ExchShopCalendar::DateRange dr;
        if (dtInd == MATCH_SAME_DEPARTURE_DATE)
        {
          const DateTime date = shiftDate(_trx.orgDest[ondIndex].travelDate, matchedTvlSegShift);
          dr = {date, date};
        }
        else
        {
          const DateTime& date = _trx.orgDest[ondIndex].travelDate;
          dr = {shiftDate(date, matchedTvlSegShift), shiftDate(date, shiftAfter)};
        }
        result = validateAndInsert(dr, ondIndex);
      }
    }
  }
  return result;
};

bool ReissueTable::matchDepartureDate(
    const FareMarket& fareMarket,
    const ReissueSequence& t988Seq,
    const ExchShopCalendar::R3ValidationResult* r3ValidationResults)
{
  Indicator dtInd = t988Seq.dateInd();

  if (UNLIKELY(_dc))
    *_dc << " DEPARTURE DATE CHECK: " << dtInd << "\n";

  if (dtInd == NOT_APPLY)
  {
    if (_trx.getTrxType() == PricingTrx::RESHOP_TRX)
    {
      // remove ExchShopCalendar::FIRST_OND on fallback removal
      if (fallback::exscGetSpecificOnd(&_trx))
      {
        _dateIndRange[NOT_APPLY] =
            r3ValidationResults->getDateRangeForOnd(ExchShopCalendar::FIRST_OND);
        if (UNLIKELY(_dc))
          *_dc << "  NOT APPLY IND FOUND, RANGE : " << _dateIndRange[NOT_APPLY] << "\n";
      }
      else
      {
        auto findOndIndex = [&]() -> int
        {
          const std::vector<TravelSeg*>& segVec = fareMarket.travelSeg();
          return r3ValidationResults->getOndIndexForSeg(*segVec.front());
        };

        const int32_t ondIndex = findOndIndex();
        if (ondIndex != ExchShopCalendar::INVALID_OND_INDEX)
        {
          _dateIndRange[NOT_APPLY] = r3ValidationResults->getDateRangeForOnd(ondIndex);
          if (UNLIKELY(_dc))
            *_dc << "  NOT APPLY IND FOUND, RANGE : " << _dateIndRange[NOT_APPLY] << "\n";
        }
        else if (UNLIKELY(_dc))
        {
          *_dc << "  OND FOR FAREMARKET NOT FOUND\n";
        }
      }
    }
    return true;
  }

  const std::vector<TravelSeg*>& segsFC = fareMarket.travelSeg();
  const std::vector<TravelSeg*>& segsNewItin = _trx.curNewItin()->travelSeg();

  std::vector<TravelSeg*> newItinDepartures;
  findPotentialFCBeginsInNewItin(segsFC.front()->boardMultiCity(), segsFC.back()->offMultiCity(),
                                 segsFC.front()->changeStatus(), segsNewItin, newItinDepartures);

  if (newItinDepartures.empty())
  {
    if (UNLIKELY(_dc))
    {
      *_dc << "  SEQ " << t988Seq.seqNo() << ": DEPARTURE DATE CHECK FAILED\n"
           << "     " << segsFC.front()->origAirport() << "-" << segsFC.back()->destAirport()
           << " FC NOT FOUND IN NEW ITIN\n";
    }
    return false;
  }

  if (segsFC.front()->segmentType() == Open)
    return true;

  bool result = false;
  switch (dtInd)
  {
  case MATCH_SAME_DEPARTURE_DATE:
  {
    result = validateDepDateInd(HasSameDepartureDate(segsFC.front()->departureDT().date(), _dc),
                                dtInd, newItinDepartures, r3ValidationResults, t988Seq.seqNo());
    break;
  }
  case MATCH_LATER_DEPARTURE_DATE:
  {
    result = validateDepDateInd(HasLaterDepartureDate(segsFC.front()->departureDT().date(), _dc),
                                dtInd, newItinDepartures, r3ValidationResults, t988Seq.seqNo());
    break;
  }
  default:
    LOG4CXX_ERROR(_logger,
                  "T988 item No. " << t988Seq.itemNo() << " seq No. " << t988Seq.seqNo()
                                   << " incorrect byte 59");
    if (UNLIKELY(_dc))
      *_dc << "\nERROR: TABLE 988 SEQ NO " << t988Seq.seqNo() << " INCORRECT BYTE 59\n";
    return false;
  }

  if (UNLIKELY(!result && _dc))
    *_dc << "  SEQ " << t988Seq.seqNo() << ": DEPARTURE DATE CHECK FAILED\n";

  return result;
}

bool
ReissueTable::matchCoupon(const ReissueSequence& t988Seq)
{
  Indicator couponInd = t988Seq.couponInd();
  bool matched = true;

  if (UNLIKELY(_dc))
    *_dc << " COUPON CHECK: " << couponInd << "\n";

  if (couponInd == 'Y' && _trx.exchangeItin().front()->domesticCouponChange())
    matched = false;

  if (UNLIKELY(!matched && _dc))
  {
    *_dc << "  DOMESTIC COUPON CANNOT BE CONVERTED TO INTERNATIONAL COUPON\n";
    *_dc << "  SEQ " << t988Seq.seqNo() << ": COUPON CHECK FAILED\n";
  }

  return matched;
}

const std::vector<CarrierApplicationInfo*>&
ReissueTable::getCarrierApplication(const VendorCode& vendor, int itemNo)
{
  return _trx.dataHandle().getCarrierApplication(vendor, itemNo);
}

const CarrierCode&
ReissueTable::findPublishingCarrier(const FareMarket& fareMarket)
{
  if (_itin->farePath().empty())
  {
    return errorCode;
  }

  std::vector<PricingUnit*>::const_iterator puIter =
                                                _itin->farePath().front()->pricingUnit().begin(),
                                            puIterEnd =
                                                _itin->farePath().front()->pricingUnit().end();

    for (; puIter != puIterEnd; puIter++)
    {
      std::vector<FareUsage*>::const_iterator fuIter = (*puIter)->fareUsage().begin();
      std::vector<FareUsage*>::const_iterator fuIterEnd = (*puIter)->fareUsage().end();
      for (; fuIter != fuIterEnd; fuIter++)
      {
        if ((&fareMarket) == (*fuIter)->paxTypeFare()->fareMarket())
        {
          LOG4CXX_DEBUG(_logger, "publishing carrier found");
          if ((*fuIter)->paxTypeFare()->carrier() == INDUSTRY_CARRIER)
            return (*fuIter)->paxTypeFare()->fareMarket()->governingCarrier();

          else
            return (*fuIter)->paxTypeFare()->carrier();
        }
      }
    }

  return fareMarket.governingCarrier();
}

bool
ReissueTable::matchCarrierRestrictions(const FareMarket& fareMarket, const ReissueSequence& t988Seq)
{
  const Indicator& carrierRestrictionIndicator = t988Seq.carrierRestInd();

  LOG4CXX_DEBUG(_logger, "carrier restriction indicator " << carrierRestrictionIndicator);

  if (UNLIKELY(_dc))
    *_dc << " CARRIER RESTRICTIONS: " << carrierRestrictionIndicator << "\n";

  RexPricingRequest* rexPricingRequest = static_cast<RexPricingRequest*>(_trx.getRequest());

  const CarrierCode& validatingCarrier = rexPricingRequest->newValidatingCarrier(); // new

  CarrierCode excValidatingCarrier;

  if (!rexPricingRequest->excValidatingCarrier().empty())
    excValidatingCarrier = rexPricingRequest->excValidatingCarrier(); // old
  else
  {
    LOG4CXX_DEBUG(_logger,
                  "no B05 XML tag for validating carrier of exchange itin, |" << validatingCarrier
                                                                              << "| set");
    excValidatingCarrier = validatingCarrier;
  }

  const CarrierCode& publishingCarrier = findPublishingCarrier(fareMarket);

  LOG4CXX_DEBUG(_logger, "validating carrier of exchange itin |" << excValidatingCarrier << "|");
  LOG4CXX_DEBUG(_logger, "validating carrier of reprice solution |" << validatingCarrier << "|");
  LOG4CXX_DEBUG(_logger, "publishing carrier |" << publishingCarrier << "|");

  if (publishingCarrier == errorCode)
  {
    return false;
  }

  if (validatingCarrier == excValidatingCarrier)
  {
    if (UNLIKELY(_dc && (carrierRestrictionIndicator == 'Y')))
      *_dc << "    TBL 990 ITEM: " << t988Seq.risRestCxrTblItemNo()
           << " CARRIER: " << validatingCarrier << "\n";
    return true;
  }

  if (validatingCarrier == publishingCarrier)
  {
    return true;
  }

  if (carrierRestrictionIndicator == 'X')
  {
    if (UNLIKELY(_dc))
      *_dc << "  SEQ " << t988Seq.seqNo() << ": CARRIER RESTRICTION FAILED\n";
    LOG4CXX_DEBUG(_logger, "entered carrierRestrictionIndicator == 'X' - fail sequence");
    return false;
  }
  else if (carrierRestrictionIndicator == 'Y')
  {
    LOG4CXX_DEBUG(_logger, "entered carrierRestrictionIndicator == 'Y'");
    const int& risRestCxrTblItemNo = t988Seq.risRestCxrTblItemNo();

    const std::vector<CarrierApplicationInfo*>& cxrAppl =
        getCarrierApplication(t988Seq.vendor(), risRestCxrTblItemNo);

    std::vector<CarrierApplicationInfo*>::const_iterator caiI = cxrAppl.begin();

    bool permission = false;

    for (; caiI != cxrAppl.end(); ++caiI)
    {
      const Indicator& currApplInd = (*caiI)->applInd();
      const CarrierCode& currCarrier = (*caiI)->carrier();

      if (currApplInd == ' ' && currCarrier == DOLLAR_CARRIER)
      {
        LOG4CXX_DEBUG(_logger, "' ' for $$ carrier in t990");
        permission = true;
      }

      else if (currApplInd == 'X' &&
               (validatingCarrier == currCarrier || publishingCarrier == currCarrier))
      {
        LOG4CXX_DEBUG(_logger, "'X' for this carrier in t990");
        permission = false;
        break;
      }

      else if (currApplInd == ' ' &&
               (validatingCarrier == currCarrier || publishingCarrier == currCarrier))
      {
        LOG4CXX_DEBUG(_logger, "' ' for this carrier in t990");
        permission = true;
        break;
      }
    }

    if (UNLIKELY(_dc))
    {
      *_dc << "    TBL 990 ITEM: " << risRestCxrTblItemNo << " CARRIER: ";

      if (!permission && (caiI == cxrAppl.end()))
        *_dc << "NO MATCH ";

      if (caiI == cxrAppl.end())
        *_dc << validatingCarrier << "\n";
      else
        *_dc << (*caiI)->carrier() << "\n";
    }

    if (!permission)
    {
      if (UNLIKELY(_dc))
        *_dc << "  SEQ " << t988Seq.seqNo() << ": CARRIER RESTRICTION FAILED\n";

      LOG4CXX_DEBUG(_logger, "carrierRestrictionIndicator == 'Y', permission = false");
      return false;
    }
  }

  // below include case when carrierRestrictionIndicator == ' ' - no restrictions
  return true;
}

bool
ReissueTable::matchAgencyRestrictions(const ReissueSequence& t988Seq)
{
  const Indicator& agencyRestrictionIndicator = t988Seq.agencyLocRest();

  LOG4CXX_DEBUG(_logger, "agency restriction indicator " << agencyRestrictionIndicator);

  if (UNLIKELY(_dc))
    *_dc << " AGENCY RESTRICTIONS: " << agencyRestrictionIndicator << "\n";

  if (agencyRestrictionIndicator == ' ')
    return true;

  if (processAgencyRestrictions(agencyRestrictionIndicator, t988Seq.iataAgencyNo()))
    return true;

  if (UNLIKELY(_dc))
    *_dc << "  SEQ " << t988Seq.seqNo() << ": AGENCY RESTRICTIONS CHECK FAILED\n";

  return false;
}

bool
ReissueTable::processAgencyRestrictions(Indicator agencyRestrictionIndicator,
                                        std::string iataAgencyNo)
{
  Agent* agent = static_cast<RexPricingRequest*>(_trx.getRequest())->currentTicketingAgent();
  if (agent->tvlAgencyPCC() == EMPTY_STRING())
    return true; // pass validation for carrier

  switch (agencyRestrictionIndicator)
  {
  case 'T': //	Travel Agency may process new transaction
    return (agent->mainTvlAgencyPCC() == iataAgencyNo);

  case 'U': //	Home Travel Agency may process new transaction
    return (agent->tvlAgencyPCC() == iataAgencyNo);

  case 'I': // IATA Travel Agency may process new transaction
    return (agent->tvlAgencyIATA() == iataAgencyNo);

  case 'H': //	Home IATA Travel Agency may process new transaction
    return (agent->homeAgencyIATA() == iataAgencyNo);
  };

  return false;
}

bool
ReissueTable::matchOutboundPortionOfTravel(const FareMarket& fareMarket,
                                           const ReissueSequence& t988Seq)
{
  if (UNLIKELY(_dc))
    *_dc << " OUTBOUND INDICATOR: " << t988Seq.outboundInd() << "\n";

  if (t988Seq.outboundInd() == ProcessTagInfo::NO_RESTRICTION)
    return true;

  const std::vector<bool>& status =
      _trx.exchangeItin().front()->getSegsChangesFor988Match(fareMarket);
  std::vector<bool>::const_iterator statusB = status.begin(), statusE = status.end();

  if (t988Seq.outboundInd() == ProcessTagInfo::ORIG_TO_STOPOVER)
  {
    std::vector<TravelSeg*>::const_iterator stopoveri =
        std::find_if(fareMarket.travelSeg().cbegin(),
                     fareMarket.travelSeg().cend(),
                     [](const TravelSeg* ts)
                     { return ts->isForcedStopOver() || ts->stopOver(); });

    if (stopoveri != fareMarket.travelSeg().end())
      statusE = statusB + (stopoveri - fareMarket.travelSeg().begin());
  }
  else if (t988Seq.outboundInd() != ProcessTagInfo::FIRST_FC)
  {
    LOG4CXX_ERROR(_logger,
                  "T988 item No. " << t988Seq.itemNo() << " seq No. " << t988Seq.seqNo()
                                   << " incorrect byte 22");
    if (UNLIKELY(_dc))
      *_dc << "\nERROR: TABLE 988 SEQ NO " << t988Seq.seqNo() << " INCORRECT BYTE 22\n";
    return false;
  }

  if (UNLIKELY(_dc))
  {
    *_dc << "  EXC FM SEGMENTS STATUSES FOR OUTBOUND VALIDATION:\n ";

    std::vector<bool>::const_iterator statusI = status.begin();
    for (const TravelSeg* ets : fareMarket.travelSeg())
    {
      *_dc << "  " << ets->origin()->loc() << "-" << ets->destination()->loc();

      if (statusI == statusE)
      {
        *_dc << " STOPOVER";
        break;
      }
      else
        *_dc << (*(statusI++) ? " C" : " U");
    }

    *_dc << "\n";
  }

  if (std::find(statusB, statusE, true) == statusE)
    return true;

  if (UNLIKELY(_dc))
  {
    *_dc << "  OUTBOUND CHECK FAILED - CHANGE NOT ALLOWED\n"
         << "  SEQ: " << t988Seq.seqNo() << " FAILED\n";
  }

  return false;
}

bool
ReissueTable::matchTag7Definition(const ReissueSequence& t988Seq)
{
  int processTag = t988Seq.processingInd();
  bool matched = true;

  if (processTag == 7)
  {
    ExcItin* excItin = _trx.exchangeItin().front();

    matched = (excItin->allSegmentsUnchangedOrInventoryChanged() &&
               excItin->travelSeg().front()->unflown());
  }

  if (UNLIKELY(!matched && _dc))
  {
    *_dc << "  SEQ 7000: TAG 7 DEFINITION FAILED \n";
  }

  return matched;
}

} // tse
