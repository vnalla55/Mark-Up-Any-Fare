//-------------------------------------------------------------------
//
//  File:        StopoversTravelSegWrapper.h
//  Created:     August 2, 2004
//  Authors:     Andrew Ahmad
//
//  Description:
//
//
//  Copyright Sabre 2004
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

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/VCTR.h"

#include <list>
#include <map>

namespace tse
{
class TravelSeg;
class FareUsage;

class StopoversTravelSegWrapper
{
public:
  class Surcharge
  {
  public:
    Surcharge(const MoneyAmount amount,
              const CurrencyCode currency,
              const CurrencyNoDec noDec,
              const bool isSegmentSpecific,
              const bool isCharge1,
              const bool isFirstAmt,
              const MoneyAmount amountLocCur,
              const CurrencyCode currencyLocCur,
              const CurrencyNoDec noDecLocCur,
              const bool chargeFromFirstInbound)
      : _amount(amount),
        _currency(currency),
        _noDec(noDec),
        _isSegmentSpecific(isSegmentSpecific),
        _isCharge1(isCharge1),
        _isFirstAmt(isFirstAmt),
        _amountLc(amountLocCur),
        _currencyLc(currencyLocCur),
        _noDecLc(noDecLocCur),
        _chargeFromFirstInbound(chargeFromFirstInbound)

    {
    }

    const MoneyAmount amount() const { return _amount; }
    MoneyAmount& amount() { return _amount; }

    const CurrencyCode currency() const { return _currency; }
    CurrencyCode& currency() { return _currency; }

    const CurrencyNoDec noDec() const { return _noDec; }
    CurrencyNoDec& noDec() { return _noDec; }

    const bool isSegmentSpecific() const { return _isSegmentSpecific; }
    bool& isSegmentSpecific() { return _isSegmentSpecific; }

    const bool isCharge1() const { return _isCharge1; }
    bool& isCharge1() { return _isCharge1; }

    const bool isFirstAmt() const { return _isFirstAmt; }
    bool& isFirstAmt() { return _isFirstAmt; }

    const MoneyAmount amountLocCur() const { return _amountLc; }
    MoneyAmount& amountLocCur() { return _amountLc; }

    const CurrencyCode currencyLocCur() const { return _currencyLc; }
    CurrencyCode& currencyLocCur() { return _currencyLc; }

    const CurrencyNoDec noDecLocCur() const { return _noDecLc; }
    CurrencyNoDec& noDecLocCur() { return _noDecLc; }

    const bool chargeFromFirstInbound() const { return _chargeFromFirstInbound; }
    bool& chargeFromFirstInbound() { return _chargeFromFirstInbound; }

  private:
    MoneyAmount _amount;
    CurrencyCode _currency;
    CurrencyNoDec _noDec;
    bool _isSegmentSpecific;
    bool _isCharge1;
    bool _isFirstAmt;
    MoneyAmount _amountLc;
    CurrencyCode _currencyLc;
    CurrencyNoDec _noDecLc;
    bool _chargeFromFirstInbound;
  };
  typedef std::list<Surcharge> SurchargeList;
  typedef SurchargeList::iterator SurchargeListI;
  typedef SurchargeList::const_iterator SurchargeListCI;

  enum MatchType
  {
    NONE_MATCHED,
    NOT_MATCHED,
    MATCHED
  };

  StopoversTravelSegWrapper()
    : _travelSeg(nullptr),
      _arunk_ts(nullptr),
      _fareUsage(nullptr),
      _isStopover(false),
      _ruleItemMatch(0),
      _ruleVCTRMatch(),
      _isFareRuleMatch(true),
      _isTentativeMatch(false),
      _passedValidation(false),
      _passedByLeastRestrictive(false),
      _maxSOExceeded(0),
      _matched(NONE_MATCHED)
  {
  }

  virtual ~StopoversTravelSegWrapper() {}

  const TravelSeg* travelSeg() const { return _travelSeg; }
  TravelSeg*& travelSeg() { return _travelSeg; }

  const FareUsage* fareUsage() const { return _fareUsage; }
  FareUsage*& fareUsage() { return _fareUsage; }

  const bool isStopover() const { return _isStopover; }
  bool& isStopover() { return _isStopover; }

  const uint32_t ruleItemMatch() const { return _ruleItemMatch; }
  uint32_t& ruleItemMatch() { return _ruleItemMatch; }

  const VCTR& ruleVCTRMatch() const { return _ruleVCTRMatch; }
  VCTR& ruleVCTRMatch() { return _ruleVCTRMatch; }

  const bool isFareRuleMatch() const { return _isFareRuleMatch; }
  bool& isFareRuleMatch() { return _isFareRuleMatch; }

  const bool isTentativeMatch() const { return _isTentativeMatch; }
  bool& isTentativeMatch() { return _isTentativeMatch; }

  const bool passedValidation() const { return _passedValidation; }
  bool& passedValidation() { return _passedValidation; }

  const bool passedByLeastRestrictive() const { return _passedByLeastRestrictive; }
  bool& passedByLeastRestrictive() { return _passedByLeastRestrictive; }

  const SurchargeList& surcharges() const { return _surcharges; }
  SurchargeList& surcharges() { return _surcharges; }

  const uint16_t maxSOExceeded() const { return _maxSOExceeded; }
  uint16_t& maxSOExceeded() { return _maxSOExceeded; }

  void clearSurcharges() { _surcharges.clear(); }

  const TravelSeg* arunkTravelSeg() const { return _arunk_ts; }
  TravelSeg*& arunkTravelSeg() { return _arunk_ts; }

  void setMatched(MatchType x) { _matched = x; }

  bool isMatched() const { return _matched == StopoversTravelSegWrapper::MATCHED; }

  bool isNotMatched() const { return _matched == StopoversTravelSegWrapper::NOT_MATCHED; }

protected:
  TravelSeg* _travelSeg;
  TravelSeg* _arunk_ts;
  FareUsage* _fareUsage;
  bool _isStopover;
  uint32_t _ruleItemMatch; // the rule item number of the record 3
  //  that this segment matched on
  VCTR _ruleVCTRMatch;
  bool _isFareRuleMatch;
  bool _isTentativeMatch;
  bool _passedValidation;
  bool _passedByLeastRestrictive;
  uint16_t _maxSOExceeded;

  MatchType _matched;
  std::list<Surcharge> _surcharges;
};

typedef std::map<const TravelSeg*, StopoversTravelSegWrapper> StopoversTravelSegWrapperMap;
typedef StopoversTravelSegWrapperMap::iterator StopoversTravelSegWrapperMapI;
typedef StopoversTravelSegWrapperMap::const_iterator StopoversTravelSegWrapperMapCI;

} // namespace tse

