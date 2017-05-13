//-------------------------------------------------------------------
//
//  File:        OriginallyScheduledFlightValidator.h
//  Created:     July 13, 2009
//
//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#pragma once

#include "Common/DateTime.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

#include <log4cxx/helpers/objectptr.h>

#include <string>
#include <vector>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{

class RexBaseTrx;
class DiagCollector;
class TravelSeg;
class Loc;

class OriginallyScheduledFlightValidator
{
public:
  OriginallyScheduledFlightValidator(RexBaseTrx& trx, DiagCollector* dc, log4cxx::LoggerPtr logger)
    : _trx(trx), _dc(dc), _logger(logger), _when(AFTER), _error(false), _softPass(false)
  {
  }

  virtual ~OriginallyScheduledFlightValidator() {}

  bool validate(uint32_t itemNoR3,
                const std::vector<TravelSeg*>& segs,
                Indicator origSchedFltInd,
                const ResPeriod& origSchedFltPeriod,
                const ResUnit& origSchedFltUnit);

  bool validate(uint32_t itemNo,
                int seqNo,
                const std::vector<TravelSeg*>& segs,
                Indicator origSchedFltInd,
                const std::string& origSchedFltPeriod,
                Indicator origSchedFltUnit);

  static constexpr Indicator NOT_APPLY = ' ';
  static constexpr Indicator ANYTIME_BEFORE = 'B';
  static constexpr Indicator DAY_BEFORE = 'D';
  static constexpr Indicator SAME_DAY_OR_EARLIER = 'X';
  static constexpr Indicator ANYTIME_AFTER = 'A';
  static constexpr Indicator SAME_DAY_OR_LATER = 'O';
  static constexpr Indicator DAY_AFTER = 'L';
  static constexpr Indicator MINUTE = 'N';
  static constexpr Indicator HOUR = 'H';
  static constexpr Indicator DAY = 'D';
  static constexpr Indicator MONTH = 'M';

protected:
  bool match(const std::vector<TravelSeg*>& segs,
             Indicator origSchedFltInd,
             const std::string& origSchedFltPeriod,
             Indicator origSchedFltUnit);

  virtual DateTime getReissueDate(const Loc& departureLoc, const DateTime& departureDT) const;

  void printMainMessage(Indicator ind, const std::string& period, Indicator unit) const;
  std::string toString(Indicator ind) const;
  std::string toString(Indicator ind, bool plural) const;

  DateTime countForward(const DateTime& refDT, int period, Indicator unit) const;
  DateTime
  countBackwards(const DateTime& refDT, int period, Indicator unit, bool doNotUseDayBoundary) const;

  RexBaseTrx& _trx;
  DiagCollector* _dc;
  log4cxx::LoggerPtr _logger;

  enum
  {
    BEFORE,
    AFTER
  } _when;
  bool _error;
  bool _softPass;

  DateTime _limitDT, _latestDT, _reissueDT;

private:
  OriginallyScheduledFlightValidator(const OriginallyScheduledFlightValidator&);
  OriginallyScheduledFlightValidator& operator=(const OriginallyScheduledFlightValidator&);

  friend class OriginallyScheduledFlightValidatorTest;
};
}
