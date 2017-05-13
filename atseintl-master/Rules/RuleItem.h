#pragma once

#include "Common/TseEnums.h"
#include "DBAccess/Record2Types.h"
#include "Diagnostic/Diagnostic.h"

#include <boost/array.hpp>

#include <memory>

namespace tse
{
class CategoryRuleItemInfo;
class RuleItemInfo;
class PricingTrx;
class PaxTypeFare;
class Itin;
class FarePath;
class PricingUnit;
class FareUsage;
class DiagCollector;
class RuleControllerDataAccess;
class DateOverrideRuleItem;
class RuleValidationChancelor;

/**
 * @class RuleItem
 *
 * @brief Defines a wrapper for a Record 3.
 */

class RuleItem
{
  friend class RuleItemTest;
  friend class TestRuleItem;

public:
  enum ProcessingPhase
  {
    UnKnown = 0,
    FarePhase,
    FCOPhase,
    PricingUnitPhase,
    PricingUnitFactoryPhase,
    FarePathFactoryPhase,
    PreCombinabilityPhase,
    FareDisplayPhase,
    RuleDisplayPhase,
    DynamicValidationPhase
  };

  typedef Record3ReturnTypes (RuleItem::*CategoryHandler)();

  RuleItem();

  void setRuleControllerDataAccess(RuleControllerDataAccess* rcDA) { _rcDataAccess = rcDA; }

  void setChancelor(std::shared_ptr<RuleValidationChancelor>& ch) { _chancelor = ch; }
  bool hasChancelor() const { return bool(_chancelor); }

  void setPhase(PricingTrx& trx, uint32_t categoryNumber);
  void setPhase(ProcessingPhase phase) { _phase = phase; }
  const ProcessingPhase& phase() const { return _phase; }

  void clearHandlers() { _handlerMap.fill(nullptr); }
  bool setHandler(uint32_t cat, CategoryHandler handler)
  {
    if (UNLIKELY(cat > MAX_RULE_HANDLED))
      return false;
    _handlerMap[cat] = handler;
    return true;
  }

  Record3ReturnTypes validate(PricingTrx& trx,
                              Itin& itin,
                              const CategoryRuleInfo& cri,
                              const std::vector<CategoryRuleItemInfo>& cfrItemSet,
                              PaxTypeFare& paxTypeFare,
                              const CategoryRuleItemInfo* rule,
                              const RuleItemInfo* ruleItemInfo,
                              uint32_t categoryNumber,
                              bool& isCat15Security,
                              bool skipCat15Security);

  Record3ReturnTypes validate(PricingTrx& trx,
                              Itin& itin,
                              const CategoryRuleInfo& cri,
                              const std::vector<CategoryRuleItemInfo>& cfrItemSet,
                              PaxTypeFare& paxTypeFare,
                              const CategoryRuleItemInfo* rule,
                              const RuleItemInfo* ruleItemInfo,
                              uint32_t categoryNumber,
                              bool& isCat15Security,
                              bool skipCat15Security,
                              bool isInbound);

  Record3ReturnTypes validate(PricingTrx& trx,
                              const CategoryRuleInfo& cri,
                              const std::vector<CategoryRuleItemInfo>& cfrItemSet,
                              FarePath& farePath,
                              const PricingUnit& pricingUnit,
                              FareUsage& fareUsage,
                              const CategoryRuleItemInfo* rule,
                              const RuleItemInfo* ruleItemInfo,
                              uint32_t categoryNumber,
                              bool& isCat15Security,
                              bool skipCat15Security);

  Record3ReturnTypes validate(PricingTrx& trx,
                              const CategoryRuleInfo& cri,
                              const std::vector<CategoryRuleItemInfo>& cfrItemSet,
                              const Itin* itin,
                              const PricingUnit& pricingUnit,
                              FareUsage& fareUsage,
                              const CategoryRuleItemInfo* rule,
                              const RuleItemInfo* ruleItemInfo,
                              uint32_t categoryNumber,
                              bool& isCat15Security,
                              bool skipCat15Security);

  Record3ReturnTypes handleSalesSecurity();
  Record3ReturnTypes handleEligibility();
  Record3ReturnTypes handleMiscFareTagsForPubl();

  static const std::string& getRulePhaseString(const int phase);
  static Record3ReturnTypes preliminaryT994Validation(PricingTrx& trx,
                                                      const CategoryRuleInfo& cri,
                                                      const PricingUnit* pricingUnit,
                                                      const PaxTypeFare& paxTypeFare,
                                                      const RuleItemInfo* ruleItemInfo,
                                                      const uint32_t categoryNumber);
private:

  static const uint32_t MAX_RULE_HANDLED = 33;
  typedef boost::array<CategoryHandler, MAX_RULE_HANDLED + 1> HandlerMap;

  RuleItem(PricingTrx& trx,
           const CategoryRuleInfo& cri,
           const PricingUnit* pricingUnit,
           const PaxTypeFare& paxTypeFare,
           const RuleItemInfo* ruleItemInfo);

  virtual Record3ReturnTypes validateDateOverrideRuleItem(const uint16_t categoryNumber,
                                                          DiagCollector* diagPtr,
                                                          const DiagnosticTypes& callerDiag);

  Record3ReturnTypes handleDayTime();
  Record3ReturnTypes handleSalesRestrictions();
  Record3ReturnTypes handleSeasonal();
  Record3ReturnTypes handleFlightApplication();
  Record3ReturnTypes handleBlackoutDates();
  Record3ReturnTypes handlePenalties();
  Record3ReturnTypes handleStopovers();
  Record3ReturnTypes handleTransfers();
  Record3ReturnTypes handleMinStayRestriction();
  Record3ReturnTypes handleMaxStayRestriction();
  Record3ReturnTypes handleAdvResTkt();
  Record3ReturnTypes handleTravelRestrictions();
  Record3ReturnTypes handleSurchargeRule();
  Record3ReturnTypes handleAccompaniedTravel();
  Record3ReturnTypes handleTicketingEndorsement();
  Record3ReturnTypes handleMiscFareTags();
  Record3ReturnTypes handleTours();
  Record3ReturnTypes handleVoluntaryExc();
  Record3ReturnTypes handleVoluntaryRefund();

  void setHandlers();
  Record3ReturnTypes callHandler(uint32_t categoryNumber);
  bool ruleIgnored(uint32_t categoryNumber);

  PaxTypeFare* determinePaxTypeFare(PaxTypeFare* ptFare, bool needBaseFare) const;

  void printFlexFaresRestriction(DiagCollector& diag,
                                 const uint16_t& catNumber,
                                 Record3ReturnTypes& retval) const;
  void printDorItem(DiagCollector& diag, const DateOverrideRuleItem& dorItem) const;
  void printDiag33XHeader(DiagCollector& diag) const;

  PricingTrx* _trx;
  Itin* _itin;
  FarePath* _farePath;
  const PricingUnit* _pricingUnit;
  FareUsage* _fareUsage;
  PaxTypeFare* _paxTypeFare;

  const CategoryRuleInfo* _cri;
  const std::vector<CategoryRuleItemInfo>* _cfrItemSet;
  const CategoryRuleItemInfo* _rule;
  const RuleItemInfo* _ruleItemInfo;

  bool* _isCat15Security;
  ProcessingPhase _phase;
  bool _skipCat15Security;
  bool _isInbound;

  HandlerMap _handlerMap;
  RuleControllerDataAccess* _rcDataAccess;
  std::shared_ptr<RuleValidationChancelor> _chancelor;
};

} // namespace tse

