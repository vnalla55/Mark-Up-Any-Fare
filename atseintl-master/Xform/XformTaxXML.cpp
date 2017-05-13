//----------------------------------------------------------------------------
//
//  File:  XformTaxXML.cpp
//  Description: See XformTaxXML.h file
//  Created:  December 15, 2004
//  Authors:  Mike Carroll
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
#include "Xform/XformTaxXML.h"

#include "Common/ErrorResponseException.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/BaggageTrx.h"
#include "DataModel/DecodeTrx.h"
#include "DataModel/FrequentFlyerTrx.h"
#include "DataModel/TktFeesPricingTrx.h"
#include "DataModel/Trx.h"
#include "Server/TseServer.h"
#include "Xform/AncillaryBaggageResponseFormatter.h"
#include "Xform/AncillaryPricingRequestHandler.h"
#include "Xform/AncillaryPricingResponseFormatter.h"
#include "Xform/AncillaryPricingResponseFormatterPostTkt.h"
#include "Xform/BaggageRequestHandler.h"
#include "Xform/BaggageResponseFormatter.h"
#include "Xform/CustomXMLParser/IXMLUtils.h"
#include "Xform/DecodeContentHandler.h"
#include "Xform/DecodeResponseFormatter.h"
#include "Xform/FrequentFlyerResponseFormatter.h"
#include "Xform/FrequentFlyerContentHandler.h"
#include "Xform/PfcDisplayModelMap.h"
#include "Xform/PfcDisplayResponseFormatter.h"
#include "Xform/STLTicketingFeesRequestHandler.h"
#include "Xform/STLTktFeesPricingResponseFormatter.h"
#include "Xform/TaxContentHandler.h"
#include "Xform/TaxDisplayModelMap.h"
#include "Xform/TaxDisplayResponseFormatter.h"
#include "Xform/TaxInfoModelMap.h"
#include "Xform/TaxInfoResponseFormatter.h"
#include "Xform/TaxModelMap.h"
#include "Xform/TaxNewShoppingRequestHandler.h"
#include "Xform/TaxOTAModelMap.h"
#include "Xform/TaxOTAResponseFormatter.h"
#include "Xform/TaxResponseFormatter.h"
#include "Xform/TaxShoppingRequestHandler.h"
#include "Xform/TaxShoppingRequestHandlerOld.h"
#include "Xform/TaxShoppingResponseFormatter.h"
#include "Xform/TicketingFeesRequestHandler.h"
#include "Xform/TktFeesPricingResponseFormatter.h"

#include <string>

#include <boost/algorithm/string.hpp>
#include <Xform/AtpcoTaxDisplayResponseFormatter.h>

using namespace XERCES_CPP_NAMESPACE;

