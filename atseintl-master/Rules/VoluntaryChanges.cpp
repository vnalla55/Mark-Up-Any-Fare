#include "Rules/VoluntaryChanges.h"

#include "Common/Logger.h"
#include "Common/PaxTypeUtil.h"
#include "Common/TravelSegUtil.h"
#include "Common/ShoppingRexUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FarePath.h"
#include "DataModel/RexBaseTrx.h"
#include "DataModel/RexPricingRequest.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CarrierApplicationInfo.h"
#include "DBAccess/DateOverrideRuleItem.h"
#include "Rules/AdvanceResTkt.h"
#include "Rules/Cat31ChangeFinder.h"
#include "Rules/DepartureValidator.h"
#include "Rules/PaxTypeCodeValidator.h"
#include "Rules/ReissueTable.h"
#include "Rules/Tab988Merger.h"
#include "Rules/WaiverCodeValidator.h"
#include "Util/BranchPrediction.h"

#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <utility>

namespace tse
{
FALLBACK_DECL(exsCalendar)
FALLBACK_DECL(exscChangeFinderRefactor)

static Logger logger("atseintl.Rules.Reissue");

VoluntaryChanges::VoluntaryChanges(RexPricingTrx& trx, const Itin* itin, const PricingUnit* pu)
  : _trx(trx),
    _itin(itin),
    _pu(pu)
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
  _firstSegment = ExchShopCalendar::getFirstDateRange(trx);
  _lastSegment = ExchShopCalendar::getLastDateRange(trx);

  if (trx.getTrxType() == PricingTrx::RESHOP_TRX)
  {
    _isEXSCalendar = ExchShopCalendar::isEXSCalendar(trx);
    _r3DateValidationResults = std::make_unique<ExchShopCalendar::R3ValidationResult>(
        trx.orgDest, *itin, *trx.curNewItin(), trx.dataHandle());
  }
}

VoluntaryChanges::~VoluntaryChanges()
{
  if (UNLIKELY(_dc != nullptr))
  {
    _dc->flushMsg();
    _dc = nullptr;
  }
}

Record3ReturnTypes
VoluntaryChanges::validate(const FareUsage& fareUsage, const VoluntaryChangesInfo& vcRec3)
{
  // switchBaseToFBR;
  PaxTypeFare* correctPtf = fareUsage.cat25Fare()
                                ? const_cast<PaxTypeFare*>(fareUsage.cat25Fare())
                                : const_cast<PaxTypeFare*>(fareUsage.paxTypeFare());

  Record3ReturnTypes result = PASS;
  _isSoftPass = false;
  _failByPrevReissued = false;
  const Cat31Info* prevalidatedCat31Info = nullptr;

  FareCompInfo* fcInfo = correctPtf->fareMarket()->fareCompInfo();
  if (!fcInfo)
  {
    LOG4CXX_ERROR(
        logger,
        "VOLUNTARYCHANGES wrong fare used for reprice: " << fareUsage.paxTypeFare()->fareClass());
    return FAIL;
  }

  // only for exchange mip
  if (_trx.getTrxType() == PricingTrx::MIP_TRX && _trx.excTrxType() == PricingTrx::AR_EXC_TRX)
  {
    std::pair<bool, const Cat31Info*> result = isInPreselectedRec3(*correctPtf, vcRec3);
    prevalidatedCat31Info = result.second;
    if (!result.first)
    {
      if (_trx.isTestRequest())
      {
        LOG4CXX_ERROR(logger, "ITEM " << vcRec3.itemNo()
                                      << " - NOT IN PREVALIDATED DATA BUT IT IS TEST REQUEST\n");
      }
      else
      {
        if (UNLIKELY(_dc))
        {
          *_dc << "  FAILED ITEM " << vcRec3.itemNo() << " - WITH RANGE "
               << prevalidatedCat31Info->rec3DateRange << " NOT IN PREVALIDATED DATA\n";
          *_dc << "  FAILED ITEM: " << correctPtf->fareMarket()->fareCompInfo()->fareCompNumber()
               << " " << correctPtf->vendor() << " " << correctPtf->carrier() << " "
               << correctPtf->fareTariff() << " " << correctPtf->ruleNumber() << " "
               << correctPtf->retrievalDate().dateToString(DDMMMYY, "") << "\n";
          _dc->flushMsg();
        }
        return FAIL;
      }
    }
  }

  if (!matchR3(*correctPtf, vcRec3))
    result = FAIL;

  if (UNLIKELY(_dc))
  {
    *_dc << "ENDORSEMENT: " << vcRec3.endorsement() << '\n';
    _dc->flushMsg();
  }

  if (result != FAIL)
  {
    bool overridenData = shouldOverrideWithIntlFc(fcInfo, vcRec3);
    if (overridenData)
      _isSoftPass = true;
    if (UNLIKELY(_dc))
      _dc->flushMsg();
    if (_trx.getTrxType() == PricingTrx::RESHOP_TRX)
    {
      if (!getConstrainsFromT988(*correctPtf, vcRec3, overridenData))
        result = FAIL;
    }
    else
    {
      ReissueTable t988Validator(_trx, _itin, _pu);

      if (!matchT988(*correctPtf, vcRec3, overridenData, t988Validator, prevalidatedCat31Info))
        result = FAIL;
    }
  }

  if (result == PASS && _isSoftPass)
    result = SOFTPASS;

  updateFailByPrevReissued(*fcInfo, correctPtf);

  return result;
}

