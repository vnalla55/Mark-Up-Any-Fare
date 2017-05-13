#include "Rules/FlightApplication.h"

#include "Common/CarrierUtil.h"
#include "Common/RtwUtil.h"
#include "Common/TSEException.h"
#include "Common/TseUtil.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/CarrierFlight.h"
#include "DBAccess/CarrierFlightSeg.h"
#include "DBAccess/FlightAppRule.h"
#include "DBAccess/RuleItemInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag304Collector.h"
#include "Diagnostic/DiagManager.h"
#include "Rules/CommonPredicates.h"
#include "Rules/ConstPredicates.h"
#include "Rules/FlightAppPredicates.h"
#include "Rules/MockTable986.h"
#include "Rules/RuleConst.h"
#include "Util/BranchPrediction.h"

#include <boost/array.hpp>

#include <algorithm>
#include <set>

using namespace std;
using namespace boost;

namespace tse
{
const CarrierCode FlightApplication::ANY_CARRIER = tse::EMPTY_STRING();

FlightApplication::FlightApplication()
  : _root(nullptr),
    _isForCppUnitTest(false),
    _isQualifiedCategory(false),
    _ruleItemInfo(nullptr),
    _rec2Vendor(""),
    _WQautopass(false),
    _isViaTSI(false),
    _pricingTrx(nullptr)
{
}

void
FlightApplication::getNotified()
{
  _WQautopass = true;
}

void
FlightApplication::initialize(const FlightAppRule* fap,
                              bool isQualifiedCat,
                              const VendorCode& vc,
                              const PricingTrx* trx,
                              const DateTime& ticketDate)
{
  _ruleItemInfo = fap;
  _isQualifiedCategory = isQualifiedCat;
  _dataHandle.setTicketDate(ticketDate);
  _rec2Vendor = vc;
  _pricingTrx = trx;
  buildPredicateTree();
}

bool
FlightApplication::initDiag(DCFactory*& factory,
                            DiagCollector*& diagPtr,
                            Diagnostic& trxDiag,
                            const PaxTypeFare& paxTypeFare) const
{
  if (UNLIKELY(trxDiag.diagnosticType() == Diagnostic304 && trxDiag.shouldDisplay(paxTypeFare) &&
               diagPtr->parseFareMarket(*_trx, *(paxTypeFare.fareMarket()))))
  {
    factory = DCFactory::instance();
    diagPtr = factory->create(*_trx);
    diagPtr->enable(Diagnostic304);
    return true;
  }
  return false;
}

Record3ReturnTypes
FlightApplication::process(PaxTypeFare& paxTypeFare, PricingTrx& trx)
{
  DCFactory* factory = nullptr;
  DiagCollector* diagPtr = nullptr;
  Diagnostic& trxDiag = trx.diagnostic();

  bool diagEnabled = false;

  if (UNLIKELY(trxDiag.diagnosticType() == Diagnostic304 && trxDiag.shouldDisplay(paxTypeFare) &&
               diagPtr->parseFareMarket(trx, *(paxTypeFare.fareMarket()))))
  {
    factory = DCFactory::instance();
    diagPtr = factory->create(trx);
    diagPtr->enable(Diagnostic304);
    diagEnabled = true;
  }
  // If the rule requires Inbound or Outbound pass, we skip
  // for now and do revalidation when PU is ready (we have both inbound
  // and outbound)
  const FlightAppRule* flightAppInfo = _ruleItemInfo;

  if (UNLIKELY(!flightAppInfo))
  {
    return diagAndRtn(FAIL, diagPtr);
  }

  if (UNLIKELY(diagEnabled))
  {
    Diag304Collector* diag304Ptr = nullptr;
    diag304Ptr = dynamic_cast<Diag304Collector*>(diagPtr);
    if (diag304Ptr)
    {
      diag304Ptr->diag304Collector(paxTypeFare, *flightAppInfo, *trx.getRequest());
      diag304Ptr->flushMsg();
    }
  }

  const Indicator unavailTag = flightAppInfo->unavailtag();
  if (UNLIKELY(unavailTag == RuleApplicationBase::dataUnavailable))
  {
    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << " UNAVAILABLE RULE DATA"
                 << "\n";
    }
    return diagAndRtn(FAIL, diagPtr);
  }
  else if (unavailTag == RuleApplicationBase::textOnly)
  {
    if (UNLIKELY(diagEnabled))
      (*diagPtr) << " TEXT DATA ONLY"
                 << "\n";

    return diagAndRtn(SKIP, diagPtr);
  }

  if (!_isQualifiedCategory)
    paxTypeFare.warningMap().set(WarningMap::cat4_warning, true);

  if (UNLIKELY(diagEnabled))
    displayRule(*flightAppInfo, trx, *diagPtr);

  const FareMarket& fareMarket = *paxTypeFare.fareMarket();
  const Indicator inOutInd = RtwUtil::isRtw(trx) ? Indicator(NOT_APPLY) : flightAppInfo->inOutInd();

  if (inOutInd != NOT_APPLY)
  {
    Record3ReturnTypes directMatch = PASS;

    if (UNLIKELY(diagEnabled))
      (*diagPtr) << " FARE MARKET DIRECTION: " << fareMarket.getDirectionAsString() << "\n";

    if (inOutInd == INBOUND)
    {
      if (fareMarket.direction() == FMDirection::UNKNOWN)
        directMatch = SOFTPASS;

      else if (fareMarket.direction() == FMDirection::OUTBOUND)
        directMatch = FAIL;
    }
    else if (inOutInd == OUTBOUND)
    {
      if (fareMarket.direction() == FMDirection::UNKNOWN)
        directMatch = SOFTPASS;

      else if (fareMarket.direction() == FMDirection::INBOUND)
        directMatch = FAIL;
    }
    else
    {
      // IN_XOR_OUTBOUND, IN_OR_OUTBOUND. IN_AND_OUTBOUND
      // Only can be validated at PricingUnit phase, when both
      // Inbound and Outbound are available for validation
      directMatch = SOFTPASS;
    }

    if (directMatch == SOFTPASS)
    {
      if (UNLIKELY(diagEnabled))
        (*diagPtr) << " REVALIDATE AT PU WHEN BOTH INBOUND AND OUTBOUND AVAILABLE"
                   << "\n";

      return diagAndRtn(SOFTPASS, diagPtr);
    }
    else if (directMatch == FAIL)
    {
      if (UNLIKELY(diagEnabled))
        (*diagPtr) << " NOT MATCH INBOUND/OUTBOUND DIRECTIONAL"
                   << "\n";

      return diagAndRtn(FAIL, diagPtr);
    }
  }

  const Record3ReturnTypes result = process(paxTypeFare, *(paxTypeFare.fareMarket()), trx);

  return diagAndRtn(result, diagPtr);
}

Record3ReturnTypes
FlightApplication::process(const PaxTypeFare& paxTypeFare,
                           const FareMarket& fareMarket,
                           PricingTrx& trx)
{
  _trx = &trx;
  _paxTypeFare = &paxTypeFare;
  CategoryGeoCommon::_r2Vendor = (!_rec2Vendor.empty()) ? _rec2Vendor : paxTypeFare.vendor();
  _farePath = nullptr;
  _pricingUnit = nullptr;
  _defaultScope = RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT;

  vector<TravelSeg*> filteredSegs;

  DCFactory* factory = nullptr;
  DiagCollector* diagPtr = nullptr;
  Diagnostic& trxDiag = trx.diagnostic();
  bool diagEnabled = initDiag(factory, diagPtr, trxDiag, paxTypeFare);

  if (UNLIKELY(diagEnabled))
  {
    (*diagPtr) << " FARE MARKET TVL SEGS: ";
    diagTvlSegs(paxTypeFare.fareMarket()->travelSeg(), *diagPtr);
  }

  Record3ReturnTypes result = validateGeoVia(trx, fareMarket, diagPtr);

  if (UNLIKELY(diagEnabled))
  {
    diagPtr->flushMsg();
  }

  if (result != NOTPROCESSED)
  {
    return result;
  }
  else
  {
    if (!_root)
      return PASS; // not initialized?

    // filter out non air segments
    const std::vector<TravelSeg*>& tvlSegs = fareMarket.travelSeg();

    std::vector<TravelSeg*> airSegs;
    airSegs.resize(tvlSegs.size());

    airSegs.erase(
        std::remove_copy_if(tvlSegs.begin(), tvlSegs.end(), airSegs.begin(), TseUtil::isNotAirSeg),
        airSegs.end());

    if (UNLIKELY(airSegs.empty()))
      return PASS;

    result = _root->test(airSegs, trx).valid;

    if (_isQualifiedCategory && (result == SOFTPASS))
    {
      return SKIP;
    }

    return (result == FAIL) ? FAIL : PASS;
  }
}

