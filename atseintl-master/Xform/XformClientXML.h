//----------------------------------------------------------------------------
//
//      File: XformClientXML.h
//      Description: Class to transform client XML to/from Trx
//      Created: March 18, 2004
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

#include "DataModel/AltPricingDetailObFeesTrx.h"
#include "DataModel/AltPricingTrx.h"
#include "DataModel/MetricsTrx.h"
#include "DataModel/PricingDetailTrx.h"
#include "DataModel/StatusTrx.h"
#include "DBAccess/DataHandle.h"
#include "FareCalc/CalcTotals.h"
#include "Xform/PricingDetailModelMap.h"
#include "Xform/Xform.h"

#include <fstream>
#include <string>
#include <vector>

namespace tse
{
class Agent;
class PricingTrx;
class RexBaseTrx;
class SelectionContentHandler;
class TseServer;

class XformClientXML : public Xform
{
  friend class XformClientXMLTest;

public:
  //--------------------------------------------------------------------------
  // @function XMLformClientXML::XformClientXML
  //
  // Description: constructor
  //
  // @param name - program name
  // @param config - a valid ConfigMan reference
  //--------------------------------------------------------------------------
  XformClientXML(const std::string& name, ConfigMan& config) : Xform(name, config) {}
  XformClientXML(const std::string& name, TseServer& srv);

  XformClientXML(const XformClientXML&) = delete;
  XformClientXML& operator=(const XformClientXML&) = delete;

  //--------------------------------------------------------------------------
  // @function XformClientXML::~XformClientXML
  //
  // Description: destructor
  //--------------------------------------------------------------------------
  virtual ~XformClientXML();

  //--------------------------------------------------------------------------
  // @function XformClientXML::initialize
  //
  // Description: Do initialization
  //
  // @param none
  // @return bool - success or failure
  //--------------------------------------------------------------------------
  virtual bool initialize(int argc, char* argv[]) override;

  //--------------------------------------------------------------------------
  // @function XformClientXML::convert
  //
  // Description: Given an XML formatted request message, take that message
  //              and decompose its component tags into a Trx object.
  //
  // @param dataHandle - a dataHandle to use
  // @param request - The XML formatted request
  // @param trx - a valid Trx reference
  // @return bool - success or failure
  //--------------------------------------------------------------------------
  virtual bool
  convert(DataHandle& dataHandle, std::string& request, Trx*& trx, bool throttled) override;

  //--------------------------------------------------------------------------
  // @function XformClientXML::convert
  //
  // Description: Take a valid Trx object and compose a response XML message.
  //
  // @param trx - a valid populated Trx object
  // @param response - an empty string where the response XML will be deposited
  // @return bool - success or failure
  //--------------------------------------------------------------------------
  virtual bool convert(Trx& trx, std::string& response) override;

  //--------------------------------------------------------------------------
  // @function XformClientXML::parse
  //
  // Description: Entry point to the DataModelMap and content handler
  //
  // @param content - allocated area of XML content
  // @param dataHandle - the dataHandle to use
  // @param trx - an empty valid Trx object reference
  // @return bool - success or failure
  //--------------------------------------------------------------------------
  virtual bool parse(const char*& content, DataHandle& dataHandle, Trx*& trx);

  //--------------------------------------------------------------------------
  // @function XformClientXML::convert
  //
  // Description: Convert an ErrorResponseException to a response.
  //
  // @param ere - Error Response Exception
  // @param response - response to fill in
  // @return bool - success or failure
  //--------------------------------------------------------------------------
  virtual bool convert(tse::ErrorResponseException& ere, Trx& trx, std::string& response) override;

  //--------------------------------------------------------------------------
  // @function XformClientXML::convert
  //
  // Description: Convert an ErrorResponseException to a response.
  //
  // @param ere - Error Response Exception
  // @param response - response to fill in
  // @return bool - success or failure
  //--------------------------------------------------------------------------
  virtual bool convert(ErrorResponseException& ere, std::string& response) override;

  //--------------------------------------------------------------------------
  // @function XformClientXML::throttle
  //
  // Description: Prepare a throttling message and form a response.
  //
  // @param request - the XML formatted request
  // @param response - response to fill in
  // @return bool - success or failure
  //--------------------------------------------------------------------------
  virtual bool throttle(std::string& request, std::string& resposne) override;

protected:
  class LowerOptionNumber
  {
  public:
    LowerOptionNumber() {}

    bool operator()(const PaxDetail* item1, const PaxDetail* item2);
  };

