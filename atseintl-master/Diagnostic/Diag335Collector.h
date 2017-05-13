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

#include "DBAccess/Record2Types.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{
class FareMarket;
class PaxType;
class NegFareRest;
class NegFareSecurityInfo;
class NegFareCalcInfo;
class MarkupCalculate;

class Diag335Collector : public DiagCollector
{
public:
  explicit Diag335Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag335Collector() {}

  virtual void printHeader() override;

  Diag335Collector& operator<<(const FareMarket& fareMarket) override;
  Diag335Collector& operator<<(const PaxTypeFare& paxFare) override;

  virtual DiagCollector& operator<<(const NegFareSecurityInfo& x) override;
  virtual DiagCollector& operator<<(const NegFareRest& x) override;
  virtual DiagCollector& operator<<(const NegFareCalcInfo& x) override;
  virtual DiagCollector& operator<<(const CategoryRuleInfo& x) override;
  virtual DiagCollector& operator<<(const CategoryRuleItemInfo& x) override;
  virtual DiagCollector& operator<<(const MarkupCalculate& x) override;

  virtual void
  doNetRemitTkt(const NegFareRest& negFareRest, bool isAxessUser, bool isRecurringSegData);
  virtual void displayFailCode(const NegFareRest& negFareRest, const char* failCode);
  virtual void displayMessage(const std::string& msg, bool showDiag);

  virtual void displayRelation(const CategoryRuleItemInfo* rule, Record3ReturnTypes statusRule) override;

  static const char* R3_PASS;
  static const char* FAIL_PTC;
  static const char* FAIL_TBL994;
  static const char* FAIL_UNAVLTAG;
  static const char* FAIL_TEXTONLY;
  static const char* FAIL_TKT_NETREMIT_SELLING;
  static const char* FAIL_COMM_NETREMIT_TYPE_L;
  static const char* FAIL_COMM_NETREMIT_TYPE_C;
  static const char* FAIL_COMM_NETREMIT_TYPE_T;
  static const char* FAIL_TKT_NETREMIT_TYPE_C_T;
  static const char* FAIL_TKT_NETREMIT_TYPE_L_C_T;
  static const char* FAIL_CARRIER_RESRT;
  static const char* R2_NOT_FOUND;
  static const char* R2_NOT_APPL;
  static const char* FAIL_REC_SEG_1_2_DATA;
  static const char* FAIL_REC_SEG_BTW_AND;
  static const char* FAIL_REC_SEG_TKT_FARE_IND;
  static const char* FAIL_REC_SEG_ITIN;
  static const char* FAIL_LOC_OUT_SIDE_KOREA;
  static const char* FAIL_TKT_FARE_DATA_IND;
  static const char* R2_FAIL_EFF_DISC_DATE;
  static const char* R2_FAIL_IF;

private:
  void displayLocKeys(const LocKey& loc1, const LocKey& loc2);
  bool displaySellingAmounts(Indicator fareInd,
                             Percent percent,
                             double sellingAmount1,
                             const tse::CurrencyCode& currency1,
                             double sellingAmount2,
                             const tse::CurrencyCode& currency2);
  bool displayMinMaxAmounts(Indicator fareInd,
                            Percent minPercent,
                            Percent maxPercent,
                            double minAmount1,
                            double maxAmount1,
                            const tse::CurrencyCode& currency1,
                            double minAmount2,
                            double maxAmount2,
                            const tse::CurrencyCode& currency2);
  void
  displayCommision(const MoneyAmount& amount, const tse::CurrencyCode& code, std::string& segment);
  void displaySegment(const Indicator& tktFareData,
                      const Indicator& owrt,
                      const Indicator& seasonType,
                      const Indicator& dowType,
                      const std::string gd,
                      const CarrierCode& carrier,
                      const RuleNumber& ruleNumber,
                      const TariffNumber& ruleTariff,
                      const FareType& fareType,
                      const FareClassCode& fareClass,
                      const LocCode& betwCity,
                      const LocCode& andCity,
                      const char* axessInd,
                      std::string& segment);
  void displayRec2(const CategoryRuleItemInfo& x);
  std::string _itemString;
  friend class Diag335CollectorTest;
};

} // namespace tse

