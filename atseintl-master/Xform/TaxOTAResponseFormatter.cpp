//----------------------------------------------------------------------------
//
//  File:  TaxOTAResponseFormatter.cpp
//  Description: See TaxOTAResponseFormatter.h file
//  Created: September, 2006
//  Authors:  Hitha Alex
//
//  Copyright Sabre 2006
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
#include "Xform/TaxOTAResponseFormatter.h"

#include "Common/Config/ConfigMan.h"
#include "Common/Config/ConfigManUtils.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/ErrorResponseException.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/Money.h"
#include "DataModel/FarePath.h"
#include "DataModel/TaxResponse.h"
#include "Taxes/LegacyTaxes/Tax.h"
#include "Taxes/LegacyTaxes/TaxRecord.h"
#include "Xform/PricingResponseXMLTags.h"
#include "Xform/XformUtil.h"

#include <vector>

namespace tse
{
static Logger
logger("atseintl.Xform.TaxOTAResponseFormatter");

const std::string TaxOTAResponseFormatter::XML_DECLARATION_TAG_TEXT =
    "xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"";

// const std::string TaxOTAResponseFormatter::TAX_OTA_XML_VERSION_TEXT
//= "2003A.TsabreXML1.0";


void
TaxOTAResponseFormatter::formatResponse(TaxTrx& taxTrx)
{
  char tmpBuf[128];
  MoneyAmount moneyAmount;
  uint16_t noDec = 2;
  XMLConstruct construct;

  addResponseHeader(taxTrx.otaRequestRootElementType(), readXMLVersion(taxTrx), construct);

  Diagnostic& diag = taxTrx.diagnostic();

  if (diag.diagnosticType() == DiagnosticNone)
  {
    construct.openElement("Success");
    construct.closeElement();
  }
  // Warnings for each itin
  if (taxTrx.itin().size() > 0 && !taxTrx.warningMsg().empty() &&
      diag.diagnosticType() == DiagnosticNone)
  {
    std::vector<Itin*>::const_iterator itinIter = taxTrx.itin().begin();
    std::vector<Itin*>::const_iterator itinIterEnd = taxTrx.itin().end();

    construct.openElement("Warnings");

    for (; itinIter != itinIterEnd; itinIter++)
    {
      Itin* itin = *itinIter;
      std::map<uint16_t, std::string>::iterator wmessageIter =
          taxTrx.warningMsg().find(itin->sequenceNumber());
      if (wmessageIter != taxTrx.warningMsg().end())
      {
        construct.openElement("Warning");
        construct.addAttribute("ShortText", (*wmessageIter).second.c_str());
        sprintf(tmpBuf, "%d", itin->sequenceNumber());
        construct.addAttribute("RPH", tmpBuf);
        construct.closeElement();
      }
    }
    construct.closeElement(); //</Warnings>
  }

  if (taxTrx.itin().size() > 0 && diag.diagnosticType() == DiagnosticNone)
  {
    construct.openElement("ItineraryInfos");
    // Go through each ITIN and extract tax information
    std::vector<Itin*>::const_iterator itinIter = taxTrx.itin().begin();
    std::vector<Itin*>::const_iterator itinIterEnd = taxTrx.itin().end();

    for (; itinIter != itinIterEnd; itinIter++)
    {
      Itin* itin = *itinIter;
      construct.openElement("ItineraryInfo");

      sprintf(tmpBuf, "%d", itin->sequenceNumber());
      construct.addAttribute("RPH", tmpBuf);

      PaxType* paxType(nullptr);
      if (itin->paxGroup().size() > 0)
      {
        construct.openElement("PTC_FareBreakdown");
        construct.openElement("PassengerType");

        paxType = itin->paxGroup().front();
        sprintf(tmpBuf, "%d", paxType->number());
        construct.addAttribute("Quantity", tmpBuf);

        construct.addAttribute("Code", paxType->paxType().c_str());

        sprintf(tmpBuf, "%d", paxType->age());
        construct.addAttribute("Age", tmpBuf);

        moneyAmount = getPassengerTotalTax(itin, paxType);
        construct.addAttributeDouble("Total", moneyAmount, noDec);

        construct.closeElement(); // PassengerType
        construct.closeElement(); // PTC_FareBreakdown
      }
      construct.openElement("TaxInfo");

      // Sequence number
      sprintf(tmpBuf, "%d", itin->sequenceNumber());
      construct.addAttribute("RPH", tmpBuf);

      // Total taxes
      const DateTime& ticketingDate = taxTrx.ticketingDate();

      moneyAmount = getTotalItinTax(itin, noDec, ticketingDate);
      construct.addAttributeDouble("Total", moneyAmount, noDec);

      construct.openElement("Taxes");
      addTaxDetails(itin, paxType, construct, ticketingDate); // Repeating Tax
      construct.closeElement(); // Taxes

      if (readXMLVersionAsInt(readXMLVersion(taxTrx))>=readXMLVersionAsInt("2.0.2"))
      {
        construct.openElement("TaxDetails");
        addTaxDetailsAll(taxTrx, itin, paxType, construct, ticketingDate); // Repeating Tax
        construct.closeElement(); // TaxDetails
      }

      construct.closeElement(); // TaxInfo
      construct.closeElement(); // ItineraryInfo
    }
    construct.closeElement(); // ItineraryInfos
  }

  if (diag.diagnosticType() != DiagnosticNone && !diag.toString().empty())
  {
    int diagnum = (int)diag.diagnosticType();
    construct.openElement(
        "DiagnosticInfos"); // Type=\"" + boost::lexical_cast<std::string>(diagnum) +"\"");
    construct.addAttribute("Type", (boost::lexical_cast<std::string>(diagnum)).c_str());
    std::string str = diag.toString();
    prepareFormatting(str, construct);
    construct.closeElement();
  }
  else
  {
    if (diag.diagnosticType() == Diagnostic854)
    {
      int diagnum = (int)diag.diagnosticType();
      construct.openElement("DiagnosticInfos");
      construct.addAttribute("Type", (boost::lexical_cast<std::string>(diagnum)).c_str());
      prepareHostPortInfo(taxTrx, construct);
      construct.closeElement();
    }
  }
  addResponseFooter(construct);
  taxTrx.response() << construct.getXMLData();
  return;
}

void
TaxOTAResponseFormatter::formatResponse(const std::string& reqRootElement,
                                        const std::string& xmlVersion,
                                        const ErrorResponseException& ere,
                                        std::string& response)
{
  XMLConstruct construct;

  addResponseHeader(reqRootElement, xmlVersion, construct);

  construct.openElement("Errors");
  construct.openElement("Error");

  std::ostringstream strErrorCode;
  strErrorCode << ere.code();

  construct.addAttribute("ErrorCode", strErrorCode.str());
  construct.addAttribute("ErrorMessage", ere.message().c_str());

  construct.closeElement(); //</Error>
  construct.closeElement(); //</Errors>

  addResponseFooter(construct);
  response = construct.getXMLData();
}

void
TaxOTAResponseFormatter::formatResponse(TaxTrx& taxTrx, ErrorResponseException& ere)
{
  std::string response;
  formatResponse(taxTrx.otaRequestRootElementType(),
                 readXMLVersion(taxTrx), ere, response);
  taxTrx.response() << response;
  return;
}

void
TaxOTAResponseFormatter::prepareFormatting(std::string str, XMLConstruct& construct)
{
  typedef boost::tokenizer<boost::char_separator<char> > Tok;
  boost::char_separator<char> sep("\n");
  Tok tok(str, sep);
  int line = 0;
  std::string linenum;
  std::string message;
  for (Tok::iterator tok_iter = tok.begin(); tok_iter != tok.end(); ++tok_iter)
  {
    line++;
    linenum = boost::lexical_cast<std::string>(line);
    message = (*tok_iter).c_str();
    construct.openElement("DiagnosticInfo Message=\"" + message + "\" Line=\"" + linenum + "\"");
    construct.closeElement();
  }
}

void
TaxOTAResponseFormatter::prepareHostPortInfo(TaxTrx& taxTrx, XMLConstruct& construct)
{
  std::vector<std::string> hostInfo;
  std::vector<std::string> buildInfo;
  std::vector<std::string> dbInfo;
  std::vector<std::string> configInfo;

  std::string fullinfo;

  if (hostDiagString(hostInfo))
  {
    for (const auto& elem : hostInfo)
      fullinfo += elem + "\n";
  }

  buildDiagString(buildInfo);
  for (const auto& elem : buildInfo)
    fullinfo += elem + "\n";

  dbDiagString(dbInfo);
  for (const auto& elem : dbInfo)
    fullinfo += elem + "\n";

  if (configDiagString(configInfo, taxTrx, true))
  {
    for (const auto& elem : configInfo)
      fullinfo += elem + "\n";
  }

  prepareFormatting(fullinfo, construct);
}

MoneyAmount
TaxOTAResponseFormatter::getTotalItinTax(const Itin* itin,
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

MoneyAmount
TaxOTAResponseFormatter::getPassengerTotalTax(const Itin* itin, const PaxType* paxType)
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

void
TaxOTAResponseFormatter::addTaxDetails(const Itin* itin,
                                       const PaxType* paxType,
                                       XMLConstruct& construct,
                                       const DateTime& ticketingDate)
{

  if (itin && paxType)
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
          construct.openElement("Tax");
          construct.addAttribute("TaxCode", (*taxIter)->taxCode().c_str());
          Money money((*taxIter)->taxCurrencyCode());
          construct.addAttributeDouble(
              "Amount", (*taxIter)->getTaxAmount(), money.noDec(ticketingDate));
          if (!(*taxIter)->taxDescription().empty())
          {
            construct.openElement("Text");
            construct.addElementData((*taxIter)->taxDescription().c_str());
            construct.closeElement(); // Text
          }
          construct.closeElement(); // Tax

          if (!_pTaxRecordXF && (*taxIter)->taxCode()==TaxRecord::TAX_CODE_XF)
             _pTaxRecordXF = *taxIter;
        }
      }
    }
  }
}

