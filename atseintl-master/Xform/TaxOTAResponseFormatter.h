//----------------------------------------------------------------------------
//
//      File: TaxOTAResponseFormatter.h
//      Description: Class to format Tax OTA responses back to sending client
//      Created: September, 2006
//      Authors: Hitha Alex
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

#pragma once

#include "Common/Money.h"
#include "Common/XMLConstruct.h"
#include "DataModel/TaxTrx.h"
#include "Xform/ResponseFormatter.h"


namespace tse
{
class TaxRecord;

class TaxOTAResponseFormatter : public ResponseFormatter
{
public:
  //--------------------------------------------------------------------------
  // @function TaxOTAResponseFormatter::formatResponse
  //
  // Description: Prepare a TaxRequest response tagged suitably for client
  //
  // @param taxTrx - a valid TaxTrx
  //--------------------------------------------------------------------------
  void formatResponse(TaxTrx& taxTrx);

  void formatResponse(TaxTrx& taxTrx, ErrorResponseException& ere);

  void formatResponse(const std::string& requestRootElement,
                      const std::string& version,
                      const ErrorResponseException& ere,
                      std::string& response);

private:
  static const std::string XML_DECLARATION_TAG_TEXT;
  static const std::string TAX_OTA_XML_VERSION_TEXT;
  const std::string XML_NAMESPACE_TEXT = readConfigXMLNamespace("OTA_XML_NAMESPACE");
  const std::string XML_NAMESPACE_XS_TEXT = readConfigXMLNamespace("OTA_XML_NAMESPACE_XS");
  const std::string XML_NAMESPACE_XSI_TEXT = readConfigXMLNamespace("OTA_XML_NAMESPACE_XSI");

  //PfcRecord doesn't have output currency, we can take it from TaxRecord
  const TaxRecord* _pTaxRecordXF = nullptr;

  //--------------------------------------------------------------------------
  // @function TaxOTAResponseFormatter::getTotalItinTax
  //
  // Description: Get an itineraries total tax
  //
  // @param itin - itinerary to be accummulated
  // @param noDec - no of decimals to be determined
  // @param ticketingDate - transaction ticketing date
  //--------------------------------------------------------------------------
  MoneyAmount getTotalItinTax(const Itin* itin, uint16_t& noDec, const DateTime& ticketingDate);

  //--------------------------------------------------------------------------
  // @function TaxOTAResponseFormatter::getPassengerTotalTax
  //
  // Description: Get passenger total tax for the given itinerary
  //
  // @param itin - itinerary to be accummulated
  // @param paxType - passenger type to be accummulated
  //--------------------------------------------------------------------------
  MoneyAmount getPassengerTotalTax(const Itin* itin, const PaxType* paxType);

  //--------------------------------------------------------------------------
  // @function TaxOTAResponseFormatter::addTaxDetails
  //
  // Description: Add passenger detail tax information to the payload
  //
  // @param itin - itinerary to be accummulated
  // @param paxType - passenger type to be accummulated
  // @param construct - XMLConstruct to use
  // @param ticketingDate - transaction ticketing date
  //--------------------------------------------------------------------------
  void addTaxDetails(const Itin* itin,
                     const PaxType* paxType,
                     XMLConstruct& construct,
                     const DateTime& ticketingDate);

  //--------------------------------------------------------------------------
    // @function TaxOTAResponseFormatter::addTaxDetailsAll
    //
    // Description: Add passenger detail tax information to the payload
    //
    // @param taxTrx - a valid TaxTrx
    // @param itin - itinerary to be accummulated
    // @param paxType - passenger type to be accummulated
    // @param construct - XMLConstruct to use
    // @param ticketingDate - transaction ticketing date
    //--------------------------------------------------------------------------
    void addTaxDetailsAll(TaxTrx& taxTrx,
                       const Itin* itin,
                       const PaxType* paxType,
                       XMLConstruct& construct,
                       const DateTime& ticketingDate) const;

    //--------------------------------------------------------------------------
    // @function TaxOTAResponseFormatter::addTaxDetailsElement
    //
    // Description: Add passenger detail tax information to the payload - single row
    //
    void addTaxDetailsElement(TaxTrx& taxTrx,
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
                              bool bGst) const;

  void addResponseHeader(const std::string& rqRootElement,
                         const std::string& version,
                         XMLConstruct& construct);
  void addResponseFooter(XMLConstruct& construct);

  static const std::string readConfigXMLNamespace(const std::string& configName);

  static const std::string readXMLVersion(TaxTrx& taxTrx);
  static unsigned int readXMLVersionAsInt(const std::string& strVer);

  void prepareHostPortInfo(TaxTrx& taxTrx, XMLConstruct& construct);

  void prepareFormatting(std::string str, XMLConstruct& construct);
}; // End class TaxOTAResponseFormatter

} // End namespace tse

