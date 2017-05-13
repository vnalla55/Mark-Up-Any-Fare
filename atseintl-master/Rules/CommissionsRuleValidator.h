//-------------------------------------------------------------------
//  Copyright Sabre 2015
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

namespace tse
{
class CommissionRuleInfo;
class Diag867Collector;
class FarePath;
class FareUsage;
class Logger;
class PricingTrx;
class TravelSeg;

class CommissionsRuleValidator
{
  friend class CommissionsRuleValidatorTest;

public:
  CommissionsRuleValidator(PricingTrx& trx,
                           const FarePath& fp,
                           const FareUsage& fu,
                           Diag867Collector* diag867)
    : _trx(trx), _fp(fp), _fu(fu), _diag867(diag867)
  {
  }

  bool isCommissionRuleMatched(const CommissionRuleInfo& commissionRule, const CarrierCode& valCxr);

protected:

  CommissionValidationStatus
  validateCommissionRule(const CommissionRuleInfo& cri,
                         const CarrierCode& valCxr) const;
  bool matchFareBasis(const CommissionRuleInfo& cri,
                      CommissionValidationStatus& rc) const;
  bool matchClassOfService(const CommissionRuleInfo& cri,
                           CommissionValidationStatus& rc) const;

  bool matchCabin(const CommissionRuleInfo& cri,
                  CommissionValidationStatus& rc) const;
  bool matchRequiredNonStop(const CommissionRuleInfo& cri) const;
  bool matchPassengerType(const CommissionRuleInfo& cri) const;
  bool matchOperatingCarrier(const CommissionRuleInfo& cri,
                             CommissionValidationStatus& rc) const;
  bool matchCarrierExclIncl(const std::vector<CarrierCode>& operV, bool cxr, bool excl=false) const;
  bool matchRoundTrip(const CommissionRuleInfo& cri) const;
  bool matchMarketingCarrier(const CommissionRuleInfo& cri,
                             CommissionValidationStatus& rc) const;
  bool matchTicketingCarrier(const CommissionRuleInfo& cri,
                             CommissionValidationStatus& rc,
                             const CarrierCode& valCxr) const;
  bool matchInterlineConnection(const CommissionRuleInfo& cri) const;
  bool matchFareBasisFragment(const CommissionRuleInfo& cri,
                              CommissionValidationStatus& rc) const;
  bool matchFareBasisFragmentExclIncl(const std::vector<std::string>& fragmV) const;
  bool matchTktDesignator(const CommissionRuleInfo& cri,
                          CommissionValidationStatus& rc) const;
  bool matchTktDesignatorFragmentExclIncl(const std::vector<TktDesignator>& excReqTktDesig) const;
  bool matchString(const std::vector<TktDesignator>& tdCol, const std::string& td) const;
  bool matchString(const std::vector<std::string>& fbCol, const std::string& fb) const;
  bool matchString(const std::string& s, const std::string& x) const;
  virtual bool matchFragment(const std::string& fragment,const std::string& target) const;
  bool matchConnectionAirports(const CommissionRuleInfo& cri,
                               CommissionValidationStatus& rc) const;
  bool matchConnections(const std::vector<LocCode>& connectV, bool exclude=false) const;
  bool matchMarketingGovCarrier(const CommissionRuleInfo& cri,
                                CommissionValidationStatus& rc) const;
  bool matchGoverningCarrier(const std::vector<CarrierCode>& operV, bool cxr=false) const;
  bool matchOperGovCarrier(const CommissionRuleInfo& cri,
                           CommissionValidationStatus& rc) const;

  virtual void getPaxTypes();
  virtual void getFareBasis();
  virtual void getClassOfService();
  const TravelSeg* getPrimarySector(uint16_t& indexTvlSeg);
  virtual bool matchFareBasisFragment(const std::string& fbrFragment) const;
  virtual bool matchSubFragment(const std::string& fragment,
                                         std::size_t& nextWild,
                                         std::size_t& found,
                                         bool& validateFragment,
                                         const std::string& wildcard) const;
  virtual bool matchFareBasisSubFragment(const std::string& fbrFragment,
                                         std::size_t& nextWild,
                                         std::size_t& found,
                                         bool& validateFragment) const;

  PricingTrx& _trx;
  const FarePath& _fp;
  const FareUsage& _fu;
  Diag867Collector* _diag867;
  static Logger _logger;
  std::string _fareBasis;
  TktDesignator _tktDesignator;
  BookingCode _rbd;
  PaxTypeCode _truePaxType;

private:
  CommissionsRuleValidator(const CommissionsRuleValidator& rhs);
  CommissionsRuleValidator& operator=(const CommissionsRuleValidator& rhs);

};
} // tse namespace

