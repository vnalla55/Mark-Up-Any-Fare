#include "Pricing/PricingUnitRequester.h"

#include "Common/Logger.h"
#include "Diagnostic/DCFactory.h"

namespace tse
{
namespace
{
Logger
PricingUnitRequesterLogger("atseintl.Pricing.PricingUnitRequester");
} // unnamed namespace

template <class Derived>
void
PricingUnitRequester::RequestBase<Derived>::runRequest()
{
  try { static_cast<Derived*>(this)->runRequestImpl(); }
  catch (ErrorResponseException& e)
  {
    LOG4CXX_ERROR(PricingUnitRequesterLogger, "Exception:" << e.message() << " - GetPU failed")
    errorResponseCode = e.code();
    errorResponseMsg = e.message();
  }
  catch (std::exception& e)
  {
    LOG4CXX_ERROR(PricingUnitRequesterLogger, "Exception:" << e.what() << " - GetPU failed")
    errorResponseCode = ErrorResponseException::SYSTEM_EXCEPTION;
    errorResponseMsg = "ATSEI SYSTEM EXCEPTION";
  }
  catch (...)
  {
    LOG4CXX_ERROR(PricingUnitRequesterLogger, "UNKNOWN Exception Caught: GetPU failed")
    errorResponseCode = ErrorResponseException::UNKNOWN_EXCEPTION;
    errorResponseMsg = "UNKNOWN EXCEPTION";
  }
}

template class PricingUnitRequester::RequestBase<PricingUnitRequester::Request>;

template class PricingUnitRequester::RequestBase<PricingUnitRequester::ClearRequest>;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
PricingUnitRequester::Request::runRequestImpl()
{
  factory->delta() = puDelta;
  pupqItem = factory->getPUPQItem(index, *diagCollector);
  getPUPQItemCounter = factory->getPUPQItemCounter();
  if (pupqItem && pupqItem->paused())
  {
    isPaused = true;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
PricingUnitRequester::ClearRequest::runRequestImpl()
{
  factory->clearFactoryForRex(*diagCollector);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
PricingUnitRequester::initialize(PricingTrx& trx)
{
  _trx = &trx;
  if (LIKELY(trx.diagnostic().diagnosticType() == DiagnosticNone))
  {
    DCFactory* dcFactory = DCFactory::instance();
    _noneDiagCollector = dcFactory->create(*_trx);
  }
}

void
PricingUnitRequester::clearFactoryForRex(const Factories& factories, DiagCollector& diagCollector)
{
  ClearRequests clearRequests;
  clearRequests.reserve(factories.size());
  for (const auto& factory : factories)
  {
    DiagCollector& diagnostic = createDiagCollector();
    ClearRequest request(*factory, diagnostic);
    request.trx(_trx);
    clearRequests.push_back(request);
  }

  runRequests(diagCollector, clearRequests);
}

template <class RequestArray>
void
PricingUnitRequester::runRequests(DiagCollector& diagCollector, RequestArray& requests)
{
  typedef typename RequestArray::value_type RequestType;
  typedef typename RequestArray::iterator request_iterator;

  assert(_trx);

  if (requests.empty())
  {
    return;
  }

  // run requests
  for (request_iterator requestIt = requests.begin(), requestEndIt = requests.end();
       requestIt != requestEndIt;
       ++requestIt)
  {
    RequestType& request = *requestIt;
    PricingUnitFactory& factory = *request.factory;
    PricingUnitRequesterData& requesterData = factory.requesterData();
    requesterData.wait();

    if (requestIt + 1 == requestEndIt)
    {
      // no need to run in another thread
      request.runRequest();
    }
    else
    {
      requesterData.setStatus(PricingUnitRequesterData::S_RUNNING);
      requesterData.executor().execute(request);
    }
  }

  // wait for finish
  for (auto& request : requests)
  {
    PricingUnitFactory& factory = *request.factory;
    PricingUnitRequesterData& requesterData = factory.requesterData();
    requesterData.wait();
    diagCollector << *request.diagCollector;
  }

  // rethrow
  for (auto& request : requests)
  {
    request.rethrow();
  }
}

template void
PricingUnitRequester::runRequests(DiagCollector& diagCollector, Requests& requests);

template void
PricingUnitRequester::runRequests(DiagCollector& diagCollector, ClearRequests& requests);

DiagCollector&
PricingUnitRequester::createDiagCollector()
{
  if (LIKELY(_noneDiagCollector))
  {
    return *_noneDiagCollector;
  }
  else
  {
    DCFactory* dcFactory = DCFactory::instance();
    DiagCollector* diagnostic = dcFactory->create(*_trx);
    if (!_requests.empty() && _requests.front().factory)
      diagnostic->enable(
          _requests.front().factory->pu(), Diagnostic603, Diagnostic601, Diagnostic605);
    else
      diagnostic->enable(Diagnostic603, Diagnostic601, Diagnostic605);

    return *diagnostic;
  }
}

} // namespace tse