  // Nothing
  ConfigMan _localConfig;
  ConfigMan _localDetailConfig;
  ConfigMan _fareDisplayConfig;
  ConfigMan _mileageConfig;
  ConfigMan _currencyConfig;
  ConfigMan _pricingDisplayConfig;
  ConfigMan _pricingConfig;
  ConfigMan _pricingDetailConfig;

  //--------------------------------------------------------------------------
  // @function XformClientXML::initializeConfig
  //
  // Description: Initialize and load the static configuration file.  May
  //              want to pass the name in as an argument.
  //
  // @param none
  // @return bool - success or failure
  //--------------------------------------------------------------------------
  const bool initializeConfig();

  int formatResponse(std::string& tmpResponse,
                     std::string& xmlResponse,
                     const std::string& msgType = "X");

  int formatResponse(std::string& tmpResponse,
                     std::string& xmlResponse,
                     int recNum,
                     const std::string& msgType = "X",
                     PricingDetailTrx* trx = nullptr);

  //--------------------------------------------------------------------------
  // @function XformClientXML::processWpdfnRequest
  //
  // Description: process a WPDFn entry which followed a WPA entry
  //
  // @param content - string containing the stored content of the previous WPA
  //                  response
  // @param selectionList - list of selection numbers
  // @param trx - should point to a PricingDetailTrx
  // @return true if successful, otherwise false
  //--------------------------------------------------------------------------
  bool processWpdfnRequest(const std::string& content,
                           DataHandle& dataHandle,
                           const std::vector<uint16_t>& selectionList,
                           Trx*& trx);

  //--------------------------------------------------------------------------
  // @function XformClientXML::parsePricingDetails
  //
  // Description: parse a pricing detail request into a PricingDetailTrx
  //
  // @param content - string containing the stored content of the previous WPA
  //                  response
  // @param trx - should point to a PricingDetailTrx
  // @param wpdfType - type of wpdf transaction
  // @return true if successful, otherwise false
  //--------------------------------------------------------------------------
  bool
  parsePricingDetails(const char* msg,
                      DataHandle& dataHandle,
                      Trx*& trx,
                      PricingDetailModelMap::WpdfType wpdfType = PricingDetailModelMap::WPDF_NONE);

  //--------------------------------------------------------------------------
  // @function XformClientXML::parseFareDisplay
  //
  // Description: parse a fare display request into a FareDisplayTrx
  //
  // @param content - string containing the request
  // @param dataHandle - reference to dataHandle object
  // @param trx - it will be a pointer to a FareDisplayTrx
  // @return true if successful, otherwise false
  //--------------------------------------------------------------------------
  bool parseFareDisplay(const char* content, DataHandle& dataHandle, Trx*& trx);

  //--------------------------------------------------------------------------
  // @function XformClientXML::narrowDetailTrx
  //
  // Description: eliminate PaxDetail records from a PricingDetailTrx that
  //              do not correspond to entries in a selection list
  //
  // @param trx - should point to a PricingDetailTrx
  // @param selectionList - list of selection numbers
  // @return true if successful, otherwise false
  //--------------------------------------------------------------------------
  bool narrowDetailTrx(PricingDetailTrx* detailTrx, const std::vector<uint16_t>& selectionList);

  bool
  narrowDetailTrxObFee(PricingDetailTrx* detailTrx, const std::vector<uint16_t>& selectionList);

  bool findOBbfInContentHeader(const std::string& content);

  //--------------------------------------------------------------------------
  // @function XformClientXML::processWpnRequest
  //
  // Description: process a WPn entry which followed a WPA entry
  //
  // @param content - string containing the stored content of the previous WPA
  //                  response
  // @param selectionList - list of selection numbers
  // @param trx - should point to a PricingDetailTrx
  // @return true if successful, otherwise false
  //--------------------------------------------------------------------------
  bool processWpnRequest(const std::string& content,
                         DataHandle& dataHandle,
                         const std::vector<uint16_t>& selectionList,
                         Trx*& trx);

  //--------------------------------------------------------------------------
  // @function XformClientXML::prepareDetailTrx
  //
  // Description: create a PricingDetailTrx based on a string containing a
  //              PricingResponse XML message and a selection message
  //
  // @param content - string containing a PricingResponse XML message
  // @param selectionList - list of selection numbers
  // @param trx - should point to a PricingDetailTrx
  // @param wpdfType - type of wpdf transaction
  // @return pointer to the transaction if successful, otherwise NULL
  //--------------------------------------------------------------------------
  PricingDetailTrx*
  prepareDetailTrx(const std::string& content,
                   DataHandle& dataHandle,
                   const std::vector<uint16_t>& selectionList,
                   Trx*& trx,
                   PricingDetailModelMap::WpdfType wpdfType = PricingDetailModelMap::WPDF_NONE);

