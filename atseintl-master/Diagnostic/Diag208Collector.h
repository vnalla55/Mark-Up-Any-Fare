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
class FareMarket;
class PaxType;
class FareByRuleApp;

class Diag208Collector : public DiagCollector
{
public:
  //@TODO will be removed, once the transition is done
  explicit Diag208Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag208Collector() {}

  virtual void printHeader() override;
  void diag208Collector(FareByRuleApp& fbrApp,
                        PricingTrx& trx,
                        const char* failCode,
                        const FareMarket& fareMarket);

  Diag208Collector& operator<<(const FareMarket& fareMarket) override;
  Diag208Collector& operator<<(const PaxType& paxType) override;

  static const char* R8_PASS;
  static const char* TRXREF_NOT_FOUND;
  static const char* TARIFF_INHIBIT;
  static const char* FAIL_TRXREF;
  static const char* FAIL_PAX_STATUS;
  static const char* FAIL_GLOBAL;
  static const char* FAIL_GEO;
  static const char* FAIL_LOC3;
  static const char* FAIL_SECURITY;
  static const char* FAIL_SAME_CARRIER;
  static const char* FAIL_CXRFLT_TBL;
  static const char* FAIL_PL_PV_TARIFF;
  static const char BLANK;

private:
  enum SeparatorType
  {
    FARE_MARKET_HEADER = 1
  };

  void writeSeparator(SeparatorType);
};

} // namespace tse

