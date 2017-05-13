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

#include "AtpcoTaxes/ServiceInterfaces/DefaultServices.h"
#include "TestServer/Xform/StringToRequestBuilder.h"
#include "TestServer/Xform/ResponseToStringBuilder.h"
#include "TestServer/Xform/XmlTagsFactory.h"

#include <boost/logic/tribool.hpp>

#include <string>

namespace tax
{
class BusinessRulesProcessor;
class XmlTagsList;

class TaxStringV2Processor
{
public:
  TaxStringV2Processor();
  TaxStringV2Processor& processString(const std::string& xmlString, std::ostream& responseStream);

  void setUseRepricing(bool useRepricing)
  {
    _useRepricing = useRepricing;
  }

private:
  void addResponseHeader(size_t payloadSize, std::string& responseMessage);
  bool convertResponse(BusinessRulesProcessor& processor, std::string& responseMessage);\
  StringToRequestBuilder _requestBuilder;
  BusinessRulesProcessor _processor;
  ResponseToStringBuilder _responseBuilder;
  XmlTagsFactory _xmlTagsFactory;
  const XmlTagsList* _xmlTagsList;
  DefaultServices _services;
  boost::logic::tribool _useRepricing;
};

} // namespace tax
