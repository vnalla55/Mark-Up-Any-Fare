// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
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

#include <cppunit/extensions/HelperMacros.h>

#include "DataModel/RequestResponse/InputRequest.h"
#include "DomainDataObjects/Request.h"
#include "DomainDataObjects/XmlCache.h"
#include "ServiceInterfaces/DefaultServices.h"
#include "TestServer/Xform/XmlTagsFactory.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <string>

namespace tax
{

class BusinessRulesProcessor;

class XmlAndDiagnosticFixture : public CppUnit::TestFixture
{
public:
  void loadXml(std::string filename);
  uint32_t colectDiagnostic(std::stringstream& buf, BusinessRulesProcessor& brp);
  uint32_t runTest(std::string xmlFileName, uint32_t diagNum, std::string& message);

protected:
  std::shared_ptr<Request> _request;
  DefaultServices _services;

private:
  InputRequest _inputRequest;
  XmlCache _xmlCache;
  XmlTagsFactory _xmlTagsFactory;
};

} // namespace tax
