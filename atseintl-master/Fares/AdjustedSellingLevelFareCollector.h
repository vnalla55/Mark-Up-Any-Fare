//-------------------------------------------------------------------
//  Copyright Sabre 2014
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

#include "DataModel/PaxType.h"
#include "DBAccess/PaxTypeInfo.h"
#include "Fares/FareRetailerRuleValidator.h"

namespace tse
{

class Logger;
class AdjustedFareCalc;
class FareRetailerCalcDetailInfo;

class PaxTypeFinder : public std::unary_function<PaxType*, bool>
{
  public:
    PaxTypeFinder(const PaxTypeFare& paxTypeFare, bool fbcSelected)
      : _matchesAll(paxTypeFare.isWebFare() &&
                    (paxTypeFare.fcasPaxType() == ADULT || paxTypeFare.fcasPaxType() == NEG)),
        _paxType(paxTypeFare.fcasPaxType()),
        _fbcSelected(fbcSelected),
        _discounted(paxTypeFare.isDiscounted()),
        _isChild(fbcSelected ? PaxTypeUtil::isChild(paxTypeFare.fcasPaxType(), paxTypeFare.vendor())
                             : false),
        _isInfant(fbcSelected
                      ? PaxTypeUtil::isInfant(paxTypeFare.fcasPaxType(), paxTypeFare.vendor())
                      : false)
    {
    }

    bool operator()(PaxType* paxType) const
    {

      if (_matchesAll || (_paxType.empty() && paxType->paxType() == ADULT))
        return true;

      if (LIKELY(!_fbcSelected || _discounted))
      {
        if (_paxType != paxType->paxType())
        {
          return false;
        }
      }
      else
      {
        // Command Pricing
        // We only care that ADT can not use CNN fare,
        // CNN can not use INF fare
        if (_isChild && !paxType->paxTypeInfo()->isChild())
        {
          return false;
        }

        if (_isInfant && !paxType->paxTypeInfo()->isInfant())
        {
          return false;
        }
      }

      return true;
    }

  private:
    const bool _matchesAll;
    const PaxTypeCode& _paxType;
    const bool _fbcSelected;
    const bool _discounted;
    const bool _isChild;
    const bool _isInfant;
};

class AdjustedSellingLevelFareCollector
{
  friend class AdjustedSellingLevelFareCollectorTest;

public:
  AdjustedSellingLevelFareCollector(PricingTrx& trx);
  virtual ~AdjustedSellingLevelFareCollector() {}

  void process();
  std::vector<PaxTypeFare*>& fareDisplayPtfVec() { return _fareDisplayPtfVec; }
  const std::vector<PaxTypeFare*>& fareDisplayPtfVec() const { return _fareDisplayPtfVec; }

protected:
  void processFareMarket(FareMarket* fm, Itin* itin);
  bool checkPtfValidity(const PaxTypeFare* ptf, const bool& isFareDisplay);
  void processFareRetailerRules(FareMarket* fm,
                                PaxTypeFare* ,
                                const std::vector<FareRetailerRuleContext>& ,
                                Itin*,
                                bool);
  bool isSuitableCalcDetail(const FareRetailerCalcDetailInfo*);
  bool keepFareRetailerMinAmt(PaxTypeFare*,
                              MoneyAmount&,
                              MoneyAmount&,
                              MoneyAmount,
                              MoneyAmount,
                              AdjustedFareCalc& calcObj,
                              AdjustedSellingCalcData*,
                              const FareRetailerRuleContext&,
                              const FareRetailerCalcDetailInfo*);
  void createFareDisplayFares(FareMarket* fm,
                              PaxTypeFare*, AdjustedSellingCalcData*, MoneyAmount, MoneyAmount);
  void getSellingInfo(AdjustedSellingCalcData* calcData, AdjustedFareCalc& calcObj);
  void updateFareMarket(FareMarket* fm);
  bool checkAge(const PaxTypeFare& ptFare, const PaxType& paxType) const;


  PricingTrx& _trx;
  FareDisplayTrx* _fdTrx;
  static Logger _logger;
  FareRetailerRuleValidator _frrv;
  std::vector<const FareRetailerRuleLookupInfo*> _frrlV;
  std::vector<PaxTypeFare*> _fareDisplayPtfVec;
  Diag868Collector* _diag868 = nullptr;;

private:
  AdjustedSellingLevelFareCollector(const FareRetailerRuleValidator& rhs);
  AdjustedSellingLevelFareCollector& operator=(const FareRetailerRuleValidator& rhs);
  bool needPrintDiagnostic(FareMarket* fm, const PaxTypeFare& ptf) const
  {
    return (_diag868 != nullptr && ffrTypeMatch() && fareClassMatch(ptf) &&
        ruleMatch(ptf) && matchFareMarket(fm));
  }

  bool ffrTypeMatch() const
  {
    const std::string& diagParam = _trx.diagnostic().diagParamMapItem(Diagnostic::FARE_RETAILER_TYPE);
    return (diagParam.empty() || diagParam == "ADJ");
  }

  bool fareClassMatch(const PaxTypeFare& ptf) const
  {
    const std::string& diagFareClass =
        _trx.diagnostic().diagParamMapItem(Diagnostic::FARE_CLASS_CODE);
    return (diagFareClass.empty() || diagFareClass == ptf.fareClass());
  }

  bool ruleMatch(const PaxTypeFare& ptf) const
  {
    const std::string& diagRule = _trx.diagnostic().diagParamMapItem(Diagnostic::RULE_NUMBER);
    return (diagRule.empty() || diagRule == ptf.ruleNumber());
  }

  bool matchFareMarket(FareMarket* fm) const
  {
    const std::string& diagFM = _trx.diagnostic().diagParamMapItem(Diagnostic::FARE_MARKET);
    if (diagFM.empty())
      return true;
    LocCode boardCity = diagFM.substr(0, 3);
    LocCode offCity = diagFM.substr(3, 3);
    if (((fm->origin()->loc() != boardCity) && (fm->boardMultiCity() != boardCity)) ||
        ((fm->destination()->loc() != offCity) && (fm->offMultiCity() != offCity)))
      return false;
    return true;
  }
};

} // tse namespace
