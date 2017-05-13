//----------------------------------------------------------------------------
//
//      File: TaxDisplayResponseFormatter.h
//      Description: Class to format Tax Display responses back to sending client
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

#include "Common/XMLConstruct.h"
#include "DataModel/TaxTrx.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"


namespace tse
{

class TaxDisplayResponseFormatter
{
public:
  //--------------------------------------------------------------------------
  // @function TaxDisplayResponseFormatter::TaxDisplayResponseFormatter
  //
  // Description: constructor
  //
  // @param none
  //--------------------------------------------------------------------------
  TaxDisplayResponseFormatter();

  //--------------------------------------------------------------------------
  // @function TaxDisplayResponseFormatter::~TaxDisplayResponseFormatter
  //
  // Description: destructor
  //--------------------------------------------------------------------------
  virtual ~TaxDisplayResponseFormatter();

  //--------------------------------------------------------------------------
  // @function TaxDisplayResponseFormatter::formatResponse
  //
  // Description: Prepare a TaxRequest response tagged suitably for client
  //
  // @param taxTrx - a valid TaxTrx
  //--------------------------------------------------------------------------
  void formatResponse(TaxTrx& taxTrx);
  void formatResponse(TaxTrx& taxTrx, ErrorResponseException& ere);

  void formatResponse(const std::string& requestRootElement, const ErrorResponseException& ere, std::string& response);

protected:
  // Nothing

private:
  static const std::string XML_DECLARATION_TAG_TEXT;
  static const std::string TAX_DISPLAY_XML_VERSION_TEXT;
  const std::string XML_NAMESPACE_TEXT;
  const std::string XML_NAMESPACE_XS_TEXT;
  const std::string XML_NAMESPACE_XSI_TEXT;

  bool isAxess;
  void vdPrefix(TaxTrx& taxTrx);

  void buildReissue(TaxTrx& taxTrx);
  void buildMenu(TaxTrx& taxTrx);
  void buildHelpInfo(TaxTrx& taxTrx);
  void buildTaxUSHelpInfo(TaxTrx& taxTrx);
  void buildStandardHeader(TaxTrx& taxTrx);
  void buildListNationDisplay(TaxTrx& taxTrx, XMLConstruct& construct);
  void buildMultiSequence(TaxTrx& taxTrx);
  void buildSequence(TaxTrx& taxTrx);
  void addResponseFooter(XMLConstruct& construct);
  void buildMessage(const std::string& response, XMLConstruct& construct, std::string msgType = "X");
  void buildOneSequence(TaxTrx& taxTrx, TaxDisplayItem* taxDisplayItem);
  void addResponseHeader(const std::string& requestRootElement, XMLConstruct& construct);
  static const std::string readConfigXMLNamespace(const std::string& configName);
}; // End class TaxDisplayResponseFormatter

} // End namespace tse