bool
FlightApplication::unknownDirectionaltyFMValidationFailed(Indicator inOutInd,
                                                          const FareUsage& fareUsage) const
{
  return fareUsage.paxTypeFare()->fareMarket()->direction() == FMDirection::UNKNOWN &&
         ((inOutInd == INBOUND && fareUsage.isOutbound()) ||
          (inOutInd == OUTBOUND && fareUsage.isInbound()));
}

Record3ReturnTypes FlightApplication::negateReturnValue(Record3ReturnTypes value)
{
  if (value == PASS) return FAIL;
  if (LIKELY(value == FAIL)) return PASS;
  return value;
}

Record3ReturnTypes
FlightApplication::process(PricingTrx& trx,
                           const PricingUnit& pricingUnit,
                           const FareUsage& fareUsage)
{
  _trx = &trx;

  DCFactory* factory = nullptr;
  DiagCollector* diagPtr = nullptr;
  Diagnostic& trxDiag = trx.diagnostic();
  bool diagEnabled = initDiag(factory, diagPtr, trxDiag, *fareUsage.paxTypeFare());

  const FlightAppRule* flightAppInfo = _ruleItemInfo;
  const Indicator inOutInd = RtwUtil::isRtw(trx) ? Indicator(NOT_APPLY) : flightAppInfo->inOutInd();

  if (UNLIKELY(diagEnabled))
  {
    diagPtr->printLine();
    (*diagPtr) << " VALIDATION ON PRICING UNIT"
               << "\n";
  }

  // Rule Only Apply to FareComponent
  // We do revalidation only because of Inbound/Outbound concern

  if (UNLIKELY(!flightAppInfo))
    return diagAndRtn(FAIL, diagPtr);

  if (UNLIKELY(diagEnabled))
  {
    (*diagPtr) << fareUsage.paxTypeFare()->fareClass() << setw(6) << flightAppInfo->vendor() << " "
               << flightAppInfo->itemNo() << "\n";

    displayRule(*flightAppInfo, trx, *diagPtr);
    diagPtr->printLine();
  }

  const Indicator unavailTag = flightAppInfo->unavailtag();
  if (unavailTag == RuleApplicationBase::dataUnavailable)
  {
    if (UNLIKELY(diagEnabled))
      (*diagPtr) << " UNAVAILABLE RULE DATA"
                 << "\n";

    return diagAndRtn(FAIL, diagPtr);
  }
  else if (unavailTag == RuleApplicationBase::textOnly)
  {
    if (UNLIKELY(diagEnabled))
      (*diagPtr) << " TEXT DATA ONLY"
                 << "\n";

    return diagAndRtn(SKIP, diagPtr);
  }

  if (!_isQualifiedCategory)
    fareUsage.paxTypeFare()->warningMap().set(WarningMap::cat4_warning, true);

  if ((inOutInd == NOT_APPLY) || (inOutInd == INBOUND) || (inOutInd == OUTBOUND))
  {
    // We will do same validation as we did on FareMarket,
    // currently there is no place saving the status, because Cat4 can
    // be under "IF" and appear again at PU revalidation, we can not
    // avoid the repeated work now (PL6657)
    // lint --e{530}
    const PaxTypeFare& ptFare = *fareUsage.paxTypeFare();
    const FareMarket& fareMarket = *(ptFare.fareMarket());
    Record3ReturnTypes result = PASS;

    // we should check FU directionalty, not FM directionalty beneath
    if (UNLIKELY(((inOutInd == INBOUND) && (fareMarket.direction() == FMDirection::OUTBOUND)) ||
        ((inOutInd == OUTBOUND) && (fareMarket.direction() == FMDirection::INBOUND)) ||
        unknownDirectionaltyFMValidationFailed(inOutInd, fareUsage)))
    {
      if (UNLIKELY(diagEnabled))
        (*diagPtr) << " NOT MATCH INBOUND/OUTBOUND DIRECTIONAL"
                   << "\n";

      return diagAndRtn(FAIL, diagPtr);
    }
    result = process(ptFare, fareMarket, trx);

    if (UNLIKELY(result == SKIP && _isQualifiedCategory && dynamic_cast<NoPNRPricingTrx*>(&trx) != nullptr))
      result = PASS; // if PU validation, do not suppress validation of other categories

    return diagAndRtn(result, diagPtr);
  }

  // inOutInd is IN_XOR_OUTBOUND, IN_OR_OUTBOUND or IN_AND_OUTBOUND
  // that only can be validated at PU phase

  if (inOutInd == IN_AND_OUTBOUND)
  {
    Record3ReturnTypes result = validate(pricingUnit.fareUsage(), trx, inOutInd);
    return diagAndRtn(result, diagPtr);
  }

  // Get OUTBOUND fareUsage and INBOUND fareUsage
  std::vector<FareUsage*> outBoundFU;
  std::vector<FareUsage*> inBoundFU;

  std::vector<FareUsage*>::const_iterator fareUsageI = pricingUnit.fareUsage().begin();
  std::vector<FareUsage*>::const_iterator fareUsageEndI = pricingUnit.fareUsage().end();

  for (; fareUsageI != fareUsageEndI; fareUsageI++)
  {
    if ((*fareUsageI)->isInbound())
      inBoundFU.push_back(*fareUsageI);
    else
      outBoundFU.push_back(*fareUsageI);
  }

  if (inOutInd == IN_OR_OUTBOUND)
  {
    Record3ReturnTypes rtnOB = validate(outBoundFU, trx, inOutInd);

    if (PASS == rtnOB)
      return diagAndRtn(rtnOB, diagPtr);

    Record3ReturnTypes rtnIB = validate(inBoundFU, trx, inOutInd);

    if (SKIP == rtnIB)
      // (rtnOB = SKIP or FAIL) and rtnIB = SKIP
      return diagAndRtn(rtnOB, diagPtr);

    // (rtnOB = SKIP or FAIL) and (rtnIB = FAIL or PASS)
    return diagAndRtn(rtnIB, diagPtr);
  }
  else if (inOutInd == IN_XOR_OUTBOUND)
  {
    const Record3ReturnTypes rtnOB = validate(outBoundFU, trx, inOutInd);
    const Record3ReturnTypes rtnIB = validate(inBoundFU, trx, inOutInd);

    // possible return PASS, SKIP, FAIL
    if ((rtnIB == SKIP) || (rtnOB == SKIP))
      return diagAndRtn(SKIP, diagPtr);
    else if (rtnIB != rtnOB)
      return diagAndRtn(PASS, diagPtr);
    else
      return diagAndRtn(FAIL, diagPtr);
  }
  else
  {
    // SHOULD NOT GET HERE
    return diagAndRtn(PASS, diagPtr);
  }
}