void
TaxOTAResponseFormatter::addTaxDetailsElement(TaxTrx& taxTrx,
                                              XMLConstruct& construct,
                                              const DateTime& ticketingDate,
                                              const char* strTaxCode,
                                              const TaxTypeCode& taxTypeCode,
                                              const MoneyAmount& amount,
                                              uint16_t unPaymentCurrencyNoDec,
                                              MoneyAmount amountPublished,
                                              Money target,
                                              const CurrencyCode& paymentCurrency,
                                              const char* strStation,
                                              const char* strCountryCode,
                                              const char* strAirlineCode,
                                              const char* strTaxDesc,
                                              bool bGst) const
{
  construct.openElement("Tax");
  construct.addAttribute("TaxCode", strTaxCode);
  construct.addAttributeChar("Type", taxTypeCode);
  construct.addAttributeDouble("Amount", amount, unPaymentCurrencyNoDec);

  CurrencyNoDec amountPublishedNoDec = target.noDec(ticketingDate);
  if (taxTypeCode == Tax::PERCENTAGE)
  {
    Money source(amount, paymentCurrency);
    TaxUtil::convertTaxCurrency(taxTrx, source, target);
    amountPublished = target.value();
  }

  construct.addAttribute("Currency", paymentCurrency);
  construct.addAttributeDouble("PublishedAmount", amountPublished, amountPublishedNoDec);
  construct.addAttribute("PublishedCurrency", target.code());
  construct.addAttribute("Station", strStation);
  construct.addAttribute("CountryCode", strCountryCode);
  construct.addAttribute("AirlineCode", strAirlineCode);
  if (strTaxDesc)
    construct.addAttribute("Text", strTaxDesc);
  if (bGst)
    construct.addAttributeBoolean("GST", bGst);

  construct.closeElement(); // Tax
}

