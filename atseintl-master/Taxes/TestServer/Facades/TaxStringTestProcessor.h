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
#pragma once

#include <string>

#include "AtpcoTaxes/ServiceInterfaces/DefaultServices.h"
#include "TestServer/Xform/StringToRequestBuilder.h"
#include "TestServer/Xform/ResponseToStringBuilder.h"
#include "TestServer/Xform/XmlTagsFactory.h"

namespace tax
{
class BusinessRulesProcessor;
class XmlTagsList;

class TaxStringTestProcessor
{
public:
  TaxStringTestProcessor();
  TaxStringTestProcessor& processString(const std::string& xmlString);

  std::string& getResponseMessage()
  {
    return _responseMessage;
  }

private:
  void addResponseHeader(size_t payloadSize, std::string& responseMessage);
  bool convertResponse(BusinessRulesProcessor& processor, std::string& responseMessage);
  std::string _responseMessage;
  StringToRequestBuilder _requestBuilder;
  BusinessRulesProcessor _processor;
  ResponseToStringBuilder _responseBuilder;
  XmlTagsFactory _xmlTagsFactory;
  const XmlTagsList* _xmlTagsList;
  DefaultServices _services;
};

} // namespace tax
