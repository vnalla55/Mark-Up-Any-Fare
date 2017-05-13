//----------------------------------------------------------------------------
//
//  File:  XformClientXML.cpp
//  Description: See XformClientXML.h file
//  Created:  March 18, 2004
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

#include "Xform/XformClientXML.h"

#include "Common/Config/DynamicConfigurableFlag.h"
#include "Common/ErrorResponseException.h"
#include "Common/FallbackUtil.h"
#include "Common/FareCalcUtil.h"
#include "Common/Logger.h"
#include "Common/NonFatalErrorResponseException.h"
#include "Common/OBFeesUtils.h"
#include "Common/TrxUtil.h"
#include "DataModel/AltPricingDetailTrx.h"
#include "DataModel/AltPricingTrx.h"
#include "DataModel/DecodeTrx.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/MultiExchangeTrx.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/PaxDetail.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/StructuredRuleTrx.h"
#include "DataModel/TaxTrx.h"
#include "DataModel/Trx.h"
#include "DBAccess/FareCalcConfig.h"
#include "Diagnostic/DiagManager.h"
#include "Diagnostic/DiagTools.h"
#include "Diagnostic/TracksPrinter.h"
#include "FareCalc/FareCalcCollector.h"
#include "FareCalc/FareCalcConsts.h"
#include "FareCalc/FareCalcController.h"
#include "Rules/AccompaniedTravel.h"
#include "Server/TseServer.h"
#include "Xform/AltPricingResponseFormatter.h"
#include "Xform/CurrencyContentHandler.h"
#include "Xform/CurrencyModelMap.h"
#include "Xform/CurrencyResponseFormatter.h"
#include "Xform/CustomXMLParser/IXMLUtils.h"
#include "Xform/DecodeContentHandler.h"
#include "Xform/DecodeResponseFormatter.h"
#include "Xform/ERDRequestProcessor.h"
#include "Xform/FareDisplayContentHandler.h"
#include "Xform/FareDisplayModelMap.h"
#include "Xform/FareDisplayResponseFormatter.h"
#include "Xform/FareDisplayResponseXMLTags.h"
#include "Xform/MileageContentHandler.h"
#include "Xform/MileageModelMap.h"
#include "Xform/MileageResponseFormatter.h"
#include "Xform/NoPNRPricingResponseFormatter.h"
#include "Xform/PricingContentHandler.h"
#include "Xform/PricingDetailContentHandler.h"
#include "Xform/PricingDetailModelMap.h"
#include "Xform/PricingDetailResponseFormatter.h"
#include "Xform/PricingDisplayContentHandler.h"
#include "Xform/PricingDisplayModelMap.h"
#include "Xform/PricingModelMap.h"
#include "Xform/PricingResponseFormatter.h"
#include "Xform/PricingResponseXMLTags.h"
#include "Xform/RebuildXMLUtil.h"
#include "Xform/SelectionContentHandler.h"
#include "Xform/STLTicketingCxrDisplayRequestHandler.h"
#include "Xform/STLTicketingCxrDisplayResponseFormatter.h"
#include "Xform/STLTicketingCxrRequestHandler.h"
#include "Xform/STLTicketingCxrResponseFormatter.h"
#include "Xform/StructuredRulesResponseFormatter.h"
#include "Xform/TaxNewShoppingRequestHandler.h"
#include "Xform/TicketingFeesRequestHandlerWPA.h"
#include "Xform/XMLConvertUtils.h"

#include <boost/algorithm/cxx11/all_of.hpp>
#include <boost/algorithm/cxx11/any_of.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <algorithm>
#include <cctype>
#include <functional>
#include <sstream>
#include <string>

#include <bzlib.h>
#include <ctype.h>

