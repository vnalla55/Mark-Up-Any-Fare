// ----------------------------------------------------------------------------
//
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
// ----------------------------------------------------------------------------


#include "Common/Logger.h"
#include "Common/FallbackUtil.h"
#include "DBAccess/DataHandle.h"
#include "AtpcoTaxes/Rules/XmlParsingError.h"
#include "Dispatcher/AtpcoTaxCalculator.h"
#include "Dispatcher/TaxStringV2Processor.h"
#include "Dispatcher/RequestToV2CacheBuilder.h"
#include "TestServer/Xform/NaturalXmlTagsList.h"
#include "TestServer/Xform/TestCacheBuilder.h"
#include "TestServer/Xform/Xml2TagsList.h"
#include "AtpcoTaxes/Processor/RequestAnalyzer.h"
#include "AtpcoTaxes/Rules/RequestLogicError.h"
#include "AtpcoTaxes/ServiceInterfaces/AtpcoDataError.h"
#include "AtpcoTaxes/ServiceInterfaces/CurrencyService.h"

#include <stdexcept>
#include <string>

namespace tse
{
FIXEDFALLBACK_DECL(atpcoBchParallelProcessing);
}

namespace tax
{

namespace
{

tse::Logger logger ("atseintl.Taxes");

} // anonymous namespace

TaxStringV2Processor::TaxStringV2Processor()
  : _requestBuilder(_xmlTagsFactory),
    _processor(_services),
    _xmlTagsList(nullptr),
    _useRepricing(boost::logic::indeterminate)
{
}

TaxStringV2Processor&
TaxStringV2Processor::processString(const std::string& xmlString, std::ostream& responseStream)
{
  _xmlTagsFactory.registerList(new NaturalXmlTagsList);
  _xmlTagsFactory.registerList(new Xml2TagsList);

  Request* request = nullptr;
  tse::DataHandle dataHandle;
  try
  {
    _requestBuilder.buildRequest(xmlString);
    request = &_requestBuilder.getRequest();
    RequestToV2CacheBuilder().buildCache(
        _services, _requestBuilder.getXmlCache(), *request, dataHandle);

    AtpcoTaxesActivationStatus activationStatus;
    if (!boost::logic::indeterminate(_useRepricing))
    {
      request->processing().setUseRepricing(_useRepricing);
    }

    if (!tse::fallback::fixed::atpcoBchParallelProcessing())
    {
      activationStatus.setTaxOnItinYqYrTaxOnTax(true);
      activationStatus.setOldTaxesCalculated(false);
      activationStatus.setTaxOnOC(false);
      activationStatus.setTaxOnBaggage(false);
      activationStatus.setTaxOnChangeFee(false);

      RequestAnalyzer(*request, _services).analyze();
      request->processing().setProcessingGroups({type::ProcessingGroup::Itinerary});

      if (request->diagnostic().number() != 0 || request->optionalServices().size() != 0)
      {
        _processor.run(*request, activationStatus);
      }
      else
      {
        bool threadInterrupted = false;
        try
        {
          ProcessingOrderer orderer;
          TaxValues lastGroup;
          OrderedTaxes orderedTaxes;
          ItinsRawPayments itinsRawPayments(request->itins().size());
          ItinsPayments itinsPayments;
          itinsPayments.resize(request->itins().size());
          BusinessRulesProcessorUtils::getOrderedTaxes(*request, _services, orderer, lastGroup, orderedTaxes);

          AtpcoTaxCalculator atp(tse::TseThreadingConst::TAX_TASK,
                                 request->itins().size(),
                                 orderedTaxes,
                                 itinsRawPayments,
                                 itinsPayments,
                                 *request,
                                 activationStatus);

          try
          {
            for (size_t i = 0 ; i < request->itins().size(); ++i)
            {
              atp.calculateTaxes(i);
            }

            atp.wait();
          }
          catch (boost::thread_interrupted& e)
          {
            threadInterrupted = true;
            _processor.run(*request, activationStatus);
          }

          if (!threadInterrupted)
          {
            boost::swap(itinsPayments._itinsRawPayments, itinsRawPayments);
            itinsPayments.paymentCurrency = request->ticketingOptions().paymentCurrency();
            itinsPayments.paymentCurrencyNoDec =
                _services.currencyService().getCurrencyDecimals(itinsPayments.paymentCurrency);

            _processor.response()._itinsPayments.emplace(std::move(itinsPayments));
          }
        }
        catch (RequestLogicError const& e)
        {
          LOG4CXX_ERROR(logger, "RequestLogicError: " << e.what());
          _processor.response()._error = ErrorMessage();
          _processor.response()._error->_content = e.what();
        }
      }
    }
    else
    {
      activationStatus.setAllEnabled();
      _processor.run(*request, activationStatus);
    }
  }
  catch (const tax::AtpcoDataError& ex)
  {
    LOG4CXX_ERROR(logger, "ATPCO DATA ERROR: " << ex.what());
    _processor.response()._error = ErrorMessage("FOUND INCOMPLETE TAX DATA", ex.what());
  }
  catch (const std::domain_error& e) // we won't handle this request
  {
    LOG4CXX_ERROR(logger, "Domain error: " << e.what());
    return *this;
  }
  catch (const XmlParsingError& e) // we can't handle this request
  {
    LOG4CXX_ERROR(logger, "TaxStringV2Processor::XmlParsingError: " << e.what());
    _processor.response()._error = ErrorMessage(e.what(), {});
  }
  catch (const std::exception& e) // probably programming error
  {
    LOG4CXX_ERROR(logger, "Exception: " << e.what());
    _processor.response()._error = ErrorMessage("PROCESSING ERROR", e.what());
  }

  _xmlTagsList = _requestBuilder.getTagsList();

  if (!_xmlTagsList) // defensive
  {
    LOG4CXX_ERROR(logger, "Logic error: got to building response without any tags list");
    return *this;
  }

  _responseBuilder.buildFromBCH(request,
                                _processor.response(),
                                *_xmlTagsList,
                                responseStream);

  return *this;
}

}