  void copyAgentInfo(const Agent& from, Agent& to);

  //--------------------------------------------------------------------------
  // @function XformClientXML::processWtfrRequest
  //
  // Description: process a WTFR entry which followed a WP or WPA entry
  //
  // @param content - string containing the stored content of the previous WP
  //                  or WPA response
  // @param selectionList - list of selection numbers
  // @param trx - should point to a PricingDetailTrx
  // @return true if successful, otherwise false
  //--------------------------------------------------------------------------
  bool processWtfrRequest(const std::string& content,
                          bool followingWp,
                          DataHandle& dataHandle,
                          const std::vector<uint16_t>& selectionList,
                          Trx*& trx,
                          bool noMatch);

  //--------------------------------------------------------------------------
  // @function XformClientXML::extractUncompressedContent
  //
  // Description: extract the uncompressed PricingResponse from a selection
  //              request message
  //
  // @param message - contains the selection request XML message
  // @return pricing response XML message that was embedded in the selection
  //         request
  //--------------------------------------------------------------------------
  std::string extractUncompressedContent(const char* message);


  //--------------------------------------------------------------------------
  // @function XformClientXML::processWpaDetailRequest
  //
  // Description: process a WPA* entry which followed a WPA entry
  //
  // @param content - string containing the stored content of the previous WPA
  //                  response
  // @param selectionList - list of selection numbers
  // @param trx - should point to a PricingDetailTrx
  // @return true if successful, otherwise false
  //--------------------------------------------------------------------------
  template <typename T>
  bool processWpaDetailRequest(PricingDetailTrx* pricingDetailTrx,
                               const std::string& content,
                               DataHandle& dataHandle,
                               Trx*& trx,
                               bool recordQuote,
                               bool rebook);

  //--------------------------------------------------------------------------
  // @function XformClientXML::extractOptionXml
  //
  // Description: break up the PricingResponse XML from a WPA entry into
  //              strings for each option
  //
  // @param message - contains the XML pricing response
  // @param optionXmlVec - a vector where each element contains the SUM XML
  //                       element for an option in the WPA response
  // @return true if successful, otherwise false
  //--------------------------------------------------------------------------
  bool extractOptionXml(const std::string& message, std::vector<std::string>& optionXmlVec);

  //--------------------------------------------------------------------------
  // @function XformClientXML::extractXmlElement
  //
  // Description: Extract an XML element from a given message based on a pattern
  //              matching the beginning of the element. This only works with
  //              elements that do not have other elements nested within.
  //
  // @param message - contains the XML to be searched
  // @param beginPattern - a string that should match the beginning of the
  //                       element. This only does an exact string comparison,
  //                       with no regular expressions or wildcards.
  // @return extracted element if successful, otherwise empty string
  //--------------------------------------------------------------------------
  std::string extractXmlElement(const std::string& message, const char* beginPattern);

  //--------------------------------------------------------------------------
  // @function XformClientXML::saveAgentBillingXml
  //
  // Description: Saves the agent and billing element XML strings from a
  //              pricing response XML message into appropriate fields of
  //              an alt pricing transaction.
  //
  // @param message - contains the XML pricing response
  // @param altTrx - the transaction into which the agent and billing XML is
  //                 to be saved
  // @return true if successful, otherwise false
  //--------------------------------------------------------------------------
  template <typename T>
  bool saveAgentBillingXml(const std::string& message, T& altTrx);

  void setDiagArguments(PricingTrx* pricingTrx) const;
  void setDiagArguments(MileageTrx* pricingTrx) const;

  void setupDiag(DataHandle& dataHandle, PricingTrx* pricingTrx, Trx*& trx) const;
  void setupDiag(DataHandle& dataHandle, MileageTrx* mileageTrx, Trx*& trx) const;

  //----------------------------------------------------------------------------
  // @function XformClientXML::skipXMLWhitespaces
  //
  //
  // Description: Given an XML formatted request message, take that message
  //              and skip XML Whitespaces.
  //
  // @param request - The XML formatted request
  // @param skipPos - Skip from position in the XML request
  // @return std::string::size_type -
  //----------------------------------------------------------------------------
  std::string::size_type
  skipXMLWhitespaces(const std::string& request, const std::string::size_type skipPos);