void
FlightApplication::buildPredicateTree()
{
  // lint --e{530}
  // Shorthand
  const FlightAppRule* data = _ruleItemInfo;
  if (UNLIKELY(!data))
    TSEException::assertOrThrow(TSEException::INVALID_FLIGHT_APPLICATION_DATA);

  _root = nullptr;

  // Shorthand
  bool notCondition = data->fltAppl() == MUST_NOT || data->fltAppl() == MUST_NOT_RANGE;

  if (data->fltAppl() != BLANK)
  {
    // When table996 is used, we prevent range fltAppl, this is to
    // fix some SITA rules that are coded illegally
    if ((data->carrierFltTblItemNo1() == 0) && (data->carrierFltTblItemNo2() == 0) &&
        (data->fltAppl() == MUST_RANGE || data->fltAppl() == MUST_NOT_RANGE))
    {
      IsFlightBetween<CxrExactMatch>* isBetween;
      _dataHandle.get(isBetween);
      isBetween->initialize(
          data->carrier1(), FlightApplication::ANY_CARRIER, data->flt1(), data->flt2());

      _root = isBetween;

      // Range can still 'connect' or 'and/or' or 'and' to another flight
      if (data->fltRelational2() != BLANK)
      {
        IsFlight<CxrExactMatch>* cond;
        _dataHandle.get(cond);
        cond->initialize(data->carrier3(), FlightApplication::ANY_CARRIER, data->flt3());

        addRelation(data->fltRelational2(), cond, notCondition);
      }
    }
    else
    { // not "range application"
      if (UNLIKELY(data->fltRelational1() == CONNECTING &&
          (data->fltRelational2() == AND || data->fltRelational2() == CONNECTING)))
      {
        IsFlight<CxrExactMatch>* flight;
        _dataHandle.get(flight);
        flight->initialize(data->carrier3(), FlightApplication::ANY_CARRIER, data->flt3());

        AndConnectingFlights* acFlts;
        _dataHandle.get(acFlts);
        acFlts->initialize(
            flightOrTable(data->flt2(), data->carrier2(), data->carrierFltTblItemNo2()),
            flight,
            flightOrTable(data->flt1(), data->carrier1(), data->carrierFltTblItemNo1()),
            data->fltRelational2() == CONNECTING,
            false,
            notCondition);
        _root = acFlts;
      }
      else if (UNLIKELY(data->fltRelational2() == CONNECTING && data->fltRelational1() == AND))
      {
        IsFlight<CxrExactMatch>* flight;
        _dataHandle.get(flight);
        flight->initialize(data->carrier3(), FlightApplication::ANY_CARRIER, data->flt3());

        AndConnectingFlights* acFlts;
        _dataHandle.get(acFlts);
        acFlts->initialize(
            flightOrTable(data->flt1(), data->carrier1(), data->carrierFltTblItemNo1()),
            flightOrTable(data->flt2(), data->carrier2(), data->carrierFltTblItemNo2()),
            flight,
            false,
            true,
            notCondition);
        _root = acFlts;
      }
      else
      {
        _root = flightOrTable(data->flt1(), data->carrier1(), data->carrierFltTblItemNo1());

        if (data->fltRelational1() != BLANK)
        {
          Predicate* cond;
          cond = flightOrTable(data->flt2(), data->carrier2(), data->carrierFltTblItemNo2());
          addRelation(data->fltRelational1(), cond, notCondition);

          if (data->fltRelational2() != BLANK)
          {
            IsFlight<CxrExactMatch>* cond2;
            _dataHandle.get(cond2);
            cond2->initialize(data->carrier3(), FlightApplication::ANY_CARRIER, data->flt3());
            addRelation(data->fltRelational2(), cond2, notCondition);
          }
        }
      }
    }
  }
  SameSegments* same;
  _dataHandle.get(same);

  if (_root)
    same->initialize(_root);
  bool sameNeeded = false;

  Predicate* dow = dayOfWeek(data);
  if (dow)
  {
    same->addComponent(dow);
    sameNeeded = true;
  }

  Predicate* travel = travelIndicator(data);
  if (UNLIKELY(travel))
  {
    same->addComponent(travel);
    sameNeeded = true;
  }

  Predicate* geo = geoTables(data);
  if (UNLIKELY(geo))
  {
    same->addComponent(geo);
    sameNeeded = true;
  }

  if (sameNeeded)
    _root = same;
  // if not needed, 'same' will be freed automatically by DataHandle.

  if (allFlights(data))
  {
    AllFlightsMatch* afm;
    _dataHandle.get(afm);
    afm->initialize(_root);
    _root = afm;
  }

  if (negate(data))
  {
    Not* notP;
    _dataHandle.get(notP);
    notP->initialize(_root);
    _root = notP;
  }

  Predicate* toFlight = typeOfFlight(data);
  Predicate* equip = equipment(data);
  if (equip)
  {
    same->addComponent(equip);
    sameNeeded = true;
  }
  if (toFlight || equip)
  {
    And* andP;
    _dataHandle.get(andP);
    if (_root)
      andP->addComponent(_root);
    if (equip)
    {
      AllFlightsMatch* afm;
      _dataHandle.get(afm);
      afm->initialize(equip);
      andP->addComponent(afm);
    }
    if (toFlight)
      andP->addComponent(toFlight);
    _root = andP;
  }
}

Predicate*
FlightApplication::equipment(const FlightAppRule* r3)
{
  IsEquipment* equip = nullptr;
  Predicate* ret = nullptr;

  if (r3->equipAppl() != BLANK)
  {
    _dataHandle.get(equip);
    // lint --e{413, 530}
    equip->initialize(r3->equipType());
    equip->init(*this);
    ret = equip;

    if (r3->equipAppl() == INVALID)
    {
      Not* notP;
      _dataHandle.get(notP);
      notP->initialize(equip);
      ret = notP;
    }
  }

  return ret;
}

Predicate*
FlightApplication::dayOfWeek(const FlightAppRule* r3)
{
  if (r3->dow().empty())
    return nullptr;

  set<DayOfWeekPredicate::DayName> days;
  unsigned dowSize = r3->dow().size();
  for (unsigned i = 0; i < dowSize; ++i)
  {
    char day[] = { '\0', '\0' };
    day[0] = r3->dow()[i];
    int dayNumber = atoi(day);
    days.insert(DayOfWeekPredicate::DayName(dayNumber));
  }
  DayOfWeekPredicate* dow;
  // lint -e{530}
  _dataHandle.get(dow);
  dow->initialize(days);
  return dow;
}

Predicate*
FlightApplication::travelIndicator(const FlightAppRule* r3) const
{
  // Not yet implemented
  /// @todo implement
  return nullptr;
}

Predicate*
FlightApplication::geoTables(const FlightAppRule* r3) const
{
  // Not yet implemented
  /// @todo implement
  return nullptr;
}

Predicate*
FlightApplication::typeOfFlight(const FlightAppRule* r3)
{
  Predicate* tof = nullptr;
  bool any = false;
  vector<Predicate*> elems;

  // lint --e{530}
  if (r3->fltNonStop() != FlightApplication::BLANK)
  {
    NonStopFlight* nsFlt;
    _dataHandle.get(nsFlt);
    nsFlt->countHiddenStop() = (r3->hidden() != tse::NO);
    elems.push_back(nsFlt);
    any = true;
  }

  if (r3->fltDirect() != FlightApplication::BLANK)
  {
    DirectFlight* dirFlt;
    _dataHandle.get(dirFlt);
    elems.push_back(dirFlt);
    any = true;
  }

  if (UNLIKELY(r3->fltMultiStop() != FlightApplication::BLANK))
  {
    MultiStopFlight* multiStop;
    _dataHandle.get(multiStop);
    multiStop->countHiddenStop() = (r3->hidden() != tse::NO);
    elems.push_back(multiStop);
    any = true;
  }

  if (UNLIKELY(r3->fltOneStop() != FlightApplication::BLANK))
  {
    OneStopFlight* oneStop;
    _dataHandle.get(oneStop);
    oneStop->countHiddenStop() = (r3->hidden() != tse::NO);
    elems.push_back(oneStop);
    any = true;
  }

  if (r3->fltOnline() != FlightApplication::BLANK)
  {
    OnlineConnection* online;
    _dataHandle.get(online);
    elems.push_back(online);
    any = true;
  }

  if (r3->fltInterline() != FlightApplication::BLANK)
  {
    InterlineConnection* interline;
    _dataHandle.get(interline);
    interline->underNegAppl() = r3->fltInterline() == MUST_NOT;
    elems.push_back(interline);
    any = true;
  }

  if (r3->fltSame() != FlightApplication::BLANK)
  {
    SameFlightNumber* same;
    _dataHandle.get(same);
    elems.push_back(same);
    any = true;
  }

  if (!any)
    return nullptr;

  bool valid = false;
  bool invalid = false;
  if (r3->fltNonStop() == FlightApplication::VALID || r3->fltDirect() == FlightApplication::VALID ||
      r3->fltMultiStop() == FlightApplication::VALID ||
      r3->fltOneStop() == FlightApplication::VALID || r3->fltOnline() == FlightApplication::VALID ||
      r3->fltInterline() == FlightApplication::VALID || r3->fltSame() == FlightApplication::VALID)
    valid = true;
  else if (LIKELY(r3->fltNonStop() == FlightApplication::INVALID ||
           r3->fltDirect() == FlightApplication::INVALID ||
           r3->fltMultiStop() == FlightApplication::INVALID ||
           r3->fltOneStop() == FlightApplication::INVALID ||
           r3->fltOnline() == FlightApplication::INVALID ||
           r3->fltInterline() == FlightApplication::INVALID ||
           r3->fltSame() == FlightApplication::INVALID))
    invalid = true;

  unsigned elemSize = elems.size();
  if (invalid)
  {
    And* andP = nullptr;
    _dataHandle.get(andP);
    for (unsigned i = 0; i < elemSize; ++i)
    {
      Not* notP;
      _dataHandle.get(notP);
      notP->initialize(elems[i]);
      // lint -e{413}
      andP->addComponent(notP);
    }
    tof = andP;
  }
  else if (LIKELY(valid))
  {
    Or* orP = nullptr;
    _dataHandle.get(orP);
    // lint -e{413}
    for (unsigned i = 0; i < elemSize; ++i)
      orP->addComponent(elems[i]);
    tof = orP;
  }

  return tof;
}