bool
VoluntaryChanges::matchR3(PaxTypeFare& paxTypeFare,
                          const VoluntaryChangesInfo& vcRec3)
{
  if (fallback::exsCalendar(&_trx))
  {
    const DateTime& applDate = paxTypeFare.fareMarket()->ruleApplicationDate();

    if (!chkNumOfReissue(*paxTypeFare.fareMarket(), vcRec3) || !matchWaiver(vcRec3, applDate) ||
        !matchPTC(paxTypeFare.carrier(), vcRec3) || !matchTicketValidity(vcRec3) ||
        !matchOverrideDateTable(
            vcRec3, applDate, _trx.curNewItin()->travelSeg().front()->departureDT()) ||
        !matchDeparture(*paxTypeFare.fareMarket(), vcRec3) ||
        !matchAdvanceReservation(*paxTypeFare.fareMarket(), vcRec3) ||
        !matchSameAirport(*paxTypeFare.fareMarket()->fareCompInfo(), vcRec3) ||
        !matchTktTimeLimit(vcRec3))
      return false;

    return true;
  }

  return validateWithoutTravelDate(paxTypeFare, vcRec3) &&
         validateUsingTravelDate(paxTypeFare, vcRec3);
}

bool
VoluntaryChanges::validateWithoutTravelDate(PaxTypeFare& paxTypeFare,
                                            const VoluntaryChangesInfo& vcRec3)
{
  const DateTime& applDate = paxTypeFare.fareMarket()->ruleApplicationDate();

  return (chkNumOfReissue(*paxTypeFare.fareMarket(), vcRec3) && matchWaiver(vcRec3, applDate) &&
          matchPTC(paxTypeFare.carrier(), vcRec3) && matchTicketValidity(vcRec3) &&
          matchDeparture(*paxTypeFare.fareMarket(), vcRec3) &&
          matchAdvanceReservation(*paxTypeFare.fareMarket(), vcRec3) &&
          matchSameAirport(*paxTypeFare.fareMarket()->fareCompInfo(), vcRec3) &&
          matchTktTimeLimit(vcRec3));
}

bool
VoluntaryChanges::validateUsingTravelDate(PaxTypeFare& paxTypeFare,
                                          const VoluntaryChangesInfo& vcRec3)
{
  bool result = false;
  const DateTime& applDate = paxTypeFare.fareMarket()->ruleApplicationDate();

  DateTime departureDate = _firstSegment.firstDate;
  for (; departureDate <= _firstSegment.lastDate; departureDate = departureDate.addDays(1))
  {
    if (matchOverrideDateTable(vcRec3, applDate, departureDate))
    {
      result = true;
      // TODO : add info to specific OND
    }
  }

  return result;
}

void
VoluntaryChanges::updateFailByPrevReissued(FareCompInfo& fcInfo, const PaxTypeFare* paxTypeFare)
{
  std::map<const PaxTypeFare*, bool>& failByPrevReissuedMap = fcInfo.failByPrevReissued();
  std::map<const PaxTypeFare*, bool>::iterator iter = failByPrevReissuedMap.find(paxTypeFare);
  if (iter != failByPrevReissuedMap.end())
  {
    if (!_failByPrevReissued)
      iter->second = false;
  }
  else
  {
    failByPrevReissuedMap.insert(std::make_pair(paxTypeFare, _failByPrevReissued));
  }
}

namespace
{
static const std::string
label("EXCHANGE");
}

bool
VoluntaryChanges::matchWaiver(const VoluntaryChangesInfo& vcRec3, const DateTime& applDate)
{
  WaiverCodeValidator val(_trx.dataHandle(), _dc, logger, label);
  bool result = val.validate(
      vcRec3.itemNo(), vcRec3.vendor(), vcRec3.waiverTblItemNo(), applDate, _trx.waiverCode());

  // Store the Record 3's which are waived
  if (result && vcRec3.waiverTblItemNo())
    _trx.waivedChangeFeeRecord3().insert(&vcRec3);
  return result;
}

bool
VoluntaryChanges::matchPTC(const CarrierCode& carrier, const VoluntaryChangesInfo& vcRec3)
{
  PaxTypeCodeValidator val(_dc, label);
  return val.validate(vcRec3.itemNo(), _trx.getExchangePaxType(), vcRec3.psgType());
}

class IsTravelAfterOnDate
{
public:
  IsTravelAfterOnDate(const DateTime& date) : _date(date) {}

  bool operator()(const TravelSeg* travelSeg)
  {
    if (travelSeg != nullptr)
      return (travelSeg->departureDT() >= _date);

    return false;
  }

protected:
  const DateTime& _date;
};

