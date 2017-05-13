#pragma once

#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

#include <log4cxx/helpers/objectptr.h>

#include <vector>

#include <stdint.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class PricingTrx;
class Itin;
class PaxTypeFare;
class PricingUnit;
class FareUsage;
class FarePath;

/// A data encapsulation class

struct FootnoteTable
{
public:
  Footnote& footNote() { return _footNote; }
  const Footnote& footNote() const { return _footNote; }
  TariffNumber& tariffNumber() { return _tariffNumber; }
  const TariffNumber& tariffNumber() const { return _tariffNumber; }

private:
  Footnote _footNote;
  TariffNumber _tariffNumber = 0;
};

class RuleControllerDataAccess
{
public:
  RuleControllerDataAccess(bool sitaVendor = false) : _sitaVendor(sitaVendor) {}

  virtual ~RuleControllerDataAccess() = default;

  virtual Itin* itin() = 0;
  virtual PaxTypeFare& paxTypeFare() const = 0;
  virtual PricingTrx& trx() = 0;
  virtual FarePath* farePath() { return nullptr; }
  virtual PricingUnit* currentPU() { return nullptr; }
  virtual FareUsage* currentFU() { return nullptr; }
  virtual void cloneFU(unsigned int) {}
  virtual bool processRuleResult(Record3ReturnTypes&, unsigned int&) { return false; }
  virtual FareUsage* getFareUsage() { return nullptr; }

  void retrieveBaseFare(bool retrieveBaseFare) { _retrieveBaseFare = retrieveBaseFare; }

  std::vector<FootnoteTable>& footNoteTbl() { return _footNoteTbl; }

  uint16_t currentCatNum() const { return _currentCatNum; }
  uint16_t& currentCatNum() { return _currentCatNum; }

  bool getTmpResultCat15HasSecurity() const { return _tmpResultCat15HasSecurity; }
  void setTmpResultCat15HasSecurity(bool cat15HasSecurity)
  {
    _tmpResultCat15HasSecurity = cat15HasSecurity;
  }

  void setRuleType(RuleType rt) { _ruleType = rt; }
  RuleType getRuleType() const { return _ruleType; }

  void setValidatingCxr(const CarrierCode& cxr) { _validatingCxr = cxr; }
  CarrierCode validatingCxr() const { return _validatingCxr; }

  void setRtnMixedResult(const uint16_t catNum) { _mixedRtnResultCategories.push_back(catNum); }
  std::vector<uint16_t>& mixedRtnResultCategroies() { return _mixedRtnResultCategories; }

  bool isSitaVendor() const { return _sitaVendor; }

protected:
  PaxTypeFare& getBaseOrPaxTypeFare(PaxTypeFare& ptFare) const;

  std::vector<FootnoteTable> _footNoteTbl;
  bool _retrieveBaseFare = false;
  bool _tmpResultCat15HasSecurity = false;
  uint16_t _currentCatNum = 0;
  RuleType _ruleType = RuleType::FareRule;
  CarrierCode _validatingCxr;
  std::vector<uint16_t> _mixedRtnResultCategories;
  bool _sitaVendor;
};
}
