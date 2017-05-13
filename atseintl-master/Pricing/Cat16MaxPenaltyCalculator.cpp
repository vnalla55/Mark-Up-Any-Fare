/*
 * Cat16MaxPenaltyCalculator.cpp
 *
 *  Created on: May 15, 2015
 *      Author: SG0217429
 */

#include "Pricing/Cat16MaxPenaltyCalculator.h"

#include "Common/FallbackUtil.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"
#include "Diagnostic/DiagManager.h"
#include "Rules/Penalties.h"

#include <boost/optional/optional_io.hpp>

#include <algorithm>

namespace tse
{
FALLBACK_DECL(smpMissingDataSingleDeparture)

namespace
{
using ResponseFees = MaxPenaltyResponse::Fees;

inline bool
isUnavailTagSkip(const PenaltyInfo* record)
{
  return record->unavailTag() == smp::UnavailTag::TEXT_ONLY;
}

inline bool
isUnavailTagFail(const PenaltyInfo* record)
{
  return record->unavailTag() == smp::UnavailTag::DATA_UNAVAILABLE;
}

inline bool
isNoShow(const PenaltyInfo* record)
{
  return record->penaltyFail() == Penalties::APPLIES &&
         record->penaltyCancel() == Penalties::BLANK &&
         record->penaltyReissue() == Penalties::BLANK &&
         record->penaltyExchange() == Penalties::BLANK &&
         record->penaltyNoReissue() == Penalties::BLANK &&
         record->penaltyRefund() == Penalties::BLANK && record->penaltyPta() == Penalties::BLANK;
}

inline bool
recordMatches(const PenaltyInfo* record, const smp::RecordApplication& application)
{
  return smp::isDepartureMatching(*record, application) &&
         !isUnavailTagSkip(record) && !isUnavailTagFail(record) && !isNoShow(record);
}

inline void
handleOtherwise(DiagManager& diag,
                smp::RecordApplication application,
                smp::RecordApplication departureInd,
                const PenaltyInfo& r3,
                ResponseFees& fees)
{
  diag << "   RECORD " << r3.itemNo() << " FAILED - CHECKING OTHERWISE\n";

  if (application & departureInd & smp::BEFORE)
  {
    diag << "    OTHERWISE BEFORE\n";
    fees._before._non = true;
  }

  if (application & departureInd & smp::AFTER)
  {
    diag << "    OTHERWISE AFTER\n";
    fees._after._non = true;
  }
}

smp::RecordApplication
calculateIsAnyDepartureExceptTextOnly(const std::unordered_set<const PenaltyInfo*>& records,
                                      smp::RecordApplication departureInd)
{
  smp::RecordApplication departure = smp::INVALID;
  for (const PenaltyInfo* record : records)
  {
    if (!isUnavailTagSkip(record))
    {
      smp::RecordApplication recordDep = smp::getRecordApplication(*record);

      departure |= (recordDep & departureInd);
    }

    if (departure == smp::BOTH)
    {
      break;
    }
  }

  return departure;
}
} // empty namespace end

Cat16MaxPenaltyCalculator::Cat16MaxPenaltyCalculator(PricingTrx& trx, bool skipDiag)
  : _trx(trx),
    _diagEnabled(trx.diagnostic().diagParamIsSet(Diagnostic::DISPLAY_DETAIL, "MAXPEN") &&
                 !skipDiag),
    _diagPrevalidationEnabled(
        trx.diagnostic().diagParamIsSet(Diagnostic::RULE_PHASE, Diagnostic::PREVALIDATION))
{
}

// remove below block with SMP_C16_PREVALIDATION_REFACTORING
bool
Cat16MaxPenaltyCalculator::areNon(const PenaltiesCollection& records,
                                  std::function<bool(const PenaltyInfo*)> isNon,
                                  std::function<bool(const PenaltyInfo*)> recordApplies,
                                  const smp::RecordApplication& application)
{
  int unavailCounter = records.size();

  for (const PenaltyInfo* record : records)
  {
    if (recordMatches(record, application) && !isNon(record) && recordApplies(record))
    {
      return false;
    }

    if (isUnavailTagSkip(record))
    {
      unavailCounter--;
    }
  }
  return !(records.empty() || !unavailCounter);
}

bool
Cat16MaxPenaltyCalculator::areNonChangeable(
    const PenaltiesCollection& penaltySet,
    const smp::RecordApplication& application)
{
  return areNon(penaltySet,
                &Cat16MaxPenaltyCalculator::isNonChangeable,
                &Cat16MaxPenaltyCalculator::changeApplies,
                application);
}

bool
Cat16MaxPenaltyCalculator::areNonRefundable(
    const PenaltiesCollection& penaltySet,
    const smp::RecordApplication& application)
{
  return areNon(penaltySet,
                &Cat16MaxPenaltyCalculator::isNonRefundable,
                &Cat16MaxPenaltyCalculator::refundApplies,
                application);
}
// remove above block with SMP_C16_PREVALIDATION_REFACTORING

bool
Cat16MaxPenaltyCalculator::isNonRefundable(const PenaltyInfo* penaltyInfo)
{
  return penaltyInfo->noRefundInd() == Penalties::TICKET_NON_REFUNDABLE ||
         penaltyInfo->noRefundInd() == Penalties::TICKETNRF_AND_RESNOCHANGE;
}

bool
Cat16MaxPenaltyCalculator::refundApplies(const PenaltyInfo* penaltyInfo)
{
  return penaltyInfo->cancelRefundAppl() == Penalties::APPLIES;
}

bool
Cat16MaxPenaltyCalculator::isNonChangeable(const PenaltyInfo* penaltyInfo)
{
  return penaltyInfo->noRefundInd() == Penalties::RESERVATIONS_CANNOT_BE_CHANGED ||
         penaltyInfo->noRefundInd() == Penalties::TICKETNRF_AND_RESNOCHANGE;
}

bool
Cat16MaxPenaltyCalculator::changeApplies(const PenaltyInfo* penaltyInfo)
{
  return penaltyInfo->volAppl() == Penalties::APPLIES ||
         penaltyInfo->involAppl() == Penalties::APPLIES;
}

ResponseFees
Cat16MaxPenaltyCalculator::calculateMaxPenalty(const PenaltiesCollection& records,
                                               const CurrencyCode& penaltyCurrencyCode,
                                               const PaxTypeFare& ptf,
                                               const FareUsage* fareUsage,
                                               smp::RecordApplication departureInd,
                                               PenaltyType penaltyType)
{
  _fees = {{}, {}};
  _fees._cat16 = departureInd;

  switch (penaltyType)
  {
  case CHANGE_PEN:
    return calculationImpl(records,
                           penaltyCurrencyCode,
                           ptf,
                           departureInd,
                           &Cat16MaxPenaltyCalculator::isNonChangeable,
                           &Cat16MaxPenaltyCalculator::changeApplies,
                           fareUsage);
  case REFUND_PEN:
    return calculationImpl(records,
                           penaltyCurrencyCode,
                           ptf,
                           departureInd,
                           &Cat16MaxPenaltyCalculator::isNonRefundable,
                           &Cat16MaxPenaltyCalculator::refundApplies,
                           fareUsage);
  default:
  {
    TSE_ASSERT(false); // This should never happen
  }
  }
}

ResponseFees
Cat16MaxPenaltyCalculator::calculationImpl(const PenaltiesCollection& records,
                                           const CurrencyCode& penaltyCurrencyCode,
                                           const PaxTypeFare& ptf,
                                           smp::RecordApplication departureInd,
                                           std::function<bool(const PenaltyInfo*)> isNon,
                                           std::function<bool(const PenaltyInfo*)> recordApplies,
                                           const FareUsage* fareUsage)
{
  const PenaltyInfo* usedRecordBefore = nullptr;
  const PenaltyInfo* usedRecordAfter = nullptr;

  DiagManager diag(_trx);

  if (_diagEnabled)
  {
    diag.activate(Diagnostic555);
    diag << "   " << DiagnosticUtil::printPaxTypeFare(ptf) << " - CAT16\n";
  }

  if (records.empty())
  {
    diag << "NO RECORDS - CAT16\n";
    _fees._before = _fees._after = MaxPenaltyResponse::Fee(Money(penaltyCurrencyCode), false);
    return _fees;
  }

  if (fallback::smpMissingDataSingleDeparture(&_trx))
  {
    if (std::all_of(records.begin(), records.end(), isUnavailTagSkip))
    {
      diag << "   ALL RECORDS ARE UNAVAILABLE\n";

      if (departureInd & smp::BEFORE)
      {
        _fees._before._non = false;
      }

      if (departureInd & smp::AFTER)
      {
        _fees._after._non = false;
      }

      return _fees;
    }
  }
  else
  {
    smp::RecordApplication departure = calculateIsAnyDepartureExceptTextOnly(records, departureInd);

    if (departure != smp::BOTH)
    {
      if (departure != smp::BEFORE)
      {
        diag << "   BEFORE RECORDS ARE UNAVAILABLE\n";
        departureInd &= smp::AFTER;
        _fees._before._non = false;
      }

      if (departure != smp::AFTER)
      {
        diag << "   AFTER RECORDS ARE UNAVAILABLE\n";
        departureInd &= smp::BEFORE;
        _fees._after._non = false;
      }

      if (departure == smp::INVALID)
      {
        diag << "   ALL RECORDS ARE UNAVAILABLE\n";
        return _fees;
      }
    }
  }

  _fees._before._non = _fees._after._non = false;

  for (const PenaltyInfo* r3 : records)
  {
    if (_diagEnabled)
      smp::printRecord3(_trx, *r3, departureInd, diag);

    smp::RecordApplication application = smp::getRecordApplication(*r3);

    if (!recordMatches(r3, departureInd))
    {
      diag << "   RECORD " << r3->itemNo() << " FAILED\n";
      continue;
    }

    if (isNon(r3))
    {
      handleOtherwise(diag, application, departureInd, *r3, _fees);

      continue;
    }

    if (!recordApplies(r3))
    {
      diag << "   RECORD " << r3->itemNo() << " DOES NOT APPLY\n";
      continue;
    }

    MoneyAmount maxFromRecord =
        Penalties::getPenaltyAmount(
            _trx, ptf, *r3, penaltyCurrencyCode, fareUsage, ConversionType::NO_ROUNDING).value();

    diag << "   RECORD " << r3->itemNo() << " PASSED - " << maxFromRecord << penaltyCurrencyCode << "\n";

    if (application & departureInd & smp::BEFORE)
    {
      if (!_fees._before._fee || maxFromRecord > _fees._before._fee.get().value())
      {
        _fees._before._fee = Money(maxFromRecord, penaltyCurrencyCode);
        usedRecordBefore = r3;
      }
    }

    if (application & departureInd & smp::AFTER)
    {
      if (!_fees._after._fee || maxFromRecord > _fees._after._fee.get().value())
      {
        _fees._after._fee = Money(maxFromRecord, penaltyCurrencyCode);
        usedRecordAfter = r3;
      }
    }
  }

  if (!_fees._before._fee && !_fees._before._non && (departureInd & smp::BEFORE))
  {
    _fees._before._fee = Money(0., penaltyCurrencyCode);
  }

  if (!_fees._after._fee && !_fees._after._non && (departureInd & smp::AFTER))
  {
    _fees._after._fee = Money(0., penaltyCurrencyCode);
  }

  if (_diagEnabled)
  {
    printRecordsAfterValidation(usedRecordBefore, usedRecordAfter, departureInd, diag);
  }

  return _fees;
}
void
Cat16MaxPenaltyCalculator::printRecordsAfterValidation(const PenaltyInfo* recordBefore,
                                                       const PenaltyInfo* recordAfter,
                                                       smp::RecordApplication departureInd,
                                                       DiagManager& diag) const
{
  if (recordBefore || recordAfter)
  {
    auto validRecordToDiag = [&diag](const std::string& departureString,
                                     const PenaltyInfo& usedRecord,
                                     const smp::RecordApplication& departureInd) -> void
    {
      diag << "    VALID RECORD " << departureString
           << " DEPARTURE FOUND IN CAT16 : " << usedRecord.itemNo()
           << " DEP : " << smp::printRecordApplication(departureInd) << "\n" << usedRecord;
    };

    if (recordBefore)
    {
      validRecordToDiag("BEFORE", *recordBefore, departureInd);
    }

    if (recordAfter)
    {
      validRecordToDiag("AFTER", *recordAfter, departureInd);
    }
  }
  else
  {
    diag << "    NO VALID RECORDS FOUND IN CAT16\n";
  }

  if (_fees.missingDataInsideCalc()) // BEFORE | AFTER | BOTH
  {
    auto missingDataToDiag = [&diag](const smp::RecordApplication& missingData) -> void
    {
      diag << "    MISSING DATA FOR : " << (missingData & smp::BEFORE ? "BEF " : "- ")
           << (missingData & smp::AFTER ? "AFT" : "-") << "\n";
    };
    missingDataToDiag(_fees.missingDataInsideCalc());
  }

  if (_diagPrevalidationEnabled)
  {
    auto addToDiagPrevalidation = [&diag](const std::string& departureString,
                                          const MaxPenaltyResponse::Fee& fee)
    {
      diag << "    " << departureString << " C16 : " << fee._fee << (fee._non ? "/NON" : "")
           << '\n';
    };

    addToDiagPrevalidation("BEFORE", _fees._before);
    addToDiagPrevalidation("AFTER", _fees._after);
  }
}

} // namespace tse
