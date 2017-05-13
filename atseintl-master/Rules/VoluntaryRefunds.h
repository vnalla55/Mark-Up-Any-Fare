//-------------------------------------------------------------------
//
//  File:        VoluntaryRefunds.h
//  Created:     June 17, 2009
//  Authors:     Grzegorz Wanke
//
//-------------------------------------------------------------------------------
// Copyright 2007, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#pragma once

#include "Common/DateTime.h"
#include "Common/TseEnums.h"
#include "Common/TsePrimitiveTypes.h"

#include <list>

namespace tse
{

class RefundPricingTrx;
class PaxTypeFare;
class VoluntaryRefundsInfo;
class DiagCollector;
class Loc;
class Itin;
class PricingUnit;

class VoluntaryRefunds
{
  friend class VoluntaryRefundsTest;

public:
  static constexpr Indicator BLANK = ' ';
  static constexpr Indicator FULLY_FLOWN = 'A';
  static constexpr Indicator PARTIALLY_FLOWN = 'B';
  static constexpr Indicator MATCH_ORIGINAL_TICKET_DATE = 'A';
  static constexpr Indicator MATCH_COMMENCE_DATE = 'B';
  static constexpr Indicator MATCH_EARLIEST_RESERVATIONS_DATE = 'R';
  static constexpr Indicator MATCH_TICKET_DATE = 'T';

  VoluntaryRefunds(RefundPricingTrx& trx,
                   const Itin* itin,
                   const PricingUnit* pu,
                   PaxTypeFare& ptf,
                   const VoluntaryRefundsInfo& vector3,
                   DiagCollector* dc);

  virtual ~VoluntaryRefunds() {}

  Record3ReturnTypes validate();

private:
  bool matchTicketValidity();
  void printTicketValidity(const std::string& tktValidity,
                           const DateTime& tktValidityDate,
                           const DateTime& adjustedRefundDT) const;
  const Loc* determineTicketValidityDeadline(DateTime& deadlineStartDT) const;

  bool matchCustomer1();
  void printCustomer1(const std::string& tktValidity,
                      const DateTime& tktValidityDate,
                      const DateTime& adjustedRefundDT) const;
  const Loc* determineCustomer1Deadline(DateTime& deadline) const;

  bool matchFullyFlown();
  bool matchWaiver();
  bool matchPTC();
  bool matchAdvCancelation();
  bool matchDeparture();
  bool matchOrigSchedFlt();
  bool matchCxrApplTbl();
  bool matchOverrideDateTable() const;

  const Loc* originalTicketIssueLoc() const;

  void printDates(const std::string& title,
                  unsigned int leftPad,
                  std::list<std::pair<std::string, const DateTime*> >& fields) const;
  void printOriginalAndRefundDT(const DateTime& adjustedRefundDT,
                                std::list<std::pair<std::string, const DateTime*> >& fields) const;
  void printByteFail(const std::string& comment);
  void printByteError(const std::string& comment);

  RefundPricingTrx& _trx;
  const Itin* _itin;
  const PricingUnit* _pu;
  PaxTypeFare& _paxTypeFare;
  const VoluntaryRefundsInfo& _record3;
  bool _softPass;
  DiagCollector* _dc;

  VoluntaryRefunds(const VoluntaryRefunds&);
  VoluntaryRefunds& operator=(const VoluntaryRefunds&);
};
}