namespace tse
{
FALLBACK_DECL(fallbackBrandedFaresPricing);
FALLBACK_DECL(fallbackValidatingCxrMultiSp);

DynamicConfigurableFlagOn dynamicSoftPassDisabledInTn("TN_PATH", "DISABLE_SOFT_PASS", false);

using namespace XERCES_CPP_NAMESPACE;

// Config file default
const char* defaultCfgFileName = "xmlConfig.cfg";
const std::string DATAHANDLER_CONFIGS_KEY = "DATAHANDLER_CONFIGS";
const std::string DEFAULT_CONFIG_KEY = "DEFAULT_CONFIG";
const std::string DETAIL_CONFIG_KEY = "DETAIL_CONFIG";
const std::string FARE_DISPLAY_CONFIG_KEY = "FARE_DISPLAY_CONFIG";
const std::string TAX_CONFIG_KEY = "TAX_CONFIG";
const std::string MILEAGE_CONFIG_KEY = "MILEAGE_CONFIG";
const std::string CURRENCY_CONFIG_KEY = "CURRENCY_CONFIG";
const std::string PRICING_DISPLAY_CONFIG_KEY = "PRICING_DISPLAY_CONFIG";
const std::string PRICING_CONFIG_KEY = "PRICING_CONFIG";
const std::string PRICING_DETAIL_CONFIG_KEY = "PRICING_DETAIL_CONFIG";

// Logger stuff
const std::string LOG_FILE_KEY = "LOG_FILE";
const std::string LOG_LEVEL_KEY = "LOG_LEVEL";
const std::string LOG_CONSOLE_KEY = "LOG_CONSOLE";

// config params
const std::string MAX_PSS_OUTPUT_KEY = "MAX_PSS_OUTPUT";

const std::string XformClientXML::ME_RESPONSE_OPEN_TAG = "<RexPricingResponse S96=\"ME\">";
const std::string XformClientXML::ME_RESPONSE_CLOSE_TAG = "</RexPricingResponse>";
const std::string XformClientXML::FC_SURFACE_RESTRICTED =
    "ISSUE SEPARATE TICKETS-INTL SURFACE RESTRICTED";

const uint16_t
TRX_ID_LENGTH(8);

static Logger
logger("atseintl.Xform.XformClientXML");

static LoadableModuleRegister<Xform, XformClientXML>
_("libXformClientXML.so");

namespace
{
std::string
getFirstTag(const char* xml)
{
  const char* firstTagStart = std::strstr(xml, "<");

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

  std::string result(begin, end - begin);
  std::transform(result.begin(), result.end(), result.begin(), (int (*)(int))toupper);
  return result;
}

template <bool IGNORE_WHITESPACES>
bool
movePtr(const char wanted, const char*& ptr)
{
  while (const char c = *ptr++)
  {
    if (c == wanted)
      return true;

    // current character is not our wanted
    // but only space is allowed here

    if (IGNORE_WHITESPACES && !isspace(c))
      break;
  }

  return false;
}

bool
movePtrAfterCharacter(const char wanted, const char*& ptr)
{
  return movePtr<false>(wanted, ptr);
}

bool
movePtrAfterCharacterIgnoringWhitespaces(const char wanted, const char*& ptr)
{
  return movePtr<true>(wanted, ptr);
}

bool
isTaxRequest(const char* xml)
{
  const char TaxRq[] = "<TaxRq ";
  const char TAX[] = "<TAX ";
  return strncmp(xml, TaxRq, sizeof(TaxRq) - 1) == 0 || strncmp(xml, TAX, sizeof(TAX) - 1) == 0;
}
} // anonymous namespace

XformClientXML::XformClientXML(const std::string& name, TseServer& srv) : Xform(name, srv.config())
{
}

XformClientXML::~XformClientXML()
{
  try
  {
    XMLPlatformUtils::Terminate();
  }
  catch (const XMLException& e)
  {
    char* msgTxt = XMLString::transcode(e.getMessage());
    delete[] msgTxt;
  }
  catch (...)
  {
    // Do nothing. This is a destructor.
  }
}

bool
XformClientXML::initialize(int argc, char* argv[])
{
  // Initialize logging and configuration
  if (!initializeConfig())
    return false;
  XMLConvertUtils::initialize(_maxTotalBuffSize);
  try
  {
    XMLPlatformUtils::Initialize();
  }
  catch (const XMLException& e)
  {
    char* msgTxt = XMLString::transcode(e.getMessage());
    LOG4CXX_ERROR(logger, "XMLException: " << msgTxt);
    delete[] msgTxt;
    return false;
  }
  return true;
}

std::string::size_type
XformClientXML::skipXMLWhitespaces(const std::string& request, const std::string::size_type skipPos)
{
  // Parse Misc ::= white spaces
  // Skip white spaces
  std::string::size_type skipLen = std::string::npos;
  if ((skipLen = request.find_first_not_of("\x20\r\n\t", skipPos, 4)) == std::string::npos)
  {
    std::string error;
    // throw exception
    error = "NO XML ELEMENTS";
    error = "ERROR PARSING REQUEST: " + error;
    LOG4CXX_DEBUG(logger, "REQUEST XML: " << error);
    throw ErrorResponseException(ErrorResponseException::SAX_PARSER_FAILURE, error.c_str());
  }

  return skipLen;
}

std::string::size_type
XformClientXML::skipXMLComment(const std::string& request, const std::string::size_type skipPos)
{
  // Parse Misc ::= comment
  // Skip comment
  std::string::size_type skipLen = std::string::npos;
  if (!(request.compare(skipPos, 4, "<!--")))
  {
    if ((skipLen = request.find("-->", skipPos + 4, 3)) != std::string::npos)
    {
      skipLen += 3;
    }
    else
    {
      // throw exception
      std::string error;
      error = "COMMENT MISSING CLOSING -->";
      error = "ERROR PARSING REQUEST: " + error;
      LOG4CXX_DEBUG(logger, "REQUEST XML: " << error);
      throw ErrorResponseException(ErrorResponseException::SAX_PARSER_FAILURE, error.c_str());
    }
  }
  return skipLen;
}

std::string::size_type
XformClientXML::skipXMLWhitespacesAndComments(const std::string& request,
                                              const std::string::size_type skipPos)
{
  // skip sequence of whitespaces and comments
  std::string::size_type skipLen = skipPos;
  std::string::size_type skipLenNext = skipLen;
  do
  {
    skipLenNext = skipXMLWhitespaces(request, skipLenNext);
    if ((skipLen = skipXMLComment(request, skipLenNext)) != std::string::npos)
    {
      skipLenNext = skipLen;
    }
  } while (skipLen != std::string::npos);

  // Start of XML element reached
  skipLen = skipLenNext;
  return skipLen;
}

std::string::size_type
XformClientXML::skipXMLDecl(const std::string& request, const std::string::size_type skipPos)
{
  // Parse xmlDecl
  // Request from PSS with XML Declaration
  // <?xml version="1.0" encoding="UTF-8"?>
  std::string::size_type skipLen = std::string::npos;
  if (!(request.compare(skipPos, 5, "<?xml")))
  {
    skipLen = skipPos + 5;

    if ((skipLen = request.find("?>", skipLen, 2)) != std::string::npos)
    {
      skipLen += 2;
    }
    else
    {
      std::string error;
      // throw exception
      error = "DECL MISSING CLOSING ?>";
      error = "ERROR PARSING REQUEST: " + error;
      LOG4CXX_DEBUG(logger, "REQUEST XML: " << error);
      throw ErrorResponseException(ErrorResponseException::SAX_PARSER_FAILURE, error.c_str());
    }
  }
  return skipLen;
}

std::string::size_type
XformClientXML::skipXMLProlog(const std::string& request, const std::string::size_type skipPos)
{
  std::string::size_type skipLen = std::string::npos;
  if ((skipLen = skipXMLDecl(request, skipPos)) != std::string::npos)
  {
    skipLen = skipXMLWhitespacesAndComments(request, skipLen);
  }
  return skipLen;
}

//----------------------------------------------------------------------------
// XformClientXML::findFirstRequestXMLElement
// Assumption - '<' character will not appear at TRX_ID_LENGTH position
//              for a request starting with root Request XML Element.
//----------------------------------------------------------------------------
std::string::size_type
XformClientXML::findFirstRequestXMLElement(const std::string& request)
{
  std::string::size_type skipLen = std::string::npos;

  // request begining with 8 byte transaction id immediatly followed by XML Prolog
  if ((skipLen = skipXMLProlog(request, TRX_ID_LENGTH)) == std::string::npos)
  {
    // request starting with XML Prolog.
    // default argument skipPos defaults to 0
    if ((skipLen = skipXMLProlog(request)) == std::string::npos)
    {
      // request begining with 8 byte transaction id immediatly followed by root element.
      if (request[TRX_ID_LENGTH] == '<')
      {
        return TRX_ID_LENGTH;
      }
      else if (request[0] == '<')
      {
        // request starting with root element.
        return 0;
      }
      else
      {
        std::string error;
        skipLen = std::string::npos;
        // throw exception
        error = "FIRST REQUEST XML ELEMENT NOT FOUND";
        error = "ERROR PARSING REQUEST: " + error;
        LOG4CXX_DEBUG(logger, "REQUEST XML: " << error);
        throw ErrorResponseException(ErrorResponseException::SAX_PARSER_FAILURE, error.c_str());
      }
    }
  }
  return skipLen;
}

std::string
XformClientXML::getElementValue(const char* xml, const char* element)
{
  const size_t elementLength = strlen(element);
  const char* ptr = xml;

  while (true)
  {
    ptr = std::strstr(ptr, element);
    if (!ptr)
      return std::string();

    ptr += elementLength;

    // found element in string
    // now only whitespaces or '=' are allowed
    if (!movePtrAfterCharacterIgnoringWhitespaces('=', ptr))
      continue;

    if (!movePtrAfterCharacterIgnoringWhitespaces('"', ptr))
      continue;

    const char* start = ptr;

    if (!movePtrAfterCharacter('"', ptr))
      continue;

    const char* end = ptr - 1;

    return std::string(start, end - start);
  }
}

//----------------------------------------------------------------------------
// @function XformClientXML::convert
//

// Description: Given an XML formatted request message, take that message
//              and decompose its component tags into a Trx object.
//
// @param dataHandle - a dataHandle to use
// @param request - The XML formatted request
// @param trx - a valid Trx reference
// @return bool - success or failure
//----------------------------------------------------------------------------
bool
XformClientXML::convert(DataHandle& dataHandle, std::string& request, Trx*& trx, bool /*throttled*/)
{
  // Parse content into Trx
  try
  {
    const char* xmlData = nullptr;

    LOG4CXX_DEBUG(logger,
                  "Request: "
                      << "convert=" << request);
    // Get to the first Request XML Element.
    xmlData = &(request[(findFirstRequestXMLElement(request))]);
    LOG4CXX_DEBUG(logger, "Request: " << xmlData);

    // Parse the XML
    if (!parse(xmlData, dataHandle, trx))
      throw ErrorResponseException(ErrorResponseException::SAX_PARSER_FAILURE);
    trx->setRawRequest(xmlData);

    if (dynamic_cast<FareDisplayTrx*>(trx))
    {
      FareDisplayTrx* fareDisplayTrx = dynamic_cast<FareDisplayTrx*>(trx);
      trx = fareDisplayTrx;
      Diagnostic& diag = fareDisplayTrx->diagnostic();

      // Leave the diagnosticType of Fare Display Specific Diagnostics to be
      // DiagnosticNone, so that Pricing code will not update this
      // diagnostic object
      diag.diagnosticType() =
          (fareDisplayTrx->isFDDiagnosticRequest()
               ? DiagnosticNone
               : DiagnosticTypes(fareDisplayTrx->getRequest()->diagnosticNumber()));

      if (diag.diagnosticType() != DiagnosticNone || fareDisplayTrx->isFDDiagnosticRequest())
      {
        fareDisplayTrx->diagnostic().activate();
        // set diag args
        if (fareDisplayTrx->getRequest()->diagArgData().size() > 0)
        {
          setDiagArguments(fareDisplayTrx);
        }
      }
    }
    else if (dynamic_cast<TaxTrx*>(trx))
    {
      // do nothing
    }
    else if (dynamic_cast<PricingTrx*>(trx))
    {
      PricingTrx* pricingTrx = static_cast<PricingTrx*>(trx);
      setupDiag(dataHandle, pricingTrx, trx);
    }
    else if (dynamic_cast<MultiExchangeTrx*>(trx))
    {
      PricingTrx* pricingTrx = (static_cast<MultiExchangeTrx*>(trx))->diagPricingTrx();
      setupDiag(dataHandle, pricingTrx, trx);
    }
    else if (dynamic_cast<MileageTrx*>(trx))
    {
      MileageTrx* mileageTrx = static_cast<MileageTrx*>(trx);
      setupDiag(dataHandle, mileageTrx, trx);
    }
    else if ((!dynamic_cast<PricingDetailTrx*>(trx)) && (!dynamic_cast<CurrencyTrx*>(trx)))
    {
      LOG4CXX_WARN(logger, "Transaction not recognized!!");
    }
  }
  catch (const char* toCatch)
  {
    LOG4CXX_WARN(logger, "Exception caught: " << toCatch);
    // @todo What next?
    return false;
  }
  return true;
}

const bool
XformClientXML::initializeConfig()
{
  // Default Pricing configuration file
  if (!_config.getValue(DEFAULT_CONFIG_KEY, _cfgFileName, DATAHANDLER_CONFIGS_KEY))
  {
    std::cerr << "Error: unable to find log file " << DEFAULT_CONFIG_KEY << " under ["
              << DATAHANDLER_CONFIGS_KEY << "]" << std::endl;
    return false;
  }
  else
  {
    // Load configuration
    if (!_localConfig.read(_cfgFileName))
    {
      // Logger not available yet.
      std::cerr << "Error: failed to read config file: " << _cfgFileName << std::endl;
      return false;
    }
  }

  // Detail Pricing configuration file
  if (!_config.getValue(DETAIL_CONFIG_KEY, _detailCfgFileName, DATAHANDLER_CONFIGS_KEY))
  {
    std::cerr << "Error: unable to find log file ["
                 "config entry " << DETAIL_CONFIG_KEY << " under [" << DATAHANDLER_CONFIGS_KEY
              << "]" << std::endl;
    return false;
  }
  else
  {
    // Load configuration
    if (!_localDetailConfig.read(_detailCfgFileName))
    {
      // Logger not available yet.
      std::cerr << "Error: failed to read config file: " << _detailCfgFileName << std::endl;
      return false;
    }
  }

  // Fare Display configuration file
  if (!_config.getValue(FARE_DISPLAY_CONFIG_KEY, _fareDisplayCfgFileName, DATAHANDLER_CONFIGS_KEY))
  {
    std::cerr << "Error: unable to find log file ["
                 "config entry " << FARE_DISPLAY_CONFIG_KEY << " under [" << DATAHANDLER_CONFIGS_KEY
              << "]" << std::endl;
    return false;
  }
  else
  {
    // Load configuration
    if (!_fareDisplayConfig.read(_fareDisplayCfgFileName))
    {
      // Logger not available yet.
      std::cerr << "Error: failed to read config file: " << _fareDisplayCfgFileName << std::endl;
      return false;
    }
  }

  // Mileage request configuration file
  if (!_config.getValue(MILEAGE_CONFIG_KEY, _mileageCfgFileName, DATAHANDLER_CONFIGS_KEY))
  {
    std::cerr << "Error: unable to find log file ["
                 "config entry " << MILEAGE_CONFIG_KEY << " under [" << DATAHANDLER_CONFIGS_KEY
              << "]" << std::endl;
    return false;
  }
  else
  {
    // Load configuration
    if (!_mileageConfig.read(_mileageCfgFileName))
    {
      // Logger not available yet.
      std::cerr << "Error: failed to read config file: " << _mileageCfgFileName << std::endl;
      return false;
    }
  }

  // Currency conversion request configuration file
  if (!_config.getValue(CURRENCY_CONFIG_KEY, _currencyCfgFileName, DATAHANDLER_CONFIGS_KEY))
  {
    std::cerr << "Error: unable to find log file ["
                 "config entry " << CURRENCY_CONFIG_KEY << " under [" << DATAHANDLER_CONFIGS_KEY
              << "]" << std::endl;
    return false;
  }
  else
  {
    // Load configuration
    if (!_currencyConfig.read(_currencyCfgFileName))
    {
      // Logger not available yet.
      std::cerr << "Error: failed to read config file: " << _currencyCfgFileName << std::endl;
      return false;
    }
  }

  // Pricing Display request configuration file
  if (!_config.getValue(
          PRICING_DISPLAY_CONFIG_KEY, _pricingDisplayCfgFileName, DATAHANDLER_CONFIGS_KEY))
  {
    std::cerr << "Error: unable to find log file ["
                 "config entry " << PRICING_DISPLAY_CONFIG_KEY << " under ["
              << DATAHANDLER_CONFIGS_KEY << "]" << std::endl;
    return false;
  }
  else
  {
    // Load configuration
    if (!_pricingDisplayConfig.read(_pricingDisplayCfgFileName))
    {
      // Logger not available yet.
      std::cerr << "Error: failed to read config file: " << _pricingDisplayCfgFileName << std::endl;
      return false;
    }
  }

  // Pricing request configuration file
  if (!_config.getValue(PRICING_CONFIG_KEY, _pricingCfgFileName, DATAHANDLER_CONFIGS_KEY))
  {
    std::cerr << "Error: unable to find log file ["
                 "config entry " << PRICING_CONFIG_KEY << " under [" << DATAHANDLER_CONFIGS_KEY
              << "]" << std::endl;
    return false;
  }
  else
  {
    // Load configuration
    if (!_pricingConfig.read(_pricingCfgFileName))
    {
      // Logger not available yet.
      std::cerr << "Error: failed to read config file: " << _pricingCfgFileName << std::endl;
      return false;
    }
  }

  // Pricing Detail request configuration file
  if (!_config.getValue(
          PRICING_DETAIL_CONFIG_KEY, _pricingDetailCfgFileName, DATAHANDLER_CONFIGS_KEY))
  {
    std::cerr << "Error: unable to find log file ["
                 "config entry " << PRICING_DETAIL_CONFIG_KEY << " under ["
              << DATAHANDLER_CONFIGS_KEY << "]" << std::endl;
    return false;
  }
  else
  {
    // Load configuration
    if (!_pricingDetailConfig.read(_pricingDetailCfgFileName))
    {
      // Logger not available yet.
      std::cerr << "Error: failed to read config file: " << _pricingDetailCfgFileName << std::endl;
      return false;
    }
  }

  // load the PSS output limit
  std::string _name = "OUTPUT";
  if (!_localConfig.getValue(MAX_PSS_OUTPUT_KEY, _maxTotalBuffSize, _name))
  {
    std::cerr << "Error: unable to find log file config entry [" << _name << "."
              << MAX_PSS_OUTPUT_KEY << std::endl;
    return false;
  }

  return true;
}

bool
XformClientXML::convert(Trx& trx, std::string& response)
{
  return trx.convert(response);
}

bool
XformClientXML::convert(tse::ErrorResponseException& ere, Trx& trx, std::string& response)
{
  trx.convert(ere, response);
  return true;
}

bool
XformClientXML::convert(ErrorResponseException& ere, std::string& response)
{
  LOG4CXX_INFO(logger, "In convert ErrorResponseException");
  std::string msg = ere.what();
  std::string::size_type firstOpen = msg.find("<");
  std::string::size_type lastClose = msg.rfind(">");
  if (firstOpen == std::string::npos || lastClose == std::string::npos || firstOpen > lastClose)
  { // not in xml format
    formatResponse(msg, response, "E");
  }
  else
  {
    response = msg;
  }
  return true;
}

bool
XformClientXML::throttle(std::string& request, std::string& response)
{
  ErrorResponseException ere(ErrorResponseException::TRANSACTION_THRESHOLD_REACHED);

  const char* xml = &(request[findFirstRequestXMLElement(request)]);

  const std::string& firstTag = getFirstTag(xml);

  if (firstTag == "PRICINGREQUEST")
  {
    PricingResponseFormatter::formatResponse(ere, false, response);
  }
  else if (firstTag == "ALTPRICINGREQUEST")
  {
    AltPricingResponseFormatter::formatResponse(ere, response);
  }
  else if (firstTag == "REXPRICINGREQUEST")
  {
    PricingResponseFormatter::formatRexResponse(ere, response);
  }
  else if (firstTag == "NOPNRPRICINGREQUEST")
  {
    NoPNRPricingResponseFormatter::formatResponse(ere, response);
  }
  else if (firstTag == "STRUCTUREDRULEREQUEST")
  {
    StructuredRulesResponseFormatter::formatResponse(ere, response);
  }
  else if (firstTag == "PRICINGDISPLAYREQUEST")
  {
    PricingResponseFormatter::formatResponse(ere, true, response);
  }
  else if (firstTag == "FAREDISPLAYREQUEST")
  {
    FareDisplayResponseFormatter::formatResponse(ere, response);
  }
  else if (firstTag == "MILEAGEREQUEST")
  {
    MileageResponseFormatter::formatResponse(ere, response);
  }
  else if (firstTag == "CURRENCYCONVERSIONREQUEST")
  {
    CurrencyResponseFormatter::formatResponse(ere, response);
  }
  else if (firstTag == "DECODEREQUEST")
  {
    DecodeResponseFormatter::formatResponse(ere, response);
  }
  else if (firstTag == "TICKETINGCXRSERVICERQ")
  {
    STLTicketingCxrResponseFormatter::formatResponse(ere, response);
  }
  else if (firstTag == "TICKETINGCXRDISPLAYRQ")
  {
    STLTicketingCxrDisplayResponseFormatter::formatResponse(ere, response);
  }
  else if (firstTag == "SELECTIONREQUEST")
  {
    const std::string a70 = getElementValue(xml, "A70");
    if (a70 == "WP")
      PricingResponseFormatter::formatResponse(ere, false, response);

    else if (a70 == "WTFR")
      AltPricingResponseFormatter::formatResponse(ere, response);

    else if (a70 == "WPA*")
      // WPA* is not handled properly in convert() overload that handles response errors.
      // Unfortunately, we have to copy this logic and produce wrong output too.
      return convert(ere, response);

    else
      // WPDF or WQDF.
      PricingDetailResponseFormatter::formatResponse(ere, response);
  }
  else if (firstTag == "DTL")
  {
    PricingDetailResponseFormatter::formatResponse(ere, response);
  }
  else
  {
    return convert(ere, response);
  }
  return true;
}

bool
XformClientXML::parse(const char*& content, DataHandle& dataHandle, Trx*& trx)
{
  bool retCode = true;
  static const char* sfrRequestXmlTag{"<StructuredRuleRequest"};

  LOG4CXX_INFO(logger, "Incoming message:\n" << content);
  if (!strncasecmp(content, "<PricingRequest", 14) ||
      !strncasecmp(content, "<AltPricingRequest", 17) ||
      !strncasecmp(content, "<RexPricingRequest", 17) ||
      !strncasecmp(content, "<NoPNRPricingRequest", 20) ||
      !strncasecmp(content, sfrRequestXmlTag, 22))
  {
    LOG4CXX_INFO(logger, "Received XML2 PricingRequest");
    // We sort the tags iff we have an SFR request
    if (!strncasecmp(content, sfrRequestXmlTag, 22))
      sortXmlTags(content);
    PricingModelMap pricingModelMap(_pricingConfig, dataHandle, trx);
    if (!pricingModelMap.createMap())
    {
      LOG4CXX_FATAL(logger, "Unable to create Pricing Model Map!");
      retCode = false;
    }
    else
    {
      PricingContentHandler docHandler(pricingModelMap);
      if (!docHandler.parse(content))
        retCode = false;
      else
      {
        trx->xml2() = true;
        // store trx id in billing
        PricingTrx* pricingTrx(dynamic_cast<PricingTrx*>(trx));
        NoPNRPricingTrx* noPNRPricingTrx = dynamic_cast<NoPNRPricingTrx*>(trx);
        if (pricingTrx != nullptr)
        {
          if (pricingTrx->getOptions() == nullptr || pricingTrx->getRequest() == nullptr ||
              pricingTrx->getRequest()->ticketingAgent() == nullptr)
          { // missing data AGI tag not found
            throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                         "Missing Agent Information");
          }

          if (!fallback::fallbackBrandedFaresPricing(trx) &&
              pricingTrx->isPbbRequest() != NOT_PBB_RQ)
          {
            std::string errorMessage;
            if (!validatePbbRequest(pricingTrx, errorMessage))
              throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                           errorMessage.data());
          }

          const FareCalcConfig* fcc =
              FareCalcUtil::getFareCalcConfig(*pricingTrx); // make sure FC Config loaded into trx

          AltPricingTrx* altPricingTrx(dynamic_cast<AltPricingTrx*>(trx));
          if (altPricingTrx != nullptr)
          {
            // If this is a WPA request and WPAPermitted in not on throw an exception
            if (fcc == nullptr ||
                (altPricingTrx->altTrxType() == AltPricingTrx::WPA && fcc->wpaPermitted() != 'Y'))
            {
              throw ErrorResponseException(ErrorResponseException::INVALID_INPUT);
            }

            // If WPA, command pricing is not compatible
            if (!noPNRPricingTrx && altPricingTrx->altTrxType() == AltPricingTrx::WPA &&
                altPricingTrx->getOptions()->fbcSelected())
            {
              throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT);
            }
          }
          if (smp::isPenaltyCalculationRequired(*pricingTrx))
          {
            smp::validatePenaltyInputInformation(*pricingTrx);
          }
        }
      }
    }
  }
  else if (!strncmp(content, "<PRICINGDISPLAYREQUEST", 21) ||
           !strncmp(content, "<PricingDisplayRequest", 21))
  {
    LOG4CXX_INFO(logger, "Received XML2 PricingDisplayRequest");
    PricingDisplayModelMap pricingDisplayModelMap(_pricingDisplayConfig, dataHandle, trx);
    if (!pricingDisplayModelMap.createMap())
    {
      LOG4CXX_FATAL(logger, "Unable to create Pricing Display Model Map!");
      retCode = false;
    }
    else
    {
      PricingDisplayContentHandler docHandler(pricingDisplayModelMap);
      if (!docHandler.parse(content))
        retCode = false;
      else
        trx->xml2() = true;
    }
  }
  else if (!strncmp(content, "<FAREDISPLAYREQUEST", 19) ||
           !strncmp(content, "<FareDisplayRequest", 19))
  {
    LOG4CXX_INFO(logger, "Received XML2 FareDisplayRequest");
    retCode = parseFareDisplay(content, dataHandle, trx);
  }
  else if (!strncmp(content, "<DTL><PRICINGRESPONSE", 21) ||
           !strncmp(content, "<DTL><PricingResponse", 21) ||
           !strncmp(content, "<DTL><UPSELLRESPONSE", 20) ||
           !strncmp(content, "<DTL><UpSellResponse", 20))

  {
    LOG4CXX_INFO(logger, "Received XML2 DTL");
    retCode = parsePricingDetails(content, dataHandle, trx, PricingDetailModelMap::WPDF_AFTER_WP);
  }
  else if (!strncmp(content, "<DTL TKN=\"", 10))
  {
    RebuildXMLUtil rebuildXML(content);
    std::string tktNum;
    rebuildXML.getAttributeValue(content, "TKN", tktNum);
    if (tktNum.empty())
      retCode = false;
    else
    {
      std::map<std::string, std::string> matchAttributes;
      matchAttributes[std::string("TKN")] = tktNum;
      std::string tagEXT("<EXT");
      rebuildXML.copyTo(tagEXT);
      rebuildXML.copyContentOf(tagEXT, "<PCR", matchAttributes, true);
      std::string emptyStr = "";
      rebuildXML.exclusiveCopyTo(emptyStr, tagEXT);
      retCode = parsePricingDetails(
          rebuildXML.outputXML().c_str(), dataHandle, trx, PricingDetailModelMap::WPDF_AFTER_WP);
    }
  }
  else if (!strncmp(content, "<SELECTIONREQUEST", 17) || !strncmp(content, "<SelectionRequest", 17))
  {
    LOG4CXX_INFO(logger, "Received XML2 SelectionRequest");
    SelectionContentHandler selectionDocHandler;
    if (!selectionDocHandler.parse(content))
      retCode = false;
    else
    {
      if (selectionDocHandler.getRequest() == "WPDF")
      {
        retCode = processWpdfnRequest(
            selectionDocHandler.content(), dataHandle, selectionDocHandler.selection(), trx);
      }
      else if (selectionDocHandler.getRequest() == "WP")
      {
        retCode = processWpRequest(trx, dataHandle, content, selectionDocHandler);
      }
      else if (selectionDocHandler.getRequest() == "WTFR")
      {
        bool followingWp = selectionDocHandler.content().empty();
        std::string wtfrContent =
            followingWp ? extractUncompressedContent(content) : selectionDocHandler.content();
        retCode = processWtfrRequest(wtfrContent,
                                     followingWp,
                                     dataHandle,
                                     selectionDocHandler.selection(),
                                     trx,
                                     selectionDocHandler.nomatch());
      }
      else if (selectionDocHandler.getRequest() == "WPA*")
      {
        retCode = processWpanRequest(trx, dataHandle, content, selectionDocHandler);
      }
      else
      {
        LOG4CXX_FATAL(logger, "Invalid Selection Request!");
        retCode = false;
      }
    }
  }

  else if (!strncmp(content, "<MILEAGEREQUEST", 14) || !strncmp(content, "<MileageRequest", 14))
  {
    LOG4CXX_INFO(logger, "Received XML2 MileageRequest");
    MileageModelMap mileageModelMap(_mileageConfig, dataHandle, trx);
    if (!mileageModelMap.createMap())
    {
      LOG4CXX_FATAL(logger, "Unable to create Mileage Model Map!");
      retCode = false;
    }
    else
    {
      MileageContentHandler docHandler(mileageModelMap);
      if (!docHandler.parse(content))
        retCode = false;
      else
        trx->xml2() = true;
    }
    MileageTrx* mileageTrx = dynamic_cast<MileageTrx*>(trx);
    if (mileageTrx == nullptr)
      retCode = false;
    else
    {
      if (mileageTrx->billing() == nullptr || mileageTrx->items().empty())
        retCode = false;
    }
  }
  else if (!strncmp(content, "<CURRENCYCONVERSIONREQUEST", 25) ||
           !strncmp(content, "<CurrencyConversionRequest", 25))
  {
    LOG4CXX_INFO(logger, "Received XML2 CurrencyConversionRequest");
    CurrencyModelMap currencyModelMap(_currencyConfig, dataHandle, trx);
    if (!currencyModelMap.createMap())
    {
      LOG4CXX_FATAL(logger, "Unable to create Currency Model Map!");
      retCode = false;
    }
    else
    {
      CurrencyContentHandler docHandler(currencyModelMap);
      if (!docHandler.parse(content))
        retCode = false;
      else
        trx->xml2() = true;
    }
  }
  else if ((!strncmp(content, "<DECODEREQUEST", 14) || !strncmp(content, "<DecodeRequest", 14)))
  {
    LOG4CXX_INFO(logger, "Received DecodeRequest");
    DecodeContentHandler dch;
    retCode = dch.parse(content);
    retCode = retCode && dch.setTrxType(dataHandle, trx);
    if (retCode)
    {
      DecodeTrx* decodeTrx = static_cast<DecodeTrx*>(trx);
      retCode = dch.setTrx(*decodeTrx);
    }
  }
  else if (!strncasecmp(content, "<TICKETINGCXRSERVICERQ", 22))
  {
    std::string strContent(content);
    IXMLUtils::stripnamespaces(strContent);
    LOG4CXX_INFO(logger, "Received STL2 TicketingCxrService Request");
    STLTicketingCxrRequestHandler tcsHandler(trx);
    tcsHandler.parse(dataHandle, strContent);
  }
  else if (!strncasecmp(content, "<TICKETINGCXRDISPLAYRQ", 22))
  {
    std::string strContent(content);
    IXMLUtils::stripnamespaces(strContent);
    LOG4CXX_INFO(logger, "Received STL2 TicketingCxrDisplayService Request");
    STLTicketingCxrDisplayRequestHandler tcsDispHandler(trx);
    tcsDispHandler.parse(dataHandle, strContent);
  }
  else if (isTaxRequest(content))
  {
    LOG4CXX_DEBUG(logger, "Received New Tax Shopping Request");
    TaxNewShoppingRequestHandler taxNewShoppingRequestHandler(trx);
    taxNewShoppingRequestHandler.parse(dataHandle, content);
  }
  else
  {
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, "Unknown request tag");
  }
  return retCode;
}