bool
FlightApplication::allFlights(const FlightAppRule* r3) const
{
  if (UNLIKELY(r3->fltRelational1() == CONNECTING || r3->fltRelational2() == CONNECTING))
    return false;

  if (!(r3->fltAppl() == MUST || r3->fltAppl() == MUST_RANGE))
    return false;

  return true;
}

bool
FlightApplication::negate(const FlightAppRule* r3) const
{
  if (r3->fltAppl() == MUST || r3->fltAppl() == MUST_RANGE || r3->fltAppl() == BLANK)
    return false;

  if (UNLIKELY(r3->fltRelational1() == CONNECTING || r3->fltRelational2() == CONNECTING))
    return false;

  return true;
}

void
FlightApplication::addRelation(const Indicator rel, Predicate* rhs, bool notIndicator)
{
  // lint --e{530}
  switch (rel)
  {
  case AND_OR:
  {
    Or* orPredicate;
    _dataHandle.get(orPredicate);
    orPredicate->addComponent(_root);
    orPredicate->addComponent(rhs);
    _root = orPredicate;
  }
  break;
  case CONNECTING:
  {
    AndConnectingFlights* acFlts;
    _dataHandle.get(acFlts);
    acFlts->initialize(_root, nullptr, rhs, true, true, notIndicator);
    _root = acFlts;
  }
  break;
  case AND:
  {
    AndFlight* andFlt;
    _dataHandle.get(andFlt);
    andFlt->initialize(_root, rhs);
    _root = andFlt;
  }
  break;
  default:
    /// @todo What is a default behavior??
    break;
  }
}

Predicate*
FlightApplication::flightOrTable(const FlightNumber flightNo,
                                 const CarrierCode& carrier,
                                 int fltTable)
{
  if (fltTable != 0)
    return addTable986(fltTable);
  else
  {
    IsFlight<CxrExactMatch>* isFlt;
    // lint -e{530}
    _dataHandle.get(isFlt);
    isFlt->initialize(carrier, FlightApplication::ANY_CARRIER, flightNo);
    return isFlt;
  }
}

Predicate*
FlightApplication::addTable986(int table)
{
  // lint --e{530}
  //    const VendorCode& vendor = _ruleItemInfo->vendor();

  std::vector<CarrierFlightSeg*>::const_iterator itemIter;
  std::vector<CarrierFlightSeg*>::const_iterator itemIterEnd;
  if (UNLIKELY(_isForCppUnitTest))
  {
    // get data from MockTable986 for cppunit test
    MockTable986* table986 = MockTable986::getTable(table);
    if (table986 == nullptr)
    {
      PassPredicate* pass;
      _dataHandle.get(pass);
      return pass;
    }
    itemIter = table986->segs.begin();
    itemIterEnd = table986->segs.end();
  }
  else
  {
    const CarrierFlight* table986 = _dataHandle.getCarrierFlight(_rec2Vendor, table);
    if (table986 == nullptr)
    {
      PassPredicate* pass;
      _dataHandle.get(pass);
      return pass;
    }
    itemIter = table986->segs().begin();
    itemIterEnd = table986->segs().end();
  }

  Or* tablePredicate;
  _dataHandle.get(tablePredicate);

  for (; itemIter != itemIterEnd; ++itemIter)
  {
    CarrierFlightSeg* item = *itemIter;

    addTableElementPredicate<CxrAllianceMatch>(tablePredicate, item);
  }

  return tablePredicate;
}

template <typename T>
void
FlightApplication::addTableElementPredicate(Or* predicate, CarrierFlightSeg* item)
{
  if (item->flt2() != 0)
  {
    IsFlightBetween<T>* fltBetween;
    _dataHandle.get(fltBetween);
    fltBetween->initialize(
        item->marketingCarrier(), item->operatingCarrier(), item->flt1(), item->flt2());
    predicate->addComponent(fltBetween);
  }
  else
  {
    IsFlight<T>* flt;
    _dataHandle.get(flt);
    flt->initialize(item->marketingCarrier(), item->operatingCarrier(), item->flt1());
    predicate->addComponent(flt);
  }
}

void
FlightApplication::printTree() const
{
  std::cout << "\n"
            << "Record3 is represented as a tree:"
            << "\n";
  if (_root)
    std::cout << _root->toString();
  else
    std::cout << "The tree is empty!"
              << "\n";
  std::cout << "---"
            << "\n";
}

void
FlightApplication::applyHidden(GeoBools& geoBools)
{
  const FlightAppRule* flightAppInfo = _ruleItemInfo;

  if (UNLIKELY(flightAppInfo->hidden() == 'Y'))
  {
    // On hidden stop segments only
    geoBools.orig = false;
    geoBools.dest = false;
    geoBools.via = false;
  }
  else if (flightAppInfo->hidden() == 'N')
    geoBools.fltStop = false;
}

