#include "Rules/VoluntaryRefunds.h"

#include "Common/FlownStatusCheck.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/ExcItin.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RexBaseRequest.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"
#include "DBAccess/VoluntaryRefundsInfo.h"
#include "Diagnostic/DiagCollector.h"
#include "Rules/AdvanceReservationValidator.h"
#include "Rules/CarrierApplicationValidator.h"
#include "Rules/DepartureValidator.h"
#include "Rules/OriginallyScheduledFlightValidator.h"
#include "Rules/PaxTypeCodeValidator.h"
#include "Rules/PeriodOfStay.h"
#include "Rules/RuleUtil.h"
#include "Rules/WaiverCodeValidator.h"
#include "Util/BranchPrediction.h"

namespace tse
{

static Logger
logger("atseintl.Rules.Refund");

VoluntaryRefunds::VoluntaryRefunds(RefundPricingTrx& trx,
                                   const Itin* itin,
                                   const PricingUnit* pu,
                                   PaxTypeFare& ptf,
                                   const VoluntaryRefundsInfo& record3,
                                   DiagCollector* dc)
  : _trx(trx), _itin(itin), _pu(pu), _paxTypeFare(ptf), _record3(record3), _softPass(false), _dc(dc)
{
}

Record3ReturnTypes
VoluntaryRefunds::validate()
{
  _trx.reachedR3Validation().insert(&_paxTypeFare);

  if (matchWaiver() && matchPTC() && matchTicketValidity() && matchDeparture() &&
      matchAdvCancelation() && matchCustomer1() && matchFullyFlown() && matchOrigSchedFlt() &&
      matchCxrApplTbl() && matchOverrideDateTable())
  {
    _trx.insertOption(&_paxTypeFare, &_record3);
    return _softPass ? SOFTPASS : PASS;
  }

  return FAIL;
}

void
VoluntaryRefunds::printDates(const std::string& title,
                             unsigned int leftPad,
                             std::list<std::pair<std::string, const DateTime*> >& fields) const
{
  *_dc << title << std::endl;
  _dc->setf(std::ios::left, std::ios::adjustfield);
  std::list<std::pair<std::string, const DateTime*> >::const_iterator it = fields.begin();
  for (; it != fields.end(); ++it)
  {
    *_dc << " " << std::setw(leftPad) << it->first << " : " << it->second->toIsoExtendedString()
         << std::endl;
  }
}

void
VoluntaryRefunds::printTicketValidity(const std::string& tktValidity,
                                      const DateTime& tktValidityDate,
                                      const DateTime& adjustedRefundDT) const
{
  if (UNLIKELY(_dc))
  {
    std::string title = "TKT VALIDITY: " + tktValidity;
    if (tktValidity != "N/A")
      title += (_record3.tktValidityInd() == MATCH_COMMENCE_DATE && !_trx.fullRefund())
                   ? " AFTER COMMENCE DATE"
                   : " AFTER TICKETING DATE";

    std::list<std::pair<std::string, const DateTime*> > fields;
    fields.push_back(
        std::make_pair("ORIGINAL TICKET TRAVEL DATE", &_trx.exchangeItin().front()->travelDate()));
    printOriginalAndRefundDT(adjustedRefundDT, fields);
    fields.push_back(std::make_pair("TICKET VALIDITY DATE", &tktValidityDate));
    printDates(title, 27, fields);
  }
}

void
VoluntaryRefunds::printOriginalAndRefundDT(
    const DateTime& adjustedRefundDT, std::list<std::pair<std::string, const DateTime*> >& fields)
    const
{
  fields.push_front(std::make_pair("ORIGINAL TICKET ISSUE DATE", &_trx.originalTktIssueDT()));
  fields.push_back(std::make_pair("REFUND DATE", &_trx.currentTicketingDT()));

  if (_trx.currentTicketingDT() != adjustedRefundDT)
    fields.push_back(std::make_pair("ADJUSTED REFUND DATE", &adjustedRefundDT));
}

void
VoluntaryRefunds::printCustomer1(const std::string& tktValidity,
                                 const DateTime& tktValidityDate,
                                 const DateTime& adjustedRefundDT) const
{
  if (UNLIKELY(_dc))
  {
    std::string title = "CUSTOMER 1ST VALIDITY: " + tktValidity;
    if (tktValidity != "N/A")
      title += (_record3.customer1ResTkt() == MATCH_EARLIEST_RESERVATIONS_DATE)
                   ? " AFTER EARLIEST RESERVATION DATE"
                   : " AFTER TICKETING DATE";

    std::list<std::pair<std::string, const DateTime*> > fields;

    fields.push_back(
        std::make_pair("EARLIEST RESERVATION DATE",
                       &_trx.exchangeItin().front()->travelSeg().front()->bookingDT()));

    printOriginalAndRefundDT(adjustedRefundDT, fields);
    fields.push_back(std::make_pair("CUSTOMER 1ST VALIDITY DATE", &tktValidityDate));
    printDates(title, 27, fields);
  }
}

const Loc*
VoluntaryRefunds::determineTicketValidityDeadline(DateTime& deadline) const
{
  if (_record3.tktValidityInd() == MATCH_COMMENCE_DATE && !_trx.fullRefund())
  {
    deadline = _trx.exchangeItin().front()->travelDate();
    return _trx.exchangeItin().front()->travelSeg().front()->origin();
  }

  return originalTicketIssueLoc();
}

const Loc*
VoluntaryRefunds::originalTicketIssueLoc() const
{
  RexBaseRequest& rexBaseRequest = static_cast<RexBaseRequest&>(*_trx.getRequest());

  if (rexBaseRequest.getOriginalTicketAgentLocation())
    return rexBaseRequest.getOriginalTicketAgentLocation();

  if (!_trx.previousExchangeDT().isEmptyDate())
  {
    LOG4CXX_ERROR(
        logger,
        "Missing original tkt agent location for refund of prevoiusly exchanged ticket !!!");
    return nullptr;
  }

  if (rexBaseRequest.prevTicketIssueAgent() &&
      !rexBaseRequest.prevTicketIssueAgent()->agentCity().empty())
    return rexBaseRequest.prevTicketIssueAgent()->agentLocation();

  LOG4CXX_ERROR(logger, "Missing original tkt agent location for refund !!!");
  return nullptr;
}

namespace
{
inline DateTime
getAdjustedOriginalTktIssueDT(const RefundPricingTrx& trx)
{
  return (!trx.originalTktIssueDT().historicalIncludesTime())
             ? DateTime(trx.originalTktIssueDT().date(), 23, 59, 59)
             : DateTime(trx.originalTktIssueDT().date(),
                        trx.originalTktIssueDT().hours(),
                        trx.originalTktIssueDT().minutes(),
                        0);
}
}

bool
VoluntaryRefunds::matchTicketValidity()
{
  if (_record3.tktValidityInd() == BLANK)
  {
    printTicketValidity("N/A", DateTime::emptyDate(), _trx.currentTicketingDT());
    return true;
  }

  // assume indicator A to avoid empty Date
  DateTime deadline = getAdjustedOriginalTktIssueDT(_trx);

  const Loc* deadlineLoc = determineTicketValidityDeadline(deadline);

  deadline = _trx.adjustToCurrentUtcZone(deadline, deadlineLoc);
  if (deadline == DateTime::emptyDate())
    LOG4CXX_ERROR(logger, "Ticket validity date utc adjustement failed !!!");

  if (_record3.tktValidityPeriod().empty() && _record3.tktValidityUnit().empty())
  {
    deadline = deadline.addYears(1);
    printTicketValidity("1 YEAR", deadline, _trx.currentTicketingDT());
  }

  else
  {
    PeriodOfStay pos(_record3.tktValidityPeriod(), _record3.tktValidityUnit());

    if (!pos.isValid() || pos.isDayOfWeek())
    {
      printByteError("14-19 - TICKET VALIDITY");
      return false;
    }

    deadline = RuleUtil::addPeriodToDate(deadline, pos);
    printTicketValidity(pos.getPeriodOfStayAsString(), deadline, _trx.currentTicketingDT());
  }

  if (deadline < _trx.currentTicketingDT())
  {
    printByteFail("TICKET VALIDITY DATE");
    return false;
  }

  if (_record3.tktValidityInd() != MATCH_ORIGINAL_TICKET_DATE &&
      _record3.tktValidityInd() != MATCH_COMMENCE_DATE)
  {
    printByteError("14-19 - TICKET VALIDITY");
    return false;
  }

  return true;
}

bool
VoluntaryRefunds::matchFullyFlown()
{
  if (UNLIKELY(_dc))
    *_dc << "FULLY FLOWN: " << _record3.fullyFlown() << "\n";

  switch (_record3.fullyFlown())
  {
  case BLANK:
    return true;
  case FULLY_FLOWN:
  {
    FlownStatusCheck flownStatus(_paxTypeFare);
    if (flownStatus.isTotallyFlown())
      return true;
    break;
  }
  case PARTIALLY_FLOWN:
  {
    FlownStatusCheck flownStatus(_paxTypeFare);
    if (flownStatus.isPartiallyFlown())
      return true;
    break;
  }
  default:
    printByteError("43 - FULLY FLOWN");
    return false;
  }

  printByteFail("FULLY FLOWN RESTR");
  return false;
}

const Loc*
VoluntaryRefunds::determineCustomer1Deadline(DateTime& deadline) const
{
  if (_record3.customer1ResTkt() == MATCH_EARLIEST_RESERVATIONS_DATE)
    deadline = _trx.exchangeItin().front()->travelSeg().front()->bookingDT();

  return originalTicketIssueLoc();
}

bool
VoluntaryRefunds::matchCustomer1()
{
  if (_record3.customer1ResTkt() == BLANK)
  {
    printCustomer1("N/A", DateTime::emptyDate(), _trx.currentTicketingDT());
    return true;
  }

  // assume indicator T to avoid empty DateTime
  DateTime deadline = getAdjustedOriginalTktIssueDT(_trx);

  const Loc* deadlineLoc = determineCustomer1Deadline(deadline);

  deadline = _trx.adjustToCurrentUtcZone(deadline, deadlineLoc);
  if (deadline == DateTime::emptyDate())
    LOG4CXX_ERROR(logger, "Customer1 date utc adjustement failed !!!");

  PeriodOfStay pos(_record3.customer1Period(), _record3.customer1Unit());

  if (!pos.isValid() || pos.isDayOfWeek())
  {
    printByteError("114-119 - CUSTOMER 1ST");
    return false;
  }

  deadline = RuleUtil::addPeriodToDate(deadline, pos);
  printCustomer1(pos.getPeriodOfStayAsString(), deadline, _trx.currentTicketingDT());

  if (deadline < _trx.currentTicketingDT())
  {
    printByteFail("CUSTOMER 1ST DATE");
    return false;
  }

  if (_record3.customer1ResTkt() != MATCH_EARLIEST_RESERVATIONS_DATE &&
      _record3.customer1ResTkt() != MATCH_TICKET_DATE)
  {
    printByteError("114-119 - CUSTOMER 1ST");
    return false;
  }

  return true;
}

void
VoluntaryRefunds::printByteFail(const std::string& comment)
{
  if (UNLIKELY(_dc))
    *_dc << "FAILED ITEM " << _record3.itemNo() << " - " << comment << " NOT MET\n";
}

void
VoluntaryRefunds::printByteError(const std::string& comment)
{
  LOG4CXX_ERROR(logger,
                "VOLUNTARYREFUNDS item No. " << _record3.itemNo() << " incorrect byte " << comment);
  if (UNLIKELY(_dc))
    *_dc << "ERROR: VOLUNTARYREFUNDS R3 ITEM NO " << _record3.itemNo() << " INCORRECT BYTE "
         << comment << "\n";
}

namespace
{
static const std::string
label("REFUND");
}

bool
VoluntaryRefunds::matchWaiver()
{
  WaiverCodeValidator val(_trx.dataHandle(), _dc, logger, label);
  bool result = val.validate(_record3.itemNo(),
                             _record3.vendor(),
                             _record3.waiverTblItemNo(),
                             _paxTypeFare.fareMarket()->ruleApplicationDate(),
                             _trx.waiverCode());

  if (result && _record3.waiverTblItemNo())
    _trx.waivedRecord3().insert(&_record3);

  return result;
}

bool
VoluntaryRefunds::matchPTC()
{
  PaxTypeCodeValidator val(_dc, label);
  return val.validate(_record3.itemNo(), _trx.getExchangePaxType(), _record3.psgType());
}

bool
VoluntaryRefunds::matchDeparture()
{
  DepartureValidator val(_itin, _paxTypeFare.fareMarket(), _pu, _dc);
  return val.validate(_record3.itemNo(),
                      _record3.puInd(),
                      _record3.depOfJourneyInd(),
                      _record3.fareComponentInd(),
                      _softPass);
}
bool
VoluntaryRefunds::matchAdvCancelation()
{
  AdvanceReservationValidator advResValidator(_trx,
                                              _dc,
                                              logger,
                                              _record3.advCancelFromTo(),
                                              _itin,
                                              _pu,
                                              *_paxTypeFare.fareMarket()->travelSeg().front(),
                                              _record3.itemNo());

  return advResValidator.validate(_softPass, _record3.advCancelPeriod(), _record3.advCancelUnit());
}

bool
VoluntaryRefunds::matchOrigSchedFlt()
{
  OriginallyScheduledFlightValidator val(_trx, _dc, logger);
  return val.validate(_record3.itemNo(),
                      _paxTypeFare.fareMarket()->travelSeg(),
                      _record3.origSchedFlt(),
                      _record3.origSchedFltPeriod(),
                      _record3.origSchedFltUnit());
}

bool
VoluntaryRefunds::matchCxrApplTbl()
{
  CarrierApplicationValidator val(
      static_cast<const RexBaseRequest&>(*_trx.getRequest()), _record3, _dc, _trx.dataHandle());
  return val.validate();
}

bool
VoluntaryRefunds::matchOverrideDateTable() const
{
  if (_record3.overrideDateTblItemNo() == 0)
    return true;
  DateTime reservationDate;
  RuleUtil::getLatestBookingDate(reservationDate, _paxTypeFare.fareMarket()->travelSeg());
  return RuleUtil::validateDateOverrideRuleItem(_trx,
                                                _record3.overrideDateTblItemNo(),
                                                _record3.vendor(),
                                                _trx.travelDate(),
                                                _trx.ticketingDate(),
                                                reservationDate,
                                                _dc,
                                                Diagnostic333);
}
}