bool
XformClientXML::processWpRequest(Trx*& trx,
                                 DataHandle& dataHandle,
                                 const char*& content,
                                 const SelectionContentHandler& selectionDocHandler)
{
  std::string reqContent = content;
  const size_t start = reqContent.find("<REQ");
  const size_t end = reqContent.find("<DTS");

  if (start != std::string::npos && end != std::string::npos)
  {
    const size_t length = reqContent.find("<DTS") - start;
    reqContent = reqContent.substr(start, length) + selectionDocHandler.content();

    return processWpnRequest(reqContent, dataHandle, selectionDocHandler.selection(), trx);
  }

  const std::string error = "INVALID SECTION REQUEST, MISSING REQ OR DTS!";
  LOG4CXX_FATAL(logger, error);
  throw ErrorResponseException(ErrorResponseException::SAX_PARSER_FAILURE, error.c_str());
}

bool
XformClientXML::processWpanRequest(Trx*& trx,
                                   DataHandle& dataHandle,
                                   const char*& content,
                                   const SelectionContentHandler& selectionDocHandler)
{
  if (findOBbfInContentHeader(selectionDocHandler.content()))
  {
    std::string reqContent = content;
    const size_t start = reqContent.find("<REQ");
    const size_t end = reqContent.find("<DTS");

    if (start != std::string::npos && end != std::string::npos)
    {
      const size_t length = reqContent.find("<DTS") - start;
      reqContent = reqContent.substr(start, length) + selectionDocHandler.content();

      Trx* tmpTrx;
      processWpnRequest(reqContent, dataHandle, selectionDocHandler.selection(), tmpTrx);
      PricingDetailTrx* pricingDetailTrx = dynamic_cast<PricingDetailTrx*>(tmpTrx);

      return processWpaDetailRequest<AltPricingDetailObFeesTrx>(pricingDetailTrx,
                                                                reqContent,
                                                                dataHandle,
                                                                trx,
                                                                selectionDocHandler.recordQuote(),
                                                                selectionDocHandler.rebook());
    }

    const std::string error = "INVALID SECTION REQUEST, MISSING REQ OR DTS!";
    LOG4CXX_FATAL(logger, error);
    throw ErrorResponseException(ErrorResponseException::SAX_PARSER_FAILURE, error.c_str());
  }

  PricingDetailTrx* pricingDetailTrx = prepareDetailTrx(
      selectionDocHandler.content(), dataHandle, selectionDocHandler.selection(), trx);

  return processWpaDetailRequest<AltPricingDetailTrx>(pricingDetailTrx,
                                                      selectionDocHandler.content(),
                                                      dataHandle,
                                                      trx,
                                                      selectionDocHandler.recordQuote(),
                                                      selectionDocHandler.rebook());
}