Record3ReturnTypes
FlightApplication::validateGeoVia(PricingTrx& trx,
                                  const FareMarket& fareMarket,
                                  DiagCollector* diag)
{
  const FlightAppRule* flightAppInfo = _ruleItemInfo;
  if (UNLIKELY(!flightAppInfo))
    TSEException::assertOrThrow(TSEException::INVALID_FLIGHT_APPLICATION_DATA);

  // Shortcuts:
  const uint32_t geoBetwVia = flightAppInfo->geoTblItemNoBetwVia();
  const uint32_t geoAndVia = flightAppInfo->geoTblItemNoAndVia();
  const uint32_t geoVia = flightAppInfo->geoTblItemNoVia();

  const Indicator locAppl = flightAppInfo->locAppl();

  if (geoBetwVia == 0 && geoAndVia == 0 && geoVia == 0)
    return NOTPROCESSED; // no geo filter, use all segments

  Record3ReturnTypes matchResult = PASS;
  GeoBools geoBools; // all members be true
  std::vector<TravelSeg*> filteredBtwAndTvlSegs;
  std::vector<TravelSeg*> filteredViaTvlSegs;

  if ((geoBetwVia != 0) || (geoAndVia != 0))
  {
    if (locAppl == BETWEEN_AND)
    {
      // In RTW we shouldn't assume that travel from A to A is within A
      if (geoBetwVia == geoAndVia && !RtwUtil::isRtw(trx))
      {
        // Travle with Geo
        applyHidden(geoBools);
        getTvlSegsWithinGeo(geoBetwVia, filteredBtwAndTvlSegs, geoBools, diag);
      }
      else if (geoBetwVia != 0 && geoAndVia != 0)
      {
        // Real between and case
        applyHidden(geoBools);
        getTvlSegsBtwAndGeo(geoBetwVia, geoAndVia, filteredBtwAndTvlSegs, geoBools, diag);
      }
      else
      {
        // from, to case
        geoBools.via = false;
        geoBools.fltStop = false;
        geoBools.matchAll = false;
        const uint32_t geoFromTo = (geoBetwVia) ? geoBetwVia : geoAndVia;
        getTvlSegsFromToViaGeo(geoFromTo, filteredBtwAndTvlSegs, geoBools, diag);
      }
    } // BETWEEN_AND
    else if (locAppl == VIA)
    {
      // VIA Geo1(BtwVia) OR VIA Geo2(AndVia)
      // check FareBreak point
      applyHidden(geoBools);
      geoBools.orig = false;
      geoBools.dest = false;

      if (LIKELY(geoBetwVia))
      {
        if (UNLIKELY(diag))
        {
          (*diag) << "GEO CALL FOR BTWVIA "
                  << "\n";
        }
        // geoCallRtn = // never used
        getTvlSegsFromToViaGeo(geoBetwVia, filteredBtwAndTvlSegs, geoBools, diag);
        if (geoBools.existTSI)
          _isViaTSI = true;
      }

      // bool geoCallRtn2 = false;

      if (geoAndVia)
      {
        if (UNLIKELY(diag))
        {
          (*diag) << "GEO CALL FOR ANDVIA "
                  << "\n";
        }
        // geoCallRtn2 = // never used
        getTvlSegsFromToViaGeo(geoAndVia, filteredBtwAndTvlSegs, geoBools, diag);
        if (geoBools.existTSI)
          _isViaTSI = true;
      }

      // geoCallRtn |= geoCallRtn2;
    }
    else // locAppl == FROM_TO_VIA
    {
      // TODO
      geoBools.via = true;
      geoBools.matchAll = false;
      applyHidden(geoBools);

      // geoCallRtn = // never used
      getTvlSegsFromToViaGeo(geoBetwVia, filteredBtwAndTvlSegs, geoBools, diag);
      if (geoBools.existTSI)
        _isViaTSI = true;
    }

    if (UNLIKELY(diag))
    {
      (*diag) << " TVL SEGS FROM GEO1 GEO2: ";
      diagTvlSegs(filteredBtwAndTvlSegs, *diag);
    }
  } // geoTblItemNoBetwVia != 0 || geoTblItemNoAndVia != 0

  if (geoVia != 0)
  {
    geoBools.orig = false;
    geoBools.dest = false;
    geoBools.fltStop = true;
    geoBools.via = true;
    geoBools.matchAll = false;

    applyHidden(geoBools);

    if (UNLIKELY(diag))
    {
      (*diag) << " GEO CALL FOR VIA "
              << "\n";
    }

    // geoCallRtn = // never used
    getTvlSegsFromToViaGeo(geoVia, filteredViaTvlSegs, geoBools, diag);

    if (UNLIKELY(diag))
    {
      (*diag) << " TVL SEGS FROM GEO3: ";
      diagTvlSegs(filteredViaTvlSegs, *diag);
    }

    if (geoBools.existTSI)
      _isViaTSI = true;
  }

  // filter out none AirSeg

  if (isRemoveArnk(flightAppInfo))
  {
    std::vector<TravelSeg*>::iterator filteredBtwAndAirSegsIEnd = std::remove_if(
        filteredBtwAndTvlSegs.begin(), filteredBtwAndTvlSegs.end(), TseUtil::isNotAirSeg);
    filteredBtwAndTvlSegs.erase(filteredBtwAndAirSegsIEnd, filteredBtwAndTvlSegs.end());
  }

  std::vector<TravelSeg*>::iterator filteredViaAirSegsIEnd =
      std::remove_if(filteredViaTvlSegs.begin(), filteredViaTvlSegs.end(), TseUtil::isNotAirSeg);

  filteredViaTvlSegs.erase(filteredViaAirSegsIEnd, filteredViaTvlSegs.end());

  matchResult =
      matchResultAndAppl(trx, *flightAppInfo, filteredBtwAndTvlSegs, filteredViaTvlSegs, diag);

  return matchResult;
}

//----------------------------------------------------------------------------
// <PRE>
//
// @MethodName               FlightApplication::matchResultAndAppl()
//
//  matchResultAndAppl()     analyse return from geo call against geoAppl,
//                           viaAppl requirement, return FAIL/SKIP/PASS
//
//  @param
//
//  @return Record3ReturnTypes - possible values are:
//                 FAIL          Did not meet MUST or MUST_NOT requirement
//                 SKIP          No match on conditional requirement
//                 PASS          Met MUST or MUST_NOT requirement
//----------------------------------------------------------------------------
Record3ReturnTypes
FlightApplication::matchResultAndAppl(PricingTrx& trx,
                                      const FlightAppRule& fa,
                                      const std::vector<TravelSeg*>& filteredBtwAndTvlSegs,
                                      const std::vector<TravelSeg*>& filteredViaTvlSegs,
                                      DiagCollector* diag)
{
  std::vector<TravelSeg*> filteredTvlSegs;

  const Indicator geoAppl = fa.geoAppl();
  const Indicator viaAppl = fa.viaInd();

  const bool geoMatched = (filteredBtwAndTvlSegs.empty()) ? false : true;
  const bool viaMatched = (filteredViaTvlSegs.empty()) ? false : true;

  if ((geoAppl == CONDITIONAL) && !geoMatched)
  {
    return SKIP;
  }

  if ((viaAppl == CONDITIONAL) && !viaMatched)
  {
    return SKIP;
  }

  GeoRelation relationGeoVia = ((fa.locAppl() == VIA) && (geoAppl == MUST) && (viaAppl == MUST))
                                   ? OR_RELATION
                                   : AND_RELATION;

  // for VIA a point case, we need only in or out of the VIA point pass

  bool isPositiveVia = ((fa.locAppl() == VIA || fa.locAppl() == FROM_TO_VIA) &&
                       ((geoAppl == MUST) || (geoAppl == CONDITIONAL))) ||
                        (viaAppl == MUST) || (viaAppl == CONDITIONAL);

  bool isVia = isPositiveVia ||
      ((fa.locAppl() == VIA || fa.locAppl() == FROM_TO_VIA) && geoAppl == MUST_NOT) ||
      (viaAppl == MUST_NOT);

  // Now see if we need further check with all filtered TravelSeg
  // When there is AND_RELATION, we search travel segments
  // got with Geo1, Geo2 and locAppl() information to find
  // travel segments with Geo3 (VIA).
  // For OR_RELATION, we combine the travel segments, avoiding
  // duplicate ones

  if (relationGeoVia == OR_RELATION)
  {
    std::vector<TravelSeg*>::const_iterator first;
    std::vector<TravelSeg*>::const_iterator last;

    if (!filteredBtwAndTvlSegs.empty())
    {
      first = filteredBtwAndTvlSegs.begin();
      last = filteredBtwAndTvlSegs.end();

      filteredTvlSegs.insert(filteredTvlSegs.end(), first, last);
    }
    if (!filteredViaTvlSegs.empty())
    {
      first = filteredViaTvlSegs.begin();
      last = filteredViaTvlSegs.end();

      filteredTvlSegs.insert(filteredTvlSegs.end(), first, last);
    }

    if (filteredTvlSegs.empty())
    {
      return FAIL;
    }

    if (UNLIKELY(diag))
    {
      (*diag) << " FINAL TVL SEGS FROM GEO1 GEO2 GEO3: ";
      diagTvlSegs(filteredTvlSegs, *diag);
    }

    if (_root)
    {
      if (isPositiveVia && !isRltOnTwoFlights(fa))
      {
        return passAnySeg(filteredTvlSegs, trx);
      }

      const Record3ReturnTypes result = _root->test(filteredTvlSegs, trx).valid;

      if (_isQualifiedCategory && (result == SOFTPASS))
      {
        return SKIP;
      }

      return (result == FAIL) ? FAIL : PASS;
    }
    else
    {
      // No Flight restriction
      return PASS;
    }
  }

  // AND_RELATION
  if (geoAppl == NOT_APPLY)
  {
    return testFlightAppl(filteredViaTvlSegs, trx, viaAppl, isPositiveVia, isVia);
  }
  else if (viaAppl == NOT_APPLY)
  {
    return testFlightAppl(filteredBtwAndTvlSegs, trx, geoAppl, isPositiveVia, isVia);
  }

  // There should be no CONDITIONAL and matched now, so
  if (geoAppl == MUST)
  {
    if (!geoMatched)
      return FAIL;

    if (viaAppl == MUST && !viaMatched)
      return FAIL;

    if (viaAppl == MUST_NOT && !viaMatched)
      return PASS;
  }

  if (geoAppl == MUST_NOT && !geoMatched)
    return PASS;

  // Both apply
  findMatchedTvlSegs(filteredTvlSegs, filteredBtwAndTvlSegs, filteredViaTvlSegs);

  if (UNLIKELY((geoAppl == CONDITIONAL) && (viaAppl == CONDITIONAL) && filteredTvlSegs.empty()))
  {
    return SKIP;
  }

  // CONDITIONAL / MUST, CONDITIONAL / MUST_NOT, MUST / MUST, MUST/MUST_NOT
  // on geoAppl/viaAppl, or viaAppl/geoAppl
  // We need to validate on combined matched segment(s)
  Indicator allAppl = MUST;
  if ((geoAppl == MUST_NOT) || (viaAppl == MUST_NOT))
    allAppl = MUST_NOT;

  return testFlightAppl(filteredTvlSegs, trx, allAppl, isPositiveVia, isVia);
}

