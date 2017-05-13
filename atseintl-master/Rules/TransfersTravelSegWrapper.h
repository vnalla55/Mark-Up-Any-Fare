//-------------------------------------------------------------------
//
//  File:        TransfersTravelSegWrapper.h
//  Created:     August 31, 2004
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

class TransfersTravelSegWrapper
{
public:
  class Surcharge
  {
  public:
    Surcharge(const MoneyAmount amount,
              const CurrencyCode currency,
              const int16_t noDec,
              const bool isSegmentSpecific,
              const bool isCharge1,
              const bool isFirstAmt,
              const MoneyAmount amountLocCur,
              const CurrencyCode currencyLocCur,
              const int16_t noDecLocCur)
      : _amount(amount),
        _currency(currency),
        _noDec(noDec),
        _isSegmentSpecific(isSegmentSpecific),
        _isCharge1(isCharge1),
        _isFirstAmt(isFirstAmt),
        _amountLc(amountLocCur),
        _currencyLc(currencyLocCur),
        _noDecLc(noDecLocCur)
    {
    }

    const MoneyAmount amount() const { return _amount; }
    MoneyAmount& amount() { return _amount; }

    const CurrencyCode currency() const { return _currency; }
    CurrencyCode& currency() { return _currency; }

    const int16_t noDec() const { return _noDec; }
    int16_t& noDec() { return _noDec; }

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

    const int16_t noDecLocCur() const { return _noDecLc; }
    int16_t& noDecLocCur() { return _noDecLc; }

  private:
    MoneyAmount _amount;
    CurrencyCode _currency;
    int16_t _noDec;
    bool _isSegmentSpecific;
    bool _isCharge1;
    bool _isFirstAmt;
    MoneyAmount _amountLc;
    CurrencyCode _currencyLc;
    int16_t _noDecLc;
  };
  typedef std::list<Surcharge> SurchargeList;
  typedef SurchargeList::iterator SurchargeListI;
  typedef SurchargeList::const_iterator SurchargeListCI;

  TransfersTravelSegWrapper()
    : _travelSeg(nullptr),
      _fareUsage(nullptr),
      _isTransfer(false),
      _ruleItemMatch(0),
      _ruleVCTRMatch(),
      _isFareRuleMatch(true),
      _isTentativeMatch(false),
      _passedValidation(false),
      _passedByLeastRestrictive(false),
      _validateEntireRule(false), // for processResults() in PU scope if segment non-match any R3
      _noMatchValidation(false),
      _maxTRExceeded(0),
      _isRecurringFCScope(false)
  {
  }
  virtual ~TransfersTravelSegWrapper() {}

  const TravelSeg* travelSeg() const { return _travelSeg; }
  TravelSeg*& travelSeg() { return _travelSeg; }

  const FareUsage* fareUsage() const { return _fareUsage; }
  FareUsage*& fareUsage() { return _fareUsage; }

  const bool isTransfer() const { return _isTransfer; }
  bool& isTransfer() { return _isTransfer; }

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

  const bool& validateEntireRule() const { return _validateEntireRule; }
  bool& validateEntireRule() { return _validateEntireRule; }

  const bool& noMatchValidation() const { return _noMatchValidation; }
  bool& noMatchValidation() { return _noMatchValidation; }

  const bool passedByLeastRestrictive() const { return _passedByLeastRestrictive; }
  bool& passedByLeastRestrictive() { return _passedByLeastRestrictive; }

  const uint16_t maxTRExceeded() const { return _maxTRExceeded; }
  uint16_t& maxTRExceeded() { return _maxTRExceeded; }

  const bool& isRecurringFCScope() const { return _isRecurringFCScope; }
  bool& isRecurringFCScope() { return _isRecurringFCScope; }

  const SurchargeList& surcharges() const { return _surcharges; }
  SurchargeList& surcharges() { return _surcharges; }

  void clearSurcharges() { _surcharges.clear(); }

protected:
  TravelSeg* _travelSeg;
  FareUsage* _fareUsage;
  bool _isTransfer;
  uint32_t _ruleItemMatch; // the rule item number of the record 3
  //  that this segment matched on
  VCTR _ruleVCTRMatch;
  bool _isFareRuleMatch;
  bool _isTentativeMatch;
  bool _passedValidation;
  bool _passedByLeastRestrictive;
  bool _validateEntireRule;
  bool _noMatchValidation;
  uint16_t _maxTRExceeded;
  bool _isRecurringFCScope;

  std::list<Surcharge> _surcharges;
};

typedef std::map<const TravelSeg*, TransfersTravelSegWrapper> TransfersTravelSegWrapperMap;
typedef TransfersTravelSegWrapperMap::iterator TransfersTravelSegWrapperMapI;
typedef TransfersTravelSegWrapperMap::const_iterator TransfersTravelSegWrapperMapCI;

} // namespace tse

