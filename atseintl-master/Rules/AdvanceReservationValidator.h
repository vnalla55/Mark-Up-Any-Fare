//-------------------------------------------------------------------
//
//  File:        AdvanceReservationValidator.h
//  Created:     June 22, 2009
//  Authors:     Artur Krezel
//
//  Description:
//
//-------------------------------------------------------------------------------
// Copyright 2007, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#pragma once
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class DiagCollector;
class RexBaseTrx;
class DateTime;
class PricingUnit;
class Itin;
class TravelSeg;
class Loc;

class AdvanceReservationValidator
{
  friend class AdvanceReservationValidatorTest;

public:
  static const Indicator NOT_APPLY;
  static const Indicator ADVRSVN_JOURNEY;
  static const Indicator ADVRSVN_PRICING_UNIT;
  static const Indicator ADVRSVN_FARE_COMPONENT;

  AdvanceReservationValidator(RexBaseTrx& trx,
                              DiagCollector* dc,
                              log4cxx::LoggerPtr& logger,
                              const Indicator& advResTo,
                              const Itin* itin,
                              const PricingUnit* pu,
                              const TravelSeg& firstSegOfFCconst,
                              const uint32_t& itemNo);
  virtual ~AdvanceReservationValidator() {}

  bool validate(bool& isSoftPass, const ResPeriod& advResPeriod, const ResUnit& advResUnit);

protected:
  DiagCollector* _dc;
  RexBaseTrx& _trx;
  const Indicator _advResTo;
  const Itin* _itin;
  const PricingUnit* _pu;
  const TravelSeg& _firstSegOfFC;
  const uint32_t _itemNo;

  bool getToDateAndLoc(DateTime& toDT, Loc& toLoc, bool& isSoftPass);
  void printInputData(const ResPeriod& advResPeriod, const ResUnit& advResUnit) const;
  virtual bool getLimitDateTime(DateTime& advResLimit,
                                const DateTime& toDT,
                                const ResPeriod& advResPeriod,
                                const ResUnit& advResUnit) const;
  void printOutputDates(const DateTime& advResLimit,
                        const DateTime& fromDT,
                        const DateTime& fromDTZone) const;
  virtual bool
  getUtcOffsetDifference(short& utcOffsetInMinutes, DateTime& fromDT, DateTime& toDT, Loc& toLoc);

private:
  log4cxx::LoggerPtr& _logger;

  AdvanceReservationValidator();
  AdvanceReservationValidator(const AdvanceReservationValidator&);
  AdvanceReservationValidator& operator=(const AdvanceReservationValidator&);
};
}