int
XformClientXML::formatResponse(std::string& tmpResponse,
                               std::string& xmlResponse,
                               const std::string& msgType)
{
  return formatResponse(tmpResponse, xmlResponse, 2, msgType);
}

int
XformClientXML::formatResponse(std::string& tmpResponse,
                               std::string& xmlResponse,
                               int recNum,
                               const std::string& msgType,
                               PricingDetailTrx* trx)
{
  LOG4CXX_INFO(logger, "Response (before XML tagging):\n" << tmpResponse);

  size_t totalBuffSize = 0;
  const int BUF_SIZE = 256;
  const int AVAILABLE_SIZE = BUF_SIZE - 52;
  char tmpBuf[BUF_SIZE];

  // Clobber the trailing newline
  while (*(tmpResponse.rbegin()) == '\n')
  {
    tmpResponse.erase(tmpResponse.size() - 1);
  }
  char* pHolder = nullptr;
  size_t tokenLen = 0;
  for (char* token = strtok_r((char*)tmpResponse.c_str(), "\n", &pHolder); token != nullptr;
       token = strtok_r(nullptr, "\n", &pHolder), recNum++)
  {
    tokenLen = strlen(token);
    if (tokenLen == 0)
      continue;
    else if (tokenLen > AVAILABLE_SIZE)
    {
      LOG4CXX_WARN(logger, "Line: [" << token << "] too long!");
      continue;
    }

    if (skipServiceFeeTemplate(trx, token))
    {
      recNum--;
      continue;
    }

    sprintf(tmpBuf, "<MSG N06=\"%s\" Q0K=\"%06d\" S18=\"%s\"/>", msgType.c_str(), recNum, token);

    // if the output buffer is too big for PSS overwrite the last line with error message and get
    // out
    totalBuffSize += strlen(tmpBuf);
    if (totalBuffSize > _maxTotalBuffSize)
    {
      sprintf(tmpBuf,
              "<MSG N06=\"%s\" Q0K=\"%06d\" S18=\"RESPONSE TOO LONG FOR CRT\"/>",
              msgType.c_str(),
              recNum + 1);
      xmlResponse.append(tmpBuf);
      break;
    }
    if (recNum == 1)
      xmlResponse = xmlResponse.insert(0, tmpBuf);
    else
      xmlResponse.append(tmpBuf);
  }
  LOG4CXX_INFO(logger, "Response (after XML tagging):\n" << xmlResponse);
  return recNum;
}