  //----------------------------------------------------------------------------
  // @function XformClientXML::skipXMLComment
  //
  //
  // Description: Given an XML formatted request message, take that message
  //              and skip XML Comment.
  //
  // @param request - The XML formatted request
  // @param skipPos - Skip from position in the XML request
  // @return std::string::size_type -
  //----------------------------------------------------------------------------
  std::string::size_type
  skipXMLComment(const std::string& request, const std::string::size_type skipPos);

  //----------------------------------------------------------------------------
  // @function XformClientXML::skipXMLWhitespacesAndComments
  //
  //
  // Description: Given an XML formatted request message, take that message
  //              and skip XML Whitespaces and XML Comments.
  //
  // @param request - The XML formatted request
  // @param skipPos - Skip from position in the XML request
  // @return std::string::size_type -
  //----------------------------------------------------------------------------
  std::string::size_type
  skipXMLWhitespacesAndComments(const std::string& request, const std::string::size_type skipPos);

  //----------------------------------------------------------------------------
  // @function XformClientXML::skipXMLDecl
  //
  //
  // Description: Given an XML formatted request message, take that message
  //              and skip XML Declaration.
  //
  // @param request - The XML formatted request
  // @param skipPos - Skip from position in the XML request.Defaults to 0
  // @return std::string::size_type -
  //----------------------------------------------------------------------------
  std::string::size_type
  skipXMLDecl(const std::string& request, const std::string::size_type skipPos = 0);

  //----------------------------------------------------------------------------
  // @function XformClientXML::skipXMLProlog
  //
  //
  // Description: Given an XML formatted request message, take that message
  //              and skip XML Prolog.
  //
  // @param request - The XML formatted request
  // @param skipPos - Skip from position in the XML request
  // @return std::string::size_type -
  //----------------------------------------------------------------------------
  std::string::size_type
  skipXMLProlog(const std::string& request, const std::string::size_type skipPos = 0);

  //----------------------------------------------------------------------------
  // @function XformClientXML::findFirstRequestXMLElement
  //
  //
  // Description: Given an XML formatted request message, take that message
  //              and finds the first Request XML Element.
  //
  // @param request - The XML formatted request
  // @param skipPos - Can take 0 or TRX_ID_LENGTH. if called within this method set
  //                  skipPos = 0. If not let it default to TRX_ID_LENGTH.
  // @return std::string::size_type -
  //----------------------------------------------------------------------------
  std::string::size_type findFirstRequestXMLElement(const std::string& request);
  std::string getElementValue(const char* xml, const char* element);

  //--------------------------------------------------------------------------
  // @function XformClientXML::eraseXmlElement
  //
  // Description: Erase an XML element from a given message based on a pattern
  //              matching the beginning of the element. This only works with
  //              elements that do not have other elements nested within.
  //
  // @param message  - contains the XML to be searched
  // @param eraseInx - number pointing to the XML element nedds to be erased
  // @return modified XML stringif successful, otherwise empty string
  //--------------------------------------------------------------------------
  virtual std::string eraseXmlElement(std::string& message, size_t eraseInx);

  //--------------------------------------------------------------------------
  // @function XformClientXML::eraseXmlMsgElement
  //
  // Description: Erase an XML element from a given message based on a pattern
  //              matching the beginning of the element. This only works with
  //              elements that do not have other elements nested within.
  //
  // @param message  - contains the XML to be searched
  // @return extracted element data if successful, otherwise empty string
  //--------------------------------------------------------------------------
  virtual std::string eraseXmlMsgElement(std::string& message);

  //--------------------------------------------------------------------------
  // @function XformClientXML::saveVclInformation
  //
  // Description: Save ValidatingCarrier Info a given message based on a pattern
  //              matching the beginning of the element.
  //
  // @param message  - contains the XML to be searched
  // Add VCL information in PAX detail
  //--------------------------------------------------------------------------
  virtual void saveVclInformation(PaxDetail* currentPaxDetail, std::string& message, Trx*& trx);

  //----------------------------------------------------------------------------
  // @function XformClientXML::getTotalAmountPerPax
  //
  // Description: Extract an XML element data from a given message based on a pattern
  //              matching the beginning of the element. This only works with
  //              elements that do not have other elements nested within.
  //
  // @param message - contains the XML to be searched
  // @param nDec - if true, function returns number of decimal points
  // @return extracted element data if successful, otherwise zero
  //--------------------------------------------------------------------------
  MoneyAmount getTotalAmountPerPax(std::string& message, bool nDec = false);

