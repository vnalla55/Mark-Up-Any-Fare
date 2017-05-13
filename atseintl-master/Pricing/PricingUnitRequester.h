#pragma once

#include "Common/ErrorResponseException.h"
#include "Common/Thread/TseCallableTrxTask.h"
#include "Common/TsePrimitiveTypes.h"
#include "Pricing/PricingUnitFactory.h"
#include "Pricing/PricingUnitRequesterData.h"

#include <cassert>
#include <string>
#include <vector>

#include <stdint.h>

namespace tse
{

class PUPQItem;
class DiagCollector;

class PricingUnitRequester
{
  template <class Derived>
  struct RequestBase : public TseCallableTrxTask
  {
    RequestBase() : errorResponseCode(ErrorResponseException::NO_ERROR) { desc("GET PU TASK"); }

    void runRequest();

    virtual void performTask() override { runRequest(); }

    void rethrow()
    {
      if (errorResponseCode != ErrorResponseException::NO_ERROR)
      {
        throw ErrorResponseException(errorResponseCode, errorResponseMsg.c_str());
      }
    }

    ErrorResponseException::ErrorResponseCode errorResponseCode;
    std::string errorResponseMsg;
  };

  struct Request : public RequestBase<Request>
  {
    Request()
      : factory(nullptr),
        index(0),
        puDelta(-1.0),
        diagCollector(nullptr),
        pupqItem(nullptr),
        getPUPQItemCounter(0),
        isPaused(false)
    {
    }

    Request(PricingUnitFactory& pFactory,
            uint32_t pIndex,
            MoneyAmount pPuDelta,
            DiagCollector& pDiagCollector)
      : factory(&pFactory),
        index(pIndex),
        puDelta(pPuDelta),
        diagCollector(&pDiagCollector),
        pupqItem(nullptr),
        getPUPQItemCounter(0),
        isPaused(false)
    {
    }

    void runRequestImpl();

    PricingUnitFactory* factory;
    uint32_t index;
    MoneyAmount puDelta;
    DiagCollector* diagCollector;
    PUPQItem* pupqItem;
    unsigned getPUPQItemCounter;
    bool isPaused;
  };

  typedef std::vector<Request> Requests;

  struct ClearRequest : public RequestBase<ClearRequest>
  {
    ClearRequest() : factory(nullptr), diagCollector(nullptr) {}

    ClearRequest(PricingUnitFactory& pFactory, DiagCollector& pDiagCollector)
      : factory(&pFactory), diagCollector(&pDiagCollector)
    {
    }

    void runRequestImpl();

    PricingUnitFactory* factory;
    DiagCollector* diagCollector;
  };

  typedef std::vector<ClearRequest> ClearRequests;

  typedef std::vector<PricingUnitFactory*> Factories;

public:
  explicit PricingUnitRequester() {}

  void initialize(PricingTrx& trx);

  void registerRequest(PricingUnitFactory& factory, uint32_t index, MoneyAmount puDelta);
  void processRequests(DiagCollector& diagCollector) { runRequests(diagCollector, _requests); }

  PUPQItem* getRequestedPUPQItem(PricingUnitFactory& factory,
                                 uint32_t index,
                                 DiagCollector& diagCollector) const;

  void clearProcessing() { _requests.clear(); }

  void clearFactoryForRex(const Factories& factories, DiagCollector& diagCollector);

private:
  template <class RequestArray>
  void runRequests(DiagCollector& diagCollector, RequestArray& requests);
  DiagCollector& createDiagCollector();

private:
  PricingUnitRequester(const PricingUnitRequester&);
  void operator=(const PricingUnitRequester&);

private:
  PricingTrx* _trx = nullptr;
  Requests _requests;
  DiagCollector* _noneDiagCollector = nullptr;
};

inline void
PricingUnitRequester::registerRequest(PricingUnitFactory& factory,
                                      uint32_t index,
                                      MoneyAmount puDelta)
{
  assert(factory.requesterData().status() == PricingUnitRequesterData::S_READY);

  if (index >= factory.sizeAlreadyBuiltPUPQItem())
  {
    DiagCollector& diagnostic = createDiagCollector();
    Request request(factory, index, puDelta, diagnostic);
    request.trx(_trx);
    _requests.push_back(request);
  }
}

inline PUPQItem*
PricingUnitRequester::getRequestedPUPQItem(PricingUnitFactory& factory,
                                           uint32_t index,
                                           DiagCollector& diagCollector) const
{
  assert(factory.requesterData().status() == PricingUnitRequesterData::S_READY);

  PUPQItem* pupqItem = factory.getAlreadyBuiltPUPQItem(index);
  if (!pupqItem)
  {
    for (const auto& request : _requests)
    {
      if (request.factory == &factory && request.index == index)
      {
        if (request.isPaused && request.getPUPQItemCounter != factory.getPUPQItemCounter())
        {
          // get "fresh" PUPQItem object because
          // any getPUPQItem calls between processing request and calling this
          // method may release paused-PUPQItem object.
          pupqItem = factory.getPUPQItem(request.index, diagCollector);
        }
        else
        {
          pupqItem = request.pupqItem;
        }
        break;
      }
    }
  }

  return pupqItem;
}

} // namespace tse