bool
XformClientXML::skipServiceFeeTemplate(PricingDetailTrx* trx, const char* msg) const
{
  if (!trx || !trx->wpnTrx())
    return false;

  return std::strstr(msg, FareCalcConsts::SERVICE_FEE_AMOUNT_LINE.c_str()) ||
         std::strstr(msg, FareCalcConsts::SERVICE_FEE_TOTAL_LINE.c_str());
}

PricingDetailTrx*
XformClientXML::prepareDetailTrx(const std::string& content,
                                 DataHandle& dataHandle,
                                 const std::vector<uint16_t>& selectionList,
                                 Trx*& trx,
                                 PricingDetailModelMap::WpdfType wpdfType)
{
  // make the data look like a regular WPDF request
  const std::string dtlMsg = "<DTL>" + content + "</DTL>";
  if (!parsePricingDetails(dtlMsg.c_str(), dataHandle, trx, wpdfType))
  {
    LOG4CXX_FATAL(logger, "Unable to parse Pricing Detail info");
    return nullptr;
  }

  PricingDetailTrx* pricingDetailTrx = dynamic_cast<PricingDetailTrx*>(trx);
  if (!pricingDetailTrx)
  {
    LOG4CXX_FATAL(logger, "Unable to cast trx to Pricing Detail Trx");
    return nullptr;
  }

  if (!narrowDetailTrx(pricingDetailTrx, selectionList))
  {
    LOG4CXX_FATAL(logger, "Unable to process WPDFn selection");
    return nullptr;
  }

  return pricingDetailTrx;
}

bool
XformClientXML::processWpdfnRequest(const std::string& content,
                                    DataHandle& dataHandle,
                                    const std::vector<uint16_t>& selectionList,
                                    Trx*& trx)
{
  PricingDetailTrx* pricingDetailTrx = prepareDetailTrx(
      content, dataHandle, selectionList, trx, PricingDetailModelMap::WPDF_AFTER_WPX);
  if (pricingDetailTrx == nullptr)
    return false;

  if (!selectionList.empty())
  {
    pricingDetailTrx->selectionChoice() = selectionList.front();
  }
  return true;
}

bool
XformClientXML::parsePricingDetails(const char* msg,
                                    DataHandle& dataHandle,
                                    Trx*& trx,
                                    PricingDetailModelMap::WpdfType wpdfType)
{
  PricingDetailModelMap pricingDetailModelMap(_pricingDetailConfig, dataHandle, trx, wpdfType);
  if (!pricingDetailModelMap.createMap())
  {
    LOG4CXX_FATAL(logger, "Unable to create Pricing Detail Model Map!");
    return false;
  }

  PricingDetailContentHandler detailDocHandler(pricingDetailModelMap);
  if (!detailDocHandler.parse(msg))
  {
    return false;
  }

  trx->xml2() = true;
  return true;
}

bool
XformClientXML::parseFareDisplay(const char* content, DataHandle& dataHandle, Trx*& trx)
{
  FareDisplayModelMap fareDisplayModelMap(_fareDisplayConfig, dataHandle, trx);

  if (!fareDisplayModelMap.createMap())
  {
    LOG4CXX_FATAL(logger, "Unable to create Fare Display Model Map!");
    return false;
  }

  FareDisplayContentHandler docHandler(fareDisplayModelMap);

  if (!docHandler.parse(content))
    return false;

  FareDisplayTrx* fdTrx = dynamic_cast<FareDisplayTrx*>(trx);

  if (!fdTrx)
    return false;

  if (fdTrx->isERD())
  {
    ERDRequestProcessor requestProcessor(*fdTrx);

    if (fdTrx->billing() == nullptr || fdTrx->billing()->requestPath() != SWS_PO_ATSE_PATH)
    {
      // Try to process DTS tag
      std::string dtsContent = docHandler.content();
      if (requestProcessor.prepareDTS(dtsContent))
      {
        const char* dts = dtsContent.c_str();
        if (!docHandler.parse(dts))
          return false;
      }
    }
    if (!requestProcessor.process())
      throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT, "");
  }

  trx->xml2() = true;
  return true;
}

bool
XformClientXML::narrowDetailTrx(PricingDetailTrx* detailTrx,
                                const std::vector<uint16_t>& selectionList)
{
  if (selectionList.empty())
  {
    LOG4CXX_WARN(logger, "invalid arguments - empty selection list");
    return false;
  }

  std::vector<PaxDetail*>& paxDetails = detailTrx->paxDetails();
  const size_t paxDetailsSize = paxDetails.size();

  // collect pointers to selected PaxDetail elements
  std::vector<PaxDetail*> selectedPaxDetails;
  for (const auto element : selectionList)
  {
    if (element > paxDetailsSize || element < 1)
    {
      LOG4CXX_WARN(logger, "selection out of range" << element);
      return false;
    }
    else
    {
      selectedPaxDetails.push_back(paxDetails[element - 1]);
    }
  }
  paxDetails.swap(selectedPaxDetails);

  return true;
}

bool
XformClientXML::narrowDetailTrxObFee(PricingDetailTrx* detailTrx,
                                     const std::vector<uint16_t>& selectionList)
{
  if (!narrowDetailTrx(detailTrx, selectionList))
    return false;

  std::vector<Itin*> itinsToProcess;
  std::vector<Itin*>& trxItins = detailTrx->itin();
  const size_t trxItinsSize = trxItins.size();
  for (const auto elem : selectionList)
  {
    if (elem > trxItinsSize || elem < 1)
      return false;

    itinsToProcess.push_back(trxItins[elem - 1]);
  }
  trxItins.swap(itinsToProcess);

  return true;
}

bool
XformClientXML::findOBbfInContentHeader(const std::string& content)
{
  const size_t lastSUM = content.rfind("</SUM");
  if (lastSUM == std::string::npos)
    return false;

  return content.find("<OBF", lastSUM) != std::string::npos;
}

bool
XformClientXML::processWpnRequest(const std::string& content,
                                  DataHandle& dataHandle,
                                  const std::vector<uint16_t>& selectionList,
                                  Trx*& trx)
{
  if (findOBbfInContentHeader(content))
  {
    const std::string dtlMsg = "<DTL>" + content + "</DTL>";

    TicketingFeesRequestHandlerWPA tktFeesHandler(trx);
    tktFeesHandler.parse(dataHandle, dtlMsg);
    trx->xml2() = true;

    PricingDetailTrx* pricingDetailTrx = dynamic_cast<PricingDetailTrx*>(trx);
    if (!pricingDetailTrx)
    {
      LOG4CXX_FATAL(logger, "Unable to cast trx to Pricing Detail Trx");
      return false;
    }

    pricingDetailTrx->ticketingAgent() = *pricingDetailTrx->getRequest()->ticketingAgent();

    if (!narrowDetailTrxObFee(pricingDetailTrx, selectionList))
    {
      LOG4CXX_FATAL(logger, "Unable to process ObFees selection");
      return false;
    }

    pricingDetailTrx->wpnTrx() = true;
    return true;
  }

  PricingDetailTrx* pricingDetailTrx = prepareDetailTrx(content, dataHandle, selectionList, trx);
  if (!pricingDetailTrx)
    return false;

  pricingDetailTrx->wpnTrx() = true;
  return true;
}

std::string
XformClientXML::formatBaggageResponse(std::string baggageResponse, int& recNum)
{
  std::string result = "";
  size_t loc = 0;
  while ((loc = baggageResponse.find("\\n", loc)) != std::string::npos)
    baggageResponse.replace(loc, 2, "\n");
  recNum = formatResponse(baggageResponse, result, recNum, "Y");
  return result;
}

void
XformClientXML::copyAgentInfo(const Agent& from, Agent& to)
{
  to.agentCity() = from.agentCity();
  to.tvlAgencyPCC() = from.tvlAgencyPCC();
  to.mainTvlAgencyPCC() = from.mainTvlAgencyPCC();
  to.tvlAgencyIATA() = from.tvlAgencyIATA();
  to.homeAgencyIATA() = from.homeAgencyIATA();
  to.agentFunctions() = from.agentFunctions();
  to.agentDuty() = from.agentDuty();
  to.airlineDept() = from.airlineDept();
  to.cxrCode() = from.cxrCode();
  to.currencyCodeAgent() = from.currencyCodeAgent();
  to.coHostID() = from.coHostID();
  to.agentCommissionType() = from.agentCommissionType();
  to.agentCommissionAmount() = from.agentCommissionAmount();
  to.commissionAmount() = from.commissionAmount();
  to.commissionPercent() = from.commissionPercent();
  to.vendorCrsCode() = from.vendorCrsCode();
  to.officeDesignator() = from.officeDesignator();
  to.officeStationCode() = from.officeStationCode();
  to.defaultTicketingCarrier() = from.defaultTicketingCarrier();
}

bool
XformClientXML::processWtfrRequest(const std::string& content,
                                   bool followingWp,
                                   DataHandle& dataHandle,
                                   const std::vector<uint16_t>& selectionList,
                                   Trx*& trx,
                                   bool noMath)
{
  PricingDetailTrx* pricingDetailTrx = prepareDetailTrx(content, dataHandle, selectionList, trx);
  if (pricingDetailTrx == nullptr)
    return false;

  // set trx to be an AltPricingTrx
  AltPricingTrx* altTrx = nullptr;
  dataHandle.get(altTrx);
  PricingRequest* request;
  altTrx->dataHandle().get(request);
  altTrx->setRequest(request);
  altTrx->wtfrFollowingWp() = followingWp;
  altTrx->noMatchPricing() = noMath;
  trx = altTrx;

  // add agent information
  Agent* newAgent;
  altTrx->dataHandle().get(newAgent);
  altTrx->getRequest()->ticketingAgent() = newAgent;
  copyAgentInfo(pricingDetailTrx->ticketingAgent(), *newAgent);

  // add billing information
  Billing* newBilling;
  altTrx->dataHandle().get(newBilling);
  newBilling = pricingDetailTrx->billing();
  altTrx->billing() = newBilling;

  // transfer data for WTFR
  std::vector<std::string> optionXmlVec;
  if (!followingWp)
  {
    if (!extractOptionXml(content, optionXmlVec) || !saveAgentBillingXml(content, *altTrx))
      return false;
  }

  altTrx->accompRestrictionVec().resize(pricingDetailTrx->paxDetails().size());
  AltPricingTrx::AccompRestrictionVec::iterator arIter = altTrx->accompRestrictionVec().begin();
  AltPricingTrx::AccompRestrictionVec::iterator arIterEnd = altTrx->accompRestrictionVec().end();
  std::vector<PaxDetail*>::const_iterator paxIter = pricingDetailTrx->paxDetails().begin();
  std::vector<PaxDetail*>::const_iterator paxIterEnd = pricingDetailTrx->paxDetails().end();

  for (; arIter != arIterEnd && paxIter != paxIterEnd; arIter++, paxIter++)
  {
    uint16_t selNum = (*paxIter)->wpnOptionNumber();
    arIter->selectionNumber() = selNum;
    arIter->validationStr() = (*paxIter)->accTvlData();

    if (std::string::npos != (*paxIter)->wpnDetails().find(FC_SURFACE_RESTRICTED))
      arIter->surfaceRestricted() = true;

    if (!followingWp)
    {
      if (selNum <= optionXmlVec.size())
      {
        arIter->selectionXml() = optionXmlVec[selNum - 1];
      }
      else
      {
        LOG4CXX_WARN(logger, "Selection XML not available for selection #" << selNum);
        return false;
      }
    }
  }

  return true;
}

