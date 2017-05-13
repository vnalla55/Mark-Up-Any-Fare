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
class CategoryRuleItemInfo;
class FareByRuleItemInfo;
class FareMarket;
class PaxType;
class FareByRuleApp;
class Itin;

class Diag325Collector : public DiagCollector
{
public:
  //@TODO will be removed, once the transition is done
  explicit Diag325Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag325Collector() {}

  void printHeader() override;

  void writeHeader(const FareMarket& fareMarket, const PaxType& paxType);

  void diag325Collector(const FareByRuleCtrlInfo* rule, const char* failCode);
  void diag325Collector(const CategoryRuleItemInfo* catRuleItemInfo,
                        const char* failCode,
                        const std::list<uint16_t>* const failedIfItins = nullptr);
  void diag325Collector(const FareByRuleItemInfo& fareByRuleItemInfo,
                        PricingTrx& trx,
                        const Itin& itin,
                        const char* failCode);

  Diag325Collector& operator<<(const FareByRuleApp& fbrApp) override;
  Diag325Collector& operator<<(const PaxTypeFare& x) override;
  Diag325Collector& operator<<(const BaseFareRule& x) override;
  std::string formatFailBaseFareMessage(const PaxTypeFare& x, const char* reason);
  void displayFailBaseFareList(std::vector<std::string>& failList);
  void displayRemovedFares(const FareMarket& fm,
                           const uint16_t& uniqueFareCount,
                           const std::vector<PaxTypeFare*>& removedFaresVec);

  static const char* R3_PASS;
  static const char* FAIL_PTC;
  static const char* FAIL_TBL994;
  static const char* FAIL_UNAVLTAG;
  static const char* FAIL_PAX_STATUS;
  static const char* FAIL_FLT_SEG_CNT;
  static const char* FAIL_WHOLLY_WITHIN_LOC;
  static const char* NO_DISC;
  static const char* FAIL_MILEAGE;
  static const char* FAIL_GLOBAL;
  static const char* FAIL_PCT;
  static const char* R2_PASS;
  static const char* R2_NOT_FOUND;
  static const char* R2_NOT_APPL;
  static const char* R2_FAIL_IF;
  static const char* R2_FAIL_DIR;
  static const char* FAIL_SECURITY;
  static const char* FAIL_CAT35;
  static const char* FAIL_NON_CAT35;

  static const char* FAIL_PRIVATE_FARE;
  static const char* FAIL_RULE_TARIFF;
  static const char* FAIL_RULE_NUMBER;
  static const char* FAIL_CARRIER_CODE;
  static const char* FAIL_OW_RT;
  static const char* FAIL_GLOBAL_DIRECTION;
  static const char* FAIL_PASSENGER_TYPE_CODE;
  static const char* FAIL_FARE_CLASS;
  static const char* FAIL_FARE_TYPE_CODE;
  static const char* FAIL_SEASON_CODE;
  static const char* FAIL_DAY_CODE;
  static const char* FAIL_PRICING_CATEGORY_CODE;
  static const char* FAIL_MILEAGE_ROUTING;
  static const char* FAIL_ROUTING_NUMBER;
  static const char* FAIL_FOOT_NOTE;
  static const char* FAIL_BOOKING_CODE;
  static const char* FAIL_MIN_MAX_FARE_RANGE;

  static const char* FAIL_FARE_BY_RULE_FARE;
  static const char* FAIL_DISCOUNTED_FARE;
  static const char* FAIL_BASE_FARE_VENDOR_MATCH;
  static const char* FAIL_BASE_FARE_SECURITY;
  static const char* FAIL_VENDOR_CROSS_REF_CARRIER_PREF_FBR;
  static const char* FAIL_RESULTING_DISPLAY_CATEGORY;
  static const char* FAIL_VALID_FOR_FBR_BASE_FARE;
  static const char* FAIL_INVALID_INDUSTRY_FARE;
  static const char* FAIL_RESULTING_PRICING_CATEGORY;
  static const char* FAIL_RESULTING_FARE_TYPE;
  static const char* FAIL_RESULTING_GLOBAL;
  static const char* FAIL_BETWEEN_AND_CITIES;
};

} // namespace tse

