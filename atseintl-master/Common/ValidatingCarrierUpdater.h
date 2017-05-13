//----------------------------------------------------------------------------
//
//  Copyright Sabre 2011
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

#include "Common/TrxUtil.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStlTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/ValidatingCxrConst.h"

#include <map>
#include <set>
#include <vector>

namespace tse
{
class AirSeg;
class Diag191Collector;
class FarePath;
class Itin;
class PricingTrx;
class TravelSeg;

class ValidatingCarrierUpdater
{
  friend class ValidatingCarrierUpdaterTest;

public:
  typedef std::vector<const AirSeg*> AirSegmentVec;
  typedef CarrierCode (ValidatingCarrierUpdater::*GetSubAreaCarrier)(const Itin& itin) const;

  explicit ValidatingCarrierUpdater(PricingTrx& trx);

  void update(Itin& itin, bool isSopProcessing = false, const SopIdVec* sops = nullptr);
  void update(FarePath& farePath) const;
  void legacyProcess(Itin& itin) const;
  CarrierCode determineValidatingCarrierLegacy(const Itin &itin) const;
  void determineTraditionalValidatingCarrier(Itin &itin) const;
  bool processSops(ShoppingTrx& shoppingTrx, const SopIdVec& sops);
  void updateFlightFinderValidatingCarrier();
  bool determineDefaultValidatingCarrier(Itin& itin,
                                          std::vector<CarrierCode>& validatingCxrs,
                                          CarrierCode& defaultValidatingCxr,
                                          CarrierCode& defaultMarketingCxr) const;

  void validatigCarrierOverrideErrorMsg(std::vector<CarrierCode>& validatingCxrs,
    Diag191Collector* diag191) const;
  CarrierCode find(const AirSegmentVec& segments) const;
  void getSegsFromCarrier(Itin& itin,
                          std::vector<CarrierCode>& marketingCxrs,
                          AirSegmentVec& segments) const;
  bool findDefValCxrFromPreferred(const std::vector<CarrierCode>& valCxrs, const std::vector<CarrierCode>& prefValCxrs, CarrierCode& defValCxr) const;
protected:
  bool updateByOverride(Itin& itin) const;
  const CarrierCode* determineValidatingCarrier(const Itin &itin) const;
  bool isGTCProcessing(Itin& itin, Diag191Collector* diag191, bool& ignoreItin) const;

  void process(Itin& itin) const;
  bool findCarrierBetweenAreas(const AirSegmentVec& segments, CarrierCode& carrier) const;
  bool findCarrierBetweenSubAreas(const AirSegmentVec& segments, CarrierCode& carrier) const;
  bool findCarrierBetweenCountries(const AirSegmentVec& segments, CarrierCode& carrier) const;
  void filterSegments(const std::vector<TravelSeg*>& travelSegs, AirSegmentVec& segments) const;

  void addNSPInCspi();
  void printNSPDiag191Info(Diag191Collector* diag191);
  void updateValidatingCxrList(Itin& itin,
                               Diag191Collector* diag191,
                               bool isSopProcessing = false,
                               const SopIdVec* sops = nullptr);
  void updateDefaultValidatingCarrier(Itin& itin) const;

  void excludeGTCCarriers(std::vector<CarrierCode>& marketingCxrs,
                          std::set<CarrierCode>& gtcCarriers);

  void setFlightFinderDefaultValidatingCarrierOld(FlightFinderTrx& trx,
                                                  Diag191Collector* diag191,
                                                  ValidatingCxrGSAData* valCxrGsaData,
                                                  const std::set<CarrierCode>& gtcCarriers);

  void setFlightFinderDefaultValidatingCarrier(FlightFinderTrx& trx,
                                               Diag191Collector* diag191,
                                               SpValidatingCxrGSADataMap* spGsaDataMap);

  void setFlightFinderDefaultValidatingCarrierBase(FlightFinderTrx& trx,
                                                   Diag191Collector* diag191,
                                                   const std::vector<SettlementPlanType>& smPlans,
                                                   SpValidatingCxrGSADataMap* spGsaDataMap);

  bool setFlightFinderItinValidatingCarrierOld(Itin& itin,
                                               Diag191Collector* diag191,
                                               ValidatingCxrGSAData* valCxrGsaData,
                                               const uint32_t sopId,
                                               const CarrierCode& carrierCode);

  bool setFlightFinderItinValidatingCarrier(Itin& itin,
                                            Diag191Collector* diag191,
                                            const SettlementPlanType stmlPlan,
                                            SpValidatingCxrGSADataMap* spGsaDataMap,
                                            const uint32_t sopId,
                                            const CarrierCode& carrierCode);

  bool setDefaultCarriers(Itin& itin,
                          std::vector<CarrierCode>& validatingCxrs,
                          CarrierCode& marketingCxr,
                          CarrierCode& defaultValidatingCxr,
                          CarrierCode& defaultMarketingCxr) const;

  void resetForSingleSettlementPlan(Itin& itin, const SpValidatingCxrGSADataMap& spGsaDataMap);

  bool setDefaultValidatingCxrForCommandPricing(Itin& itin,
                                                const SettlementPlanType& primarySp,
                                                CarrierCode& defaultValidatingCxr,
                                                CarrierCode& defaultMarketingCxr) const;

  DiagCollector& diag() const;
  bool isRexWithSingleItin();

  PricingTrx& _trx;
  bool _isShopping;
  mutable DiagCollector* _diag;
};

template <typename T>
inline void
updateValidatingCarrier(PricingTrx& trx, T& elem)
{
  ValidatingCarrierUpdater updater(trx);
  updater.update(elem);
}

inline void
updateValidatingCarrier(PricingTrx& trx, Itin& itin)
{
  ValidatingCarrierUpdater updater(trx);
  updater.update(itin);
}

} // tse

