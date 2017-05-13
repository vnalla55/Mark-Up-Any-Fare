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

#include "AtpcoTaxes/Rules/XmlParsingError.h"
#include "AtpcoTaxes/Common/AtpcoTaxesActivationStatus.h"
#include "AtpcoTaxes/ServiceInterfaces/AtpcoDataError.h"
#include "TestServer/Facades/TaxStringTestProcessor.h"
#include "TestServer/Xform/NaturalXmlTagsList.h"
#include "TestServer/Xform/Xml2TagsList.h"
#include "TestServer/Xform/TestCacheBuilder.h"

#include <iostream>

using namespace tax;

TaxStringTestProcessor::TaxStringTestProcessor()
  : _requestBuilder(_xmlTagsFactory), _processor(_services), _xmlTagsList(0)
{
}

TaxStringTestProcessor&
TaxStringTestProcessor::processString(const std::string& xmlString)
{
  _xmlTagsFactory.registerList(new NaturalXmlTagsList);
  _xmlTagsFactory.registerList(new Xml2TagsList);

  Request* request = 0;
  try
  {
    _requestBuilder.buildRequest(xmlString);
    TestCacheBuilder().buildCache(_services, _requestBuilder.getXmlCache());

    AtpcoTaxesActivationStatus activationStatus;
    activationStatus.setAllEnabled();
    request = &_requestBuilder.getRequest();
    _processor.run(_requestBuilder.getRequest(), activationStatus);
  }
  catch (const tax::AtpcoDataError& ex)
  {
    std::cout << "ATPCO DATA ERROR: " << ex.what() << std::endl;
    _processor.response()._error = ErrorMessage("FOUND INCOMPLETE TAX DATA", ex.what());
  }
  catch (std::domain_error const& e) // we won't handle this request
  {
    std::cout << "Domain error: " << e.what() << std::endl;
    return *this;
  }
  catch (const XmlParsingError& e) // we can't handle this request
  {
    std::cout << "TaxStringTestProcessor::XmlParsingError: " << e.what() << std::endl;
    _processor.response()._error = ErrorMessage{e.what(), ""};
  }
  catch (std::exception const& e) // probably programming error
  {
    std::cout << "Exception: " << e.what() << std::endl;
    _processor.response()._error = ErrorMessage{"PROCESSING ERROR", e.what()};
  }

  _xmlTagsList = _requestBuilder.getTagsList();

  if (!_xmlTagsList) // defensive
  {
    std::cout << "Logic error: got to building response w/o any tags list" << std::endl;
    return *this;
  }
  std::ostringstream response;
  _responseBuilder.buildFrom(request,
                             _processor.response(),
                             *_xmlTagsList,
                             _processor.detailsLevel(),
                             response);
  _responseMessage = response.str();

  return *this;
}