Record3ReturnTypes
FlightApplication::testFlightAppl(const std::vector<TravelSeg*>& tvlSegs,
                                  PricingTrx& trx,
                                  const Indicator& geoAppl,
                                  const bool isPositiveVia,
                                  const bool isVia)
{
  if (tvlSegs.empty())
  {
    if (geoAppl == MUST_NOT)
      return PASS;
    return FAIL;
  }

  if (_root == nullptr)
  {
    if (geoAppl == MUST_NOT)
      return FAIL;
    return PASS;
  }

  Record3ReturnTypes result;

  if (geoAppl == MUST_NOT)
  {
    // ATPCO Cat4_dapp_C.pdf, page E.03.04.0172
    //
    //  2.  NegativeBetween/And Geo (Geo Table 1, 2) with PositiveFlight Application:
    //    a.  If the Between/And geography is not within the fare component being validated,
    //        pass the Category 4 Record 3 Table.
    //    b.  If the Between/And geography is within the fare component being validated,
    //        geography cannot include the Flight Application specified.
    //
    // This code is point 2b. "Flight Application specified" is given by tree root_. We must validate
    // it against matched travel segments and negate the result, since the Flight Application specified
    // __cannot__ be within matched geography.
    if (isVia && !isRltOnTwoFlights(*_ruleItemInfo))
      return negateReturnValue(passAnySeg(tvlSegs, trx));

    result = negateReturnValue(_root->test(tvlSegs, trx).valid);
  }
  else
  {
    // positive, validation flight application
    if (isPositiveVia && !isRltOnTwoFlights(*_ruleItemInfo))
      return passAnySeg(tvlSegs, trx);

    result = _root->test(tvlSegs, trx).valid;
  }

  if (UNLIKELY(_isQualifiedCategory && (result == SOFTPASS)))
    return SKIP;

  return (result == FAIL) ? FAIL : PASS;
}

//-------------------------------------------------------------------------
// <PRE>
//
// @MethodName               FlightApplication::findMatchedTvlSegs()
//
//  findMatchedTvlSegs()     search travel segments vector1, find any segment
//                           defined in segments vector2 and push_back found
//                           ones to return travel segment vector
//
//  @param td::vector<TravelSeg*>&        tvlSegsRtn
//  @param const std::vector<TravelSeg*>& totalTvlSegs
//  @param const std::vector<TravelSeg*>& keyTvlSegs
//
//  @return bool -
//           true      found at least one
//           false     did not find any segment in both totalTvlSegs and
//                     keyTvlSegs
//----------------------------------------------------------------------------
bool
FlightApplication::findMatchedTvlSegs(std::vector<TravelSeg*>& tvlSegsRtn,
                                      const std::vector<TravelSeg*>& totalTvlSegs,
                                      const std::vector<TravelSeg*>& keyTvlSegs)
{
  std::vector<TravelSeg*>::const_iterator tvlSegI = totalTvlSegs.begin();
  std::vector<TravelSeg*>::const_iterator tvlSegEndI = totalTvlSegs.end();

  uint32_t numOfKeyTvlSeg = keyTvlSegs.size();

  for (; tvlSegI != tvlSegEndI; tvlSegI++)
  {
    std::vector<TravelSeg*>::const_iterator keyTvlSegI = keyTvlSegs.begin();
    std::vector<TravelSeg*>::const_iterator keyTvlSegEndI = keyTvlSegs.end();

    for (; keyTvlSegI != keyTvlSegEndI; keyTvlSegI++)
    {
      if (*keyTvlSegI == *tvlSegI)
      {
        tvlSegsRtn.push_back(*keyTvlSegI);
        numOfKeyTvlSeg--;
        break;
      }
    }
    if (numOfKeyTvlSeg == 0)
    {
      // already searched all possible match
      break;
    }
  }

  if (tvlSegsRtn.empty())
    return false;
  else
    return true;
}

void
FlightApplication::diagTvlSegs(const std::vector<TravelSeg*>& tvlSegs, DiagCollector& diag)
{
  if (!diag.isActive())
    return;

  bool displayHiddenStop = false;
  const FlightAppRule* flightAppInfo = _ruleItemInfo;
  if (flightAppInfo->hidden() != tse::NO)
  {
    displayHiddenStop = true;
  }

  std::vector<TravelSeg*>::const_iterator tvlSegI = tvlSegs.begin();
  std::vector<TravelSeg*>::const_iterator tvlSegEndI = tvlSegs.end();

  for (; tvlSegI != tvlSegEndI; tvlSegI++)
  {
    diag << " " << (*tvlSegI)->origAirport();
    if (displayHiddenStop)
    {
      std::vector<const Loc*>::const_iterator hStopIter = (*tvlSegI)->hiddenStops().begin();
      const std::vector<const Loc*>::const_iterator hStopIterEnd = (*tvlSegI)->hiddenStops().end();
      for (; hStopIter != hStopIterEnd; hStopIter++)
      {
        diag << "-" << (*hStopIter)->loc() << "-";
      }
    }
    diag << (*tvlSegI)->destAirport();
  }
  diag << "\n";
}

//
// inOutInd is IN_XOR_OUTBOUND, IN_OR_OUTBOUND or IN_AND_OUTBOUND
//
Record3ReturnTypes
FlightApplication::validate(const std::vector<FareUsage*>& fareUsages,
                            PricingTrx& trx,
                            const Indicator inOutInd)
{
  if (fareUsages.empty())
  {
    if (inOutInd == IN_AND_OUTBOUND)
      return PASS;
    else
      // inOutInd == IN_OR_OUTBOUND, or IN_XOR_OUTBOUND
      return FAIL;
  }

  // ATPCO requirement:
  //   IN_XOR_OUTBOUND: one or multiple Fare Components satisfy, then pass
  //   IN_OR_OUTBOUND: one or multiple Fare Components satisfy, then pass
  //   IN_AND_OUTBOUND: all Fare Components satisfy, then pass
  //
  std::vector<FareUsage*>::const_iterator fareUsageI = fareUsages.begin();
  std::vector<FareUsage*>::const_iterator fareUsageEndI = fareUsages.end();

  bool skipped = true;

  for (; fareUsageI != fareUsageEndI; fareUsageI++)
  {
    // lint -e{578, 530}
    PaxTypeFare* paxTypeFare = (*fareUsageI)->paxTypeFare();

    Record3ReturnTypes rtn = process(*paxTypeFare, *(paxTypeFare->fareMarket()), trx);

    if (rtn != SKIP)
      skipped = false;

    if (inOutInd == IN_AND_OUTBOUND)
    {
      if (rtn == FAIL)
        return FAIL;
    }
    else
    {
      // only need once PASS
      if (rtn == PASS)
        return PASS;
    }
  }

  if (skipped)
    return SKIP; // all FareUsage returned SKIP

  if (inOutInd == IN_AND_OUTBOUND)
  {
    // if we got here, it means no failure
    return PASS;
  }
  else
  {
    // if we got here, it means all failed
    return FAIL;
  }
}