bool
VoluntaryChanges::matchTicketValidity(const VoluntaryChangesInfo& vcRec3)
{
  bool result = false;

  if (UNLIKELY(_dc))
  {
    *_dc << "TKT VALIDITY: " << vcRec3.tktValidityInd() << "\n";
  }

  if (vcRec3.tktValidityInd() == TICKET_VALIDITY_DATE_CHECK_REQUIRED)
  {
    const Itin& newItin = *_trx.curNewItin();
    const ExcItin& exchangeItin = *_trx.exchangeItin().front();

    if (UNLIKELY(_dc))
    {
      *_dc << "  ORIGINAL TICKET ISSUE DATE: "
           << _trx.originalTktIssueDT().dateToString(YYYYMMDD, "-") << "\n";
      if (_trx.applyReissueExchange() && !_trx.previousExchangeDT().isEmptyDate())
        *_dc << "      PREVIOUS EXCHANGE DATE: "
             << _trx.previousExchangeDT().dateToString(YYYYMMDD, "-") << "\n";
      *_dc << "   EXCHANGE ITIN TRAVEL DATE: "
           << exchangeItin.travelSeg().front()->departureDT().dateToString(YYYYMMDD, "-") << "\n";
      *_dc << "        TICKET VALIDITY DATE: "
           << exchangeItin.tktValidityDate().dateToString(YYYYMMDD, "-") << "\n";
    }

    if(fallback::exsCalendar(&_trx))
    {
      auto found = std::find_if(newItin.travelSeg().begin(),
                                newItin.travelSeg().end(),
                                IsTravelAfterOnDate(exchangeItin.tktValidityDate()));

      result = (found == newItin.travelSeg().end());
      if (!result && _dc)
        *_dc << "     FAILED ITEM " << vcRec3.itemNo() << " - TICKET VALIDITY DATE NOT MET\n";
    }
    else
    {
      DateTime depDate = _lastSegment.firstDate;
      for (; depDate <= _lastSegment.lastDate; depDate = depDate.addDays(1))
      {
        bool matched = (depDate < exchangeItin.tktValidityDate());
        if(UNLIKELY(_dc))
        {
          *_dc << "    LAST SEGMENT DEPARTURE DATE: "
                 << depDate.dateToString(YYYYMMDD, "-") << "\n";

          if (!matched)
            *_dc << "     FAILED ITEM " << vcRec3.itemNo() << " - TICKET VALIDITY DATE NOT MET\n";
          else
            *_dc << "     PASSED\n";
        }
        result = result || matched;

        if (!matched)
          break;
      }

      if (_isEXSCalendar)
      {
        uint32_t lastONDindex = _trx.orgDest.size() - 1;
        ExchShopCalendar::DateRange validityPeriod = {_lastSegment.firstDate,
                                                      depDate.subtractDays(1)};
        _r3DateValidationResults->addDateRange(validityPeriod, lastONDindex);
      }

    }
  }
  else
  {
    result = true;
  }

  if (UNLIKELY(_dc))
  {
    *_dc << std::endl;
  }
  return result;
}

bool
VoluntaryChanges::matchT988(PaxTypeFare& ptf,
                            const VoluntaryChangesInfo& vcRec3,
                            bool overridenData,
                            ReissueTable& t988Validator,
                            const Cat31Info* prevalidatedCat31Info)
{
  const DateTime& ruleApplicationDate = ptf.fareMarket()->ruleApplicationDate();
  const std::vector<ReissueSequence*>& t988Seqs =
      t988Validator.getMatchedT988Seqs(*ptf.fareMarket(),
                                       vcRec3,
                                       overridenData,
                                       _skippedValidations,
                                       ruleApplicationDate,
                                       prevalidatedCat31Info);

  bool foundMatchedSeqs = !t988Seqs.empty();

  if (foundMatchedSeqs)
  {
    _trx.reissueOptions().insertOption(&ptf, &vcRec3);

    for (const auto t988Seq : t988Seqs)
    {
      _trx.reissueOptions().insertOption(&ptf, &vcRec3, t988Seq);
    }
  }
  return foundMatchedSeqs;
}

bool
VoluntaryChanges::matchOverrideDateTable(const VoluntaryChangesInfo& vcRec3,
                                         const DateTime& applDate,
                                         const DateTime& newTravelDate)
{
  if (UNLIKELY(_dc))
  {
    *_dc << "TBL 994 ITEM: " << vcRec3.overrideDateTblItemNo() << " " << vcRec3.vendor()
         << std::endl;
  }

  if (vcRec3.overrideDateTblItemNo() == 0)
    return true;

  const std::vector<DateOverrideRuleItem*>& dorItemList = _trx.dataHandle().getDateOverrideRuleItem(
      vcRec3.vendor(), vcRec3.overrideDateTblItemNo(), applDate);

  if (dorItemList.empty())
    return true;

  DateTime newItinTicketingDate = _trx.currentTicketingDT();
  if (UNLIKELY(_dc))
  {
    *_dc << "NEW ITIN TRAVEL DATE: " << newTravelDate.dateToString(YYYYMMDD, "-") << std::endl;
    *_dc << "NEW ITIN TICKET DATE: " << newItinTicketingDate.dateToString(YYYYMMDD, "-")
         << std::endl;
  }

  std::vector<DateOverrideRuleItem*>::const_iterator i = dorItemList.begin();
  for (; i != dorItemList.end(); ++i)
  {
    DateOverrideRuleItem& dorItem = **i;
    if (matchOverrideDateRule(dorItem, newTravelDate, newItinTicketingDate))
      return true;
  }

  if (UNLIKELY(_dc))
    *_dc << "  FAILED ITEM " << vcRec3.itemNo() << " - OVERRIDE DATES NOT MET\n";

  return false;
}

