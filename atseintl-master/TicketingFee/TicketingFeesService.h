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
class ConfigMan;
class Itin;
class Trx;
class MetricsTrx;
class AltPricingDetailObFeesTrx;
class PricingDetailTrx;
class PricingTrx;
class ShoppingTrx;
class TktFeesPricingTrx;
// ----------------------------------------------------------------------------
//
// class TicketingFees Service
// Description:
//
// ----------------------------------------------------------------------------

class TicketingFeesService : public Service
{
  friend class TicketingFeesServiceTest;

public:
  TicketingFeesService(const std::string& name, TseServer& srv);

  bool initialize(int argc, char* argv[]) override;

  bool process(MetricsTrx& trx) override;
  bool process(PricingTrx& trx) override;
  bool process(ShoppingTrx& trx) override;
  bool process(TktFeesPricingTrx& trx) override;
  bool process(PricingDetailTrx& trx) override;
  bool process(AltPricingDetailObFeesTrx& trx);
  bool process(AltPricingTrx& trx) override;

protected:
  ConfigMan& _config;

private:
  TicketingFeesService(const TicketingFeesService& rhs);
  TicketingFeesService& operator=(const TicketingFeesService& rhs);
  bool collectTicketingFees(PricingTrx& pricingTrx) const;
  bool processTicketingFees(PricingTrx& pricingTrx);
  void createPseudoPricingSolutions(TktFeesPricingTrx& trx);
  void setTimeOut(TktFeesPricingTrx& trx) const;
  bool processOB(TktFeesPricingTrx& trx);
  bool processTicketingFeesOB(TktFeesPricingTrx& trx);
  void printOBNotRequested(PricingTrx& trx);
  void setDataInRequest(TktFeesPricingTrx& trx, const Itin* itin);
  void setFOPBinNumber(TktFeesPricingTrx& trx, const Itin* itin);
};
}
