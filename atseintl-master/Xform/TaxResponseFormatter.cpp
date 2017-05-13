//----------------------------------------------------------------------------
//
//  File:  TaxResponseFormatter.cpp
//  Description: See TaxResponseFormatter.h file
//  Created:  February 17, 2005
//  Authors:  Mike Carroll
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

#include "Xform/TaxResponseFormatter.h"

#include "Common/ErrorResponseException.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "DataModel/FarePath.h"
#include "DataModel/TaxResponse.h"
#include "Taxes/LegacyTaxes/TaxRecord.h"
#include "Xform/PricingResponseXMLTags.h"

#include <vector>

using namespace tse;
using namespace std;

static Logger
logger("atseintl.Xform.TaxResponseFormatter");

//----------------------------------------------------------------------------
// TaxResponseFormatter::TaxResponseFormatter
//----------------------------------------------------------------------------
TaxResponseFormatter::TaxResponseFormatter() {}

//----------------------------------------------------------------------------
// TaxResponseFormatter::~TaxResponseFormatter
//----------------------------------------------------------------------------
TaxResponseFormatter::~TaxResponseFormatter() {}

//----------------------------------------------------------------------------
// TaxResponseFormatter::formatResponse
//----------------------------------------------------------------------------
void
TaxResponseFormatter::formatResponse(TaxTrx& taxTrx)
{
  char tmpBuf[128];
  MoneyAmount moneyAmount;
  uint16_t noDec = 2;
  XMLConstruct construct;
  construct.openElement("TaxResponse");

  // Go through each ITIN and extract tax information
  std::vector<Itin*>::const_iterator itinIter = taxTrx.itin().begin();
  std::vector<Itin*>::const_iterator itinIterEnd = taxTrx.itin().end();

  for (; itinIter != itinIterEnd; itinIter++)
  {
    construct.openElement("ITN");

    // Sequence number

    sprintf(tmpBuf, "%d", (*itinIter)->sequenceNumber());
    construct.addAttribute("Q1K", tmpBuf);

    // Total taxes

    const DateTime& ticketingDate = taxTrx.ticketingDate();

    moneyAmount = getTotalItinTax(*itinIter, noDec, ticketingDate);

    construct.addAttributeDouble(xml2::TotalTaxes, moneyAmount, noDec);

    std::map<uint16_t, std::string>::iterator wmessageIter =
        taxTrx.warningMsg().find((*itinIter)->sequenceNumber());

    if (wmessageIter != taxTrx.warningMsg().end())
    {
      construct.addAttribute("S32", (*wmessageIter).second.c_str());
    }

    std::vector<PaxType*>::const_iterator paxIter = (*itinIter)->paxGroup().begin();
    std::vector<PaxType*>::const_iterator paxIterEnd = (*itinIter)->paxGroup().end();
    for (; paxIter != paxIterEnd; paxIter++)
    {
      construct.openElement("PXI");
      construct.addAttribute("B70", (*paxIter)->paxType().c_str());

      sprintf(tmpBuf, "%d", (*paxIter)->number());
      construct.addAttribute("Q0U", tmpBuf);

      sprintf(tmpBuf, "%d", (*paxIter)->age());
      construct.addAttribute("Q0T", tmpBuf);

      moneyAmount = getPassengerTotalTax((*itinIter), (*paxIter));

      construct.addAttributeDouble(xml2::TotalPerPassenger, moneyAmount, noDec);

      addTaxDetails(*itinIter, *paxIter, construct, ticketingDate);
      construct.closeElement();
    }
    construct.closeElement();
  }
  construct.closeElement();

  taxTrx.response() << construct.getXMLData();
  return;
}