void
TaxOTAResponseFormatter::addTaxDetailsAll(TaxTrx& taxTrx,
                                       const Itin* itin,
                                       const PaxType* paxType,
                                       XMLConstruct& construct,
                                       const DateTime& ticketingDate) const
{
  if (itin && paxType)
  {
    for (const TaxResponse* pResponse : itin->getTaxResponses())
    {
      if (paxType == pResponse->farePath()->paxType())
      {
        for (const TaxItem* pTaxItem : pResponse->taxItemVector())
        {
          addTaxDetailsElement(taxTrx
              , construct
              , ticketingDate
              , pTaxItem->taxCode().c_str()
              , pTaxItem->taxType()
              , pTaxItem->taxAmount()
              , pTaxItem->paymentCurrencyNoDec()
              , pTaxItem->taxAmt()
              , pTaxItem->taxCur()
              , pTaxItem->paymentCurrency()
              , pTaxItem->taxLocalBoard().c_str()
              , pTaxItem->nation().c_str()
              , MCPCarrierUtil::swapToPseudo(&taxTrx, pTaxItem->carrierCode()).c_str()
              , pTaxItem->taxDescription().empty() ? nullptr : pTaxItem->taxDescription().c_str()
              , pTaxItem->gstTax());
        }

        for (const PfcItem* pPfcItem : pResponse->pfcItemVector())
        {
          CurrencyNoDec curNoDec = pPfcItem->pfcDecimals();
          Money source(pPfcItem->pfcAmount(), pPfcItem->pfcCurrencyCode());
          Money target(pPfcItem->pfcAmount(), pPfcItem->pfcCurrencyCode());
          if (_pTaxRecordXF)
          {
            target.setCode(_pTaxRecordXF->taxCurrencyCode());
            curNoDec = _pTaxRecordXF->taxNoDec();
            TaxUtil::convertTaxCurrency(taxTrx, source, target);
          }

          addTaxDetailsElement(taxTrx
              , construct
              , ticketingDate
              , TaxRecord::TAX_CODE_XF.c_str()
              , 'F'
              , target.value()
              , curNoDec
              , pPfcItem->pfcAmount()
              , pPfcItem->pfcCurrencyCode()
              , target.code()
              , pPfcItem->pfcAirportCode().c_str()
              , _pTaxRecordXF ? _pTaxRecordXF->taxNation().c_str() : "US"
              , MCPCarrierUtil::swapToPseudo(&taxTrx, pPfcItem->carrierCode()).c_str()
              , TaxRecord::PFC.c_str()
              , false);
        }
      }
    }
  }
}

