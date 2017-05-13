//----------------------------------------------------------------------------
//
//      File: XformTaxXML.h
//      Description: Class to transform tax request XML to/from Trx
//      Created: December 15, 2004
//      Authors: Mike Carroll
//
//  Copyright Sabre 2004
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

#include "Common/Config/ConfigMan.h"
#include "Common/TaxShoppingConfig.h"
#include "Xform/Xform.h"


namespace tse
{
class TseServer;
class XformTaxXML;
class DataModelMap;

class XformTaxXML : public Xform
{
public:
  XformTaxXML(const std::string& name, ConfigMan& config);

  XformTaxXML(const std::string& name, TseServer& srv);
  XformTaxXML(const XformTaxXML& rhs) = delete;
  XformTaxXML& operator=(const XformTaxXML& rhs) = delete;

  //--------------------------------------------------------------------------
  // @function XformTaxXML::initialize
  //
  // Description: Do initialization
  //
  // @param none
  // @return bool - success or failure
  //--------------------------------------------------------------------------
  virtual bool initialize(int argc, char* argv[]) override;

  //--------------------------------------------------------------------------
  // @function XformTaxXML::convert
  //
  // Description: Given an tax XML formatted request message, take that
  //                message and decompose its component tags into a Trx object.
  //
  // @param dataHandle - a dataHandle to use
  // @param request - The XML formatted request
  // @param trx - a valid Trx reference
  // @return bool - success or failure
  //--------------------------------------------------------------------------
  virtual bool convert(DataHandle& dataHandle, std::string& request, Trx*& trx, bool throttled) override;

  //--------------------------------------------------------------------------
  // @function XformTaxXML::convert
  //
  // Description: Take a valid Trx object and compose a response XML message.
  //
  // @param trx - a valid populated Trx object
  // @param response - an empty string where the response XML will be deposited
  // @return bool - success or failure
  //--------------------------------------------------------------------------
  virtual bool convert(Trx& trx, std::string& response) override;

  //--------------------------------------------------------------------------
  // @function XformTaxXML::parse
  //
  // Description: Entry point to the DataModelMap and content handler
  //
  // @param content - allocated area of XML content
  // @param dataHandle - the dataHandle to use
  // @param trx - an empty valid Trx object reference
  // @return bool - success or failure
  //--------------------------------------------------------------------------
  virtual bool parse(const char*& content, DataHandle& dataHandle, Trx*& trx, bool throttled);

  //--------------------------------------------------------------------------
  // @function XformTaxXML::convert
  //
  // Description: Convert an ErrorResponseException to a response.
  //
  // @param ere - Error Response Exception
  // @param response - response to fill in
  // @return bool - success or failure
  //--------------------------------------------------------------------------
  virtual bool convert(tse::ErrorResponseException& ere, Trx& trx, std::string& response) override;

  //--------------------------------------------------------------------------
  // @function XformTaxXML::throttle
  //
  // Description: Format a throttling message response.
  //
  // @param request - the XML formatted request
  // @param response - response to fill in
  // @return bool - success or failure
  //--------------------------------------------------------------------------
  virtual bool throttle(std::string& request, std::string& response) override;

protected:
  ConfigMan _localConfig; // For regular V2 requests
  ConfigMan _localOTAConfig; // For OTA requests
  ConfigMan _localDisplayConfig; // For Tax Display requests
  ConfigMan _localPfcDisplayConfig; // For PFC Display requests
  ConfigMan _localTaxInfoInterfaceConfig; // For Tax common interface requests

  std::string createErrorMessage(const tse::ErrorResponseException& ere);

  void collectNamespaces(std::vector<std::string>& namespaces, const std::string& request);

private:
  static const std::string _namespaceTag;

  // Config stuff
  std::string _cfgFileName; // For regular V2 requests
  std::string _otaCfgFileName; // For OTA requests
  std::string _displayCfgFileName; // For Tax Display requests
  std::string _pfcDisplayCfgFileName; // For PFC Display requests
  std::string _taxInfoInterfaceCfgFileName; // For Tax common XML interface

  const TaxShoppingConfig _taxShoppingCfg; // shopping tax variables initialized once on server startup

  //--------------------------------------------------------------------------
  // @function XformTaxXML::initializeConfig
  //
  // Description: Initialize and load the static configuration file.  May
  //              want to pass the name in as an argument.
  //
  // @param none
  // @return bool - success or failure
  //--------------------------------------------------------------------------
  const bool initializeConfig();

  //--------------------------------------------------------------------------
  // @function XformTaxXML::formatResponse
  //
  // Description: Format a response in XML tags given a content buffer
  //
  // @param tmpResponse - string containing information to be formatted
  // @param tmpResponse - string containing XML formatted information
  // @return void
  //--------------------------------------------------------------------------
  std::string formatResponse(const std::string& response);

  bool docHnd(tse::ConfigMan& config, DataModelMap& dataModelMap, const char* content);
}; // End class XformTaxXML

} // End namespace tse

