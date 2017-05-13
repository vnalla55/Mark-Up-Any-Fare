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

#include <sstream>
#include <iostream>
#include <rapidxml_wrapper.hpp>

#include "DomainDataObjects/Request.h"
#include "DomainDataObjects/Response.h"
#include "DomainDataObjects/XmlCache.h"
#include "Factories/RequestFactory.h"
#include "TestServer/Facades/AKHIFactorServiceServer.h"
#include "TestServer/Facades/CarrierApplicationServiceServer.h"
#include "TestServer/Facades/CarrierFlightServiceServer.h"
#include "TestServer/Facades/CurrencyServiceServer.h"
#include "TestServer/Facades/LocServiceServer.h"
#include "TestServer/Facades/MileageServiceServer.h"
#include "AtpcoTaxes/Factories/OutputConverter.h"
#include "TestServer/Facades/PassengerMapperServer.h"
#include "TestServer/Facades/PassengerTypesServiceServer.h"
#include "TestServer/Facades/ReportingRecordServiceServer.h"
#include "TestServer/Facades/RulesRecordsServiceServer.h"
#include "TestServer/Facades/SectorDetailServiceServer.h"
#include "TestServer/Facades/ServiceBaggageServiceServer.h"
#include "TestServer/Facades/ServiceFeeSecurityServiceServer.h"
#include "TestServer/Xform/StringToRequestBuilder.h"
#include "TestServer/Xform/XmlParser.h"
#include "TestServer/Xform/BuildInfo.h"
#include "TestServer/Xform/SelectTagsList.h"

#include "TestServer/Xform/XmlTagsFactory.h"

namespace tax
{

StringToRequestBuilder::StringToRequestBuilder(const XmlTagsFactory& xmlTagsFactory)
  : _xmlTagsFactory(xmlTagsFactory), _xmlTagsList(0)
{
}

void
StringToRequestBuilder::buildRequest(std::string xmlRequest)
try
{
  setBuildInfo();

  rapidxml::xml_document<> parsedRequest;
  parsedRequest.parse<0>(&xmlRequest[0]);

  _xmlTagsList = &selectTagsList(parsedRequest, _xmlTagsFactory);

  XmlParser xmlParser;
  xmlParser.parse(_inputRequest, _xmlCache, parsedRequest, *_xmlTagsList);

  inputToRegular();
}
catch (const rapidxml::parse_error& exc)
{
  throw std::domain_error(exc.what());
}

void
StringToRequestBuilder::inputToRegular()
{
  RequestFactory factory;
  factory.createFromInput(_inputRequest, _request);
}

void
StringToRequestBuilder::setBuildInfo()
{
  _inputRequest.buildInfo() = "Built: " + BuildInfo::date() + "\nUser: " + BuildInfo::user() +
                             "\nCommit: " + BuildInfo::commit() + "\nServer start date: " +
                             BuildInfo::startTime() + "\n";
}

} // namespace tax
