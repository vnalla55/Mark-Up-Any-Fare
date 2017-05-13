#pragma once

#include "Rules/RuleControllerDataAccess.h"

#include <vector>

namespace tse
{
class FarePath;
class FareUsage;
class PricingTrx;
class SurchargeData;

struct PricingUnitDataAccess : public RuleControllerDataAccess
{
  PricingUnitDataAccess(
      PricingTrx& transaction, FarePath* fPath, PricingUnit& pUnit, FareUsage& fUsage, Itin* pItin)
    : _trx(transaction), _itin(pItin), _farePath(fPath), _pricingUnit(pUnit), _fareUsage(fUsage)
  {
  }

  Itin* itin() override;
  PaxTypeFare& paxTypeFare() const override;

  FareUsage& fareUsage() { return (_clonedFU ? *_clonedFU : _fareUsage); }
  virtual FareUsage* getFareUsage() override { return &_fareUsage; }
  PricingTrx& trx() override { return _trx; }
  virtual FarePath* farePath() override { return _farePath; }
  PricingUnit& pricingUnit() { return _pricingUnit; }
  virtual PricingUnit* currentPU() override { return &_pricingUnit; }
  virtual FareUsage* currentFU() override { return &fareUsage(); }

  void cloneFU(unsigned int ruleCount) override;

  // returns true if no more rule processing is needed, false otherwise
  bool processRuleResult(Record3ReturnTypes& ruleRet, unsigned int& ruleIndex) override;

private:
  PricingTrx& _trx;
  Itin* _itin = nullptr;
  FarePath* _farePath = nullptr;
  PricingUnit& _pricingUnit;
  FareUsage& _fareUsage;
  FareUsage* _clonedFU = nullptr;
  struct RuleSurcharge
  {
    MoneyAmount surcharge = 0;
    std::vector<SurchargeData*> surchargeData;
    Record3ReturnTypes ruleRet = Record3ReturnTypes::NOTPROCESSED;
    unsigned int ruleIndex = 0;
  };

  RuleSurcharge _ruleSurcharge;
  bool _nonFailedRuleExists = false;
  unsigned int _ruleCount = 0;
};
}