namespace tse
{
FIXEDFALLBACK_DECL(freqFlyerNewFormat)
FIXEDFALLBACK_DECL(memoryOptimizationForTaxShopping)
namespace
{

bool
isFirstTag(const char* content, const std::string& tag)
{
  const char* firstTagStart = strstr(content, "<");
  std::string tagStartString = "<" + tag;
  return (tagStartString.compare(0, std::string::npos,
                                 firstTagStart, 0, tagStartString.size()) == 0);
}

std::string getFirstTag(const char* xml)
{
  const char* firstTagStart = strstr(xml, "<");

  if (!firstTagStart)
    return std::string();

  const char* begin = firstTagStart + 1;
  const char* end = firstTagStart;
  while (const char c = *end)
  {
    if (isspace(c) || c == '>' || c == '/')
      break;
    end++;
  }

  return std::string(begin, end - begin);
}

/// helper, for namepaces
const char*
findTag(const char* content, const char* tag)
{
  std::string tagname(tag);
  std::string tagtmp;
  tagtmp = "<" + tagname;
  const char* tmp = strstr(content, tagtmp.c_str());
  if (tmp == nullptr)
  {
    tagtmp = ":" + tagname;
    tmp = strstr(content, tagtmp.c_str());
  }
  return tmp;
}

// Tries exact match, if fails try match tag in upper case
const char*
findTagTryUpper(const char* content, const char* tag)
{
  const char* ret = nullptr;

  ret = findTag(content, tag);

  if (ret)
    return ret;

  std::string tagStr(tag);
  boost::to_upper(tagStr);

  ret = findTag(content, tagStr.c_str());

  return ret;
}

const std::string TAX_SHOPPING_ROOT_ELEMENT = "TRQ";
const std::string TAX_NEW_SHOPPING_ROOT_ELEMENT = "TAX";
const std::string TAX_NEW_SHOPPING_ROOT_ELEMENT_FULL = "TaxRq";
const std::string FREQUENT_FLYER_ROOT_ELEMENT = "FrequentFlyerRequest";
} // namespace

// Config file default
const char* defaultTaxCfgFileName = "taxRequest.cfg";
const std::string DATAHANDLER_CONFIGS_KEY = "DATAHANDLER_CONFIGS";
const std::string TAX_CONFIG_KEY = "TAX_CONFIG";
const std::string OTA_TAX_CONFIG_KEY = "TAX_OTA_CONFIG";
const std::string TAX_DISPLAY_CONFIG_KEY = "TAX_DISPLAY_CONFIG";
const std::string PFC_DISPLAY_CONFIG_KEY = "PFC_DISPLAY_CONFIG";
const std::string TAX_INFO_CONFIG_KEY = "TAX_INFO_CONFIG";

const uint16_t TRX_ID_LENGTH(8);

// Logger stuff
const std::string LOG_FILE_KEY = "LOG_FILE";
const std::string LOG_LEVEL_KEY = "LOG_LEVEL";
const std::string LOG_CONSOLE_KEY = "LOG_CONSOLE";

static Logger
logger("atseintl.Xform.XformTaxXML");

const std::string XformTaxXML::_namespaceTag = "xmlns:";

static LoadableModuleRegister<Xform, XformTaxXML>
_("libXformTaxXML.so");

//----------------------------------------------------------------------------
// XformTaxXML::XformTaxXML
//----------------------------------------------------------------------------
XformTaxXML::XformTaxXML(const std::string& name, ConfigMan& config)
  : Xform(name, config),
  _taxShoppingCfg(config)
{}

XformTaxXML::XformTaxXML(const std::string& name, TseServer& srv)
  : Xform(name, srv.config()),
  _taxShoppingCfg(srv.config())
{}

//----------------------------------------------------------------------------
// XformTaxXML::initialize
//----------------------------------------------------------------------------
bool
XformTaxXML::initialize(int argc, char* argv[])
{
  // Initialize logging and configuration
  if (!initializeConfig())
    return false;
  try { XMLPlatformUtils::Initialize(); }
  catch (const XMLException& e)
  {
    char* msgTxt = XMLString::transcode(e.getMessage());
    LOG4CXX_ERROR(logger, "XMLException: " << msgTxt);
    delete[] msgTxt;
    return false;
  }
  return true;
}

//----------------------------------------------------------------------------
// @function XformTaxXML::convert
//
// Description: Given an XML formatted TAX request message, take that message
//              and decompose its component tags into a Trx object.
//
// @param dataHandle - a dataHandle to use
// @param request - The XML formatted TAX request
// @param trx - a valid Trx reference
// @return bool - success or failure
//----------------------------------------------------------------------------
bool
XformTaxXML::convert(DataHandle& dataHandle, std::string& request, Trx*& trx, bool throttled)
{
  // Parse content into Trx
  try
  {
    boost::replace_all(request, "\0", "");

    std::vector<std::string> namespaces;
    collectNamespaces(namespaces, request);

    for (const std::string& ns : namespaces)
    {
      boost::replace_all(request, ns + ":", "");
    }

    const char* xmlData = request.c_str();
    LOG4CXX_DEBUG(logger, "Request: " << xmlData);

    // Parse the XML
    if (!parse(xmlData, dataHandle, trx, throttled))
      return false;

    if (!trx)
      return false;

    trx->setRawRequest(request);
  }
  catch (const char* toCatch)
  {
    LOG4CXX_WARN(logger, "Exception caught: " << toCatch);
    return false;
  }

  return true;
}

//----------------------------------------------------------------------------
// XformTaxXML::initializeConfig
//----------------------------------------------------------------------------
const bool
XformTaxXML::initializeConfig()
{

  const std::string errorMsg = "Error: failed to read config file: ";
  // Default V2 tax configuration file
  if (!_config.getValue(TAX_CONFIG_KEY, _cfgFileName, DATAHANDLER_CONFIGS_KEY))
  {
    std::cerr << "Error: unable to find log file " << TAX_CONFIG_KEY << " under ["
              << DATAHANDLER_CONFIGS_KEY << "]" << std::endl;
    return false;
  }
  else
  {
    // Load configuration
    if (!_localConfig.read(_cfgFileName))
    {
      // Logger not available yet.
      LOG4CXX_ERROR(logger, errorMsg << _cfgFileName);
      return false;
    }
  }
  // Default OTATax Request configuration file
  if (!_config.getValue(OTA_TAX_CONFIG_KEY, _otaCfgFileName, DATAHANDLER_CONFIGS_KEY))
  {
    std::cerr << "Error: unable to find log file " << OTA_TAX_CONFIG_KEY << " under ["
              << DATAHANDLER_CONFIGS_KEY << "]" << std::endl;
    return false;
  }
  else
  {
    // Load configuration
    if (!_localOTAConfig.read(_otaCfgFileName))
    {
      // Logger not available yet.
      LOG4CXX_ERROR(logger, "Error: failed to read config file: " << _otaCfgFileName);
      return false;
    }
  }
  // Default Tax Display Request configuration file
  if (!_config.getValue(TAX_DISPLAY_CONFIG_KEY, _displayCfgFileName, DATAHANDLER_CONFIGS_KEY))
  {
    std::cerr << "Error: unable to find log file " << TAX_DISPLAY_CONFIG_KEY << " under ["
              << DATAHANDLER_CONFIGS_KEY << "]" << std::endl;
    return false;
  }
  else
  {
    // Load configuration
    if (!_localDisplayConfig.read(_displayCfgFileName))
    {
      // Logger not available yet.
      LOG4CXX_ERROR(logger, "Error: failed to read config file: " << _displayCfgFileName);
      return false;
    }
  }
  // Default PFC Display Request configuration file
  if (!_config.getValue(PFC_DISPLAY_CONFIG_KEY, _pfcDisplayCfgFileName, DATAHANDLER_CONFIGS_KEY))
  {
    std::cerr << "Error: unable to find log file " << PFC_DISPLAY_CONFIG_KEY << " under ["
              << DATAHANDLER_CONFIGS_KEY << "]" << std::endl;
    return false;
  }
  else
  {
    // Load configuration
    if (!_localPfcDisplayConfig.read(_pfcDisplayCfgFileName))
    {
      // Logger not available yet.
      LOG4CXX_ERROR(logger, "Error: failed to read config file: " << _pfcDisplayCfgFileName);
      return false;
    }
  }

  // Default info tax interface request configuration file
  if (!_config.getValue(TAX_INFO_CONFIG_KEY, _taxInfoInterfaceCfgFileName, DATAHANDLER_CONFIGS_KEY))
  {
    std::cerr << "Error: unable to find log file " << TAX_INFO_CONFIG_KEY << " under ["
              << DATAHANDLER_CONFIGS_KEY << "]" << std::endl;
    return false;
  }
  else
  {
    // Load configuration
    if (!_localTaxInfoInterfaceConfig.read(_taxInfoInterfaceCfgFileName))
    {
      // Logger not available yet.
      LOG4CXX_ERROR(logger, "Error: failed to read config file: " << _taxInfoInterfaceCfgFileName);
      return false;
    }
  }

  return true;
}

//----------------------------------------------------------------------------
// XformTaxXML::convert
//----------------------------------------------------------------------------
bool
XformTaxXML::convert(Trx& trx, std::string& response)
{
  std::string tmpResponse;

  // Currently, only supporting Tax transactions
  if (PricingTrx* pricingTrx = dynamic_cast<PricingTrx*>(&trx))
  {
    if (!pricingTrx->taxRequestToBeReturnedAsResponse().empty())
    {
      response  = pricingTrx->taxRequestToBeReturnedAsResponse();
      LOG4CXX_DEBUG(logger, "Response: " << response);
      return true;
    }
  }

  if (typeid(trx) == typeid(TaxTrx))
  {
    TaxTrx& taxTrx = static_cast<TaxTrx&>(trx);
    LOG4CXX_DEBUG(logger, taxTrx.requestType());

    if (taxTrx.requestType() == OTA_REQUEST)
    {
      if (taxTrx.otaRequestRootElementType() == TAX_SHOPPING_ROOT_ELEMENT)
      {
        TaxShoppingResponseFormatter responseFormatter(&taxTrx);
        responseFormatter.formatResponse();
      }
      else
      {
        TaxOTAResponseFormatter responseFormatter;
        responseFormatter.formatResponse(taxTrx);
      }
    }
    else if (taxTrx.requestType() == NEW_OTA_REQUEST)
    {
    }
    else if (taxTrx.requestType() == DISPLAY_REQUEST)
    {
      TaxDisplayResponseFormatter responseFormatter;
      responseFormatter.formatResponse(taxTrx);
    }
    else if (taxTrx.requestType() == PFC_DISPLAY_REQUEST)
    {
      PfcDisplayResponseFormatter responseFormatter;
      responseFormatter.formatResponse(taxTrx);
    }
    else if (taxTrx.requestType() == ATPCO_DISPLAY_REQUEST)
    {
      AtpcoTaxDisplayResponseFormatter responseFormatter;
      responseFormatter.formatResponse(taxTrx);
    }
    else if (taxTrx.requestType() == TAX_INFO_REQUEST)
    {
      // new common interface response formatter
      TaxInfoResponseFormatter responseFormatter;
      responseFormatter.formatResponse(taxTrx);
    }
    else
    {
      TaxResponseFormatter responseFormatter;
      responseFormatter.formatResponse(taxTrx);
    }
    response = taxTrx.response().str();
    LOG4CXX_DEBUG(logger, "Response: " << response);
  }
  else if (typeid(trx) == typeid(AncillaryPricingTrx))
  {
    LOG4CXX_DEBUG(logger, "AncillaryPricingTrx response: " << trx.xml2());
    AncillaryPricingTrx& ancPrcTrx = static_cast<AncillaryPricingTrx&>(trx);
    AncRequest* ancReq = static_cast<AncRequest*>(ancPrcTrx.getRequest());

    if (ancReq->isPostTktRequest())
    {
      AncillaryPricingResponseFormatterPostTkt responseFormatter;
      response = responseFormatter.formatResponse(tmpResponse, ancPrcTrx);
    }
    else if (ancReq->isWPBGRequest())
    {
      AncillaryBaggageResponseFormatter responseFormatter(ancPrcTrx);
      response = responseFormatter.formatResponse(tmpResponse);
    }
    else
    {
      AncillaryPricingResponseFormatter responseFormatter;
      response = responseFormatter.formatResponse(tmpResponse, ancPrcTrx);
    }
    LOG4CXX_DEBUG(logger, "Response: " << response);
  }
  else if (typeid(trx) == typeid(BaggageTrx))
  {
    LOG4CXX_DEBUG(logger, "BaggageTrx response: " << trx.xml2());
    BaggageTrx& bgTrx = static_cast<BaggageTrx&>(trx);
    BaggageResponseFormatter responseFormatter;
    response = responseFormatter.formatResponse(tmpResponse, bgTrx);
    LOG4CXX_DEBUG(logger, "Response: " << response);
  }
  else if (typeid(trx) == typeid(TktFeesPricingTrx))
  {
    LOG4CXX_DEBUG(logger, "TktFeesPricingTrx response: " << trx.xml2());
    TktFeesPricingTrx& tktFeesTrx = static_cast<TktFeesPricingTrx&>(trx);
    STLTktFeesPricingResponseFormatter responseFormatter;
    response = responseFormatter.formatResponse(tmpResponse, tktFeesTrx);
    LOG4CXX_DEBUG(logger, "Response: " << response);
  }
  else if (typeid(trx) == typeid(DecodeTrx))
  {
    LOG4CXX_DEBUG(logger, "Generating DecodeTrx response");
    DecodeTrx& decodeTrx = static_cast<DecodeTrx&>(trx);
    DecodeResponseFormatter formatter(decodeTrx);
    response = formatter.formatResponse();
    LOG4CXX_DEBUG(logger, "response: " << response);
  }
  else if (typeid(trx) == typeid(FrequentFlyerTrx))
  {
    LOG4CXX_DEBUG(logger, "FrequentFlyerTrx response: " << trx.xml2());
    FrequentFlyerTrx& frequentFlyerTrx = static_cast<FrequentFlyerTrx&>(trx);
    FrequentFlyerResponseFormatter responseFormatter(frequentFlyerTrx);
    response = responseFormatter.formatResponse();
  }
  else
  {
    LOG4CXX_WARN(logger, "Transaction not supported!");
    return false;
  }

  return true;
}

//----------------------------------------------------------------------------
// XformTaxXML::convert
//----------------------------------------------------------------------------
bool
XformTaxXML::convert(tse::ErrorResponseException& ere, Trx& trx, std::string& response)
{
  if (typeid(trx) == typeid(TaxTrx))
  {
    TaxTrx* taxTrx = static_cast<TaxTrx*>(&trx);
    LOG4CXX_DEBUG(logger, taxTrx->requestType());
    if (taxTrx->requestType() == OTA_REQUEST)
    {
      if (taxTrx->otaRequestRootElementType() == TAX_SHOPPING_ROOT_ELEMENT)
      {
        TaxShoppingResponseFormatter responseFormatter(taxTrx);
        responseFormatter.formatResponse(ere);
      }
      else
      {
        TaxOTAResponseFormatter responseFormatter;
        responseFormatter.formatResponse(*taxTrx, ere);
      }

      response = taxTrx->response().str();
    }
    else if (taxTrx->requestType() == DISPLAY_REQUEST)
    {
      TaxDisplayResponseFormatter responseFormatter;
      responseFormatter.formatResponse(*taxTrx, ere);
      response = taxTrx->response().str();
    }
    else if (taxTrx->requestType() == PFC_DISPLAY_REQUEST)
    {
      PfcDisplayResponseFormatter responseFormatter;
      responseFormatter.formatResponse(*taxTrx, ere);
      response = taxTrx->response().str();
    }
    else if (taxTrx->requestType() == TAX_INFO_REQUEST)
    {
    }
    else
    {
      response = formatResponse(ere.message());
    }
    LOG4CXX_DEBUG(logger, "Response: " << response);
  }
  else if (typeid(trx) == typeid(AncillaryPricingTrx))
  {
    LOG4CXX_DEBUG(logger, "AncillaryPricingTrx response: " << trx.xml2());
    AncillaryPricingTrx& ancPrcTrx = static_cast<AncillaryPricingTrx&>(trx);
    // Ancillary Response Formatter
    AncRequest* ancReq = static_cast<AncRequest*>(ancPrcTrx.getRequest());
    if(ancReq->isPostTktRequest())
    {
      AncillaryPricingResponseFormatterPostTkt responseFormatter;
      response = responseFormatter.formatResponse(createErrorMessage(ere), ancPrcTrx, ere.code());
    }
    else
    {
      AncillaryPricingResponseFormatter responseFormatter;
      response = responseFormatter.formatResponse(createErrorMessage(ere), ancPrcTrx, ere.code());
    }
    LOG4CXX_DEBUG(logger, "Response: " << response);
  }
  else if (typeid(trx) == typeid(BaggageTrx))
  {
    LOG4CXX_DEBUG(logger, "BaggageTrx response: " << trx.xml2());
    BaggageTrx& bgTrx = static_cast<BaggageTrx&>(trx);
    BaggageResponseFormatter responseFormatter;
    response = responseFormatter.formatResponse(createErrorMessage(ere), bgTrx, ere.code());
    LOG4CXX_DEBUG(logger, "Response: " << response);
  }
  else if (typeid(trx) == typeid(TktFeesPricingTrx))
  {
    LOG4CXX_DEBUG(logger, "TktFeesPricingTrx response: " << trx.xml2());
    TktFeesPricingTrx& tktFeesTrx = static_cast<TktFeesPricingTrx&>(trx);
    STLTktFeesPricingResponseFormatter responseFormatter;
    response = responseFormatter.formatResponse(createErrorMessage(ere), tktFeesTrx, ere.code());
  }
  else if (typeid(trx) == typeid(DecodeTrx))
  {
    LOG4CXX_DEBUG(logger, "DecodeTrx response: " << ere.message());
    DecodeTrx& decodeTrx = static_cast<DecodeTrx&>(trx);
    decodeTrx.addToResponse(ere.message());
    DecodeResponseFormatter formatter(decodeTrx);
    response = formatter.formatResponse('E');
  }
  else if (typeid(trx) == typeid(FrequentFlyerTrx))
  {
    LOG4CXX_DEBUG(logger, "FrequentFlyerTrx response: " << trx.xml2());
    FrequentFlyerTrx& frequentFlyerTrx = static_cast<FrequentFlyerTrx&>(trx);
    FrequentFlyerResponseFormatter responseFormatter(frequentFlyerTrx);
    response = responseFormatter.formatResponse();
  }
  else
  {
    LOG4CXX_WARN(logger, "Transaction not supported!");
    return false;
  }
  return true;
}

bool
XformTaxXML::throttle(std::string& request, std::string& response)
{
  const ErrorResponseException ere(ErrorResponseException::TRANSACTION_THRESHOLD_REACHED);

  const std::string& firstTag = getFirstTag(request.c_str());

  //find response formatter
  if (firstTag == "TRQ")
  {
    TaxShoppingResponseFormatter::formatResponse(ere, response);
  }
  else if (firstTag == "TAX" || firstTag == "TaxRq" || firstTag == "TaxRQ")
  {
    TaxOTAResponseFormatter formatter;
    formatter.formatResponse("", "1.0.0", ere, response);
  }
  else if (firstTag == "AirTaxRQ")
  {
    TaxOTAResponseFormatter formatter;
    formatter.formatResponse("AIRTAXRQ", "1.0.0", ere, response);
  }
  else if (firstTag == "TaxDisplayRQ")
  {
    TaxDisplayResponseFormatter formatter;
    formatter.formatResponse("", ere, response);
  }
  else if (firstTag == "AirTaxDisplayRQ")
  {
    TaxDisplayResponseFormatter formatter;
    formatter.formatResponse("AIRTAXDISPLAYRQ", ere, response);
  }
  else if (firstTag == "PFCDisplayRequest")
  {
    PfcDisplayResponseFormatter::formatResponse(ere, response);
  }
  else if (firstTag == "TaxInfoRequest")
  {
    TaxInfoResponseFormatter::formatResponse(ere, response);
  }
  else if (firstTag == "AncillaryPricingRequest")
  {
    AncillaryPricingResponseFormatter formatter;
    formatter.formatResponse(ere, response);
  }
  else if (firstTag == "BaggageRequest")
  {
    BaggageResponseFormatter formatter;
    formatter.formatResponse(ere, response);
  }
  else if (firstTag == "DecodeRequest")
  {
    DecodeResponseFormatter::formatResponse(ere, response);
  }
  else if (firstTag == "TicketingFeesRequest")
  {
    TktFeesPricingResponseFormatter formatter;
    formatter.formatResponse(ere, response);
  }
  else if (firstTag == "OBTicketingFeeRQ")
  {
    STLTktFeesPricingResponseFormatter formatter;
    formatter.formatResponse(ere, response);
  }
  else if (firstTag == "TAXRequest")
  {
    response = formatResponse(ere.message());
  }
  else
  {
    response = ere.message();
  }
  return true;
}

bool
XformTaxXML::docHnd(tse::ConfigMan& config, DataModelMap& dataModelMap, const char* content)
{
  TaxContentHandler docHandler(dataModelMap);
  if (!docHandler.parse(content))
  {
    LOG4CXX_ERROR(logger, "docHandler.parse failed");
    return false;
  }
  return true;
}

//----------------------------------------------------------------------------
// XformTaxXML::parse
//----------------------------------------------------------------------------
bool
XformTaxXML::parse(const char*& content, DataHandle& dataHandle, Trx*& trx, bool throttled)
{
  bool retCode = true;

  if (isFirstTag(content, "TRQ"))
  {
    LOG4CXX_DEBUG(logger, "Received Tax Shopping Request");

    if (!fallback::fixed::memoryOptimizationForTaxShopping())
    {
      TaxShoppingRequestHandler taxShoppingRequestHandler(trx, _taxShoppingCfg, throttled);
      taxShoppingRequestHandler.parse(dataHandle, content);
    }
    else
    {
      TaxShoppingRequestHandlerOld taxShoppingRequestHandler(trx, _taxShoppingCfg, throttled);
      taxShoppingRequestHandler.parse(dataHandle, content);
    }

    return true;
  }

  if (isFirstTag(content, TAX_NEW_SHOPPING_ROOT_ELEMENT) ||
      isFirstTag(content, TAX_NEW_SHOPPING_ROOT_ELEMENT_FULL))
  {
    LOG4CXX_DEBUG(logger, "Received New Tax Shopping Request");

    TaxNewShoppingRequestHandler taxNewShoppingRequestHandler(trx);
    taxNewShoppingRequestHandler.parse(dataHandle, content);

    return true;
  }

  // OTA Tax Requests

  const char* otaContent;
  const char* flightInfo;

  if ((otaContent = findTag(content, "TaxRQ")) == nullptr)
    otaContent = findTag(content, "AirTaxRQ");

  // Tax Display Requests

  const char* displayContent;

  if ((displayContent = findTag(content, "TaxDisplayRQ")) == nullptr)
    displayContent = findTag(content, "AirTaxDisplayRQ");

  // PFC Display Request
  const char* pfcDisplayContent = findTag(content, "PFCDisplayRequest");

  // Tax common request
  const char* taxInfoContent = findTag(content, "TaxInfoRequest");

  const char* pricingContent;

  if (pfcDisplayContent != nullptr)
  {
    PfcDisplayModelMap pfcDisplayModelMap(_localPfcDisplayConfig, dataHandle, trx);

    if (!pfcDisplayModelMap.createMap())
    {
      LOG4CXX_ERROR(logger, "Unable to create Pfc Display Model Map!");
      retCode = false;
    }
    else
      retCode = docHnd(_localPfcDisplayConfig, pfcDisplayModelMap, pfcDisplayContent);
  }
  else if (otaContent != nullptr)
  {
    if ((flightInfo = findTag(content, "FlightSegment")) == nullptr)
    {
      LOG4CXX_ERROR(logger, "XML must contain flight segment information ");
      return false;
    }

    LOG4CXX_DEBUG(logger, "Received OTA TaxRequest");

    TaxOTAModelMap taxOTAModelMap(_localOTAConfig, dataHandle, trx);
    if (!taxOTAModelMap.createMap())
    {
      LOG4CXX_ERROR(logger, "Unable to create OTA Tax Model Map!");
      retCode = false;
    }
    else
      retCode = docHnd(_localOTAConfig, taxOTAModelMap, otaContent);
  }
  else if (displayContent != nullptr)
  {
    LOG4CXX_DEBUG(logger, "Received Tax Display Request");
    TaxDisplayModelMap taxDisplayModelMap(_localDisplayConfig, dataHandle, trx);
    if (!taxDisplayModelMap.createMap())
    {
      LOG4CXX_ERROR(logger, "Unable to create Tax Display Model Map!");
      retCode = false;
    }
    else
      retCode = docHnd(_localDisplayConfig, taxDisplayModelMap, displayContent);
  }
  else if (taxInfoContent != nullptr)
  {
    LOG4CXX_DEBUG(logger, "Received Tax Info Request");
    TaxInfoModelMap taxInfoModelMap(_localTaxInfoInterfaceConfig, dataHandle, trx);
    if (!taxInfoModelMap.createMap())
    {
      LOG4CXX_ERROR(logger, "Unable to create Tax Info Model Map!");
      retCode = false;
    }
    else
      retCode = docHnd(_localTaxInfoInterfaceConfig, taxInfoModelMap, taxInfoContent);
  }
  else if ((pricingContent = findTag(content, "AncillaryPricingRequest")) != nullptr)
  {
    std::string strContent(content);
    IXMLUtils::stripnamespaces(strContent);
    LOG4CXX_DEBUG(logger, "Received Ancillary Pricing Info Request");
    AncillaryPricingRequestHandler ancPricingHandler(trx);
    ancPricingHandler.parse(dataHandle, strContent);
  }
  else if ((pricingContent = findTag(content, "BaggageRequest")) != nullptr)
  {
    LOG4CXX_DEBUG(logger, "Received Baggage Request");

    std::string strContent(pricingContent);
    IXMLUtils::stripnamespaces(strContent);

    BaggageRequestHandler baggageHandler(trx);
    baggageHandler.parse(dataHandle, strContent);
  }
  else if ((pricingContent = findTagTryUpper(content, "DecodeRequest")) != nullptr)
  {
    std::string strContent(pricingContent);
    IXMLUtils::stripnamespaces(strContent);

    DecodeContentHandler dch;
    retCode = dch.parse(strContent.c_str());
    retCode = retCode && dch.setTrxType(dataHandle, trx);
    if (retCode)
    {
      DecodeTrx* decodeTrx = static_cast<DecodeTrx*>(trx);
      retCode = dch.setTrx(*decodeTrx);
    }

    if (!retCode)
      LOG4CXX_ERROR(logger, "Unable to handle DecodeRequest");
  }
  else if ((pricingContent = findTag(content, "TicketingFeesRequest")) != nullptr)
  {
    std::string strContent(content);
    IXMLUtils::stripnamespaces(strContent);
    LOG4CXX_DEBUG(logger, "Received Ticketing Fees Info Request");
    TicketingFeesRequestHandler tktFeesHandler(trx);
    tktFeesHandler.parse(dataHandle, strContent);
  }
  else if ((pricingContent = findTag(content, "OBTicketingFeeRQ")) != nullptr)
  {
    std::string strContent(content);
    IXMLUtils::stripnamespaces(strContent);
    LOG4CXX_DEBUG(logger, "Received STL Ticketing Fees Info Request");
    STLTicketingFeesRequestHandler tktFeesHandler(trx);
    tktFeesHandler.parse(dataHandle, strContent);
  }
  else if (!fallback::fixed::freqFlyerNewFormat() &&
           (pricingContent = findTag(content, FREQUENT_FLYER_ROOT_ELEMENT.c_str())))
  {
    LOG4CXX_DEBUG(logger, "Received Frequent Flyer Request");
    FrequentFlyerContentHandler ffch;
    retCode = ffch.parse(pricingContent);
    ffch.setTrxType(dataHandle, trx);
    if (retCode)
    {
      FrequentFlyerTrx* ffTrx = static_cast<FrequentFlyerTrx*>(trx);
      ffch.setTrx(ffTrx);
    }
  }
  else // Non OTA requests ( regular v2 requests)
  {
    TaxModelMap taxModelMap(_localConfig, dataHandle, trx);
    if (!taxModelMap.createMap())
    {
      LOG4CXX_ERROR(logger, "Unable to create Tax Data Model Map!");
      return false;
    }
    else
    {
      if (!trx)
        return false;

      TaxContentHandler docHandler(taxModelMap);
      if (!docHandler.parse(content))
      {
        LOG4CXX_ERROR(logger, "docHandler.parse failed");
        return false;
      }
    }
  }

  return retCode;
}

std::string
XformTaxXML::formatResponse(const std::string& response)
{
  LOG4CXX_INFO(logger, "Response (before XML tagging): " << response);
  std::string xmlResponse;
  xmlResponse.append("<TaxResponse><MSG TXT=\"");
  xmlResponse.append(response);
  xmlResponse.append("\"/></TaxResponse>");
  LOG4CXX_INFO(logger, "Response (after XML tagging): " << xmlResponse);
  return xmlResponse;
}

std::string
XformTaxXML::createErrorMessage(const tse::ErrorResponseException& ere)
{
  return (ere.code() > 0 && ere.message().empty()) ? "UNKNOWN EXCEPTION" : ere.message();
}

void
XformTaxXML::collectNamespaces(std::vector<std::string>& namespaces, const std::string& request)
{
  std::size_t begin = request.find(_namespaceTag);
  uint32_t tagLength = _namespaceTag.length();

  while(begin != std::string::npos)
  {
    std::size_t end = request.find("=", begin);

    namespaces.push_back(request.substr(begin + tagLength, end - begin - tagLength));

    begin = request.find(_namespaceTag, end);
  }
}
} //tse