bool
VoluntaryChanges::matchOverrideDateRule(const DateOverrideRuleItem& dorItem,
                                        const DateTime& travelDate,
                                        const DateTime& ticketingDate)
{
  if (UNLIKELY(_dc))
  {
    _dc->setf(std::ios::left, std::ios::adjustfield);

    *_dc << "    TVL EFF DTE  - " << std::setw(10)
         << dorItem.tvlEffDate().dateToString(YYYYMMDD, "-") << "  TVL DISC DTE - " << std::setw(10)
         << dorItem.tvlDiscDate().dateToString(YYYYMMDD, "-") << std::endl;
    *_dc << "    TKT EFF DTE  - " << std::setw(10)
         << dorItem.tktEffDate().dateToString(YYYYMMDD, "-") << "  TKT DISC DTE - " << std::setw(10)
         << dorItem.tktDiscDate().dateToString(YYYYMMDD, "-") << std::endl;
  }

  if ((!dorItem.tvlEffDate().isValid() || (dorItem.tvlEffDate().date() <= travelDate.date())) &&
      (!dorItem.tvlDiscDate().isValid() || (dorItem.tvlDiscDate().date() >= travelDate.date())) &&
      (!dorItem.tktEffDate().isValid() || (dorItem.tktEffDate().date() <= ticketingDate.date())) &&
      (!dorItem.tktDiscDate().isValid() || (dorItem.tktDiscDate().date() >= ticketingDate.date())))
  {
    return true;
  }

  return false;
}

bool
VoluntaryChanges::matchDeparture(const FareMarket& fareMarket, const VoluntaryChangesInfo& vcRec3)
{
  DepartureValidator val(_itin, &fareMarket, _pu, _dc);
  return val.validate(vcRec3.itemNo(),
                      vcRec3.priceableUnitInd(),
                      vcRec3.departureInd(),
                      vcRec3.fareComponentInd(),
                      _isSoftPass);
}

namespace
{

class DifferentCarrier : std::unary_function<const FareCompInfo*, bool>
{
  const CarrierCode& _carrier;

public:
  DifferentCarrier(const FareCompInfo& fci) : _carrier(fci.fareMarket()->governingCarrier()) {}

  bool operator()(const FareCompInfo* fci) const
  {
    return fci->fareMarket()->governingCarrier() != _carrier;
  }
};

class UniqueSegMatched : std::unary_function<const TravelSeg*, bool>
{
  const TravelSeg& _oldSeg;

public:
  UniqueSegMatched(const TravelSeg& oldSeg) : _oldSeg(oldSeg) {}

  bool operator()(const TravelSeg* newSeg) const
  {
    if (newSeg->boardMultiCity() != _oldSeg.boardMultiCity() ||
        newSeg->offMultiCity() != _oldSeg.offMultiCity() ||
        newSeg->pssDepartureDate() != _oldSeg.pssDepartureDate())
      return false;

    const AirSeg* newAs = dynamic_cast<const AirSeg*>(newSeg);
    const AirSeg* excAs = dynamic_cast<const AirSeg*>(&_oldSeg);
    if (!newAs && !excAs)
      return true;

    if ((newAs && !excAs) || (!newAs && excAs))
      return false;

    if (newAs->carrier() != excAs->carrier() || newAs->flightNumber() != excAs->flightNumber())
      return false;

    return true;
  }
};

class ChangeFinderOld : public AbstractChangeFinder, std::unary_function<const TravelSeg*, bool>
{
public:
  ChangeFinderOld(const std::vector<TravelSeg*>& newTravelSegs) : _newTravelSegs(newTravelSegs) {}

  bool operator()(const TravelSeg* ts) const
  {
    if (ts->changeStatus() == TravelSeg::CHANGED)
    {
      return std::find_if(_newTravelSegs.begin(), _newTravelSegs.end(), UniqueSegMatched(*ts)) ==
             _newTravelSegs.end();
    }

    return false;
  }

  bool notChanged(const std::vector<TravelSeg*>& travelSegs)
  {
    return std::find_if(travelSegs.begin(), travelSegs.end(), *this) == travelSegs.end();
  }

private:
  const std::vector<TravelSeg*>& _newTravelSegs;
};
}

bool
VoluntaryChanges::oneCarrierTicket()
{
  const std::vector<FareCompInfo*>& allFcs = *getAllFc();

  return std::find_if(++allFcs.begin(), allFcs.end(), DifferentCarrier(*allFcs.front())) ==
         allFcs.end();
}

bool
VoluntaryChanges::segsNotChanged(FareMarket& fareMarket, const VoluntaryChangesInfo& vcRec3)
{
  std::unique_ptr<AbstractChangeFinder> finder;
  if (fallback::exscChangeFinderRefactor(&_trx))
    finder.reset(new ChangeFinderOld(_trx.curNewItin()->travelSeg()));
  else
    finder.reset(new Cat31ChangeFinder(_trx.curNewItin()->travelSeg(), _trx.orgDest,
                                       _r3DateValidationResults.get(), _isEXSCalendar, _trx));

  switch (vcRec3.changeInd())
  {
  case NOT_PERMITTED:
    return finder->notChanged(fareMarket.travelSeg());

  case CHG_IND_P:
    return finder->notChanged(_pu->travelSeg());

  case CHG_IND_J:
    if (oneCarrierTicket())
      return finder->notChanged(_itin->travelSeg());
    return finder->notChanged(_pu->travelSeg());
  }

  // '1' to '9' would be treated the same for now, as reissue
  // permitted only once
  _failByPrevReissued =
      !_trx.lastTktReIssueDT().isEmptyDate() || !_trx.previousExchangeDT().isEmptyDate();

  return !_failByPrevReissued;
}