  //--------------------------------------------------------------------------
  // @function XformClientXML::createAltPricingDetailTrx
  //
  // Description: Creates a AltPricingDetailTrx transaction object
  //
  // @param pricingDetailTrx - WPA* transaction
  // @param DataHandle - a dataHandle to use
  // @param trx - should point to a PricingDetailTrx
  // @param recordQuote - indicator about RQ usage
  // @param rebook - rebook indicator
  // @return AltPricingDetailTrx object
  //--------------------------------------------------------------------------
  template <typename T>
  T* createAltPricingDetailTrx(PricingDetailTrx* pricingDetailTrx,
                               DataHandle& dataHandle,
                               bool recordQuote,
                               bool rebook);

  AltPricingDetailObFeesTrx* createAltPricingDetailTrx(PricingDetailTrx* pricingDetailTrx,
                                                       DataHandle& dataHandle,
                                                       bool recordQuote,
                                                       bool rebook);
  //--------------------------------------------------------------------------
  // @function XformClientXML::getSelectionXml
  //
  // Description: Erases parts from the response to make it suitable for WPA* output
  //
  // @param paxDetailsOrder - Ordered vector of PaxDetails
  // @param accompRestrictionInfo - Accompanied travel restrictions info
  // @param optionXmlVec - vector that has xml data for passenger options
  // @param currentPaxDetail - Pax detail to process
  // @param rebook - rebook indicator
  // @param totals - Money amount to be returned
  // @return true if successful, otherwise false
  //--------------------------------------------------------------------------
  bool getSelectionXml(Trx*& trx,
                       const std::vector<PaxDetail*>& paxDetailsOrder,
                       AltPricingTrx::AccompRestrictionInfo* accompRestrictionInfo,
                       std::vector<std::string>& optionXmlVec,
                       PaxDetail* currentPaxDetail,
                       bool rebook,
                       MoneyAmount& totals);

  //--------------------------------------------------------------------------
  // @function XformClientXML::updateC56Tag
  //
  // Description:Updates C56 tag (total price amount) in the XML response
  //
  // @param selXml - xml string to be updated
  // @param totals - Money amount for C56
  // @param nDec - Number of decimals to be recorded
  // @return true if successful, otherwise false
  //--------------------------------------------------------------------------
  bool updateC56Tag(std::string& selXml, const MoneyAmount& totals, const CurrencyNoDec& nDec);
  std::string formatBaggageResponse(std::string baggageResponse, int& recNum);
  std::string createErrorMessage(const tse::ErrorResponseException& ere);
  bool validatePbbRequest(PricingTrx* pricingTrx, std::string& errorMessage);
  bool isValidBrandCode(const BrandCode& brandCode);
  void eraseVclInformation(std::string& selectionXml, Trx*& trx  );
  void addVCLinPXI(std::string& selectionXML, std::string& vclInfo);

  bool processWpRequest(Trx*& trx,
                        DataHandle& dataHandle,
                        const char*& content,
                        const SelectionContentHandler& selectionDocHandler);
  bool processWpanRequest(Trx*& trx,
                          DataHandle& dataHandle,
                          const char*& content,
                          const SelectionContentHandler& selectionDocHandler);
  bool skipServiceFeeTemplate(PricingDetailTrx* trx, const char *msg) const;

private:
  bool validateLegIds(PricingTrx* pricingTrx, std::string& errorMessage);
  void initArunkSegLegIds(TravelSegPtrVec& segments);
  bool validateBrandParity(const TravelSegPtrVec& segments) const;
  void sortXmlTags(const char* content) const;

  // Config stuff
  std::string _cfgFileName;
  std::string _detailCfgFileName;
  std::string _fareDisplayCfgFileName;
  std::string _mileageCfgFileName;
  std::string _currencyCfgFileName;
  std::string _pricingDisplayCfgFileName;
  std::string _pricingCfgFileName;
  std::string _pricingDetailCfgFileName;

  const static std::string ME_RESPONSE_OPEN_TAG;
  const static std::string ME_RESPONSE_CLOSE_TAG;
  const static std::string FC_SURFACE_RESTRICTED;
  const static std::string FF_OPENING_TAG;

  uint32_t _maxTotalBuffSize = 0;

}; // End class XformClientXML

} // End namespace tse