Record3ReturnTypes
FlightApplication::passAnySeg(const std::vector<TravelSeg*>& tvlSegs,
                              PricingTrx& trx,
                              const Indicator geoAppl)
{
  if (UNLIKELY(!_root))
    return PASS;

  const bool mustNot = (geoAppl == MUST_NOT);
  Record3ReturnTypes result = mustNot ? PASS : FAIL;

  std::vector<TravelSeg*>::const_iterator tvlSegI = tvlSegs.begin();
  std::vector<TravelSeg*>::const_iterator tvlSegEndI = tvlSegs.end();

  for (; tvlSegI != tvlSegEndI; tvlSegI++)
  {
    std::vector<TravelSeg*> tmpTvlSegVec;

    tmpTvlSegVec.push_back(*tvlSegI);

    result = _root->test(tmpTvlSegVec, trx).valid;

    if (UNLIKELY(mustNot))
    {
      if (result == SOFTPASS)
        result = SKIP;
      else if (result == PASS)
        return FAIL;
      else
        result = PASS;
    }
    else if (UNLIKELY(_isQualifiedCategory && (result == SOFTPASS)))
    {
      result = SKIP;
    }
    else if (result != FAIL)
    {
      return PASS;
    }
  }

  return result;
}

void
FlightApplication::displayRule(const FlightAppRule& fa, PricingTrx& trx, DiagCollector& diag)
{
  const VendorCode& vendor = _ruleItemInfo->vendor();

  // In Out Bound
  const Indicator inOutInd = fa.inOutInd();
  if (inOutInd != NOT_APPLY)
  {
    diag << "FARE CAN BE USED AT ";
    if (inOutInd == INBOUND)
    {
      diag << "INBOUND ONLY";
    }
    else if (inOutInd == OUTBOUND)
    {
      diag << "OUTBOUND ONLY";
    }
    else if (inOutInd == IN_XOR_OUTBOUND)
    {
      diag << "ONE OR MULTI INBOUND OR ONE OR MULTI OUTBOUND BUT NOT BOTH";
    }
    else if (inOutInd == IN_OR_OUTBOUND)
    {
      diag << "ONE OR MULTI INBOUND AND/OR OUTBOUND";
    }
    else if (inOutInd == IN_AND_OUTBOUND)
    {
      diag << "INBOUND AND OUTBOUND";
    }
    diag << "\n";
  }

  // Geo Table
  const uint32_t geoBetw = fa.geoTblItemNoBetwVia();
  const uint32_t geoAnd = fa.geoTblItemNoAndVia();
  const Indicator locAppl = fa.locAppl();

  diag << "TRAVEL ";

  const Indicator geoAppl = fa.geoAppl();

  if (geoBetw || geoAnd)
  {
    if (geoAppl == MUST)
    {
      diag << "MUST ";
    }
    else if (geoAppl == MUST_NOT)
    {
      diag << "MUST NOT ";
    }
    else if (geoAppl == CONDITIONAL)
    {
      diag << "IF ";
    }

    if (locAppl == BETWEEN_AND)
    {
      if (geoBetw == geoAnd)
      {
        diag << "INCL WITHIN ";
        RuleUtil::diagGeoTblItem(geoBetw, vendor, trx, diag);
      }
      else if ((geoBetw != 0) && (geoAnd != 0))
      {
        diag << "INCL BTW ";
        RuleUtil::diagGeoTblItem(geoBetw, vendor, trx, diag);
        diag << " AND ";
        RuleUtil::diagGeoTblItem(geoAnd, vendor, trx, diag);
      }
      else
      {
        diag << "FROM/TO ";
        if (geoBetw)
        {
          RuleUtil::diagGeoTblItem(geoBetw, vendor, trx, diag);
        }
        else
        {
          RuleUtil::diagGeoTblItem(geoAnd, vendor, trx, diag);
        }
      }
    }
    else if (locAppl == VIA)
    {
      diag << "VIA ";
      if (geoBetw)
      {
        RuleUtil::diagGeoTblItem(geoBetw, vendor, trx, diag);
      }
      if (geoAnd)
      {
        RuleUtil::diagGeoTblItem(geoAnd, vendor, trx, diag);
      }
    }
    else if (locAppl == FROM_TO_VIA)
    {
      diag << "FROM/TO/VIA ";
      RuleUtil::diagGeoTblItem(geoBetw, vendor, trx, diag);
    }
    else
    {
      diag << "." << locAppl << ".";
    }
    diag << "\n";
  }

  const uint32_t geoVia = fa.geoTblItemNoVia();
  const Indicator viaInd = fa.viaInd();

  if (geoVia != 0)
  {
    if (geoBetw || geoAnd)
    {
      if (locAppl == VIA)
      {
        diag << "\n"
             << "  OR"
             << "\n";
      }
      else
      {
        diag << "\n"
             << "  AND"
             << "\n";
      }
    }

    if (viaInd == MUST)
    {
      diag << "MUST ";
    }
    else if (viaInd == MUST_NOT)
    {
      diag << "MUST NOT ";
    }
    else if (viaInd == CONDITIONAL)
    {
      diag << "IF ";
    }
    diag << "VIA ";
    RuleUtil::diagGeoTblItem(geoVia, vendor, trx, diag);
    diag << "\n";
  }

  // HIDDEN
  const Indicator hidden = fa.hidden();

  if (hidden != BLANK)
  {
    if (hidden == tse::YES)
    {
      diag << "VALIDATE ON HIDDEN STOP SEGMENTS ONLY"
           << "\n";
    }
    else
    {
      diag << "DO NOT VALIDATE ON HIDDEN STOP SEGMENTS"
           << "\n";
    }
  }

  bool isVia = ((fa.locAppl() == VIA) && ((geoAppl == MUST) || (geoAppl == CONDITIONAL))) ||
               (viaInd == MUST) || (viaInd == CONDITIONAL);

  // Flight Application
  if (isVia && !isRltOnTwoFlights(fa))
  {
    // for VIA a point case, we need only in or out of the VIA point pass
    diag << "IN OR OUT FLIGHT SHOULD MEET FLIGHT RESTRICTION AS:"
         << "\n";
  }
  else
  {
    diag << "SHOULD MEET FLIGHT RESTRICTION AS:"
         << "\n";
  }

  const Indicator fltAppl = fa.fltAppl();

  if (fltAppl != BLANK)
  {
    if (((fa.carrierFltTblItemNo1() != 0) || (fa.carrierFltTblItemNo2() != 0)) &&
        ((fltAppl == MUST_RANGE) || (fltAppl == MUST_NOT_RANGE)))
    {
      // Right now SITA had some rules like this, they should be
      // prevent not to happen
      diag << "----------\n";
      diag << "DATA ERROR - FLTAPPL MUST NOT BE CODED 2 OR 3 WHEN TBL996 USED\n";
      if (fltAppl == MUST_NOT_RANGE)
        diag << " WE WILL SEE 2 AS 0 NOW\n";
      else
        diag << " WE WILL SEE 3 AS 1 NOW\n";
      diag << "----------\n";
    }

    if ((fltAppl == MUST_NOT) || (fltAppl == MUST_NOT_RANGE))
    {
      diag << " MAY NOT VIA";
    }
    else if ((fltAppl == MUST) || (fltAppl == MUST_RANGE))
    {
      diag << " MUST VIA";
    }

    if (fa.carrierFltTblItemNo1())
    {
      const CarrierFlight* table986 =
          _dataHandle.getCarrierFlight(_rec2Vendor, fa.carrierFltTblItemNo1());

      if (!table986)
      {
        diag << " **** ERROR READ TABLE986:" << fa.carrierFltTblItemNo1() << "\n";
      }
      else
      {
        vector<CarrierFlightSeg*>::const_iterator carrierFlightI = table986->segs().begin();
        vector<CarrierFlightSeg*>::const_iterator carrierFlightEndI = table986->segs().end();
        for (; carrierFlightI != carrierFlightEndI; carrierFlightI++)
        {
          diag << "\n"
               << "      ";
          CarrierFlightSeg* item = *carrierFlightI;
          if (item->flt2() != 0)
          {
            diag << "BETWEEN " << item->marketingCarrier() << item->flt1() << " AND "
                 << item->marketingCarrier() << item->flt2();
          }
          else
          {
            diag << item->marketingCarrier();
            if (item->flt1() != RuleConst::ANY_FLIGHT)
            {
              diag << item->flt1();
            }
          }
          if (!item->operatingCarrier().empty())
          {
            diag << " OPERATED BY " << item->operatingCarrier();
          }
        }
      }
    }
    else if (!fa.carrier1().empty())
    {
      diag << " " << fa.carrier1();
      if (fa.flt1() > 0)
      {
        diag << " FLIGHT " << fa.flt1() << "\n";
      }
    }

    if (fa.carrierFltTblItemNo2())
    {
      diagFltRelat(fa.fltRelational1(), fltAppl, diag);

      const CarrierFlight* table986 =
          _dataHandle.getCarrierFlight(_rec2Vendor, fa.carrierFltTblItemNo2());
      if (!table986)
      {
        diag << " ERROR READ TABLE986:" << fa.carrierFltTblItemNo2();
      }
      else
      {
        vector<CarrierFlightSeg*>::const_iterator carrierFlightI = table986->segs().begin();
        vector<CarrierFlightSeg*>::const_iterator carrierFlightEndI = table986->segs().end();
        for (; carrierFlightI != carrierFlightEndI; carrierFlightI++)
        {
          diag << "\n"
               << "      ";
          CarrierFlightSeg* item = *carrierFlightI;
          if (item->flt2() != 0)
          {
            diag << "BETWEEN " << item->marketingCarrier() << item->flt1() << " AND "
                 << item->marketingCarrier() << item->flt2();
          }
          else
          {
            diag << item->marketingCarrier();
            if (item->flt1() != RuleConst::ANY_FLIGHT)
            {
              diag << item->flt1();
            }
          }
          if (!item->operatingCarrier().empty())
          {
            diag << " OPERATED BY " << item->operatingCarrier();
          }
        }
      }
    }
    else if (!fa.carrier2().empty())
    {
      diagFltRelat(fa.fltRelational1(), fltAppl, diag);

      diag << fa.carrier2();

      if (fa.flt2() > 0)
      {
        diag << fa.flt2();
      }
    }

    if (!fa.carrier3().empty())
    {
      diagFltRelat(fa.fltRelational2(), fltAppl, diag);

      diag << fa.carrier3();

      if (fa.flt3() > 0)
      {
        diag << fa.flt3();
      }
    }

    diag << "\n";
  } // fltAppl != BLANK

  Indicator serviceRestr = NOT_APPLY;

  if (fa.fltNonStop() != NOT_APPLY)
  {
    diagSvcRestr(serviceRestr, fa.fltNonStop(), diag);
    diag << " NONSTOP";
  }

  if (fa.fltDirect() != NOT_APPLY)
  {
    diagSvcRestr(serviceRestr, fa.fltDirect(), diag);
    diag << " DIRECT";
  }

  if (fa.fltMultiStop() != NOT_APPLY)
  {
    diagSvcRestr(serviceRestr, fa.fltMultiStop(), diag);
    diag << " MULTISTOP";
  }

  if (fa.fltOneStop() != NOT_APPLY)
  {
    diagSvcRestr(serviceRestr, fa.fltOneStop(), diag);
    diag << " ONESTOP";
  }

  if (fa.fltOnline() != NOT_APPLY)
  {
    diagSvcRestr(serviceRestr, fa.fltOnline(), diag);
    diag << " ONLINE CONNECTING";
  }

  if (fa.fltInterline() != NOT_APPLY)
  {
    diagSvcRestr(serviceRestr, fa.fltInterline(), diag);
    diag << " INTERLINE CONNECTING";
  }

  if (fa.fltSame() != NOT_APPLY)
  {
    diagSvcRestr(serviceRestr, fa.fltSame(), diag);
    diag << " SAME FLIGHT NO";
  }

  if (serviceRestr != NOT_APPLY)
  {
    diag << " FLIGHT"
         << "\n";
  }

  if (fa.equipAppl() != NOT_APPLY)
  {
    if (fa.equipAppl() == MUST_NOT)
    {
      diag << " MAY NOT USE ";
    }
    else
    {
      diag << " MUST USE ";
    }
    diag << fa.equipType() << "\n";
  }

  diag << "\n";

  // Day of Week
  if (!fa.dow().empty())
  {
    diag << "  TRAVEL ON";
    uint16_t dowSize = fa.dow().size();
    for (uint16_t i = 0; i < dowSize; ++i)
    {
      int dayOfWeekNo = fa.dow()[i] - '0';
      if ((dayOfWeekNo >= 7) || (dayOfWeekNo < 0))
        diag << " " << WEEKDAYS_UPPER_CASE[0];
      else
        diag << " " << WEEKDAYS_UPPER_CASE[dayOfWeekNo];
    }
    diag << "\n";
  }

  diag.flushMsg();
}

