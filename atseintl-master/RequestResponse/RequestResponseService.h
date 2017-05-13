/**
 *  Copyright Sabre 2005
 *
 *          The copyright to the computer program(s) herein
 *          is the property of Sabre.
 *          The program(s) may be used and/or copied only with
 *          the written permission of Sabre or in accordance
 *          with the terms and conditions stipulated in the
 *          agreement/contract under which the program(s)
 *          have been supplied.
 *
 */

#pragma once

#include "ClientSocket/ClientSocket.h"
#include "Common/XMLConstruct.h"
#include "Service/Service.h"

namespace tse
{
class Billing;
class PricingRequest;
class RequestResponseService;
class PricingTrx;
class FareDisplayTrx;
class TaxTrx;
class ShoppingTrx;
class AltPricingTrx;
class NoPNRPricingTrx;
class CurrencyTrx;
class MileageTrx;
class TicketingCxrTrx;
class TicketingCxrDisplayTrx;
class Agent;
class RexPricingTrx;
class RefundPricingTrx;
class ExchangePricingTrx;
class FlightFinderTrx;
class MultiExchangeTrx;
class PricingDetailTrx;
class RexShoppingTrx;
class RexExchangeTrx;
class AncillaryPricingTrx;
class BaggageTrx;
class TktFeesPricingTrx;
class ConfigMan;
class BrandingTrx;
class SettlementTypesTrx;
class DecodeTrx;
class StructuredRuleTrx;
class FrequentFlyerTrx;

class RequestResponseService final : public Service
{
public:
  RequestResponseService(const std::string& name, tse::TseServer& server);

  bool initialize(int argc = 0, char* argv[] = nullptr) override { return true; }

  virtual bool process(PricingTrx& trx) override;
  virtual bool process(FareDisplayTrx& trx) override;
  virtual bool process(ShoppingTrx& trx) override;
  virtual bool process(TaxTrx& trx) override;
  virtual bool process(AltPricingTrx& trx) override;
  virtual bool process(NoPNRPricingTrx& trx) override;
  virtual bool process(CurrencyTrx& trx) override;
  virtual bool process(MileageTrx& trx) override;
  virtual bool process(TicketingCxrTrx& trx) override;
  virtual bool process(TicketingCxrDisplayTrx& trx) override;
  virtual bool process(RexPricingTrx& trx) override;
  virtual bool process(ExchangePricingTrx& trx) override;
  virtual bool process(FlightFinderTrx& trx) override;
  virtual bool process(MultiExchangeTrx& trx) override;
  virtual bool process(PricingDetailTrx& trx) override;
  virtual bool process(RexShoppingTrx& trx) override;
  virtual bool process(RexExchangeTrx& trx) override;
  virtual bool process(RefundPricingTrx& trx) override;
  virtual bool process(AncillaryPricingTrx& trx) override;
  virtual bool process(BaggageTrx& trx) override;
  virtual bool process(TktFeesPricingTrx& trx) override;
  virtual bool process(BrandingTrx& trx) override;
  virtual bool process(SettlementTypesTrx& trx) override;
  virtual bool process(DecodeTrx& trx) override;
  virtual bool process(StructuredRuleTrx& trx) override;
  virtual bool process(FrequentFlyerTrx& trx) override { return true; }

  friend class RequestResponseServiceTest;

private:
  bool
  processRequestResponse(Trx& trx, const Billing& billing, const PricingRequest& request) const;

  bool send(const std::string& message, Trx& trx) const;

  std::string getResponse(PricingTrx& trx, tse::ConfigMan& config) const;

  std::string buildRequest(const Trx& trx,
                           const Billing& billing,
                           const PricingRequest& request,
                           const Agent& agent) const;
  std::string buildResponse(const Trx& trx, const Billing& billing) const;
  std::string buildReqResp(const Trx& trx,
                           const Billing& billing,
                           const PricingRequest& request,
                           const Agent* agent) const;

  std::string
  buildReqResp(const MultiExchangeTrx& trx, const Billing& billing, const Agent* agent) const;

  void buildReqRespTrxPart(XMLConstruct& construct,
                           const Trx& trx,
                           const Billing& billing,
                           const Agent* agent,
                           std::string& startTime,
                           std::string& stopTime) const;

  std::string addGeoTravelType(const Trx& trx) const;

  std::string _nodeId;
  std::string _groupName;
};

} // namespace tse
