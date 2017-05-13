#include "Rules/AdvanceReservationValidator.h"

#include "Common/TseUtil.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RexBaseTrx.h"
#include "Diagnostic/DiagCollector.h"
#include "Rules/AdvanceResTkt.h"
#include "Rules/PeriodOfStay.h"
#include "Util/BranchPrediction.h"

namespace tse
{
const Indicator AdvanceReservationValidator::NOT_APPLY = ' ';
const Indicator AdvanceReservationValidator::ADVRSVN_JOURNEY = 'J';
const Indicator AdvanceReservationValidator::ADVRSVN_PRICING_UNIT = 'P';
const Indicator AdvanceReservationValidator::ADVRSVN_FARE_COMPONENT = 'F';

AdvanceReservationValidator::AdvanceReservationValidator(RexBaseTrx& trx,
                                                         DiagCollector* dc,
                                                         log4cxx::LoggerPtr& logger,
                                                         const Indicator& advResTo,
                                                         const Itin* itin,
                                                         const PricingUnit* pu,
                                                         const TravelSeg& firstSegOfFC,
                                                         const uint32_t& itemNo)
  : _dc(dc),
    _trx(trx),
    _advResTo(advResTo),
    _itin(itin),
    _pu(pu),
    _firstSegOfFC(firstSegOfFC),
    _itemNo(itemNo),
    _logger(logger)
{
}

bool
AdvanceReservationValidator::getToDateAndLoc(DateTime& toDT, Loc& toLoc, bool& isSoftPass)
{
  switch (_advResTo)
  {
  case ADVRSVN_JOURNEY:
  {
    if (!_itin || _itin->travelSeg().front()->hasEmptyDate())
    {
      if (UNLIKELY(_dc))
        *_dc << "  NO ITIN INFO: SOFTPASS\n";
      isSoftPass = true;
      return true;
    }
    toDT = _itin->travelSeg().front()->departureDT();
    toLoc = *_itin->travelSeg().front()->origin();

    break;
  }
  case ADVRSVN_PRICING_UNIT:
  {
    if (!_pu || _pu->travelSeg().front()->hasEmptyDate())
    {
      if (UNLIKELY(_dc))
        *_dc << "  NO PU INFO: SOFTPASS\n";

      isSoftPass = true;
      return true;
    }
    toDT = _pu->travelSeg().front()->departureDT();
    toLoc = *_pu->travelSeg().front()->origin();
    break;
  }
  case ADVRSVN_FARE_COMPONENT:
  {
    if (_firstSegOfFC.hasEmptyDate())
    {
      if (UNLIKELY(_dc))
        *_dc << "  NO FC INFO: SOFTPASS\n";

      isSoftPass = true;
      return true;
    }
    toDT = _firstSegOfFC.departureDT();
    toLoc = *_firstSegOfFC.origin();
    break;
  }
  default:
  {
    LOG4CXX_ERROR(_logger, "VOLUNTARY REFUND item No. " << _itemNo << " incorrect byte 23");
    if (UNLIKELY(_dc))
      *_dc << "ERROR: VOLUNTARY REFUND R3 ITEM NO " << _itemNo << " INCORRECT BYTE 23\n";
    return false;
  }
  }
  return true;
}

void
AdvanceReservationValidator::printInputData(const ResPeriod& advResPeriod,
                                            const ResUnit& advResUnit) const
{
  if (LIKELY(!_dc))
    return;

  *_dc << "\nADVANCE CANCELLATION: ";

  if (_advResTo != ' ')
  {
    PeriodOfStay pos(advResPeriod, advResUnit);

    *_dc << pos.getPeriodOfStayAsString() << " BEFORE DEPARTURE OF ";
    switch (_advResTo)
    {
    case 'J':
      *_dc << "JOURNEY";
      break;
    case 'P':
      *_dc << "PU";
      break;
    case 'F':
      *_dc << "FC";
      break;
    default:
      break;
    }
  }
  *_dc << "\n";
}

bool
AdvanceReservationValidator::getLimitDateTime(DateTime& advResLimit,
                                              const DateTime& toDT,
                                              const ResPeriod& advResPeriod,
                                              const ResUnit& advResUnit) const
{
  if (!AdvanceResTkt::getLimitDateTime(advResLimit,
                                       toDT,
                                       -1,
                                       advResPeriod,
                                       advResUnit,
                                       AdvanceResTkt::BEFORE_REF_TIME,
                                       false,
                                       true))
  {
    if (UNLIKELY(_dc))
      *_dc << "WRONG DATA FOR ADV RES VALIDATION\n";
    return false;
  }
  return true;
}

void
AdvanceReservationValidator::printOutputDates(const DateTime& advResLimit,
                                              const DateTime& fromDT,
                                              const DateTime& fromDTZone) const
{
  if (UNLIKELY(_dc))
  {
    *_dc << " REFUND DATE/TIME          : " << fromDT.toIsoExtendedString() << std::endl
         << " REFUND DATE DEP TIME ZONE : " << fromDTZone.toIsoExtendedString() << std::endl
         << " LIMIT DATE/TIME           : " << advResLimit.toIsoExtendedString() << std::endl;
  }
}

bool
AdvanceReservationValidator::validate(bool& isSoftPass,
                                      const ResPeriod& advResPeriod,
                                      const ResUnit& advResUnit)
{
  printInputData(advResPeriod, advResUnit);

  if (_advResTo == ' ')
    return true;

  DateTime fromDT = DateTime(_trx.currentTicketingDT().date(),
                             _trx.currentTicketingDT().hours(),
                             _trx.currentTicketingDT().minutes(),
                             0);

  DateTime toDT;
  Loc toLoc;
  if (!getToDateAndLoc(toDT, toLoc, isSoftPass))
    return false;
  if (isSoftPass)
    return true;

  DateTime advResLimit;
  if (!getLimitDateTime(advResLimit, toDT, advResPeriod, advResUnit))
    return false;

  short utcOffsetInMinutes = 0;
  if (!getUtcOffsetDifference(utcOffsetInMinutes, fromDT, toDT, toLoc))
    return false;

  printOutputDates(advResLimit, fromDT, fromDT - Minutes(utcOffsetInMinutes));

  bool result = (fromDT - Minutes(utcOffsetInMinutes)) <= advResLimit;

  if (UNLIKELY(!result && _dc))
    *_dc << "  FAILED ITEM " << _itemNo << " - ADV CANX DATE NOT MET\n";

  return result;
}

bool
AdvanceReservationValidator::getUtcOffsetDifference(short& utcOffsetInMinutes,
                                                    DateTime& fromDT,
                                                    DateTime& toDT,
                                                    Loc& toLoc)
{
  return LocUtil::getUtcOffsetDifference(
      *_trx.currentSaleLoc(), toLoc, utcOffsetInMinutes, _trx.dataHandle(), fromDT, toDT);
}
}
