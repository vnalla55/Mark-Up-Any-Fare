#pragma once

#include "Common/LocUtil.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"

#include <vector>

namespace tse
{
class AirSeg;

using AirSegmentVec = std::vector<const AirSeg*>;

class DefaultValidatingCarrierFinder
{
  friend class DefaultValidatingCarrierFinderTest;

public:
  DefaultValidatingCarrierFinder(PricingTrx& trx, Itin& itin, SettlementPlanType sp = "");

  bool determineDefaultValidatingCarrier(const std::vector<CarrierCode>& valCxrs,
                                         CarrierCode& defValcxr,
                                         CarrierCode& defMarketingCxr) const;

  const SettlementPlanType& settlementPlan() const { return _sp; }
  SettlementPlanType& settlementPlan() { return _sp; }
  bool findDefValCxrFromPreferred(const std::vector<CarrierCode>& valCxrs, const std::vector<CarrierCode>& prefValCxrs, CarrierCode& defValCxr) const;

private:
  bool checkNeutralValCxr(const std::vector<CarrierCode>& valCxrs, CarrierCode& defValCxr) const;

  bool isNoGsaSwapWithSingleValidatingCxr(const std::vector<CarrierCode>& valCxrs) const
  {
    return (1 == valCxrs.size() && _itin.gsaSwapMap(_sp).empty());
  }

  bool isValidatingCxrOverride() const { return (!_trx.getRequest()->validatingCarrier().empty()); }

  void
  getValidatingCxrsForMarketingCxr(const CarrierCode& mktCxr, std::set<CarrierCode>& valCxrs) const;
  void getMarketingCxrsForValidatingCxrs(const std::vector<CarrierCode>& valCxrs,
                                         std::vector<CarrierCode>& marketingCxrs) const;
  bool setDefaultCarriers(const std::vector<CarrierCode>& valCxrs,
                          CarrierCode& marketingCxr,
                          CarrierCode& defValCxr,
                          CarrierCode& defMktCxr) const;
  bool findDefaultBasedOnIATA(CarrierCode& marketingCxr,
                              CarrierCode& defValCxr,
                              CarrierCode& defMktCxr,
                              const std::vector<CarrierCode>& valCxrs,
                              std::vector<CarrierCode>& marketingCxrs) const;

  void getSegsFromCarrier(std::vector<CarrierCode>& marketingCxrs, AirSegmentVec& segments) const;
  CarrierCode findDefaultValidatingCxr(const AirSegmentVec& segments) const;
  bool findCarrierBetweenAreas(const AirSegmentVec& segments, CarrierCode& carrier) const;
  bool findCarrierBetweenCountries(const AirSegmentVec& segments, CarrierCode& carrier) const;
  bool findCarrierBetweenSubAreas(const AirSegmentVec& segments, CarrierCode& carrier) const;

private:
  PricingTrx& _trx;
  Itin& _itin;
  SettlementPlanType _sp;
};
}
