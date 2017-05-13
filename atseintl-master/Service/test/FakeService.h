#ifndef FAKE_SERVICE_H
#define FAKE_SERVICE_H

#include "Service/Service.h"

namespace tse
{

class FakeService : public Service
{
public:
  bool _procResult;
  FakeService(const std::string& name, TseServer& server, bool rc = true)
    : Service(name, server), _procResult(rc), _ran(false)
  {
  }

  bool initialize(int argc, char** argv) { return true; }
  bool process(Trx& trx) { return go(); }
  bool process(CurrencyTrx& trx) { return go(); }
  bool process(MetricsTrx& trx) { return go(); }
  bool process(MileageTrx& trx) { return go(); }
  bool process(FareDisplayTrx& trx) { return go(); }
  bool process(PricingTrx& trx) { return go(); }
  bool process(ShoppingTrx& trx) { return go(); }
  bool process(RexShoppingTrx& trx) { return go(); }
  bool process(RepricingTrx& trx) { return go(); }
  bool process(CacheTrx& trx) { return go(); }
  bool process(PricingDetailTrx& trx) { return go(); }
  bool process(TaxTrx& trx) { return go(); }
  bool process(AltPricingTrx& trx) { return go(); }
  bool process(NoPNRPricingTrx& trx) { return go(); }
  bool process(RexPricingTrx& trx) { return go(); }
  bool process(ExchangePricingTrx& trx) { return go(); }
  bool process(FlightFinderTrx& trx) { return go(); }
  bool process(RefundPricingTrx& trx) { return go(); }

  virtual bool go()
  {
    _ran = true;
    return _procResult;
  }

  bool _ran;
};

class FakeServiceThrowEREx : public FakeService
{
public:
  FakeServiceThrowEREx(const std::string& name, TseServer& server, const ErrorResponseException& e)
    : FakeService(name, server), _ex(e)
  {
  }
  virtual bool go() { throw _ex; }
  ErrorResponseException _ex;
};

class FakeServiceThrowStdEx : public FakeService
{
public:
  FakeServiceThrowStdEx(const std::string& name, TseServer& server) : FakeService(name, server) {}
  virtual bool go() { throw std::bad_cast(); }
};

class FakeServiceThrowOtherEx : public FakeService
{
public:
  static const int ERROR_CODE = 666;

  FakeServiceThrowOtherEx(const std::string& name, TseServer& server) : FakeService(name, server) {}
  virtual bool go() { throw ERROR_CODE; }
};

} // end of tse

#endif