void
TaxOTAResponseFormatter::addResponseHeader(const std::string& rootElement,
                                           const std::string& version,
                                           XMLConstruct& construct)
{
  construct.addSpecialElement(XML_DECLARATION_TAG_TEXT.c_str());
  if (rootElement == "AIRTAXRQ")
    construct.openElement("AirTaxRS");
  else
    construct.openElement("TaxRS");
  construct.addAttribute("xmlns", XML_NAMESPACE_TEXT);
  construct.addAttribute("xmlns:xs", XML_NAMESPACE_XS_TEXT);
  construct.addAttribute("xmlns:xsi", XML_NAMESPACE_XSI_TEXT);
  construct.addAttribute("Version", version);
}

void
TaxOTAResponseFormatter::addResponseFooter(XMLConstruct& construct)
{
  construct.closeElement(); // TaxRS
}

const std::string
TaxOTAResponseFormatter::readConfigXMLNamespace(const std::string& configName)
{
  tse::ConfigMan& config = Global::config();

  std::string xmlNamespace;

  if (!config.getValue(configName, xmlNamespace, "TAX_SVC"))
  {
    CONFIG_MAN_LOG_KEY_ERROR(logger, configName, "TAX_SVC");
  }

  return xmlNamespace;
}

const std::string
TaxOTAResponseFormatter::readXMLVersion(TaxTrx& taxTrx)
{
  return taxTrx.otaXmlVersion();
}

//returns version as number; two digits for each part; max 4 parts
//a.b.c.d -> aabbccdd (e.g 1.2.3.4->1020304, 1.2.3->1020300)
//
//NOTE: current version of this method doesn't process version in this
//format "2003A.TsabreXML1.0.2"
unsigned int
TaxOTAResponseFormatter::readXMLVersionAsInt(const std::string& strVer)
{
  unsigned int unResult = 0;

  using tokenizer = boost::tokenizer<boost::char_separator<char>>;
  boost::char_separator<char> sep(".");
  tokenizer tokens(strVer, sep);

  int nCount = 4;
  for (tokenizer::iterator tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter)
  {
    try
    {
      unResult += boost::lexical_cast<int>(*tok_iter);
    }
    catch (boost::bad_lexical_cast&)
    {
      LOG4CXX_DEBUG(logger, "Unexpected format version: " << strVer);
      return 0;
    }

    if (--nCount==0)
      break;

    unResult *= 100;
  }

  //faster then pow
  switch (nCount)
  {
    case 3: unResult *= 100;  //Fallthrough
    case 2: unResult *= 100;
  }

  return unResult;
}
}