//----------------------------------------------------------------------------
// TaxResponseFormatter::getTotalItinTax
//----------------------------------------------------------------------------
MoneyAmount
TaxResponseFormatter::getTotalItinTax(const Itin* itin,
                                      uint16_t& noDec,
                                      const DateTime& ticketingDate)
{
  LOG4CXX_DEBUG(logger, "Itinerary Sequence: " << itin->sequenceNumber());

  MoneyAmount theTotal = 0;

  std::vector<TaxResponse*>::const_iterator taxResponseIter = itin->getTaxResponses().begin();
  std::vector<TaxResponse*>::const_iterator taxResponseIterEnd = itin->getTaxResponses().end();

  for (; taxResponseIter != taxResponseIterEnd; taxResponseIter++)
  {
    if (!(*taxResponseIter)->taxRecordVector().empty())
      LOG4CXX_DEBUG(logger, "Pax Type: " << (*taxResponseIter)->paxTypeCode());

    std::vector<TaxRecord*>::const_iterator taxIter = (*taxResponseIter)->taxRecordVector().begin();
    std::vector<TaxRecord*>::const_iterator taxIterEnd =
        (*taxResponseIter)->taxRecordVector().end();

    if (taxIter != taxIterEnd)
    {
      Money money((*taxIter)->taxCurrencyCode());
      noDec = money.noDec(ticketingDate);
    }

    for (; taxIter != taxIterEnd; taxIter++)
    {
      theTotal += (*taxIter)->getTaxAmount();

      LOG4CXX_DEBUG(logger, "Tax Code: " << (*taxIter)->taxCode());
      LOG4CXX_DEBUG(logger, "Tax Amount: " << (*taxIter)->getTaxAmount());
    }
  }
  return theTotal;
}

//----------------------------------------------------------------------------
// TaxResponseFormatter::getPassengerTotalTax
//----------------------------------------------------------------------------
MoneyAmount
TaxResponseFormatter::getPassengerTotalTax(const Itin* itin, const PaxType* paxType)
{
  MoneyAmount theTotal = 0;

  std::vector<TaxResponse*>::const_iterator taxResponseIter = itin->getTaxResponses().begin();
  std::vector<TaxResponse*>::const_iterator taxResponseIterEnd = itin->getTaxResponses().end();
  for (; taxResponseIter != taxResponseIterEnd; taxResponseIter++)
  {
    if (paxType == (*taxResponseIter)->farePath()->paxType())
    {
      std::vector<TaxRecord*>::const_iterator taxIter =
          (*taxResponseIter)->taxRecordVector().begin();
      std::vector<TaxRecord*>::const_iterator taxIterEnd =
          (*taxResponseIter)->taxRecordVector().end();
      for (; taxIter != taxIterEnd; taxIter++)
      {
        theTotal += (*taxIter)->getTaxAmount();
      }
    }
  }
  return theTotal;
}

//----------------------------------------------------------------------------
// TaxResponseFormatter::addTaxDetails
//----------------------------------------------------------------------------
void
TaxResponseFormatter::addTaxDetails(const Itin* itin,
                                    const PaxType* paxType,
                                    XMLConstruct& construct,
                                    const DateTime& ticketingDate)
{
  std::vector<TaxResponse*>::const_iterator taxResponseIter = itin->getTaxResponses().begin();
  std::vector<TaxResponse*>::const_iterator taxResponseIterEnd = itin->getTaxResponses().end();
  for (; taxResponseIter != taxResponseIterEnd; taxResponseIter++)
  {
    if (paxType == (*taxResponseIter)->farePath()->paxType())
    {
      std::vector<TaxRecord*>::const_iterator taxIter =
          (*taxResponseIter)->taxRecordVector().begin();
      std::vector<TaxRecord*>::const_iterator taxIterEnd =
          (*taxResponseIter)->taxRecordVector().end();

      for (; taxIter != taxIterEnd; taxIter++)
      {
        construct.openElement(xml2::TaxInformation);
        construct.addAttribute(xml2::ATaxCode, (*taxIter)->taxCode().c_str());
        construct.addAttribute(xml2::ATaxDescription, (*taxIter)->taxDescription().c_str());
        Money money((*taxIter)->taxCurrencyCode());
        construct.addAttributeDouble(
            xml2::TaxAmount, (*taxIter)->getTaxAmount(), money.noDec(ticketingDate));
        construct.closeElement();
      }
    }
  }
}