bool
VoluntaryChanges::chkNumOfReissue(FareMarket& fareMarket, const VoluntaryChangesInfo& vcRec3)
{
  if (UNLIKELY(_dc))
  {
    *_dc << "CHANGE INDICATOR: " << vcRec3.changeInd() << "\n";
    if (!_trx.lastTktReIssueDT().isEmptyDate())
    {
      *_dc << "LAST REISSUE DATE: " << _trx.lastTktReIssueDT().dateToString(YYYYMMDD, "-") << "\n";
    }
  }

  if (vcRec3.changeInd() == NOT_APPLY)
    return true;

  bool result = segsNotChanged(fareMarket, vcRec3);

  if (UNLIKELY(!result && _dc))
  {
    *_dc << "  FAILED ITEM " << vcRec3.itemNo() << " - CHANGE INDICATOR\n";
  }

  return result;
}

bool
VoluntaryChanges::matchAdvanceReservation(const FareMarket& fareMarket,
                                          const VoluntaryChangesInfo& vcRec3)
{
  DateTime fromDT;
  DateTime toDT;

  switch (vcRec3.advResFrom())
  {
  case NOT_APPLY:
    return true;
  case ADVRSVN_ORIG_TKT_DT:
    fromDT = _trx.originalTktIssueDT();
    break;
  case ADVRSVN_REISSUE_DT:
    fromDT = _trx.currentTicketingDT();
    break;
  default:
    LOG4CXX_ERROR(logger, "VOLUNTARYCHANGES item No. " << vcRec3.itemNo() << " incorrect byte 18");
    if (UNLIKELY(_dc))
      *_dc << "ERROR: VOLUNTARYCHANGES R3 ITEM NO " << vcRec3.itemNo() << " INCORRECT BYTE 18\n";
    return false;
  }

  switch (vcRec3.advResTo())
  {
  case ADVRSVN_JOURNEY:
    toDT = _itin->travelDate();
    break;
  case ADVRSVN_PRICING_UNIT:
    if (!_pu)
    {
      if (UNLIKELY(_dc))
      {
        *_dc << "  NO PU INFO: SOFTPASS\n";
      }

      _isSoftPass = true;
      return true;
    }
    toDT = _pu->travelSeg().front()->departureDT();
    break;
  case ADVRSVN_FARE_COMPONENT:
    toDT = fareMarket.travelSeg().front()->departureDT();
    break;
  default:
    LOG4CXX_ERROR(logger, "VOLUNTARYCHANGES item No. " << vcRec3.itemNo() << " incorrect byte 19");
    if (UNLIKELY(_dc))
      *_dc << "ERROR: VOLUNTARYCHANGES R3 ITEM NO " << vcRec3.itemNo() << " INCORRECT BYTE 19\n";
    return false;
  }

  if (UNLIKELY(_dc))
  {
    std::ostringstream lastTOD;
    int16_t lTOD = vcRec3.advResLastTOD();
    if (lTOD >= 0)
      lastTOD << (lTOD / 60) << ":" << (lTOD % 60);
    *_dc << "\nADV RSVN FROM PT: " << vcRec3.advResFrom() << ", TO PT: " << vcRec3.advResTo()
         << "\nADV RSVN PERIOD: " << vcRec3.advResPeriod() << ", UNIT: " << vcRec3.advResUnit()
         << "\nADV RSVN LAST TOD: " << lastTOD.str() << std::endl;
  }

  DateTime advResLimit;

  if (!AdvanceResTkt::getLimitDateTime(advResLimit,
                                       toDT,
                                       vcRec3.advResLastTOD(),
                                       vcRec3.advResPeriod(),
                                       vcRec3.advResUnit(),
                                       AdvanceResTkt::BEFORE_REF_TIME))
  {
    if (UNLIKELY(_dc))
      *_dc << "WRONG DATA FOR ADV RES VALIDATION\n";
    return false;
  }
  if (UNLIKELY(_dc))
  {
    *_dc << "          TKT DATE/TIME: " << fromDT.toIsoExtendedString() << std::endl
         << "        LIMIT DATE/TIME: " << advResLimit.toIsoExtendedString() << std::endl;
  }
  bool result = fromDT < advResLimit;

  if (UNLIKELY(!result && _dc))
    *_dc << "  FAILED ITEM " << vcRec3.itemNo() << " - ADV RSVN\n";

  return result;
}

bool
VoluntaryChanges::matchTktTimeLimit(const VoluntaryChangesInfo& vcRec3)
{
  bool matched = true;
  DateTime& ticketDT = _trx.currentTicketingDT();

  const Indicator tktTimeLimitInd = vcRec3.tktTimeLimit();

  if (UNLIKELY(_dc))
    *_dc << "\nTICKETING TIME LIMIT INDICATOR: " << vcRec3.tktTimeLimit() << "\n";

  if (vcRec3.tktTimeLimit() == NOT_APPLY)
    return true;

  if (UNLIKELY(_dc))
    *_dc << " REISSUE DATE: " << ticketDT.toIsoExtendedString();

  if (tktTimeLimitInd == VoluntaryChanges::BEFORE || tktTimeLimitInd == VoluntaryChanges::AFTER)
    matched = matchJourneyTktTimeLimit(tktTimeLimitInd, ticketDT);
  else if (tktTimeLimitInd == VoluntaryChanges::BEFORE_PU)
    matched = matchPUTktTimeLimit(VoluntaryChanges::BEFORE, _pu, ticketDT);
  else if (tktTimeLimitInd == VoluntaryChanges::AFTER_PU)
    matched = matchPUTktTimeLimit(VoluntaryChanges::AFTER, _pu, ticketDT);

  if (UNLIKELY(_dc))
  {
    if (!matched)
      *_dc << "\n  FAILED ITEM " << vcRec3.itemNo() << " - TICKETING TIME LIMIT";
    *_dc << "\n";
  }

  return matched;
}

