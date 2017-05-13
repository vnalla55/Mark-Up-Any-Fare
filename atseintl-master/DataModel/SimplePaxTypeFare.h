//-------------------------------------------------------------------
//
//  File:        SimlePaxTypeFare.h
//  Created:     Feb 27, 2006
//  Authors:
//
//  Description:
//
//  Copyright Sabre 2006
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseUtil.h"
#include "DataModel/AccTvlChkList.h"
#include "DataModel/PaxType.h"

namespace tse
{

struct SimpleAccTvlRule
{
public:
  bool negAppl() const { return _chkList.isPsgNegAppl(); }
  bool reqBkgCds() const { return _chkList.isChkBkgCode(); }
  bool reqSameCpmt() const { return _chkList.isChkSameCpmt(); }
  bool reqSameRule() const { return _chkList.isChkSameRule(); }
  bool reqChkNumPsg() const { return _chkList.isChkNumPsg(); }
  bool isTktGuaranteed() const { return _chkList.isTktGuaranteed(); }

  struct AccTvlOpt
  {
    std::vector<const PaxTypeCode*> _paxTypes;
    std::vector<const std::string*> _fareClassBkgCds;
  };

  AccTvlChkList _chkList;
  uint16_t numOfAccPsg;
  std::vector<const AccTvlOpt*> _accTvlOptions;
};

class SimplePaxTypeFare
{
public:
  SimplePaxTypeFare()
    : _requestedPaxTypeCode(""),
      _paxTypeCode(""),
      _paxNumber(0),
      _origSegNo(0),
      _destSegNo(0),
      _cabin(' '),
      _fareClass(""),
      _ruleNumber(""),
      _tcrRuleTariff(0),
      _bookingCode(' ') {};

  void setPaxType(PaxTypeCode paxTypeCode)
  {
    _paxTypeCode = _requestedPaxTypeCode = paxTypeCode;

    if (isDigit(paxTypeCode[1]) && isDigit(paxTypeCode[2]))
      _paxTypeCode[1] = _paxTypeCode[2] = 'N';
  }

  PaxTypeCode const& requestedPaxTypeCode() const { return _requestedPaxTypeCode; }
  PaxTypeCode const& paxTypeCode() const { return _paxTypeCode; }

  uint16_t& paxNumber() { return _paxNumber; }
  const uint16_t& paxNumber() const { return _paxNumber; }

  uint16_t& origSegNo() { return _origSegNo; }
  const uint16_t& origSegNo() const { return _origSegNo; }

  uint16_t& destSegNo() { return _destSegNo; }
  const uint16_t& destSegNo() const { return _destSegNo; }

  Indicator& cabin() { return _cabin; }
  const Indicator& cabin() const { return _cabin; }

  FareClassCode& fareClass() { return _fareClass; }
  const FareClassCode& fareClass() const { return _fareClass; }

  RuleNumber& ruleNumber() { return _ruleNumber; }
  const RuleNumber& ruleNumber() const { return _ruleNumber; }

  TariffNumber& tcrRuleTariff() { return _tcrRuleTariff; }
  const TariffNumber& tcrRuleTariff() const { return _tcrRuleTariff; }

  BookingCode& bookingCode() { return _bookingCode; }
  const BookingCode& bookingCode() const { return _bookingCode; }

  std::vector<SimpleAccTvlRule*>& simpleAccTvlRule() { return _simpleAccTvlRule; }
  const std::vector<SimpleAccTvlRule*>& simpleAccTvlRule() const { return _simpleAccTvlRule; }

private:
  PaxTypeCode _requestedPaxTypeCode;
  PaxTypeCode _paxTypeCode;
  uint16_t _paxNumber;
  uint16_t _origSegNo;
  uint16_t _destSegNo;
  Indicator _cabin;
  FareClassCode _fareClass;
  RuleNumber _ruleNumber;
  TariffNumber _tcrRuleTariff;
  BookingCode _bookingCode;
  std::vector<SimpleAccTvlRule*> _simpleAccTvlRule;
};

} // tse namespace

