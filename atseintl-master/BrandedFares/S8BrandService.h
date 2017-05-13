// -------------------------------------------------------------------------
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//

#pragma once

#include "Common/IAIbfUtils.h"
#include "Common/MetricsMan.h"
#include "Service/Service.h"

namespace tse
{
class ConfigMan;
class Trx;
class PricingTrx;
class RexExchangeTrx;
class FareDisplayTrx;
class RexPricingTrx;
class PaxType;
class FareMarket;
class BrandingRequestResponseHandler;

class S8BrandService : public Service
{
  friend class S8BrandServiceTest;

public:
  S8BrandService(const std::string& name, TseServer& srv);

  S8BrandService(const S8BrandService& rhs) = delete;
  S8BrandService& operator=(const S8BrandService& rhs) = delete;

  bool initialize(int argc, char* argv[]) override;

  bool process(MetricsTrx& trx) override;
  bool process(PricingTrx& trx) override;
  bool process(RexExchangeTrx& trx) override;
  bool process(FareDisplayTrx& trx) override;
  bool process(RexPricingTrx& trx) override;
  bool process(ExchangePricingTrx& trx) override;
  void printBFErrorCodeFDDiagnostic(FareDisplayTrx& trx);

  void processImpl(PricingTrx& trx, const std::vector<FareMarket*>& fareMarkets);

private:
  void getPaxTypes(PricingTrx& trx, std::vector<PaxTypeCode>& paxTypes);
  void collectCarriers(PricingTrx& trx, std::set<CarrierCode>& cxrSet);
  void collectPtcCodes(std::set<CarrierCode>& cxrSet,
                       PaxType& curPaxType,
                       std::vector<PaxTypeCode>& paxTypes);
  void collectPaxTypes(const CarrierCode& carrier,
                       const PaxType& paxType,
                       std::vector<PaxTypeCode>& paxTypes);
  void buildBrandingRequest(const std::vector<FareMarket*>& markets,
                            PricingTrx& trx,
                            BrandingRequestResponseHandler& bRRH,
                            IAIbfUtils::FMsForBranding& fMsForBranding);
  void buildASBrandingRequest(const std::vector<FareMarket*>& markets,
                              PricingTrx& trx,
                              BrandingRequestResponseHandler& bRRH,
                              IAIbfUtils::FMsForBranding& fMsForBranding,
                              const BrandRetrievalMode mode,
                              const IAIbfUtils::TripType tripType,
                              bool& haveYYFmsBeenIgnored);
  void getMarketsToFillWithBrands(const std::vector<FareMarket*>& allMarkets,
                                  std::vector<FareMarket*>& marketsToFill);

  void getBrandsForFMs(PricingTrx& trx, std::vector<FareMarket*>& fareMarkets);
  void collectAllFMsFromNewItins(PricingTrx& trx, std::vector<FareMarket*>& fareMarkets);

  virtual void validateBrandsAndPrograms(PricingTrx& trx, const std::vector<FareMarket*>& markets) const;

  ConfigMan& _config;
};
}