bool
VoluntaryChanges::matchJourneyTktTimeLimit(const Indicator tktTimeLimitInd,
                                           const DateTime& ticketDT)
{
  bool matched = true;
  if (!_trx.exchangeItin().empty() && !_trx.exchangeItin()[0]->farePath().empty())
  {
    FarePath& farePath = *(_trx.exchangeItin()[0]->farePath()[0]);
    if (!farePath.pricingUnit().empty())
    {
      std::vector<PricingUnit*>::const_iterator pui = farePath.pricingUnit().begin();
      std::vector<PricingUnit*>::const_iterator pue = farePath.pricingUnit().end();

      for (; pui != pue; pui++)
      {
        matched = matchPUTktTimeLimit(tktTimeLimitInd, *pui, ticketDT);

        if (tktTimeLimitInd == VoluntaryChanges::BEFORE && matched == false)
          return false;
        if (tktTimeLimitInd == VoluntaryChanges::AFTER && matched == true)
          return true;
      }
    }
  }

  return matched;
}

bool
VoluntaryChanges::matchPUTktTimeLimit(const Indicator beforeAfter,
                                      const PricingUnit* pu,
                                      const DateTime& ticketDT)
{
  bool matched = true;
  if (!pu)
  {
    if (UNLIKELY(_dc))
    {
      *_dc << "\n  NO PU INFO: SOFTPASS";
    }

    _isSoftPass = true;
    return true;
  }

  if (!pu->latestTktDT().isValid())
  {
    if (UNLIKELY(_dc))
      *_dc << "\n  TICKETING TIME LIMIT DATE: N/A";
    return true;
  }

  if (UNLIKELY(_dc))
  {
    *_dc << "\n  TICKETING TIME LIMIT DATE: " << pu->latestTktDT().toIsoExtendedString();
  }

  if (beforeAfter == VoluntaryChanges::BEFORE)
    matched = (ticketDT <= pu->latestTktDT());
  else if (beforeAfter == VoluntaryChanges::AFTER)
    matched = (ticketDT > pu->latestTktDT());

  if (UNLIKELY(!matched && _dc))
  {
    if (beforeAfter == VoluntaryChanges::BEFORE)
      *_dc << "\n  REISSUE DATE IS NOT ON OR BEFORE TICKETING TIME LIMIT";
    else if (beforeAfter == VoluntaryChanges::AFTER)
      *_dc << "\n  REISSUE DATE IS NOT AFTER TICKETING TIME LIMIT";
  }

  return matched;
}

bool
VoluntaryChanges::shouldOverrideWithIntlFc(FareCompInfo* fc, const VoluntaryChangesInfo& vcRec3)
{
  if (!vcRec3.reissueTblItemNo())
    return false;

  std::vector<FareCompInfo*>* allFcs = getAllFc();

  if (!isInternationalItin() || !fc || !allFcs || allFcs->size() <= 1 ||
      fc->fareMarket()->geoTravelType() == GeoTravelType::International)
  {
    return false;
  }

  if (UNLIKELY(_dc))
  {
    *_dc << "DOMESTICINTLCOMB IND: " << vcRec3.domesticIntlComb() << std::endl;
    *_dc << "CARRIERAPPLTBL ITEM NO: " << vcRec3.carrierApplTblItemNo() << std::endl;
  }

  if (vcRec3.domesticIntlComb() == DOMINTL_COMB_NOT_APPLY)
  {
    if (UNLIKELY(_dc))
      *_dc << "OVERRIDING INTL FC: NONE ELIGIBLE" << std::endl;
    return false;
  }

  const CarrierCode& domCarrier = fc->fareMarket()->governingCarrier();
  std::vector<CarrierApplicationInfo*> carrierLst;
  bool anyCarrier(!vcRec3.carrierApplTblItemNo()),
      domIntlCombApply(vcRec3.domesticIntlComb() == DOMINTL_COMB_APPLY);
  const DateTime& applDate = fc->fareMarket()->ruleApplicationDate();
  if (domIntlCombApply && !anyCarrier)
    carrierLst = getCarrierApplication(vcRec3.vendor(), vcRec3.carrierApplTblItemNo(), applDate);

  std::vector<FareCompInfo*>::const_iterator bI = allFcs->begin(), eI = allFcs->end();
  std::vector<FareCompInfo*>::const_iterator rightI = bI + (fc->fareCompNumber() - 1);
  std::vector<FareCompInfo*>::const_reverse_iterator reI = allFcs->rend(), leftI(rightI);
  leftI--;
  bool leftValid, rightValid;

  while (leftI != reI || rightI != eI)
  {
    leftValid = rightValid = false;


    if (leftI != reI && ++leftI != reI &&
        validateOverridenIntlFc(**leftI, domCarrier, domIntlCombApply, anyCarrier, carrierLst))
    {
      leftValid = true;
    }

    if (!leftValid && rightI != eI && ++rightI != eI &&
        validateOverridenIntlFc(**rightI, domCarrier, domIntlCombApply, anyCarrier, carrierLst))
    {
      rightValid = true;
    }

    if (leftValid || rightValid)
    {
      storeOverridenIntlFc(*fc, leftValid ? **leftI : **rightI, vcRec3);
      return true;
    }
  }

  if (UNLIKELY(_dc))
    *_dc << "OVERRIDING INTL FC: NONE ELIGIBLE" << std::endl;
  return false;
}

