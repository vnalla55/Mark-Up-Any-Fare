//----------------------------------------------------------------------------
//
//  File:  BrandingRequestFormatter.cpp
//  Description: See BrandingRequestFormatter.h file
//  Created:  FEB 24, 2008
//
//  Copyright Sabre 2003
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

#include "BrandingService/BrandingRequestFormatter.h"

#include "Common/ErrorResponseException.h"
#include "Common/FareDisplayUtil.h"
#include "Common/XMLConstruct.h"
#include "DataModel/Agent.h"
#include "DataModel/FareDisplayTrx.h"

namespace tse
{
void
BrandingRequestFormatter::addRequestHeader(XMLConstruct& construct)
{
  construct.addSpecialElement("xml version=\"1.0\" encoding=\"UTF-8\"");
  construct.openElement("AirlineBrandingRequest");
  construct.addAttribute("VER", "1.0");
}

void
BrandingRequestFormatter::addXRAType(XMLConstruct& construct, FareDisplayTrx& trx)
{
  addXRAType(construct, trx.getXrayJsonMessage());
}

void
BrandingRequestFormatter::addXRAType(XMLConstruct& construct, xray::JsonMessage* jsonMessage)
{
  if (jsonMessage == nullptr)
    return;

  construct.openElement("XRA");
  construct.addAttribute("MID", jsonMessage->generateMessageId("CBASBRAND").c_str());
  construct.addAttribute("CID", jsonMessage->generateConversationId().c_str());
  construct.closeElement();
}

void
BrandingRequestFormatter::addSRCType(XMLConstruct& construct, FareDisplayTrx& trx)

{
  addSRCType(construct, trx, "FQ");
}

void
BrandingRequestFormatter::addSRCType(XMLConstruct& construct, PricingTrx& trx, const char* clientId)
{
  construct.openElement("SRC");
  if (trx.getRequest()->ticketingAgent()->tvlAgencyPCC().empty())
  {
    construct.addAttribute("A20", trx.getRequest()->ticketingAgent()->airlineDept().c_str());
  }
  else
  {
    construct.addAttribute("A20", trx.getRequest()->ticketingAgent()->tvlAgencyPCC().c_str());
  }
  construct.addAttribute("AQ1", clientId);
  construct.closeElement();
}

void
BrandingRequestFormatter::addBCRType(XMLConstruct& construct, FareDisplayTrx& trx)

{
  addBCRType(construct,
             trx,
             trx.travelDate().dateToString(YYYYMMDD, "-").c_str(),
             trx.travelSeg().front()->origAirport().c_str(),
             trx.travelSeg().front()->destAirport().c_str(),
             trx.requestedCarrier().c_str());
}

void
BrandingRequestFormatter::addBCRType(XMLConstruct& construct,
                                     PricingTrx& trx,
                                     const char* travelDate,
                                     const char* orgCity,
                                     const char* destCity,
                                     const char* carrierCode)

{
  construct.openElement("BCR");
  if (!trx.getRequest()->corporateID().empty())
  {
    construct.addAttribute("P93", trx.getRequest()->corporateID().c_str());
  }
  else if (!trx.getRequest()->accountCode().empty())
  {
    construct.addAttribute("P93", trx.getRequest()->accountCode().c_str());
  }

  construct.openElement("CRI");
  construct.openElement("D01");
  construct.addElementData(travelDate);
  construct.closeElement(); // close D01

  construct.openElement("A01");
  construct.addElementData(orgCity);
  construct.closeElement(); // close A01

  construct.openElement("A02");
  construct.addElementData(destCity);
  construct.closeElement(); // close A02

  construct.openElement("SB0");
  construct.addElementData(carrierCode);
  construct.closeElement(); // close SB0

  construct.closeElement(); // close CRI

  construct.closeElement(); // close BCR

  construct.closeElement(); // close AirlineBrandingRequest
}
}
