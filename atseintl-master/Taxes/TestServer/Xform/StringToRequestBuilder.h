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

#include "AtpcoTaxes/DataModel/RequestResponse/InputRequest.h"
#include "AtpcoTaxes/DomainDataObjects/Request.h"
#include "AtpcoTaxes/DomainDataObjects/AtpcoTaxesDriver.h"
#include "AtpcoTaxes/DomainDataObjects/XmlCache.h"

#include "AtpcoTaxes/Processor/BusinessRulesProcessor.h"

namespace tax
{
class Response;
class XmlTagsFactory;
class XmlTagsList;

class StringToRequestBuilder
{
  friend class StringToRequestBuilderTest;

public:
  StringToRequestBuilder(const XmlTagsFactory& xmlTagsFactory);
  virtual ~StringToRequestBuilder() {};

  virtual void buildRequest(std::string xmlRequest);

  const InputRequest& getInputRequest() const { return _inputRequest; }
  const XmlTagsList* getTagsList() const { return _xmlTagsList; }

  Request& getRequest() { return _request; }
  XmlCache& getXmlCache() { return _xmlCache; }

private:
  void addResponseHeader(size_t payloadSize);
  void setBuildInfo();
  void inputToRegular();

  InputRequest _inputRequest;
  Request _request;
  XmlCache _xmlCache;
  const XmlTagsFactory& _xmlTagsFactory;
  const XmlTagsList* _xmlTagsList;
};

} // namespace tax