bool
VoluntaryChanges::validateOverridenIntlFc(const FareCompInfo& fc,
                                          const CarrierCode& domCarrier,
                                          bool domIntlCombApply,
                                          bool anyCarrier,
                                          std::vector<CarrierApplicationInfo*>& carrierLst)
{
  if (fc.fareMarket()->geoTravelType() != GeoTravelType::International)
    return false;

  const CarrierCode& intCarrier = fc.fareMarket()->governingCarrier();
  if (intCarrier == domCarrier)
    return true;

  if (domIntlCombApply)
    return anyCarrier ? true : validateIntlCarrierWithCarrApplData(intCarrier, carrierLst);

  return false;
}

void
VoluntaryChanges::storeOverridenIntlFc(FareCompInfo& domFc,
                                       FareCompInfo& intlFc,
                                       const VoluntaryChangesInfo& vcRec3)
{
  domFc.overridingFcs()[&vcRec3] =
      std::make_pair(intlFc.fareCompNumber(), FareCompInfo::SkippedValidationsSet());
  _skippedValidations = domFc.getSkippedValidationsSet(domFc.findOverridingData(&vcRec3));

  if (UNLIKELY(_dc))
  {
    *_dc << "OVERRIDING INTL FC: FC" << intlFc.fareCompNumber() << " "
         << intlFc.fareMarket()->boardMultiCity() << intlFc.fareMarket()->offMultiCity()
         << std::endl;
  }
}

bool
VoluntaryChanges::validateIntlCarrierWithCarrApplData(
    const CarrierCode& carrier, std::vector<CarrierApplicationInfo*>& carrierLst)
{
  std::vector<CarrierApplicationInfo*>::const_iterator curI = carrierLst.begin();
  std::vector<CarrierApplicationInfo*>::const_iterator eI = carrierLst.end();
  bool isValid(false);

  for (; curI != eI; curI++)
  {
    if ((*curI)->carrier() == DOLLAR_CARRIER)
      isValid = true;
    else if ((*curI)->carrier() == carrier)
    {
      isValid = (*curI)->applInd() == NOT_APPLY ? true : false;
      break;
    }
  }

  return isValid;
}

const std::vector<CarrierApplicationInfo*>&
VoluntaryChanges::getCarrierApplication(const VendorCode& vendor,
                                        int itemNo,
                                        const DateTime& applDate)
{
  return _trx.dataHandle().getCarrierApplication(vendor, itemNo, applDate);
}

bool
VoluntaryChanges::isInternationalItin()
{
  return _trx.exchangeItin().front()->geoTravelType() == GeoTravelType::International;
}

std::vector<FareCompInfo*>*
VoluntaryChanges::getAllFc()
{
  return (_trx.exchangeItin().empty() || !_trx.exchangeItin().front())
             ? nullptr
             : &_trx.exchangeItin().front()->fareComponent();
}
struct IsUnflownChanged // : public std::unary_function<const TravelSeg*, bool>
{
  bool operator()(const TravelSeg* seg) const
  {
    return seg->unflown() && seg->segmentType() == Air && seg->changeStatus() == TravelSeg::CHANGED;
  }
};

struct IsChanged //: public std::unary_function<const TravelSeg*, bool>
{
  bool operator()(const TravelSeg* seg) const { return seg->changeStatus() == TravelSeg::CHANGED; }
};

struct IsCarrierChange //  : public std::unary_function<const TravelSeg*, bool>
{
  CarrierCode _carrier;
  IsCarrierChange(const CarrierCode& carrier) : _carrier(carrier) {}
  bool operator()(const TravelSeg* seg) const
  {
    if (seg->segmentType() != Air)
      return true; // treat surface part as change of carriers
    const AirSeg* airSeg = static_cast<const AirSeg*>(seg);
    return (_carrier != airSeg->carrier());
  }
};