std::string
XformClientXML::extractUncompressedContent(const char* message)
{
  static const char* const startTag = "<PricingResponse>";
  static const char* const endTag = "</PricingResponse>";
  std::string messageStr(message);
  size_t start = messageStr.find(startTag);
  size_t end = messageStr.find(endTag) + strlen(endTag);
  size_t len = end - start;
  return messageStr.substr(start, len);
}

bool
XformClientXML::extractOptionXml(const std::string& message, std::vector<std::string>& optionXmlVec)
{
  const char* const beginPattern = "<SUM";
  const char* const endPattern = "</SUM>";
  size_t optionBegin = 0;
  size_t optionEnd = 0;
  while ((optionBegin = message.find(beginPattern, optionEnd)) != std::string::npos)
  {
    optionEnd = message.find(endPattern, optionBegin);
    if (optionEnd == std::string::npos)
    {
      LOG4CXX_FATAL(logger, "Could not find end of SUM element");
      return false;
    }
    size_t optionLen = optionEnd + strlen(endPattern) - optionBegin;
    optionXmlVec.push_back(message.substr(optionBegin, optionLen));
  }

  return true;
}

std::string
XformClientXML::extractXmlElement(const std::string& message, const char* beginPattern)
{
  const char* const elementEndPattern = "/>";
  size_t strBegin = message.find(beginPattern);
  if (strBegin == std::string::npos)
    return "";
  size_t strEnd = message.find(elementEndPattern, strBegin);
  if (strEnd == std::string::npos)
    return "";
  size_t strLen = strEnd + strlen(elementEndPattern) - strBegin;
  return message.substr(strBegin, strLen);
}

template <typename T>
bool
XformClientXML::saveAgentBillingXml(const std::string& message, T& altTrx)
{
  const char* const agentBeginPattern = "<AGI";
  const char* const billingBeginPattern = "<BIL";
  altTrx.agentXml() = extractXmlElement(message, agentBeginPattern);
  altTrx.billingXml() = extractXmlElement(message, billingBeginPattern);
  return !altTrx.agentXml().empty() && !altTrx.billingXml().empty();
}

void
XformClientXML::setDiagArguments(PricingTrx* pricingTrx) const
{
  std::string noValue;
  std::map<std::string, std::string>& argMap = pricingTrx->diagnostic().diagParamMap();
  std::string diagArgString;

  PricingRequest* theRequest = pricingTrx->getRequest();
  for (const auto& elem : theRequest->diagArgData())
  {
    size_t len = elem.length();
    if (len > 2)
    {
      argMap[elem.substr(0, 2)] = elem.substr(2);
    }
    else if (len == 2)
    {
      argMap[elem.substr(0, 2)] = noValue;
    }
    else
    {
      LOG4CXX_WARN(logger, "Untidy diagnostic arg: " << elem);
    }
  }
  // Check NFFX to set FareX indicator
  std::map<std::string, std::string>::iterator diagParamI = argMap.find("NF");
  if ((diagParamI != argMap.end()) && (diagParamI->second == "FX") &&
      (pricingTrx->getOptions() != nullptr))
  {
    pricingTrx->getOptions()->fareX() = true;
  }
}

void
XformClientXML::setDiagArguments(MileageTrx* mileageTrx) const
{
  std::string noValue;
  std::map<std::string, std::string>& argMap = mileageTrx->diagnostic().diagParamMap();
  PricingRequest* theRequest = mileageTrx->getRequest();

  for (const auto& elem : theRequest->diagArgData())
  {
    size_t len = elem.length();
    if (len > 2)
    {
      argMap[elem.substr(0, 2)] = elem.substr(2);
    }
    else if (len == 2)
    {
      argMap[elem.substr(0, 2)] = noValue;
    }
    else
    {
      LOG4CXX_WARN(logger, "Untidy diagnostic arg: " << elem);
    }
  }
}

void
XformClientXML::setupDiag(DataHandle& dataHandle, PricingTrx* pricingTrx, Trx*& trx) const
{
  Diagnostic& diag = pricingTrx->diagnostic();
  diag.diagnosticType() = DiagnosticTypes(pricingTrx->getRequest()->diagnosticNumber());

  if (diag.diagnosticType() == Diagnostic150) // Status
  {
    StatusTrx* statusTrx;
    dataHandle.get(statusTrx); // lint !e530
    statusTrx->response() << "OK--FINE\n";
    trx = statusTrx;
  }
  else if (diag.diagnosticType() == Diagnostic151) // Metrics
  {
    MetricsTrx* metricsTrx;
    dataHandle.get(metricsTrx); // lint !e530
    trx = metricsTrx;
  }
  else if (diag.diagnosticType() != DiagnosticNone)
  {
    pricingTrx->diagnostic().activate();

    // Set diag args
    if (pricingTrx->getRequest()->diagArgData().size() > 0)
    {
      setDiagArguments(pricingTrx);
    }
  }

  if (diag.diagParamIsSet(Diagnostic::DISPLAY_DETAIL, Diagnostic::TOPLINE_METRICS))
  {
    pricingTrx->recordMetrics() = true;
    pricingTrx->recordTopLevelMetricsOnly() = true;
  }

  if (diag.diagParamIsSet(Diagnostic::DISPLAY_DETAIL, Diagnostic::FULL_METRICS))
  {
    pricingTrx->recordMetrics() = true;
    pricingTrx->recordTopLevelMetricsOnly() = false;
  }
}

void
XformClientXML::setupDiag(DataHandle& dataHandle, MileageTrx* mileageTrx, Trx*& trx) const
{
  Diagnostic& diag = mileageTrx->diagnostic();
  diag.diagnosticType() = DiagnosticTypes(mileageTrx->getRequest()->diagnosticNumber());

  if (diag.diagnosticType() != DiagnosticNone)
  {
    mileageTrx->diagnostic().activate();
    // Set diag args
    if (mileageTrx->getRequest()->diagArgData().size() > 0)
    {
      setDiagArguments(mileageTrx);
    }
  }
}

template <typename T>
T*
XformClientXML::createAltPricingDetailTrx(PricingDetailTrx* pricingDetailTrx,
                                          DataHandle& dataHandle,
                                          bool recordQuote,
                                          bool rebook)
{
  T* altTrx = nullptr;
  dataHandle.get(altTrx);
  PricingRequest* request;
  PricingOptions* options;
  altTrx->dataHandle().get(request);
  altTrx->dataHandle().get(options);
  altTrx->setRequest(request);
  altTrx->setOptions(options);
  altTrx->getOptions()->recordQuote() = recordQuote ? 'T' : 'F';
  altTrx->rebook() = rebook;

  // add agent information
  Agent* newAgent;
  altTrx->dataHandle().get(newAgent);
  altTrx->getRequest()->ticketingAgent() = newAgent;
  copyAgentInfo(pricingDetailTrx->ticketingAgent(), *newAgent);
  altTrx->vendorCrsCode() = pricingDetailTrx->ticketingAgent().vendorCrsCode();

  // add billing information
  Billing* newBilling;
  altTrx->dataHandle().get(newBilling);
  newBilling = pricingDetailTrx->billing();
  altTrx->billing() = newBilling;

  return altTrx;
}

template <>
AltPricingDetailObFeesTrx*
XformClientXML::createAltPricingDetailTrx(PricingDetailTrx* pricingDetailTrx,
                                          DataHandle& dataHandle,
                                          bool recordQuote,
                                          bool rebook)
{
  AltPricingDetailObFeesTrx* altTrx = nullptr;
  dataHandle.get(altTrx);
  altTrx->setRequest(pricingDetailTrx->getRequest());
  altTrx->setOptions(pricingDetailTrx->getOptions());
  altTrx->getOptions()->recordQuote() = recordQuote ? 'T' : 'F';
  altTrx->rebook() = rebook;
  altTrx->billing() = pricingDetailTrx->billing();
  altTrx->diagnostic().diagnosticType() = pricingDetailTrx->diagnostic().diagnosticType();

  for (TravelSeg* trvSeg : pricingDetailTrx->travelSeg())
    altTrx->travelSeg().push_back(trvSeg);

  for (Itin* itin : pricingDetailTrx->itin())
    altTrx->itin().push_back(itin);

  return altTrx;
}

