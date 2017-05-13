//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------

#pragma once

#include "Diagnostic/DiagCollector.h"

namespace tse
{
class FareByRuleCtrlInfo;
class FareMarket;
class PaxType;

class Diag225Collector : public DiagCollector
{
public:
  //@TODO will be removed, once the transition is done
  explicit Diag225Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag225Collector() {}

  virtual void printHeader() override;
  void writeHeader();

  Diag225Collector& operator<<(const FareMarket& fareMarket) override;
  Diag225Collector& operator<<(const PaxType& paxType) override;

  void
  diag225Collector(const FareByRuleCtrlInfo* rule, const char* failCode, FareMarket& fareMarket);

  static const char* R2_PASS;
  static const char* R2_FAIL_GEO;
  static const char* R2_FAIL_EFF_DISC_DATE;

private:
  enum SeparatorType
  {
    RULE_HEADER = 1
  };

  void writeSeparator(SeparatorType);
};

} // namespace tse