bool
VoluntaryChanges::matchSameAirport(FareCompInfo& fareCompInfo, const VoluntaryChangesInfo& vcRec3)
{
  LOG4CXX_DEBUG(logger, "Same Airport indicator " << vcRec3.sameAirport());

  if (UNLIKELY(_dc))
  {
    *_dc << "SAME AIRPORT INDICATOR: " << vcRec3.sameAirport() << "\n";
  }
  const FareMarket* fareMarket = fareCompInfo.fareMarket();
  if (vcRec3.sameAirport() == BLANK || fareMarket->geoTravelType() == GeoTravelType::International)
  {
    if (UNLIKELY(_dc))
      *_dc << "SAME AIRPORT INDICATOR: PASSED\n";
    return true;
  }
  // Byte 93 = 'X' and travel within single country
  if (fareCompInfo.sameAirportResult() == SAME_AIRPORT_PASS)
    return true;
  if (fareCompInfo.sameAirportResult() == SAME_AIRPORT_FAIL)
    return false;

  const std::vector<TravelSeg*>& segs = fareMarket->travelSeg();

  const std::vector<TravelSeg*>::const_iterator firstUnflownChangedTS =
      std::find_if(segs.begin(), segs.end(), IsUnflownChanged());
  if (firstUnflownChangedTS == segs.end()) // Fare Component fully flown or unchanged
  {
    if (UNLIKELY(_dc))
      *_dc << "SAME AIRPORT INDICATOR: PASSED\n";
    fareCompInfo.sameAirportResult() = SAME_AIRPORT_PASS;
    return true;
  }
  RexPricingRequest* rexPricingRequest = static_cast<RexPricingRequest*>(_trx.getRequest());
  Agent* agent = rexPricingRequest->ticketingAgent();

  if ((*firstUnflownChangedTS)->boardMultiCity() != // A01 Departure Airport
      agent->agentCity()) // A10
  {
    if (UNLIKELY(_dc))
      *_dc << "SAME AIRPORT INDICATOR: FAILED - ORIGIN " << agent->agentCity() << "/DPTR "
           << (*firstUnflownChangedTS)->boardMultiCity() << "\n";
    fareCompInfo.sameAirportResult() = SAME_AIRPORT_FAIL;
    return false;
  }

  // else departure airport is same as agent city
  // find last changed travel segment
  const std::vector<TravelSeg*>::const_reverse_iterator rLastChangedTS =
      std::find_if(segs.rbegin(), segs.rend(), IsChanged());
  if (*rLastChangedTS == *firstUnflownChangedTS) // only one changed
  {
    if (UNLIKELY(_dc))
      *_dc << "SAME AIRPORT INDICATOR: PASSED\n";
    fareCompInfo.sameAirportResult() = SAME_AIRPORT_PASS;
    return true;
  }

  // check for stopovers and carrier change between first and last revised travel segment
  const std::vector<TravelSeg*>::const_iterator lastChangedTS = rLastChangedTS.base() - 1;
  const AirSeg* airSeg = static_cast<const AirSeg*>(*firstUnflownChangedTS);
  const std::vector<TravelSeg*>::const_iterator stopOverTS =
      std::find_if(firstUnflownChangedTS,
                   lastChangedTS,
                   [](const TravelSeg* seg)
                   { return seg->stopOver(); });
  if (stopOverTS != lastChangedTS) // stopover found
  {
    if (UNLIKELY(_dc))
      *_dc << "SAME AIRPORT INDICATOR: FAILED - STOPOVER NOT ALLOWED\n";
    fareCompInfo.sameAirportResult() = SAME_AIRPORT_FAIL;
    return false;
  }
  const std::vector<TravelSeg*>::const_iterator carrierChangeTS = std::find_if(
      firstUnflownChangedTS + 1, lastChangedTS + 1, IsCarrierChange(airSeg->carrier()));
  if (carrierChangeTS != lastChangedTS + 1) // carrier change found
  {
    if (UNLIKELY(_dc))
      *_dc << "SAME AIRPORT INDICATOR: FAILED - CARRIER CHANGE NOT ALLOWED\n";
    fareCompInfo.sameAirportResult() = SAME_AIRPORT_FAIL;
    return false;
  }

  if (UNLIKELY(_dc))
    *_dc << "SAME AIRPORT INDICATOR: PASSED\n";
  fareCompInfo.sameAirportResult() = SAME_AIRPORT_PASS;
  return true;
}

bool
VoluntaryChanges::getConstrainsFromT988(PaxTypeFare& ptf,
                                        const VoluntaryChangesInfo& vcRec3,
                                        bool overridenData)
{
  const DateTime& ruleApplicationDate = ptf.fareMarket()->ruleApplicationDate();

  return Tab988Merger(_trx, _itin, _pu).merge(
      ptf, vcRec3, overridenData, ruleApplicationDate, _r3DateValidationResults.get());
}

std::pair<bool, const Cat31Info*>
VoluntaryChanges::isInPreselectedRec3(const PaxTypeFare& paxTypeFare,
                                      const VoluntaryChangesInfo& vcRec3) const
{
  const uint32_t& itemNo = vcRec3.itemNo();
  const uint16_t& fareCompNumber = paxTypeFare.fareMarket()->fareCompInfo()->fareCompNumber();

  for (const auto* fcInfo : _trx.excFareCompInfo())
  {
    if (fareCompNumber == fcInfo->fareCompNumber())
    {
      for (const auto* cat31Info : fcInfo->cat31Info())
      {
        if (cat31Info->rec3ItemNo == itemNo)
        {
          if (!fallback::exsCalendar(&_trx) && ExchShopCalendar::isEXSCalendar(_trx))
          {
            bool result = std::any_of(_trx.orgDest.cbegin(),
                                      _trx.orgDest.cend(),
                                      [&](const PricingTrx::OriginDestination& ond)
                                      { return cat31Info->rec3DateRange.isInDateRange(ond.travelDate); });
            return std::make_pair(result, cat31Info);
          }
          return std::make_pair(true, cat31Info);
        }
      }
    }
  }

  return std::make_pair(false, nullptr);
}

} //tse