bool
XformClientXML::getSelectionXml(Trx*& trx,
                                const std::vector<PaxDetail*>& paxDetailsOrder,
                                AltPricingTrx::AccompRestrictionInfo* accompRestrictionInfo,
                                std::vector<std::string>& optionXmlVec,
                                PaxDetail* currentPaxDetail,
                                bool rebook,
                                MoneyAmount& totals)
{
  PaxDetail* paxItemFirst = paxDetailsOrder.front();
  PaxDetail* paxItemLast = paxDetailsOrder.back();

  // sets up the erase ind to let know what to erase
  // default = 0 when only one pax info present, erase S84 only
  // = 1, at least two pax info present, erase </SUM> and S84 as well
  // = 2, at least > than 2 pax info present, erase <SUM , </SUM> and S84 as well
  // = 3, the last pax info where more than one of them, erase <SUM and S84 as well
  size_t eraseInx = 0;
  uint16_t selNum = currentPaxDetail->wpnOptionNumber();
  accompRestrictionInfo->selectionNumber() = selNum;
  accompRestrictionInfo->validationStr() = currentPaxDetail->accTvlData();
  if (!selNum || (selNum > optionXmlVec.size()))
  {
    LOG4CXX_WARN(logger, "Selection XML not available for selection #" << selNum);
    return false;
  }

  if (paxDetailsOrder.size() > 1)
  {
    MoneyAmount total = 0.f;
    total = getTotalAmountPerPax(optionXmlVec[selNum - 1]);
    totals = totals + total;

    if (!total)
    {
      LOG4CXX_WARN(logger, "invalid selection XML for total");
    }

    if ((currentPaxDetail == paxItemFirst) && (paxItemFirst != paxItemLast))
    {
      eraseInx = 1;
    }
    else if ((currentPaxDetail != paxItemFirst) && (currentPaxDetail != paxItemLast))
    {
      eraseInx = 2;
    }
    else
    {
      eraseInx = 3;
    }
  }
  // Before erase get VCL information
  saveVclInformation(currentPaxDetail, optionXmlVec[selNum - 1], trx);
  std::string tmpResponse = eraseXmlElement(optionXmlVec[selNum - 1], eraseInx);
  std::string tmpResponse1;
  if (rebook && ((tmpResponse1 = eraseXmlMsgElement(tmpResponse)) != ""))
    accompRestrictionInfo->selectionXml() = tmpResponse1;
  else
    accompRestrictionInfo->selectionXml() = tmpResponse;

  return true;
}

bool
XformClientXML::updateC56Tag(std::string& selXml,
                             const MoneyAmount& totals,
                             const CurrencyNoDec& nDec)
{
  char buff[15];
  sprintf(buff, "%-.*f", nDec, totals);
  std::ostringstream ss;
  ss << buff;

  std::string attrStr(xml2::TotalPriceAll);
  attrStr += "=\"";
  attrStr += ss.str();
  attrStr += "\"";

  const char* const searchTag = "C56=";

  size_t idx = selXml.find(searchTag);
  size_t idxEnd = selXml.find('"', idx + 5);

  if (idx == std::string::npos || idxEnd == std::string::npos)
  {
    LOG4CXX_WARN(logger, "invalid selection XML");
    return false;
  }

  selXml.replace(idx, (idxEnd + 1 - idx), attrStr);
  return true;
}

template <typename T>
bool
XformClientXML::processWpaDetailRequest(PricingDetailTrx* pricingDetailTrx,
                                        const std::string& content,
                                        DataHandle& dataHandle,
                                        Trx*& trx,
                                        bool recordQuote,
                                        bool rebook)
{
  if (pricingDetailTrx == nullptr)
    return false;

  // set trx to be an AltPricingTrx
  T* altTrx = createAltPricingDetailTrx<T>(pricingDetailTrx, dataHandle, recordQuote, rebook);
  trx = altTrx;

  // transfer data for WPA*n
  std::vector<std::string> optionXmlVec;
  if (!extractOptionXml(content, optionXmlVec) || !saveAgentBillingXml(content, *altTrx))
    return false;

  std::vector<PaxDetail*>::const_iterator paxIter = pricingDetailTrx->paxDetails().begin();
  std::vector<PaxDetail*>::const_iterator paxIterEnd = pricingDetailTrx->paxDetails().end();

  altTrx->paxDetails().insert(altTrx->paxDetails().end(), paxIter, paxIterEnd);

  // instantiate vector for temporary use, put in all options in order
  std::vector<PaxDetail*> paxDetailsOrder = pricingDetailTrx->paxDetails();

  // sort options in order
  LowerOptionNumber numberSort;
  std::stable_sort(paxDetailsOrder.begin(), paxDetailsOrder.end(), numberSort);

  altTrx->accompRestrictionVec().resize(altTrx->paxDetails().size());
  AltPricingTrx::AccompRestrictionVec::iterator arIter = altTrx->accompRestrictionVec().begin();
  AltPricingTrx::AccompRestrictionVec::iterator arIterEnd = altTrx->accompRestrictionVec().end();

  CurrencyNoDec nDec = static_cast<int>(getTotalAmountPerPax(optionXmlVec[0], true));

  MoneyAmount totals = 0.f;

  paxIter = paxDetailsOrder.begin();
  paxIterEnd = paxDetailsOrder.end();
  for (; arIter != arIterEnd && paxIter != paxIterEnd; arIter++, paxIter++)
  {
    if (std::string::npos != (*paxIter)->wpnDetails().find(FC_SURFACE_RESTRICTED))
      arIter->surfaceRestricted() = true;

    if (!getSelectionXml(trx, paxDetailsOrder, &(*arIter), optionXmlVec, *paxIter, rebook, totals))
      return false;
  }

  bool multiVCL = false;
  paxIter = paxDetailsOrder.begin();
  if (paxDetailsOrder.size() > 1)
  {
    for (; paxIter != paxIterEnd - 1; paxIter++)
    {
      if ((*paxIter)->vclInfo() != (*(paxIter + 1))->vclInfo())
      {
        multiVCL = true;
        break;
      }
    }
  }
  if (multiVCL)
  {
    // remove VCL from SUM section
    eraseVclInformation(altTrx->accompRestrictionVec().front().selectionXml(), trx);

    // add VCL information into PXI section
    arIter = altTrx->accompRestrictionVec().begin();
    paxIter = paxDetailsOrder.begin();
    for (; arIter != arIterEnd && paxIter != paxIterEnd; arIter++, paxIter++)
    {
      addVCLinPXI((*arIter).selectionXml(), (*paxIter)->vclInfo());
    }
  }

  if (altTrx->accompRestrictionVec().size() > 1)
    updateC56Tag(altTrx->accompRestrictionVec().front().selectionXml(), totals, nDec);

  return true;
}

std::string
XformClientXML::eraseXmlElement(std::string& message, size_t eraseInx)
{
  const char* const searchTagSum = "<SUM ";
  const char* const searchTagPxi = "<PXI ";
  const char* const searchTagEndSum = "</SUM>";
  const char* const elementBeginPattern = "S84=";
  size_t strBegin = 0;
  size_t strEnd = 0;
  size_t strLen = 0;

  if (eraseInx > 1) // erase <SUM ... till <PXI ...
  {
    strBegin = message.find(searchTagSum);
    if (strBegin == std::string::npos)
      return "";
    strEnd = message.find(searchTagPxi);
    if (strEnd == std::string::npos)
      return "";
    strLen = strEnd - strBegin;

    message = message.erase(strBegin, strLen);
  }

  if (eraseInx > 0 && eraseInx < 3) // erase </SUM>
  {
    strBegin = message.find(searchTagEndSum);
    if (strBegin == std::string::npos)
      return "";
    strLen = 6;

    message = message.erase(strBegin, strLen);
  }

  strBegin = message.find(elementBeginPattern);
  if (strBegin == std::string::npos)
    return "";
  strEnd = message.find('"', strBegin + 6);
  if (strEnd == std::string::npos)
    return "";
  strLen = strEnd + 1 - strBegin;

  return message.erase(strBegin, strLen);
}

MoneyAmount
XformClientXML::getTotalAmountPerPax(std::string& message, bool nDec)
{
  const char* const elementBeginPattern = "C56=";
  size_t strBegin = message.find(elementBeginPattern);
  if (strBegin == std::string::npos)
    return 0;
  strBegin = strBegin + 5;
  size_t strEnd = message.find('"', strBegin);
  if (strEnd == std::string::npos)
    return 0;
  size_t strLen = strEnd - strBegin;

  std::string value = message.substr(strBegin, strLen);

  if (nDec)
  {
    size_t strDot = value.find('.');
    if (strDot == std::string::npos)
      return 0;

    return static_cast<MoneyAmount>(strLen - strDot - 1);
  }
  return static_cast<MoneyAmount>(atof(value.c_str()));
}

//**********************************************************************************
//  Sort Differential Items before display them in Fare Calc Line
//**********************************************************************************
bool
XformClientXML::LowerOptionNumber::
operator()(const PaxDetail* item1, const PaxDetail* item2)
{
  return (item1 && item2 && (item1->wpnOptionNumber() < item2->wpnOptionNumber()));
}

std::string
XformClientXML::eraseXmlMsgElement(std::string& message)
{
  const char* const searchTagMsg = "<MSG ";
  const char* const searchEndTag = "/>";
  const char* const searchBookCodes = "APPLICABLE BOOKING CLASS - ";
  size_t strBegin = 0;
  size_t strEnd = 0;

  size_t strRebook = 0;
  size_t strLen = 0;

  // find the rebook message if exists
  strRebook = message.find(searchBookCodes);
  if (strRebook == std::string::npos)
    return "";

  // find the first MSG tag
  strBegin = message.find(searchTagMsg);
  if (strBegin == std::string::npos)
    return message;

  // find the end tag for the MSG element
  strEnd = message.find(searchEndTag, strBegin);
  if (strEnd == std::string::npos)
    return "";

  while (strEnd < strRebook)
  {
    // find the next MSG element and compare with rebook message
    strBegin = message.find(searchTagMsg, strEnd);
    if (strBegin == std::string::npos)
      return "";
    strEnd = message.find(searchEndTag, strBegin);
    if (strEnd == std::string::npos)
      return "";
  }

  // the MSG element has been found;
  // need to find the End for the next MSG - assuming it's
  // "REBOOK OPTION OF CHOICE BEFORE STORING FARE"
  strEnd = message.find(searchEndTag, strEnd + 1);
  if (strEnd == std::string::npos)
    return "";

  strLen = strEnd - strBegin + 2;

  return message.erase(strBegin, strLen);
}

std::string
XformClientXML::createErrorMessage(const tse::ErrorResponseException& ere)
{
  return (ere.code() > 0 && ere.message().empty()) ? "UNKNOWN EXCEPTION" : ere.message();
}

