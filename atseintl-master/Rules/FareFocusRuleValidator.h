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

#include "Common/DateTime.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

namespace tse
{
class PricingTrx;
class FareMarket;
class Diag240Collector;
class Loc;
class PaxTypeFare;
class CustomerSecurityHandshakeInfo;
class FareFocusRuleInfo;
class FareFocusBookingCodeInfo;
class Logger;

class FareFocusRuleValidator
{
  friend class FareFocusRuleValidatorTest;

public:
  FareFocusRuleValidator(PricingTrx& trx, FareMarket& _fareMarket);
  virtual ~FareFocusRuleValidator();

  void process();

  static const Indicator DirectionalityFrom;
  static const Indicator DirectionalityBoth;

protected:
  void createDiagnostic();
  bool isFareFocusApplicable() const;
  bool printDiagAndReturn(const std::string& msg) const;
  bool matchFMForDiagnostic();
  bool matchPTFareForDiagnostic(const PaxTypeFare& paxTypeFare);
  void printFareMarketHeader();
  void endDiag() const;
  void printPaxTypeFare(const PaxTypeFare& paxTypeFare);
  const std::vector<CustomerSecurityHandshakeInfo*>& getCustomerSecurityHandshake() const;
  void printDiagSecurityHShakeNotFound() const;
  bool matchOWRT(const Indicator owrtFromFareFocusRule, const Indicator owrtFromFare) const;
  bool matchVendor(const VendorCode& vendorRule, const VendorCode& vendorFare) const;
  bool matchRule(const RuleNumber& ruleNumberRule, const RuleNumber& ruleNumberFare) const;
  bool matchRuleTariff(const TariffNumber& tariffNumberRule, const TariffNumber& tariffNumberFare) const;
  bool matchFareType(const FareTypeAbbrevC& fareTypeRule, const FareType& fareTypeFare) const;
  bool matchCarrier(const CarrierCode& carrierRule, const PaxTypeFare& ptf) const;
  bool matchPublicInd(const Indicator publicIndRule, const TariffCategory publicIndFare) const;
  void getFareFocusRules(const std::vector<CustomerSecurityHandshakeInfo*>& cSH,
                         std::vector<const FareFocusRuleInfo*>& ffriV);
  bool validateSecurityHandshake(const FareFocusRuleInfo& ffri,
                                 const std::vector<CustomerSecurityHandshakeInfo*>& cSH) const;
  void printDiagFareFocusRulesNotFound() const;
  void printFareFocusRuleNoneFound() const;
  void printFareFocusNoData(const StatusFFRuleValidation rc) const;
  void printFareFocusLookup(const uint64_t fareFocusRuleId, const StatusFFRuleValidation rc) const;
  bool isFareFocusRuleMatched(const PaxTypeFare& ptf, const std::vector<const FareFocusRuleInfo*>& ffRulesV) const;
  void printFareFocusRuleProcess(const FareFocusRuleInfo* ffri, StatusFFRuleValidation rc) const;
  bool isDiagRuleNumberMatch(const FareFocusRuleInfo* ffri) const;
  bool matchDirectionality(const FareFocusRuleInfo& ffri,
                           const PaxTypeFare& paxTypeFare) const;
  bool isValidFareDirectionality(const PaxTypeFare& paxTypeFare,
                                 const FareFocusRuleInfo& ffri) const;
  bool validGeoType(const FareFocusRuleInfo& ffri) const;
  bool matchFareClass(const uint64_t& fareClassItemNo, const PaxTypeFare& ptf) const;
  bool matchTravelRangeX5(PricingTrx& trx,
                          uint64_t dayTimeApplItemNo,
                          const PaxTypeFare& ptf,
                          DateTime adjustedTicketDate) const;

  virtual bool getFareClassMatchExpressions(const uint64_t& fareClassItemNo,
                                            std::vector<std::string>& result) const;
  virtual bool matchGeo(const FareFocusRuleInfo& ffri, const PaxTypeFare& paxTypeFare) const;

  typedef std::map<uint64_t, std::vector<std::string> > MatchExpressionMap;
  PricingTrx& _trx;
  FareMarket& _fareMarket;
  Diag240Collector* _diag;
  bool _diagInfo;
  static Logger _logger;
  MatchExpressionMap _matchExpressions;
  DateTime _adjustedTicketDate;

private:
  FareFocusRuleValidator(const FareFocusRuleValidator& rhs);
  FareFocusRuleValidator& operator=(const FareFocusRuleValidator& rhs);

};
} // tse namespace

