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

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

namespace tse
{
class Agent;
class PricingTrx;
class FareMarket;
class Diag868Collector;
class PaxTypeFare;
class CustomerSecurityHandshakeInfo;
class FareRetailerRuleInfo;
class FareRetailerRuleLookupInfo;
class FareFocusAccountCdDetailInfo;
class FareFocusLocationPairDetailInfo;
class Logger;

struct FareRetailerRuleContext
{
  PseudoCityCode _sourcePcc;
  const FareRetailerRuleInfo* _frri;
  const std::vector<FareRetailerCalcInfo*>& _frciV;
  const FareRetailerResultingFareAttrInfo* _frrfai;
  const FareFocusSecurityInfo* _ffsi;
  const NegFareSecurityInfo* _t983;

  FareRetailerRuleContext(PseudoCityCode sourcePcc,
                          const FareRetailerRuleInfo* frri,
                          const std::vector<FareRetailerCalcInfo*>& frciV,
                          const FareRetailerResultingFareAttrInfo* frrfai,
                          const FareFocusSecurityInfo* ffsi)
    : _sourcePcc(sourcePcc),
      _frri(frri),
      _frciV(frciV),
      _frrfai(frrfai),
      _ffsi(ffsi),
      _t983(0) {}
};

class FareRetailerRuleValidator
{

friend class FareRetailerRuleValidatorTest;

public:

  FareRetailerRuleValidator(PricingTrx& trx);
  virtual ~FareRetailerRuleValidator();

  static constexpr Indicator DirectionalityFrom = 'F';
  static constexpr Indicator DirectionalityBoth = 'B';

  // Public functions must call matchApplicationForDiagnostic() to prevent large
  // diagnostic output.
  void getFRRLookupAllSources(std::vector<const FareRetailerRuleLookupInfo*>& frrlV,
                              const FareMarket* fm,
                              const Code<8>& productCD,
                              const Indicator applicationType,
                              const bool printFMHdr);

  bool validateFRR(const PaxTypeFare& ptFare,
                   const FareMarket* fm,
                   const Indicator applicationType,
                   const std::vector<const FareRetailerRuleLookupInfo*>& frrlV,
                   std::vector<FareRetailerRuleContext>& frrcV);

  virtual const FareFocusPsgTypeInfo* getFareFocusPsgType(uint64_t psgTypeItemNo) const;
protected:

  void createDiagnostic();
  void endDiag() const;
  void printFareMarketHeader(const FareMarket* fm);
  void printDiagSecurityHShakeNotFound(const PseudoCityCode& pcc) const;
  bool matchFMForDiagnostic(const FareMarket* fm) const;
  void matchApplicationForDiagnostic(const Indicator applicationType);
  void printFareRetailerRule(StatusFRRuleValidation rc,
                             const FareRetailerRuleInfo* frri,
                             const FareFocusSecurityInfo* ffsi) const;
  const Agent* getAgent() const;

  virtual const std::vector<CustomerSecurityHandshakeInfo*>& getCustomerSecurityHandshake(
    const Code<8> productCD,
    const PseudoCityCode& pcc ) const;

  void getFRRLookupSources(std::vector<const FareRetailerRuleLookupInfo*>& frrlV,
                           const PseudoCityCode& pcc,
                           const Code<8>& productCD,
                           const Indicator applicationType);

  virtual const FareRetailerRuleLookupInfo* getFareRetailerRuleLookup(
    Indicator applicationType,
    const PseudoCityCode& sourcePcc,
    const PseudoCityCode& pcc) const;

  void mergeFrrls(std::vector<const FareRetailerRuleLookupInfo*>& frrl1,
                  const std::vector<const FareRetailerRuleLookupInfo*>& frrl2) const;

  FareRetailerRuleLookupInfo* mergeRules(const FareRetailerRuleLookupInfo* f1,
                                         const FareRetailerRuleLookupInfo* f2) const;

  virtual const FareRetailerRuleInfo* getFareRetailerRule(uint64_t fareRetailerRuleId) const;

  virtual const std::vector<FareRetailerCalcInfo*>&
    getFareRetailerCalc(uint64_t fareRetailerCalcItemNo) const;

