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
#include "test/XmlAndDiagnosticFixture.h"

#include <fstream>
#include <iterator>
#include "Processor/BusinessRulesProcessor.h"
#include "Factories/RequestFactory.h"
#include "TestServer/Facades/LocServiceServer.h"
#include "TestServer/Facades/MileageServiceServer.h"
#include "TestServer/Facades/RulesRecordsServiceServer.h"

// TODO: get rid of the XmlParser in tests
#include "../TestServer/Xform/XmlParser.h"
#include "TestServer/Xform/XmlTagsFactory.h"
#include "TestServer/Xform/NaturalXmlTagsList.h"
#include "TestServer/Xform/SelectTagsList.h"

namespace tax
{
void
XmlAndDiagnosticFixture::loadXml(std::string filename)
{
  std::ifstream xmlFile(filename.c_str());
  std::string xmlBuf((std::istreambuf_iterator<char>(xmlFile)), std::istreambuf_iterator<char>());
  _xmlTagsFactory.registerList(new NaturalXmlTagsList);
  rapidxml::xml_document<> parsedRequest;
  parsedRequest.parse<0>(&xmlBuf[0]);
  const XmlTagsList& tags = selectTagsList(parsedRequest, _xmlTagsFactory);

  XmlParser().parse(_inputRequest, _xmlCache, parsedRequest, tags);
  RequestFactory factory;
  _request.reset(new Request());
  factory.createFromInput(_inputRequest, *_request);

  RulesRecordsServiceServer* rulesRecordsServiceServer = new RulesRecordsServiceServer();
  rulesRecordsServiceServer->rulesRecords() = _xmlCache.rulesRecords();
  _services.setRulesRecordsService(rulesRecordsServiceServer);
  rulesRecordsServiceServer->updateKeys();

  LocServiceServer* locServiceServer = new LocServiceServer();
  locServiceServer->nations() = _xmlCache.nations();
  locServiceServer->isInLocs() = _xmlCache.isInLocs();
  _services.setLocService(locServiceServer);

  MileageServiceServer* mileageServiceServer = new MileageServiceServer();
  mileageServiceServer->mileages() = _xmlCache.mileages();
  _services.setMileageService(mileageServiceServer);
}

uint32_t
XmlAndDiagnosticFixture::colectDiagnostic(std::stringstream& buf, BusinessRulesProcessor& brp)
{
  uint32_t numOfLines = 0;
  for (const Message message : brp.response()._diagnosticResponse->_messages)
  {
    buf << message._content << "\n";
    ++numOfLines;
  }
  return numOfLines;
}

uint32_t
XmlAndDiagnosticFixture::runTest(std::string xmlFileName, uint32_t diagNum, std::string& message)
{
  loadXml(xmlFileName);
  _request->diagnostic().number() = diagNum;
  BusinessRulesProcessor brp(_services);

  AtpcoTaxesActivationStatus activationStatus;
  activationStatus.setAllEnabled();
  brp.run(*_request, activationStatus);

  std::stringstream buf;
  uint32_t numOfLines = colectDiagnostic(buf, brp);
  message = buf.str();
  return numOfLines;
}

} // namespace tax