void
FlightApplication::diagSvcRestr(Indicator& serviceRestr,
                                const Indicator& restrInd,
                                DiagCollector& diag)
{
  if (serviceRestr == NOT_APPLY)
  {
    serviceRestr = restrInd;
    if (serviceRestr == MUST)
    {
      diag << " MUST USE";
    }
    else
    {
      diag << " MUST NOT USE";
    }
  }
  else
  {
    diag << " OR";
  }
}

void
FlightApplication::diagFltRelat(const Indicator& fltRelat,
                                const Indicator& fltAppl,
                                DiagCollector& diag)
{
  if (fltRelat == AND_OR)
  {
    if (fltAppl == MUST_RANGE || fltAppl == MUST_NOT_RANGE)
    {
      diag << " TO ";
    }
    else
    {
      diag << " OR ";
    }
  }
  else if (fltRelat == CONNECTING)
  {
    diag << " CONNECTING ";
  }
  else if (fltRelat == AND)
  {
    diag << " AND ";
  }
  else
  { // BLANK
    diag << " THROUGH ";
  }
}

void
FlightApplication::diagStatus(const Record3ReturnTypes result, DiagCollector& diag) const
{
  diag << "FLIGHT APPLICATION RETURN - ";

  switch (result)
  {
  case PASS:
    if (_WQautopass)
    {
      diag << "AUTOPASS";
      break;
    }
    else
    {
      diag << "PASS";
      break;
    }
  case SOFTPASS:
    diag << "SOFTPASS";
    break;
  case FAIL:
    diag << "FAIL";
    break;
  case SKIP:
    diag << "SKIP";
    break;
  case STOP:
    diag << "STOP";
    break;
  case NOTPROCESSED:
    diag << "NOTPROCESSED";
    break;
  default:
    break;
  }
  diag << "\n"
       << " "
       << "\n"
       << "\n";
}

Record3ReturnTypes
FlightApplication::diagAndRtn(const Record3ReturnTypes result, DiagCollector* diagPtr) const
{
  if (UNLIKELY(diagPtr))
  {
    diagStatus(result, *diagPtr);
    diagPtr->flushMsg();
  }
  return result;
}

Record3ReturnTypes
FlightApplication::diagAndRtn(const Record3ReturnTypes result, DiagManager& diag, bool diagEnabled)
    const
{
  if (UNLIKELY(diagEnabled))
    diagStatus(result, diag.collector());

  return result;
}

bool
FlightApplication::isRltOnTwoFlights(const FlightAppRule& flightAppInfo)
{
  if (_isViaTSI)
    return true;

  if (flightAppInfo.fltAppl() == MUST_NOT || flightAppInfo.fltAppl() == MUST_NOT_RANGE)
    return true;

  return (flightAppInfo.fltRelational1() == CONNECTING || flightAppInfo.fltRelational1() == AND ||
          flightAppInfo.fltOnline() != NOT_APPLY || flightAppInfo.fltInterline() != NOT_APPLY ||
          flightAppInfo.fltDirect() != NOT_APPLY || flightAppInfo.fltSame() != NOT_APPLY);
}

bool
FlightApplication::isRemoveArnk(const FlightAppRule* flightAppInfo) const
{
  if (UNLIKELY(flightAppInfo == nullptr))
    return true;
  else if (flightAppInfo->carrier1().empty() && flightAppInfo->carrier2().empty() &&
           flightAppInfo->carrier3().empty() && flightAppInfo->flt1() == 0 &&
           flightAppInfo->flt2() == 0 && flightAppInfo->flt3() == 0 &&
           flightAppInfo->carrierFltTblItemNo1() == 0 &&
           flightAppInfo->carrierFltTblItemNo2() == 0 && flightAppInfo->dow().empty())
    return false;
  else
    return true;
}

} // namespace tse