  virtual const FareRetailerResultingFareAttrInfo*
    getFareRetailerResultingFareAttr(uint64_t resultingFareAttrItemNo) const;

  virtual const FareFocusSecurityInfo* getFareFocusSecurity(uint64_t securityItemNo) const;

  virtual const std::vector<FareFocusRuleCodeInfo*>&
    getFareFocusRuleCode(uint64_t ruleCdItemNo) const;

  virtual const std::vector<FareFocusCarrierInfo*>&
    getFareFocusCarrier(uint64_t carrierItemNo) const;

  virtual const std::vector<FareFocusFareClassInfo*>&
    getFareFocusFareClass(uint64_t fareClassItemNo) const;

  virtual const std::vector<FareFocusAccountCdDetailInfo*>*
    getFareFocusAccountCdDetailInfo(uint64_t accountCdItemNo) const;

  virtual const FareFocusDisplayCatTypeInfo*
    getFareFocusDisplayCatType(uint64_t displayCatTypeitemNo) const;

  virtual const std::vector<FareFocusLocationPairDetailInfo*>*
    getExcludeLocations(const uint64_t& locationPairExcludeItemNo) const;

  bool isFareRetailerRuleMatch(const PaxTypeFare& ptFare,
                               const FareRetailerRuleInfo* frri,
                               const FareFocusSecurityInfo* ffsi) const;
  bool matchVendor(const VendorCode& vendorRule, const VendorCode& vendorFare) const;
  bool matchRuleTariff(const TariffNumber& tariffNumberRule,
                       const TariffNumber& tariffNumberFare) const;

  bool matchFareType(const FareTypeAbbrevC& fareTypeRule, const FareType& fareTypeFare) const;

  bool matchExcludeGeo(const uint64_t& locationPairExcludeItemNo,
                       const PaxTypeFare& paxTypeFare) const;

  bool matchTravelRangeX5(PricingTrx& trx,
                          uint64_t dayTimeApplItemNo,
                          const PaxTypeFare& ptf,
                          DateTime adjustedTicketDate) const;

  bool matchGeo(const LocKey& loc1,
                const LocKey& loc2,
                const PaxTypeFare& ptf,
                bool matchingExclude) const;

  bool matchDirectionality(const FareRetailerRuleInfo& frri, const PaxTypeFare& paxTypeFare) const;
  bool matchRule(const FareRetailerRuleInfo& frri, const RuleNumber& ruleNumberFare) const;
  bool matchAccountCode(const uint64_t& accountCdItemNo, const PaxTypeFare& paxTypeFare) const;
  bool matchPassengerTypeCode(const uint64_t& psgTypeItemNo, const PaxTypeFare& paxTypeFare) const;
  bool matchCarriers(const FareRetailerRuleInfo& frri, const PaxTypeFare& paxTypeFare) const;
  bool matchSecurity(const FareFocusSecurityInfo* ffsi) const;
  bool matchFareClass(const uint64_t& fareClassItemNo,
                      const PaxTypeFare& ptf,
                      bool matchingExclude = false) const;
  bool matchLocation(const LocKey& loc, const LocCode& market, const PaxTypeFare& ptf) const;
  bool matchExcludeFareDisplayCatType(const uint64_t& displayCatTypeitemNo, const PaxTypeFare& paxTypeFare) const;
  bool matchPublicPrivateInd(const Indicator publicPrivateIndrule, const TariffCategory publicIndFare) const;
  AccountCode getAccountCode(const PaxTypeFare& paxTypeFare) const;

  bool isValidFareDirectionality(const PaxTypeFare& paxTypeFare,
                                 const FareRetailerRuleInfo& frri) const;
  bool validGeoType(const LocTypeCode lType) const;
  bool matchRetailerCode(const FareRetailerCode& frri) const;

  PricingTrx& _trx;
  DateTime _adjustedTicketDate;

  Diag868Collector* _diag;
  static Logger _logger;

private:

  FareRetailerRuleValidator(const FareRetailerRuleValidator& rhs);
  FareRetailerRuleValidator& operator=(const FareRetailerRuleValidator& rhs);

};
} // tse namespace



