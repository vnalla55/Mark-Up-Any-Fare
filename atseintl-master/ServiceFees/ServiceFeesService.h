// -------------------------------------------------------------------------
//  Copyright Sabre 2009
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ---------------------------------------------------------------------------
#pragma once

#include "Common/MetricsMan.h"
#include "Service/Service.h"


namespace tse
{
class MetricsTrx;
class PricingTrx;
class AncillaryPricingTrx;
class ExchangePricingTrx;
class OptionalFeeCollector;
class ConfigMan;

// ----------------------------------------------------------------------------
//
// class ServiceFees Service
// Description:
//
// ----------------------------------------------------------------------------

class ServiceFeesService : public Service
{
  friend class ServiceFeesServiceTest;

public:
  ServiceFeesService(const std::string& name, TseServer& srv);

  virtual bool initialize(int argc, char* argv[]) override;

  virtual bool process(MetricsTrx& trx) override;
  virtual bool process(PricingTrx& trx) override;
  virtual bool process(AncillaryPricingTrx& trx) override;
  virtual bool process(ExchangePricingTrx& trx) override;

protected:
  tse::ConfigMan& _config;

private:
  ServiceFeesService(const ServiceFeesService& rhs);
  ServiceFeesService& operator=(const ServiceFeesService& rhs);

  virtual bool collectOCFee(PricingTrx& trx) const;
  virtual void processOCFee(OptionalFeeCollector& ocCollector) const;
  virtual void processAncillaryFee(OptionalFeeCollector& ancTrx) const;
  virtual void adjustTimeOut(AncillaryPricingTrx& trx) const;
  virtual bool checkTimeOut(PricingTrx& trx) const;
  virtual bool diagActive(PricingTrx& trx) const;

  void applyPriceModification(AncillaryPricingTrx& trx);
};
}