bool
XformClientXML::validatePbbRequest(PricingTrx* pricingTrx, std::string& errorMessage)
{
  bool hasBrandCode = false;
  for (const TravelSeg* travelSeg : pricingTrx->travelSeg())
  {
    if (!travelSeg->getBrandCode().empty())
    {
      if (!hasBrandCode)
        hasBrandCode = true;

      if (!isValidBrandCode(travelSeg->getBrandCode()))
      {
        errorMessage = "BRAND CODE INVALID - ENTER 2-10 ALPHA/NUMERIC CODE";
        return false;
      }
    }
  }

  if (!hasBrandCode)
  {
    errorMessage = "MISSING BRAND CODE";
    return false;
  }

  if (pricingTrx->isCommandPricingRq())
  {
    SegmentBrandCodeComparator brandCodeComp(pricingTrx->travelSeg().front());

    if (boost::algorithm::all_of(
            pricingTrx->travelSeg().begin() + 1, pricingTrx->travelSeg().end(), brandCodeComp))
    {
      pricingTrx->setPbbRequest(PBB_RQ_DONT_PROCESS_BRANDS);
    }
    else
    {
      errorMessage = "INVALID INPUT FORMAT";
      return false;
    }
  }

  if (!validateLegIds(pricingTrx, errorMessage))
    return false;

  initArunkSegLegIds(pricingTrx->travelSeg());

  // in pricing in TN path disable soft passes (if option set to true)
  if (!TrxUtil::isRequestFromAS(*pricingTrx))
    pricingTrx->setSoftPassDisabled(
        dynamicSoftPassDisabledInTn.isValid(pricingTrx));

  return true;
}

bool
XformClientXML::isValidBrandCode(const BrandCode& brandCode)
{
  if (brandCode.length() < 2 || brandCode.length() > 10)
  {
    return false;
  }

  for (const char elem : brandCode)
  {
    if (!isalnum(elem))
    {
      return false;
    }
  }

  return true;
}

bool
XformClientXML::validateLegIds(PricingTrx* pricingTrx, std::string& errorMessage)
{
  TravelSegPtrVec& segments = pricingTrx->travelSeg();

  if (segments.empty())
    return true;

  // filter travel segments
  TravelSegPtrVec segmentsForLegValidation(segments.size());
  // ARUNKs don't have to have legId already set but if they do -> it has to be a correct one
  // For that reason we validate airSegments and Arunks with legId provided in the request
  auto segmentFilter = [](TravelSeg* seg)
  { return !seg->isArunk() || seg->legId() != -1; };

  auto it = std::copy_if(
      segments.begin(), segments.end(), segmentsForLegValidation.begin(), segmentFilter);

  segmentsForLegValidation.resize(std::distance(segmentsForLegValidation.begin(), it));

  // check if leg ids are in ascending order and diff between ids is max 1
  auto checkOrder = [](TravelSeg* curr, TravelSeg* prev)
  { return (curr->legId() < prev->legId()) || ((curr->legId() - prev->legId()) > 1); };

  auto itFirstInvalid = std::is_sorted_until(
      segmentsForLegValidation.begin(), segmentsForLegValidation.end(), checkOrder);

  int16_t firstLegId = segmentsForLegValidation.front()->legId();
  int16_t lastLegId = segmentsForLegValidation.back()->legId();

  if (itFirstInvalid != segmentsForLegValidation.end())
  {
    std::ostringstream os;
    if ((*itFirstInvalid)->legId() == -1 || firstLegId == -1)
      os << "INCOMPLETE LEG INFO";
    else
      os << "INVALID LEG ID: " << ((*itFirstInvalid)->legId() + 1);
    errorMessage = os.str();
    return false;
  }

  // check if none of legIds are set, legId is -1 when not set
  // we already know they're sorted so checking this is enough
  if (lastLegId == -1)
  {
    // none is set
    pricingTrx->setAssignLegsRequired(true);
    return true;
  }
  else if (firstLegId == -1)
  {
    // some set, some not
    errorMessage = "INCOMPLETE LEG INFO";
    return false;
  }
  else if (firstLegId != 0)
  {
    // leg numbering starts from 1 but internally process them from 0
    errorMessage = "INCORRECT LEG NUMBERING. MUST START FROM 1";
    return false;
  }

  if (segmentsForLegValidation.size() < 2)
    return true;

  if (!validateBrandParity(segmentsForLegValidation))
  {
    errorMessage = "ONLY ONE BRAND PER LEG ALLOWED";
    return false;
  }

  return true;
}

bool
XformClientXML::validateBrandParity(const TravelSegPtrVec& segments) const
{
  if (segments.size() < 2)
    return true;

  int16_t previousLegId = -1;
  BrandCode previousBrandCode = "";

  // assumes data is sorted
  for (auto it = segments.begin(); it != segments.end(); ++it)
  {
    int16_t currentLegId = (*it)->legId();
    const BrandCode& currentBrandCode = (*it)->getBrandCode();

    if ((*it)->isArunk())
      continue; // Arunks don't have brandCodes

    if (currentLegId != previousLegId)
    {
      previousLegId = currentLegId;
      previousBrandCode = currentBrandCode;
    }
    else if (currentBrandCode != previousBrandCode)
      return false;
  }
  return true;
}

void
XformClientXML::initArunkSegLegIds(std::vector<TravelSeg*>& segments)
{
  int16_t lastLegId = -1;

  for (TravelSeg* seg : segments)
  {
    int16_t& currentLegId = seg->legId();

    if (!seg->isArunk())
    {
      lastLegId = currentLegId;
      continue;
    }

    // processing ARUNKs. It is assumed that first segment cannot be an Arunk
    if (lastLegId != -1 && currentLegId == -1)
      currentLegId = lastLegId;
  }
}

void
XformClientXML::saveVclInformation(PaxDetail* currentPaxDetail, std::string& message, Trx*& trx)
{
  const char* const searchTagVCL = "<VCL ";
  const char* const searchTagEndVCL = "</VCL>";

  size_t strEnd = 0;
  size_t strLen = 0;

  size_t strBegin = message.find(searchTagVCL);
  if (strBegin != std::string::npos)
  {
    if (!fallback::fallbackValidatingCxrMultiSp(trx) || trx->overrideFallbackValidationCXRMultiSP())
      strEnd = message.rfind(searchTagEndVCL);
    else
      strEnd = message.find(searchTagEndVCL);

    if (strEnd != std::string::npos)
    {
      strLen = (strEnd + 6) - strBegin;
      currentPaxDetail->vclInfo() = message.substr(strBegin, strLen);
    }
  }
}

void
XformClientXML::eraseVclInformation(std::string& selectionXml, Trx*& trx)
{
  const char* const searchTagVCL = "<VCL ";
  const char* const searchTagEndVCL = "</VCL>";

  size_t strEnd = 0;
  size_t strLen = 0;

  size_t strBegin = selectionXml.find(searchTagVCL);

  if (!fallback::fallbackValidatingCxrMultiSp(trx) || trx->overrideFallbackValidationCXRMultiSP())
    strEnd = selectionXml.rfind(searchTagEndVCL);
  else
    strEnd = selectionXml.find(searchTagEndVCL);

  if (strBegin != std::string::npos && strEnd != std::string::npos)
  {
    strLen = (strEnd + 6) - strBegin;
    selectionXml.erase(strBegin, strLen);
  }
}

void
XformClientXML::addVCLinPXI(std::string& selectionXML, std::string& vclInfo)
{
  const char* const searchTag = "</PXI";
  size_t strBegin = 0;

  strBegin = selectionXML.find(searchTag);
  if (strBegin != std::string::npos)
  {
    selectionXML.insert(strBegin, vclInfo);
  }
}

void
XformClientXML::sortXmlTags(const char* content) const
{
  constexpr auto emptyPair = std::make_pair(std::string::npos, std::string::npos);
  static const char* bilXmlTag{"<BIL"};
  static const char* pxiXmlTag{"<PXI"};
  static const char* sgiXmlTag{"<SGI"};

  size_t length = std::strlen(content);

  const XMLSingleLevelFinder xml{content, length};
  auto bil = xml.singleTagLookup(bilXmlTag);
  auto pxis = xml.multipleTagLookup(pxiXmlTag);

  // We care only about the position of the first <SGI/> tag
  auto sgi = xml.singleTagLookup(sgiXmlTag);
  if (bil == emptyPair || sgi == emptyPair)
    return;
  auto precedes = [](const auto& x, const auto& y)
  {
    std::less<> less;
    return less(x.second, y.first);
  };

  // The extra complication of this code is related to the fact that for one particular request
  // <PXI/> is
  // optional:
  //                       <BIL/> <PXI/> <SGI/>
  // NoPNRPricingRequest   1      0/4    1/24
  // PricingRequest        1      1/4    1/24
  // StructuredRuleRequest 1      1/4    1/24
  // AltPricingRequest     1      1/4    1/24
  bool isBILBeforePIX = pxis.size() ? precedes(bil, *pxis.begin()) : true;
  bool areAllPXIBeforeTheFirstSGI{false};

  if (!isBILBeforePIX)
    areAllPXIBeforeTheFirstSGI = pxis.size() ? std::all_of(pxis.cbegin(),
                                                           pxis.cend(),
                                                           [&sgi, precedes](const auto& p)
                                                           { return precedes(p, sgi); })
                                             : precedes(bil, sgi);
  if (isBILBeforePIX && areAllPXIBeforeTheFirstSGI)
    return;

  std::string copy{content, length};
  // We need to have that <BIL/> precedes <PXI/>, and <PXI/> precede <SGI/>.
  // We first of all move all <PXI/> tags after the first <SGI/> in front of the the
  // first <SGI/>.
  if (!areAllPXIBeforeTheFirstSGI)
  {
    for (const auto& pxi : pxis)
    {
      if (!precedes(pxi, sgi))
      {
        bool mustUpdateBILPosition = precedes(bil, pxi) && precedes(sgi, bil);
        XMLSingleLevelFinder::swapStringsByPosition(copy, pxi, sgi);
        std::size_t offset = pxi.second - pxi.first + 1;
        sgi.first += offset;
        sgi.second += offset;

        // The only case when we need to update bil's position is when it is between sgi && pxi.
        // This should really never happen, but better to deal with it.
        if (mustUpdateBILPosition)
        {
          bil.first += offset;
          bil.second += offset;
        }
      }
    }
  }

  XMLSingleLevelFinder last{copy.c_str(), copy.size()};
  auto firstUpdatedPXI = last.singleTagLookup(pxiXmlTag);
  if (firstUpdatedPXI == emptyPair)
    firstUpdatedPXI = sgi;
  if (!precedes(bil, (firstUpdatedPXI)))
    XMLSingleLevelFinder::swapStringsByPosition(copy, bil, firstUpdatedPXI);

  // I know, this is crap but I didn't find a better way of doing this.
  // Check the code for XformClientXML::convert(). The original request is an std::string,
  // but what we get is a pointer to a portion of an std::string internal buffer. Considering
  // that the original object we get is an std::string& we can modify it.
  std::strcpy(const_cast<char*>(content), copy.c_str());
}
} // tse
