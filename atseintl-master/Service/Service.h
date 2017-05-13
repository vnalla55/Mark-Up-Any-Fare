//----------------------------------------------------------------------------
//
//  File:               service.h
//  Description:        Generic service class to process
//                      requests from a client
//  Created:            11/16/2003
//  Authors:            Doug Steeb, Bruce Melberg
//
//  Description:        see .C module
//
//  Return types:
//
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Common/TseStringTypes.h"

#include <string>
#include <typeinfo>

namespace tse
{
class TseServer;
class Trx;
class CurrencyTrx;
class DecodeTrx;
class FareDisplayTrx;
class MetricsTrx;
class MileageTrx;
class PricingTrx;
class ShoppingTrx;
class StatusTrx;
class RepricingTrx;
class CacheTrx;
class PricingDetailTrx;
class TaxTrx;
class AltPricingTrx;
class NoPNRPricingTrx;
class RexPricingTrx;
class ExchangePricingTrx;
class FlightFinderTrx;
class MultiExchangeTrx;
class RexShoppingTrx;
class RefundPricingTrx;
class RexExchangeTrx;
class AncillaryPricingTrx;
class BaggageTrx;
class TktFeesPricingTrx;
class BrandingTrx;
class SettlementTypesTrx;
class TicketingCxrTrx;
class TicketingCxrDisplayTrx;
class StructuredRuleTrx;
class FrequentFlyerTrx;

class Service
{
protected:
  std::string _name;
  TseServer& _server;

  Service(const std::string& aName, TseServer& aServer) : _name(aName), _server(aServer) {}

  void statusLine(StatusTrx& trx, bool ok) { Service::statusLine(trx, _name, ok); }

  void statusLine(StatusTrx& trx, void* object, bool ok)
  {
    if (object == nullptr)
      return;

    std::string aName = typeid(object).name();
    Service::statusLine(trx, aName, ok);
  }

  static void statusLine(StatusTrx& trx, const std::string& name, bool ok);

private:
  bool warnMissing(Trx& trx);

public:
  virtual ~Service() = default;

  TseServer& server() { return _server; }
  const std::string& name() const { return _name; }

  // Note try not to override the Trx implementation,
  // the base class has the appropriate logic to invoke
  // the other methods and handle the errors sanely
  virtual bool process(CurrencyTrx& trx);
  virtual bool process(DecodeTrx& trx);
  virtual bool process(MetricsTrx& trx);
  virtual bool process(MileageTrx& trx);
  virtual bool process(FareDisplayTrx& trx);
  virtual bool process(PricingTrx& trx);
  virtual bool process(ShoppingTrx& trx);
  virtual bool process(RexShoppingTrx& trx);
  virtual bool process(RepricingTrx& trx);
  virtual bool process(CacheTrx& trx);
  virtual bool process(PricingDetailTrx& trx);
  virtual bool process(TaxTrx& trx);
  virtual bool process(AltPricingTrx& trx);
  virtual bool process(NoPNRPricingTrx& trx);
  virtual bool process(RexPricingTrx& trx);
  virtual bool process(ExchangePricingTrx& trx);
  virtual bool process(FlightFinderTrx& trx);
  virtual bool process(MultiExchangeTrx&)
  {
    return true;
  }
  virtual bool process(RefundPricingTrx& trx);
  virtual bool process(RexExchangeTrx& trx);
  virtual bool process(AncillaryPricingTrx& trx);
  virtual bool process(StatusTrx& trx);
  virtual bool process(BaggageTrx& trx);
  virtual bool process(TktFeesPricingTrx& trx);
  virtual bool process(BrandingTrx& trx);
  virtual bool process(SettlementTypesTrx& trx);
  virtual bool process(TicketingCxrTrx& trx);
  virtual bool process(TicketingCxrDisplayTrx& trx);
  virtual bool process(StructuredRuleTrx& trx);
  virtual bool process(FrequentFlyerTrx& trx);

  virtual bool initialize(int argc = 0, char* *argv = nullptr) = 0;
  virtual void buildSubItinVectors(PricingTrx&) {}

  virtual void postInitialize() {}
  virtual void preShutdown() {}

  virtual uint32_t getActiveThreads();
};
} /* end namespace tse */

