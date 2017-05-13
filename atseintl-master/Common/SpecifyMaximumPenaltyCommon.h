//----------------------------------------------------------------------------
//  Copyright Sabre 2015
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

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingTrx.h"

#include <utility>
#include <vector>

namespace tse
{
class DiagManager;
class FareInfo;
class FareMarket;
class FarePath;
class Itin;
class PaxTypeFare;
class PenaltyInfo;
class PricingTrx;
class VoluntaryChangesInfoW;
class VoluntaryRefundsInfo;

namespace smp
{
enum RecordApplication : unsigned
{ INVALID = 0,
  BEFORE = 1,
  AFTER = 2,
  BOTH = BEFORE | AFTER };

namespace DepartureAppl
{
const Indicator INVALID = 'X';
const Indicator BEFORE = 'B';
const Indicator AFTER = 'A';
const Indicator BOTH = ' ';
}

namespace PenaltyAppl
{
const Indicator ANYTIME_BLANK = ' ';
const Indicator ANYTIME = '1';
const Indicator ANYTIME_CHILD_DSC = '4';
const Indicator BEFORE_DEP = '2';
const Indicator BEFORE_DEP_CHILD_DSC = '5';
const Indicator AFTER_DEP = '3';
const Indicator AFTER_DEP_CHILD_DSC = '6';
}

namespace UnavailTag
{
const Indicator TEXT_ONLY = 'Y';
const Indicator DATA_UNAVAILABLE = 'X';
const Indicator BLANK = ' ';
}

namespace AdvCancellationInd
{
constexpr Indicator JOURNEY = 'J';
constexpr Indicator FARE_COMPONENT = 'F';
constexpr Indicator PRICING_UNIT = 'P';
}

enum Mode
{ INFO,
  OR,
  AND };

enum ChangeQuery
{ CHANGEABLE,
  NONCHANGEABLE };

RecordApplication operator|(const RecordApplication app1, const RecordApplication app2);
RecordApplication& operator|=(RecordApplication& app1, const RecordApplication app2);
RecordApplication operator&(const RecordApplication app1, const RecordApplication app2);
RecordApplication& operator&=(RecordApplication& app1, const RecordApplication app2);

RecordApplication
getRecordApplication(const VoluntaryRefundsInfo& refundInfo, bool isFirstFc, bool isFirstPu);

RecordApplication
getRecordApplication(const VoluntaryChangesInfoW& changeInfo,
                     RecordApplication targetApplication,
                     bool isFirstFC,
                     bool isFirstPU);

RecordApplication
getRecordApplication(const PenaltyInfo& penaltyInfo);

bool
isDepartureMatching(RecordApplication app1, RecordApplication app2);

template <class Record>
bool
isDepartureMatching(const Record& record, RecordApplication app)
{
  return getRecordApplication(record) & app;
}

bool
isPenaltyCalculationRequired(const PricingTrx& trx);

bool
voluntaryChangesNotPermitted(const VoluntaryChangesInfoW& changeInfo);

template <class RecordType>
extern bool
isPsgMatch(const PaxType& paxType, const RecordType& record);

inline Indicator
toDepartureAppl(smp::RecordApplication app)
{
  switch (app)
  {
  case smp::INVALID:
    return DepartureAppl::INVALID;
  case smp::BEFORE:
    return DepartureAppl::BEFORE;
  case smp::AFTER:
    return DepartureAppl::AFTER;
  default:
    return DepartureAppl::BOTH;
  }
}

void
validatePenaltyInputInformation(PricingTrx& trx);

void
preValidateFareMarket(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);

template <typename Record3>
extern bool
printRecord3(const PricingTrx& trx,
             const Record3& record,
             smp::RecordApplication departureInd,
             DiagManager& diag);

bool
validateOverrideDateTable(PricingTrx& trx,
                          const VendorCode& vendor,
                          const uint32_t& overrideDateTblItemNo,
                          const DateTime& applicationDate);

const std::pair<const PaxTypeFare*, const int16_t>
grabMissingDataFareInformationAndCleanUp(const FareInfo& fareInfo,
                                         std::vector<const PaxTypeFare*>& allPTFs);

bool ptfHasCat31Rec2(const PaxTypeFare& ptf);

std::string
printRecordApplication(const smp::RecordApplication& application);

CurrencyCode
getOriginCurrency(const FarePath& farePath);

std::vector<const PaxTypeFare*> getAllPaxTypeFares(const FarePath& farePath);
std::vector<const PaxTypeFare*>
getAllPaxTypeFaresForSfrMultipax(const FarePath& farePath, const MultiPaxFCMapping& fcMap);

template <typename Fees>
void
printDiagnosticFareFees(DiagManager& diag,
                        const PaxTypeFare& ptf,
                        const Fees& fees,
                        const bool additionalInfo = true);

} /* namespace SMP */
} /* namespace tse */
