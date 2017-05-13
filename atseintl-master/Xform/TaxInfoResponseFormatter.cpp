//----------------------------------------------------------------------------
//
//  File:  PfcDisplayResponseFormatter.cpp
//  Description: See PfcDisplayResponseFormatter.h file
//  Created: December, 2008
//  Authors:  Jakub Kubica
//
//  Copyright Sabre 2008
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "Xform/TaxInfoResponseFormatter.h"

#include "Common/Logger.h"
#include "Xform/TaxInfoXmlBuilder.h"

using namespace tse;
using namespace std;

static Logger
logger("atseintl.Xform.TaxInfoResponseFormatter");

//----------------------------------------------------------------------------
// TaxInfoResponseFormatter::TaxInfoResponseFormatter
//----------------------------------------------------------------------------
TaxInfoResponseFormatter::TaxInfoResponseFormatter() {}

//----------------------------------------------------------------------------
// TaxInfoResponseFormatter::~TaxInfoResponseFormatter
//----------------------------------------------------------------------------
TaxInfoResponseFormatter::~TaxInfoResponseFormatter() {}

//----------------------------------------------------------------------------
// TaxInfoResponseFormatter::formatResponse
//----------------------------------------------------------------------------
void
TaxInfoResponseFormatter::formatResponse(TaxTrx& taxTrx)
{
  XMLConstruct construct;

  construct.openElement("TaxInfoResponse");

  buildMessage(taxTrx, construct);

  construct.closeElement();

  taxTrx.response().str(EMPTY_STRING());
  taxTrx.response() << construct.getXMLData();

  return;
}

//----------------------------------------------------------------------------
// TaxInfoResponseFormatter::formatResponse
//----------------------------------------------------------------------------
void
TaxInfoResponseFormatter::formatResponse(const ErrorResponseException& ere, std::string& response)
{
  XMLConstruct construct;

  TaxInfoXmlBuilder xmlBuilder;
  TaxInfoResponse item;
  std::get<TaxInfoResponse::TAX::ERROR>(item.taxAttrValues()) = ere.message();
  std::string msg = xmlBuilder.build(item);

  construct.openElement("TaxInfoResponse");
  construct.addElementData(msg.c_str(), msg.size());
  construct.closeElement();

  response = construct.getXMLData();
}

//----------------------------------------------------------------------------
// TaxInfoResponseFormatter::formatResponse
//----------------------------------------------------------------------------
void
TaxInfoResponseFormatter::buildMessage(TaxTrx& taxTrx, XMLConstruct& construct)
{
  std::string xmlResponse;

  TaxInfoXmlBuilder xmlBuilder;

  std::vector<TaxInfoResponse>::iterator item = taxTrx.taxInfoResponseItems().begin();
  std::vector<TaxInfoResponse>::iterator itemEnd = taxTrx.taxInfoResponseItems().end();
  for (; item != itemEnd; item++)
  {
    xmlResponse += xmlBuilder.build(*item);
  }

  LOG4CXX_DEBUG(logger, "Before XML:\n" << xmlResponse);
  construct.addElementData(xmlResponse.c_str(), xmlResponse.size());
  LOG4CXX_DEBUG(logger, "Finished in formatResponse");

  return;
}
