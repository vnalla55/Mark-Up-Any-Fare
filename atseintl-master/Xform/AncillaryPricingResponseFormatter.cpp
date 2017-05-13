//----------------------------------------------------------------------------
//
//  Copyright Sabre 2010
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

#include "Xform/AncillaryPricingResponseFormatter.h"

#include "Common/BaggageStringFormatter.h"
#include "Common/Config/ConfigManUtils.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/Config/DynamicConfigurableNumber.h"
#include "Common/CurrencyRoundingUtil.h"
#include "Common/ErrorResponseException.h"
#include "Common/FallbackUtil.h"
#include "Common/FreeBaggageUtil.h"
#include "Common/Logger.h"
#include "Common/MetricsUtil.h"
#include "Common/ServiceFeeUtil.h"
#include "Common/TrxUtil.h"
#include "Common/XMLConstruct.h"
#include "DataModel/AmVatTaxRatesOnCharges.h"
#include "DataModel/AncRequest.h"
#include "DataModel/BaggageCharge.h"
#include "DBAccess/CarrierFlightSeg.h"
#include "DBAccess/Loc.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/ServicesDescription.h"
#include "DBAccess/ServicesSubGroup.h"
#include "DBAccess/SubCodeInfo.h"
#include "DBAccess/SvcFeesCxrResultingFCLInfo.h"
#include "DBAccess/SvcFeesResBkgDesigInfo.h"
#include "DBAccess/TaxText.h"
#include "Diagnostic/Diagnostic.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/OCFeesUsage.h"
#include "ServiceFees/ServiceFeesGroup.h"
#include "Xform/AncillaryPricingResponseSumDataFiller.h"
#include "Xform/DataModelMap.h"
#include "Xform/OCFeesPrice.h"
#include "Xform/PricingResponseXMLTags.h"
#include "Xform/TaxFormatter.h"
#include "Xform/XformUtil.h"

#include <iomanip>
#include <iterator>
#include <sstream>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>

namespace
{
// config params
static const std::string SERVICE_FEE = "SERVICE_FEES_SVC";
static const std::string MAX_RESPONSE_SIZE_KEY = "MAX_RESPONSE_SIZE";
static const std::string MAX_RESPONSE_SIZE_AB240_KEY = "MAX_RESPONSE_SIZE_AB240";
static const int32_t DEFAULT_MAX = 3145800;
static const int32_t DEFAULT_MAX_AB240 = DEFAULT_MAX;
static const int32_t DEFAULT_MAX_FEES = 4000;
static const int32_t DEFAULT_MAX_FEES_AB240 = DEFAULT_MAX_FEES;
static const std::string MAX_OC_FEES_KEY = "MAX_OC_IN_ANCPRICING_RESPONSE";
static const std::string MAX_OC_FEES_AB240_KEY = "MAX_OC_IN_ANCPRICING_RESPONSE_AB240";
static const int32_t DEFAULT_MAX_WPDISPAE = 290000;
static const int32_t DEFAULT_MAX_WPDISPAE_AB240 = DEFAULT_MAX_WPDISPAE;
static const std::string CONFIG_STANZA = "OUTPUT_LIMITS";
static const std::string MAX_PSS_OUTPUT_KEY = "MAX_PSS_OUTPUT";
static const std::string MAX_PSS_OUTPUT_AB240_KEY = "MAX_PSS_OUTPUT_AB240";
static const std::string SERVICE_DESCRIPTION = "SUB_CODE_DEFINITIONS";
static const unsigned int WINDOW_WIDTH = 63;
static const std::string MAX_NUMBER_OF_FEES_KEY = "MAX_NUMBER_OF_FEES";
static const std::string MAX_NUMBER_OF_FEES_AB240_KEY = "MAX_NUMBER_OF_FEES_AB240";
}

namespace tse
{
FIXEDFALLBACK_DECL(AB240_DecoupleServiceFeesAndFreeBag);
FALLBACK_DECL(fallbackAB240);
FALLBACK_DECL(fallbackAMChargesTaxes);
FALLBACK_DECL(fallbackAncillaryPricingMetrics);
FALLBACK_DECL(fallbackAddC52ForRecordsWithNoCharge);
FALLBACK_DECL(fallbackAddC52ForRecordsWithNoChargeAB240);
FALLBACK_DECL(fallbackDisplayPriceForBaggageAllowance);
FALLBACK_DECL(fallbackOperatingCarrierForAncillaries);
FALLBACK_DECL(fallbackAddSequenceNumberInOOS);
FALLBACK_DECL(fallbackAlways_display_purchase_by_date_for_AB240);
FALLBACK_DECL(fallbackSurroundOscWithOccForMonetaryDiscount);
FALLBACK_DECL(latencyDataInAncillaryPricingResponse);

log4cxx::LoggerPtr
AncillaryPricingResponseFormatter::_logger(
    log4cxx::Logger::getLogger("atseintl.Xform.AncillaryPricingResponseFormatter"));

namespace
{
ConfigurableValue<int32_t>
maxTotalBuffSize(SERVICE_FEE, MAX_RESPONSE_SIZE_KEY, DEFAULT_MAX);
ConfigurableValue<int32_t>
maxOCInResponse(SERVICE_FEE, MAX_OC_FEES_KEY, DEFAULT_MAX_FEES);
ConfigurableValue<int32_t>
maxTotalBuffSizeForWPDispAE(CONFIG_STANZA, MAX_PSS_OUTPUT_KEY, DEFAULT_MAX_WPDISPAE);
ConfigurableValue<int32_t>
maxTotalBuffSizeAb240(SERVICE_FEE, MAX_RESPONSE_SIZE_AB240_KEY, DEFAULT_MAX_AB240);
ConfigurableValue<int32_t>
maxOCInResponseAb240(SERVICE_FEE, MAX_OC_FEES_AB240_KEY, DEFAULT_MAX_FEES_AB240);
ConfigurableValue<int32_t>
maxTotalBuffSizeForWPDispAEAb240(CONFIG_STANZA,
                                 MAX_PSS_OUTPUT_AB240_KEY,
                                 DEFAULT_MAX_WPDISPAE_AB240);
ConfigurableValue<uint16_t>
maxNumberOfOCFeesAb240Key(SERVICE_FEE, MAX_NUMBER_OF_FEES_AB240_KEY);
ConfigurableValue<uint16_t>
maxNumberOfOCFeesKey(SERVICE_FEE, MAX_NUMBER_OF_FEES_KEY);
ConfigurableValue<std::string>
ancPrcXMLNamespace(SERVICE_FEE, "ANC_PRC_XML_NAMESPACE");
}

struct CheckRequestedServiceType : std::unary_function<RequestedOcFeeGroup, bool>
{
  CheckRequestedServiceType(const Indicator& ancillaryServiceType)
    : _ancillaryServiceType(ancillaryServiceType)
  {
  }

  bool operator()(const RequestedOcFeeGroup& requestedOcFeeGroup) const
  {
    return requestedOcFeeGroup.isAncillaryServiceType(_ancillaryServiceType);
  }

private:
  Indicator _ancillaryServiceType;
};

AncillaryPricingResponseFormatter::AncillaryPricingResponseFormatter()
{
  _maxTotalBuffSize = maxTotalBuffSize.getValue();
  _maxOCInResponse = maxOCInResponse.getValue();
  _maxTotalBuffSizeForWPDispAE = maxTotalBuffSizeForWPDispAE.getValue();
}

void
AncillaryPricingResponseFormatter::formatResponse(const ErrorResponseException& ere,
                                                  std::string& response)
{
  XMLConstruct construct;
  construct.openElement("AncillaryPricingResponse");
  if (XML_NAMESPACE_TEXT != "")
    construct.addAttribute("xmlns", XML_NAMESPACE_TEXT);

  prepareMessage(construct, Message::TYPE_ERROR, Message::errCode(ere.code()), ere.message());
  construct.closeElement();

  response = construct.getXMLData();
}

std::string
AncillaryPricingResponseFormatter::formatResponse(const std::string& responseString,
                                                  AncillaryPricingTrx& ancTrx,
                                                  ErrorResponseException::ErrorResponseCode errCode)
{
  LOG4CXX_INFO(_logger, "Response (before XML tagging):\n" << responseString);

  if (ancTrx.activationFlags().isAB240())
  {
    _maxTotalBuffSize = maxTotalBuffSizeAb240.getValue();
    _maxOCInResponse = maxOCInResponseAb240.getValue();
    _maxTotalBuffSizeForWPDispAE = maxTotalBuffSizeForWPDispAEAb240.getValue();

    if (!fallback::fallbackAMChargesTaxes(&ancTrx))
    {
      const Loc* pointOfSaleLocation = TrxUtil::saleLoc(ancTrx);
      if (pointOfSaleLocation)
        _nation = pointOfSaleLocation->nation();
    }
  }

  const AncRequest* req = static_cast<AncRequest*>(ancTrx.getRequest());

  if (ancTrx.billing()->requestPath() == ACS_PO_ATSE_PATH ||
      ancTrx.billing()->actionCode().substr(0, 5) == "MISC6" || req->isWPBGRequest() ||
      ancTrx.isBaggageRequest())
  {
    populateSubCodeDefinitions(ancTrx);
  }

  XMLConstruct construct;
  construct.openElement("AncillaryPricingResponse");
  if (XML_NAMESPACE_TEXT != "")
    construct.addAttribute("xmlns", XML_NAMESPACE_TEXT);

  Diagnostic& diag = ancTrx.diagnostic();

  if (ancTrx.diagnostic().diagnosticType() == Diagnostic854)
  {
    prepareHostPortInfo(ancTrx, construct);
  }

  if (!fallback::fallbackAncillaryPricingMetrics(&ancTrx))
  {
    // Metrics
    if ((ancTrx.recordMetrics() == true) && (diag.diagnosticType() != DiagnosticNone))
    {
      std::ostringstream metrics;
      MetricsUtil::trxLatency(metrics, ancTrx);
      construct.openElement("DIA");
      construct.openElement("MTP");
      construct.addElementData(metrics.str().c_str());
      construct.closeElement();
      construct.closeElement();
    }
  }

  if (errCode == ErrorResponseException::ANCILLARY_TIME_OUT_R7 && req &&
      req->ancRequestType() != AncRequest::M70Request)
  {
    for (Itin* itin : ancTrx.itin())
    {
      if (itin->timeOutForExceeded() || itin->timeOutOCFForWP() ||
          itin->timeOutForExceededSFGpresent())
        errCode = ErrorResponseException::NO_ERROR;
    }
  }

  bool unlimitedResponse =
      diag.diagParamIsSet(Diagnostic::NO_LIMIT, Diagnostic::UNLIMITED_RESPONSE);

  if (unlimitedResponse)
    _maxSizeAllowed = true;

  if (errCode > 0)
  {
    if (diag.diagnosticType() != DiagnosticNone && !diag.toString().empty())
    {
      prepareResponseText(diag.toString(), construct, unlimitedResponse);
    }
    // Parsing failed, return error message
    prepareMessage(construct, Message::TYPE_ERROR, Message::errCode(errCode), responseString);
    LOG4CXX_DEBUG(_logger,
                  "AncillaryPricingResponseFormatter::formatResponse() - errCode:" << errCode);
  }
  else if (diag.diagnosticType() != DiagnosticNone && diag.diagnosticType() != Diagnostic854)
  {
    std::string tmpResponse = diag.toString();
    if (tmpResponse.length() == 0)
    {
      char tmpBuf[256];
      sprintf(tmpBuf, "DIAGNOSTIC %d RETURNED NO DATA", diag.diagnosticType());
      tmpResponse.insert(0, tmpBuf);
      prepareResponseText(tmpResponse, construct, unlimitedResponse);
    }
    else
      prepareResponseText(tmpResponse, construct, unlimitedResponse);
  }
  else
  {
    if (ancTrx.activationFlags().isAB240())
    {
      // AncillaryPricingResponse v3 is always processed as M70
      _isWPDispAE = false;
      _isWpAePostTicket = false;
    }
    else
    {
      if (req->isWPAERequest())
        _isWPDispAE = true;

      if (req->isPostTktRequest())
        _isWpAePostTicket = true;
    }

    if (ancTrx.billing()->requestPath() == AEBSO_PO_ATSE_PATH ||
        (!_isWPDispAE && !_isWpAePostTicket))
      _maxSizeAllowed = true;

    formatAEFeesResponse(construct, ancTrx);
    if (_doNotProcess)
    {
      construct.openElement(xml2::AncillaryRespOptions);
      construct.addAttributeChar(xml2::TruncRespIndicator, 'T');
      if ((_isWPDispAE || _isWpAePostTicket) && addPrefixWarningForOCTrailer(ancTrx, true))
      {
        construct.addAttributeBoolean(xml2::AttnInd, true);
      }
      construct.closeElement();
    }
    else
    {
      if ((_isWPDispAE || _isWpAePostTicket) && addPrefixWarningForOCTrailer(ancTrx, true))
      {
        construct.openElement(xml2::AncillaryRespOptions);
        construct.addAttributeBoolean(xml2::AttnInd, true);
        construct.closeElement();
      }
    }
  }

  appendCDataToResponse(ancTrx, construct);

  if (!fallback::latencyDataInAncillaryPricingResponse(&ancTrx))
    prepareLatencyDataInResponse(ancTrx, construct);

  construct.closeElement();
  LOG4CXX_DEBUG(_logger,
                "AncillaryPricingResponseFormatter::formatResponse() - XML response ready ");

  return construct.getXMLData();
}

void
AncillaryPricingResponseFormatter::buildErrorAndDiagnosticElements(
    PricingTrx& trx,
    XMLConstruct& construct,
    const std::string& errorString,
    ErrorResponseException::ErrorResponseCode errorCode)
{
  Diagnostic& diag = trx.diagnostic();

  if (diag.diagnosticType() == Diagnostic854)
  {
    prepareHostPortInfo(trx, construct);
  }
  if (errorCode > 0)
  {
    if (diag.diagnosticType() != DiagnosticNone && !diag.toString().empty())
    {
      prepareResponseText(diag.toString(), construct);
    }
    prepareMessage(construct, Message::TYPE_ERROR, Message::errCode(errorCode), errorString);
  }
  else if (diag.diagnosticType() != DiagnosticNone && diag.diagnosticType() != Diagnostic854)
  {
    std::string diagResponse = diag.toString();

    if (diagResponse.empty())
    {
      std::ostringstream diagResponseStream;
      diagResponseStream << "DIAGNOSTIC " << diag.diagnosticType() << " RETURNED NO DATA";
      diagResponse = diagResponseStream.str();
    }
    prepareResponseText(diagResponse, construct);
  }
}

const std::string
AncillaryPricingResponseFormatter::readConfigXMLNamespace(const std::string& configName)
{
  return ancPrcXMLNamespace.getValue();
}

void
AncillaryPricingResponseFormatter::prepareResponseText(const std::string& responseString,
                                                       XMLConstruct& construct,
                                                       bool noSizeLImit) const
{
  std::string tmpResponse = responseString;

  size_t lastPos = 0;
  int recNum = 2;
  const int BUF_SIZE = 256;
  const int AVAILABLE_SIZE = BUF_SIZE - 52;
  // char tmpBuf[BUF_SIZE];
  // Clobber the trailing newline
  while (1)
  {
    lastPos = tmpResponse.rfind("\n");
    if (lastPos != std::string::npos && lastPos > 0 &&
        lastPos == (tmpResponse.length() - 1)) // lint !e530
      tmpResponse.replace(lastPos, 1, "\0");
    else
      break;
  }
  char* pHolder = nullptr;
  int tokenLen = 0;
  for (char* token = strtok_r((char*)tmpResponse.c_str(), "\n", &pHolder); token != nullptr;
       token = strtok_r(nullptr, "\n", &pHolder), recNum++)
  {
    tokenLen = strlen(token);

    if (tokenLen == 0)
      continue;
    else if (tokenLen > AVAILABLE_SIZE && !noSizeLImit)
    {
      LOG4CXX_WARN(_logger, "Line: [" << token << "] too long!");
      continue;
    }
    prepareMessage(construct, Message::TYPE_GENERAL, recNum + 1, token);

    // limit the size of the output returned
    if (((!_maxSizeAllowed &&
          construct.getXMLData().size() >
              static_cast<size_t>(_maxTotalBuffSizeForWPDispAE)) || // lint !e574
         (_maxSizeAllowed &&
          construct.getXMLData().size() > static_cast<size_t>(_maxTotalBuffSize))) &&
        !noSizeLImit) // lint !e574
    {
      const char* msg = "RESPONSE TOO LONG FOR CRT";

      prepareMessage(construct, Message::TYPE_GENERAL, recNum + 2, msg);
      break;
    }
  }
}

// Dynamic configurables for overriding the response size limits.
// Used in comparison tests, when comparing AncillaryPricing v1, v2 and v3
namespace
{
DynamicConfigurableNumber
testOverrideMaxResponseSize(SERVICE_FEE, MAX_RESPONSE_SIZE_KEY, DEFAULT_MAX);
DynamicConfigurableNumber
testOverrideMaxOcFees(SERVICE_FEE, MAX_OC_FEES_KEY, DEFAULT_MAX_FEES);
DynamicConfigurableNumber
testOverrideMaxPssOutput(CONFIG_STANZA, MAX_PSS_OUTPUT_KEY, DEFAULT_MAX_WPDISPAE);
DynamicConfigurableNumber
testOverrideMaxNumberOfFees(SERVICE_FEE, MAX_NUMBER_OF_FEES_KEY, DEFAULT_MAX_FEES);
DynamicConfigurableNumber
testOverrideMaxNumberOfFeesAb240(SERVICE_FEE, MAX_NUMBER_OF_FEES_AB240_KEY, DEFAULT_MAX_FEES_AB240);
}

void
AncillaryPricingResponseFormatter::formatAEFeesResponse(XMLConstruct& construct,
                                                        AncillaryPricingTrx& ancTrx)
{
  LOG4CXX_DEBUG(_logger,
                "AncillaryPricingResponseFormatter::formatAEFeesResponse() - entered, "
                    << ancTrx.itin().size() << " itineraries");

  setAeFeesResponseLimits(ancTrx);

  for (auto& itin : ancTrx.itin())
  {
    currentItin() = itin;
    bool isNoServiceGroups = itin->ocFeesGroup().empty();
    if (!fallback::fixed::AB240_DecoupleServiceFeesAndFreeBag())
      isNoServiceGroups = isNoServiceGroups && itin->ocFeesGroupsFreeBag().empty();

    if (isNoServiceGroups)
    {
      if (_isWPDispAE && isTimeOutBeforeStartOCFees(ancTrx, construct))
        break;
      continue;
    }

    if (UNLIKELY(ancTrx.isSecondCallForMonetaryDiscount()))
      buildOcFeesSecondResponseForMonetaryDiscount(ancTrx, currentItin(), construct);
    else
      buildOCFeesResponse(ancTrx, currentItin(), construct); // call for WP*AE and M70 response

    if (_doNotProcess)
      break;
  }
}

void
AncillaryPricingResponseFormatter::setAeFeesResponseLimits(AncillaryPricingTrx& ancTrx)
{
  if (testOverrideMaxOcFees.isValid(&ancTrx))
    _maxOCInResponse = testOverrideMaxOcFees.getValue(&ancTrx);

  if (testOverrideMaxPssOutput.isValid(&ancTrx))
    _maxTotalBuffSizeForWPDispAE = testOverrideMaxPssOutput.getValue(&ancTrx);

  if (testOverrideMaxResponseSize.isValid(&ancTrx))
    _maxTotalBuffSize = testOverrideMaxResponseSize.getValue(&ancTrx);
}

void AncillaryPricingResponseFormatter::buildOperatingCarrierInfo(AncillaryPricingTrx& ancTrx,
                                                                  Itin* itin,
                                                                  XMLConstruct& construct)
{
  if (UNLIKELY(!itin || itin->travelSeg().size() == 0))
    return;

  const bool cabinFromAnswer = TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(ancTrx);
  size_t travelSegCounter = 1;
  for (const TravelSeg* travelSeg : itin->travelSeg())
  {
    const AirSeg* airSeg = travelSeg->toAirSeg();
    if (UNLIKELY(airSeg == nullptr))
      continue;

    construct.openElement(xml2::SegmentInformation);
    construct.addAttributeInteger(xml2::GenId, travelSegCounter++);
    construct.addAttribute(xml2::OperatingCarrier, airSeg->operatingCarrierCode());
    construct.addAttributeInteger(xml2::FlightNum1, airSeg->operatingFlightNumber());
    construct.addAttributeChar(xml2::SegmentCabinCode,
                               airSeg->bookedCabin().getClassAlphaNum(cabinFromAnswer));
    construct.closeElement();
  }
}

class FindFeeGroup
{
  const ServiceGroup& _groupCode;

public:
  FindFeeGroup(const ServiceGroup& groupCode) : _groupCode(groupCode) {}

  bool operator()(const ServiceFeesGroup* group) const { return group->groupCode() == _groupCode; }
};

void
AncillaryPricingResponseFormatter::prepareHostPortInfo(PricingTrx& trx, XMLConstruct& construct)
{
  std::vector<std::string> hostInfo;
  std::vector<std::string> buildInfo;
  std::vector<std::string> dbInfo;
  std::vector<std::string> configInfo;

  if (hostDiagString(hostInfo))
    for (const auto& elem : hostInfo)
      prepareResponseText(elem, construct);

  buildDiagString(buildInfo);
  for (const auto& elem : buildInfo)
    prepareResponseText(elem, construct);

  dbDiagString(dbInfo);
  for (const auto& elem : dbInfo)
    prepareResponseText(elem, construct);

  if (configDiagString(configInfo, trx))
    for (const auto& elem : configInfo)
      prepareResponseText(elem, construct);
}

bool
AncillaryPricingResponseFormatter::isTimeOutBeforeStartOCFees(AncillaryPricingTrx& ancTrx,
                                                              XMLConstruct& construct)
{
  LOG4CXX_DEBUG(_logger,
                "AncillaryPricingResponseFormatter::isTimeOutBeforeStartOCFees() - entered");
  Itin* itin = currentItin();
  if (!itin->timeOutForExceeded())
    return false;
  // time out for WPAE or WPAE-BG entry right at OC process starts
  construct.openElement(xml2::ItinInfo);
  construct.addAttributeInteger(xml2::GenId, itin->getItinOrderNum());
  if (ancTrx.getRequest()->isMultiTicketRequest())
  {
    construct.addAttributeInteger(xml2::GroupNumber, itin->getItinOrderNum());
    construct.addAttributeInteger(xml2::MultiTicketNum, itin->getItinOrderNum());
  }

  if (ancTrx.billing()->requestPath() != SWS_PO_ATSE_PATH)
  {
    formatOCGenericMsg(construct, "MAX AIR EXTRAS EXCEEDED/USE AE SVC QUALIFIER OR SEG SELECT");
  }
  construct.closeElement();
  LOG4CXX_DEBUG(_logger,
                "AncillaryPricingResponseFormatter::isTimeOutBeforeStartOCFees() - complete");
  return true;
}

void
AncillaryPricingResponseFormatter::buildDisclosureText(AncillaryPricingTrx& ancTrx,
                                                       Itin* itin,
                                                       XMLConstruct& construct)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::buildDisclosureText() - entered");
  std::vector<ServiceFeesGroup*>& baggageGroups =
      fallback::fixed::AB240_DecoupleServiceFeesAndFreeBag() ? itin->ocFeesGroup()
                                                             : itin->ocFeesGroupsFreeBag();

  bool disclosureFound = false;
  for (const auto baggageGroup : baggageGroups)
  {
    if ((baggageGroup->state() == ServiceFeesGroup::VALID) &&
        ((baggageGroup->groupCode().equalToConst("BG")) ||
         (baggageGroup->groupCode().equalToConst("BD"))))
      disclosureFound = true;
  }

  if (!disclosureFound)
  {
    prepareMessage(construct, Message::TYPE_GENERAL, 0, "DISCLOSURE TEXT NOT FOUND");
    LOG4CXX_DEBUG(_logger,
                  "AncillaryPricingResponseFormatter::buildDisclosureText() - complete: "
                  "disclosure data not found");
    return;
  }

  std::vector<FarePath*>& farePathes = itin->farePath();
  std::vector<std::string> msgLines;
  for (const auto farePath : farePathes)
  {
    const PaxType* paxType = farePath->paxType();
    std::ostringstream paxTypeAndNumber;
    paxTypeAndNumber << paxType->paxType() << "-" << std::setfill('0') << std::setw(2)
                     << paxType->number();
    msgLines.push_back(paxTypeAndNumber.str());

    XformMsgFormatter::splitBaggageText(farePath->baggageResponse(), WINDOW_WIDTH, msgLines);
    XformMsgFormatter::splitBaggageText(
        farePath->baggageEmbargoesResponse(), WINDOW_WIDTH, msgLines);
  }

  for (const auto& msgLine : msgLines)
  {
    if (isXmlSizeLimit(construct, 0))
    {
      LOG4CXX_DEBUG(_logger,
                    "AncillaryPricingResponseFormatter::buildDisclosureText() - complete: "
                    "XML size limit reached");
      return;
    }

    if (msgLine == "<ADD>")
      prepareMessage(
          construct, Message::TYPE_GENERAL, 0, "ADDITIONAL ALLOWANCES AND/OR DISCOUNTS MAY APPLY");
    else
      prepareMessage(construct, Message::TYPE_GENERAL, 0, msgLine);
  }

  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::buildDisclosureText() - complete: OK");
}

void
AncillaryPricingResponseFormatter::buildOCFeesResponse(AncillaryPricingTrx& ancTrx,
                                                       Itin* itin,
                                                       XMLConstruct& construct)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::buildOCFeesResponse() - entered");
  if (!_doNotProcess)
  {
    construct.openElement(xml2::ItinInfo);
    construct.addAttributeInteger(xml2::GenId, itin->getItinOrderNum());

    if (ancTrx.activationFlags().isAB240())
    {
      if (ancTrx.getOptions()->isGroupRequested(RequestedOcFeeGroup::CatalogData) ||
          ancTrx.getOptions()->isGroupRequested(RequestedOcFeeGroup::DisclosureData))
      {
        const bool isUsDot = itin->getBaggageTripType().isUsDot();
        construct.addAttributeBoolean(xml2::UsDotItinIndicator, isUsDot);
      }

      if (ancTrx.getOptions()->isOcOrBaggageDataRequested(RequestedOcFeeGroup::DisclosureData))
        buildDisclosureText(ancTrx, currentItin(), construct);
    }
    if (ancTrx.getRequest()->isMultiTicketRequest())
    {
      construct.addAttributeInteger(xml2::GroupNumber, itin->getItinOrderNum());
      construct.addAttributeInteger(xml2::MultiTicketNum, itin->getItinOrderNum());
    }
    // Message tags may appear here for future
    if (!fallback::fallbackOperatingCarrierForAncillaries(&ancTrx))
    {
      if (ancTrx.activationFlags().isAB240())
        buildOperatingCarrierInfo(ancTrx, itin, construct);
    }

    _ancillariesNonGuarantee = false;
    AncRequest* ancReq = static_cast<AncRequest*>(ancTrx.getRequest());
    std::map<const Itin*, bool>::const_iterator guarantPIIter =
        ancReq->ancillNonGuaranteePerItin().find(currentItin());
    if (guarantPIIter != ancReq->ancillNonGuaranteePerItin().end())
      _ancillariesNonGuarantee = guarantPIIter->second;

    if ((_isWPDispAE || _isWpAePostTicket) && !ancTrx.activationFlags().isAB240())
    {
      if (anyTimeOutMaxCharCountIssue(ancTrx, construct))
      {
        LOG4CXX_DEBUG(
            _logger,
            "AncillaryPricingResponseFormatter::buildOCFeesResponse() - complete, Itin skipped");
        _doNotProcess = true;
        construct.closeElement(); // Close Itin
        return;
      }

      if (!ancTrx.activationFlags().isAB240() ||
          ancTrx.getOptions()->isOcOrBaggageDataRequested(RequestedOcFeeGroup::AncillaryData))
      {
        if (currentItin()->isOcFeesFound())
        {
          if (_isHeader)
          {
            formatOCHeaderMsg(construct);
            _isHeader = false;
            if (_ocfXmlConstruct.getXMLData().size() > 0)
              construct.addElementData(_ocfXmlConstruct.getXMLData().c_str(),
                                       _ocfXmlConstruct.getXMLData().size());
          }
        }
        else
        {
          builTrailerOCF(ancTrx, construct, "AIR EXTRAS NOT FOUND", false);
        }
      }

      builPNMData(construct);
      if (!_isWpAePostTicket)
      {
        XMLConstruct constructZ;

        formatOCFeesForR7U(ancTrx, constructZ); // time out checks included below
        LOG4CXX_DEBUG(_logger,
                      "AncillaryPricingResponseFormatter::buildOCFeesResponse() - WP*AE "
                      "formatOCFeeForR7 complete");
        if (_doNotProcess)
        {
          _doNotProcess = false;
          _isHeader = true;

          if (ancTrx.getOptions() && !ancTrx.getOptions()->serviceGroupsVec().empty()) // WP*AE-XX
            timeOutMaxCharCountRequestedOCFeesReturned(ancTrx, construct);
          else
            timeOutMaxCharCountNoOCFeesReturned(ancTrx, construct);
        }
        else
        {
          construct.addElementData(constructZ.getXMLData().c_str(), constructZ.getXMLData().size());
        }
      }
      else
      {
        formatOCFeesForR7(ancTrx, construct); // time out checks included below
        LOG4CXX_DEBUG(_logger,
                      "AncillaryPricingResponseFormatter::buildOCFeesResponse() - WPAE*T "
                      "formatOCFeeForR7 complete");
        if (_doNotProcess)
        {
          if (ancTrx.getOptions() && !ancTrx.getOptions()->serviceGroupsVec().empty()) // WP*AE-XX
            timeOutMaxCharCountRequestedOCFeesReturned(ancTrx, construct);
          else
            timeOutMaxCharCountNoOCFeesReturned(ancTrx, construct);
        }
      }
    }
    else
      formatOCFees(ancTrx, construct); // M70

    construct.closeElement();
  }
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::buildOCFeesResponse() - complete");
}

void
AncillaryPricingResponseFormatter::buildOcFeesSecondResponseForMonetaryDiscount(
    AncillaryPricingTrx& ancTrx, Itin* itin, XMLConstruct& construct)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::buildOcFeesSecondResponseForMonetaryDiscount() - entered");
  if (!_doNotProcess)
  {
    construct.openElement(xml2::ItinInfo);
    construct.addAttributeInteger(xml2::GenId, itin->getItinOrderNum());
    formatOCFees(ancTrx, construct);
    construct.closeElement();
  }
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::buildOcFeesSecondResponseForMonetaryDiscount() - complete");
}

void
AncillaryPricingResponseFormatter::builPNMData(XMLConstruct& construct)
{
}

bool
AncillaryPricingResponseFormatter::anyTimeOutMaxCharCountIssue(AncillaryPricingTrx& ancTrx,
                                                               XMLConstruct& construct)
{
  LOG4CXX_DEBUG(_logger,
                "AncillaryPricingResponseFormatter::anyTimeOutMaxCharCountIssue() - entered");
  Itin* itin = currentItin();
  if (itin->timeOutForExceeded() || itin->timeOutOCFForWP())
  {
    timeOutMaxCharCountNoOCFeesReturned(ancTrx, construct);
    return true;
  }
  if (itin->timeOutForExceededSFGpresent())
  {
    timeOutMaxCharCountRequestedOCFeesReturned(ancTrx, construct);
    return true;
  }
  return false;
}

bool
AncillaryPricingResponseFormatter::samePaxType(AncillaryPricingTrx& ancTrx)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::samePaxType() - entered");
  Itin* itin = currentItin();
  AncRequest* req = static_cast<AncRequest*>(ancTrx.getRequest());
  if (req && itin)
  {
    std::vector<PaxType*> itinPaxTypes = req->paxType(itin);
    PaxTypeCode paxCode = itinPaxTypes.front()->paxType();
    for (const PaxType* paxT : req->paxType(itin))
      if (paxCode != paxT->paxType())
        return false;

    return true;
  }
  return false;
}

void
AncillaryPricingResponseFormatter::timeOutMaxCharCountNoOCFeesReturned(AncillaryPricingTrx& ancTrx,
                                                                       XMLConstruct& construct)
{
  LOG4CXX_DEBUG(
      _logger,
      "AncillaryPricingResponseFormatter::timeOutMaxCharCountNoOCFeesReturned() - entered");
  Itin* itin = currentItin();
  // time out for WP*AE or WP*AE-BG entry before completing OC Fee process

  if (ancTrx.billing()->requestPath() != SWS_PO_ATSE_PATH)
  {
    if (_isWpAePostTicket)
    {
      AncRequest* req = static_cast<AncRequest*>(ancTrx.getRequest());

      if ((itin->timeOutForExceeded() || itin->timeOutForExceededSFGpresent()) &&
          ancTrx.itin().size() == 1 && (req->paxType(itin).size() == 1 ||
                                        (req->paxType(itin).size() > 1 && samePaxType(ancTrx))))
        formatOCGenericMsg(construct, "MAX AIR EXTRAS EXCEEDED/USE AE SVC QUALIFIER");
      else if (itin->timeOutForExceeded() || itin->timeOutForExceededSFGpresent())
        formatOCGenericMsg(construct, "MAX AIR EXTRAS EXCEEDED/USE AE SVC QUALIFIER OR TKT SELECT");
    }
    else
      formatOCGenericMsg(construct, "AIR EXTRAS MAY APPLY - USE WPAE WITH SERVICE QUALIFIER");
  }

  // Create empty OC Fee groups
  // iterate groups
  emptyOCFeeGroups(construct);
}

void
AncillaryPricingResponseFormatter::emptyOCFeeGroups(XMLConstruct& construct)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::emptyOCFeeGroups() - entered");
  Itin* itin = currentItin();
  std::vector<ServiceFeesGroup*>::iterator sfgIter = itin->ocFeesGroup().begin();
  std::vector<ServiceFeesGroup*>::iterator sfgIterEnd = itin->ocFeesGroup().end();
  for (; sfgIter != sfgIterEnd; ++sfgIter)
  {
    ServiceFeesGroup* sfg = (*sfgIter);
    ServiceGroup groupCode = sfg->groupCode();

    construct.openElement(xml2::OCGroupInfo);
    construct.addAttribute(xml2::OCFeeGroupCode, groupCode);
    construct.closeElement();
  }
}

void
AncillaryPricingResponseFormatter::timeOutMaxCharCountRequestedOCFeesReturned(
    AncillaryPricingTrx& ancTrx, XMLConstruct& constructA)
{
  LOG4CXX_DEBUG(
      _logger,
      "AncillaryPricingResponseFormatter::timeOutMaxCharCountRequestedOCFeesReturned() - entered");
  XMLConstruct construct;
  // time out for WP*AE or WP*AE-BG entry before completing OC Fee process

  if (ancTrx.billing()->requestPath() != SWS_PO_ATSE_PATH)
  {
    if (_isHeader)
    {
      formatOCHeaderMsg(construct);
      _isHeader = false;
    }
  }
  builPNMData(construct);
  if (_isWpAePostTicket && !preformattedMsgEmpty() &&
      !currentItin()->timeOutForExceededSFGpresent())
    return;
  if (!_isWpAePostTicket)
    formatOCFeesForR7U(ancTrx, construct, true);
  else
    formatOCFeesForR7(ancTrx, construct, true);

  constructA.addElementData(construct.getXMLData().c_str(), construct.getXMLData().size());
}

bool
AncillaryPricingResponseFormatter::isGenericTrailer(AncillaryPricingTrx& ancTrx,
                                                    XMLConstruct& construct)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::isGenericTrailer() - entered");
  if (!checkIfAnyGroupValid(ancTrx)) // When no valid group
    return builTrailerOCF(ancTrx, construct, "AIR EXTRAS NOT FOUND", false);

  return false;
}

void
AncillaryPricingResponseFormatter::createOCGSection(PricingTrx& pricingTrx, XMLConstruct& construct)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::createOCGSection() - entered");
  std::vector<ServiceFeesGroup*>::const_iterator sfgIter, sfgIterEnd;
  Itin* itin = currentItin();
  sfgIter = itin->ocFeesGroup().begin();
  sfgIterEnd = itin->ocFeesGroup().end();

  for (; sfgIter != sfgIterEnd; ++sfgIter)
  {
    const ServiceFeesGroup* sfg = (*sfgIter);
    const ServiceGroup sfgGroupCode = sfg->groupCode();
    construct.openElement(xml2::OCGroupInfo);
    construct.addAttribute(xml2::OCFeeGroupCode, sfgGroupCode);
    construct.closeElement();
  }
}

bool
AncillaryPricingResponseFormatter::checkIfAnyGroupValid(AncillaryPricingTrx& ancTrx)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::checkIfAnyGroupValid() - entered");

  std::vector<ServiceFeesGroup*>::const_iterator sfgIter, sfgIterEnd;
  Itin* itin = currentItin();
  sfgIter = itin->ocFeesGroup().begin();
  sfgIterEnd = itin->ocFeesGroup().end();

  for (; sfgIter != sfgIterEnd; ++sfgIter)
  {
    const ServiceFeesGroup* sfg = (*sfgIter);
    if (sfg->state() == ServiceFeesGroup::VALID)
    {
      LOG4CXX_DEBUG(_logger,
                    "AncillaryPricingResponseFormatter::checkIfAnyGroupValid_DBG() - complete, "
                    "valid group found: "
                        << sfg->groupCode());
      return true;
    }
  }

  LOG4CXX_DEBUG(_logger,
                "AncillaryPricingResponseFormatter::checkIfAnyGroupValid() - complete, no "
                "valid groups found");
  return false;
}

void
AncillaryPricingResponseFormatter::formatOCFeesForR7(AncillaryPricingTrx& ancTrx,
                                                     XMLConstruct& construct,
                                                     bool timeOutMax)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::formatOCFeesForR7() - entered");
  std::vector<ServiceGroup> groupCodes; // requested Group codes
  ancTrx.getOptions()->getGroupCodes(ancTrx.getOptions()->serviceGroupsVec(), groupCodes);

  uint16_t maxNumberOfOCFees = 0;
  if (ancTrx.activationFlags().isAB240())
  {
    maxNumberOfOCFees = maxNumberOfOCFeesAb240Key.getValue();
  }
  else
  {
    maxNumberOfOCFees = maxNumberOfOCFeesKey.getValue();

    if (testOverrideMaxNumberOfFees.isValid(&ancTrx))
    {
      maxNumberOfOCFees = testOverrideMaxNumberOfFees.getValue(&ancTrx);
    }
  }

  uint16_t feesCount = 0;
  bool maxNumOfFeesReached = false;
  uint16_t dispOnlyFeesCount = 0;
  GroupFeesVector groupFeesVector;

  // iterate groups
  std::vector<ServiceFeesGroup*>::const_iterator sfgIter = currentItin()->ocFeesGroup().begin();
  std::vector<ServiceFeesGroup*>::const_iterator sfgIterEnd = currentItin()->ocFeesGroup().end();
  for (; sfgIter != sfgIterEnd; ++sfgIter)
  {
    const ServiceFeesGroup* sfg = (*sfgIter);
    const ServiceGroup sfgGroupCode = sfg->groupCode();
    std::vector<PaxOCFees> paxOcFees;
    switch (sfg->state())
    {
    case ServiceFeesGroup::VALID:
    {
      if (timeOutMax &&
          (std::find(groupCodes.begin(), groupCodes.end(), sfgGroupCode) == groupCodes.end()))
      {
        // group code is NOT in the request
        ServiceFeesGroup* sfg1 = const_cast<ServiceFeesGroup*>(sfg);
        sfg1->state() = ServiceFeesGroup::EMPTY;
        break;
      }

      if (maxNumOfFeesReached)
      {
        break;
      }
      ServiceFeeUtil::createOCFeesUsages(*sfg, ancTrx);
      AncRequest* ancReq = static_cast<AncRequest*>(ancTrx.getRequest());
      paxOcFees = isOriginalOrderNeeded(sfg, ancTrx)
                      ? ServiceFeeUtil::getFees(*sfg, ancReq->paxType(currentItin()))
                      : ServiceFeeUtil::getSortedFees(*sfg, ancReq->paxType(currentItin()));

      // iterate fees
      std::vector<PaxOCFees>::iterator paxOcFeesIter = paxOcFees.begin();
      std::vector<PaxOCFees>::iterator paxOcFeesIterEnd = paxOcFees.end();
      for (; paxOcFeesIter != paxOcFeesIterEnd; ++paxOcFeesIter, ++feesCount)
      {
        if (feesCount >= maxNumberOfOCFees)
        {
          maxNumOfFeesReached = true;
          // remove fees exceeding MAX_NUMBER_OF_FEES
          // paxOcFees.erase(paxOcFeesIter, paxOcFeesIterEnd); //Code commented as max number of
          // fees needs to be checked later
          break;
        }

        if ((*paxOcFeesIter).fees()->isDisplayOnly() ||
            ((*paxOcFeesIter).fees()->optFee() &&
             (*paxOcFeesIter).fees()->optFee()->notAvailNoChargeInd() == 'X'))
        {
          dispOnlyFeesCount++;
        }
      }
    }
    break;

    case ServiceFeesGroup::EMPTY:
    case ServiceFeesGroup::NOT_AVAILABLE:
      break;

    default:
      break;
    }
    groupFeesVector.push_back(std::make_pair(sfg, paxOcFees));
  }

  bool allFeesDisplayOnly = false;
  // No need to check historical for R7 post ticket, it doesn't have historical
  // R7 Tuning has historical and lowfarerebook
  if (feesCount && (isOcFeesTrxDisplayOnly(ancTrx) || (dispOnlyFeesCount == feesCount)))
    allFeesDisplayOnly = true;

  if (maxNumOfFeesReached || timeOutMax)
  {
    if (ancTrx.billing()->requestPath() != SWS_PO_ATSE_PATH)
    {
      if (_isWpAePostTicket)
      {
        if (ancTrx.itin().size() > 1 || (ancTrx.itin().size() == 1 && groupCodes.size() > 1))
          formatOCGenericMsg(construct, "MORE AIR EXTRAS AVAILABLE - USE WPAE*T WITH QUALIFIERS");
      }
      else
        formatOCGenericMsg(construct, "MORE AIR EXTRAS AVAILABLE - USE WPAE WITH QUALIFIERS");
    }
  }
  _currentIndex = 1;
  formatOCFeesGroupsForR7(ancTrx, construct, groupFeesVector, allFeesDisplayOnly, timeOutMax);

  if (allFeesDisplayOnly)
    formatOCTrailer(construct, "AL"); // ST0 - AL
}

void
AncillaryPricingResponseFormatter::processServiceGroupForR7U(
    ServiceFeesGroup* sfg,
    AncillaryPricingTrx& ancTrx,
    bool timeOutMax,
    const std::vector<ServiceGroup>& groupCodes,
    uint16_t& maxNumberOfOCFees,
    bool& maxNumOfFeesReached,
    uint16_t& feesCount,
    uint16_t& dispOnlyFeesCount,
    std::vector<PaxOCFeesUsages>& paxOcFees)
{
  const ServiceGroup sfgGroupCode = sfg->groupCode();
  switch (sfg->state())
  {
  case ServiceFeesGroup::VALID:
  {
    if (timeOutMax &&
        (std::find(groupCodes.begin(), groupCodes.end(), sfgGroupCode) == groupCodes.end()))
    {
      // group code is NOT in the request
      ServiceFeesGroup* sfg1 = const_cast<ServiceFeesGroup*>(sfg);
      sfg1->state() = ServiceFeesGroup::EMPTY;
      break;
    }

    if (maxNumOfFeesReached)
    {
      break;
    }
    ServiceFeeUtil::createOCFeesUsages(*sfg, ancTrx);
    AncRequest* ancReq = static_cast<AncRequest*>(ancTrx.getRequest());
    paxOcFees = isOriginalOrderNeeded(sfg, ancTrx)
                    ? ServiceFeeUtil::getFeesUsages(*sfg, ancReq->paxType(currentItin()))
                    : ServiceFeeUtil::getSortedFeesUsages(*sfg, ancReq->paxType(currentItin()));

    // iterate fees
    std::vector<PaxOCFeesUsages>::iterator paxOcFeesIter = paxOcFees.begin();
    std::vector<PaxOCFeesUsages>::iterator paxOcFeesIterEnd = paxOcFees.end();
    for (; paxOcFeesIter != paxOcFeesIterEnd; ++paxOcFeesIter, ++feesCount)
    {
      if (feesCount >= maxNumberOfOCFees)
      {
        maxNumOfFeesReached = true;
        // remove fees exceeding MAX_NUMBER_OF_FEES
        // paxOcFees.erase(paxOcFeesIter, paxOcFeesIterEnd); //Code commented as max number of
        // fees needs to be checked later
        break;
      }

      if ((*paxOcFeesIter).fees()->isDisplayOnly() ||
          ((*paxOcFeesIter).fees()->optFee() &&
           (*paxOcFeesIter).fees()->optFee()->notAvailNoChargeInd() == 'X'))
      {
        dispOnlyFeesCount++;
      }
    }
  }
  break;

  case ServiceFeesGroup::EMPTY:
  case ServiceFeesGroup::NOT_AVAILABLE:
    break;

  default:
    break;
  }
}

void
AncillaryPricingResponseFormatter::formatOCFeesForR7U(AncillaryPricingTrx& ancTrx,
                                                      XMLConstruct& construct,
                                                      bool timeOutMax)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::formatOCFeesForR7U() - entered");
  std::vector<ServiceGroup> groupCodes; // requested Group codes
  ancTrx.getOptions()->getGroupCodes(ancTrx.getOptions()->serviceGroupsVec(), groupCodes);

  uint16_t maxNumberOfOCFees;
  if (ancTrx.activationFlags().isAB240())
  {
    maxNumberOfOCFees = maxNumberOfOCFeesAb240Key.getValue();
  }
  else
  {
    maxNumberOfOCFees = maxNumberOfOCFeesAb240Key.getValue();
    if (testOverrideMaxNumberOfFees.isValid(&ancTrx))
    {
      maxNumberOfOCFees = testOverrideMaxNumberOfFees.getValue(&ancTrx);
    }
  }

  uint16_t feesCount = 0;
  bool maxNumOfFeesReached = false;
  uint16_t dispOnlyFeesCount = 0;
  GroupFeesUsagesVector groupFeesUsagesVector;

  // iterate groups
  std::vector<ServiceFeesGroup*>::const_iterator sfgIter = currentItin()->ocFeesGroup().begin();
  std::vector<ServiceFeesGroup*>::const_iterator sfgIterEnd = currentItin()->ocFeesGroup().end();
  for (; sfgIter != sfgIterEnd; ++sfgIter)
  {
    LOG4CXX_DEBUG(_logger,
                  "AncillaryPricingResponseFormatter::formatOCFeesForR7U() - processing group \""
                      << (*sfgIter)->groupCode() << "\", state=" << (*sfgIter)->stateStr());
    std::vector<PaxOCFeesUsages> paxOcFees;
    processServiceGroupForR7U(*sfgIter,
                              ancTrx,
                              timeOutMax,
                              groupCodes,
                              maxNumberOfOCFees,
                              maxNumOfFeesReached,
                              feesCount,
                              dispOnlyFeesCount,
                              paxOcFees);
    groupFeesUsagesVector.push_back(std::make_pair(*sfgIter, paxOcFees));
  }

  if (!fallback::fixed::AB240_DecoupleServiceFeesAndFreeBag())
  {
    sfgIter = currentItin()->ocFeesGroupsFreeBag().begin();
    sfgIterEnd = currentItin()->ocFeesGroupsFreeBag().end();
    for (; sfgIter != sfgIterEnd; ++sfgIter)
    {
      LOG4CXX_DEBUG(_logger,
                    "AncillaryPricingResponseFormatter::formatOCFeesForR7U() - processing group \""
                        << (*sfgIter)->groupCode() << "\", state=" << (*sfgIter)->stateStr());
      std::vector<PaxOCFeesUsages> paxOcFees;
      processServiceGroupForR7U(*sfgIter,
                                ancTrx,
                                timeOutMax,
                                groupCodes,
                                maxNumberOfOCFees,
                                maxNumOfFeesReached,
                                feesCount,
                                dispOnlyFeesCount,
                                paxOcFees);
      groupFeesUsagesVector.push_back(std::make_pair(*sfgIter, paxOcFees));
    }
  }

  bool allFeesDisplayOnly = false;
  // No need to check historical for R7 post ticket, it doesn't have historical
  // R7 Tuning has historical and lowfarerebook
  if (feesCount && (isOcFeesTrxDisplayOnly(ancTrx) || (dispOnlyFeesCount == feesCount)))
    allFeesDisplayOnly = true;

  if (maxNumOfFeesReached || timeOutMax)
  {
    if (ancTrx.billing()->requestPath() != SWS_PO_ATSE_PATH)
    {
      if (_isWpAePostTicket)
      {
        if (ancTrx.itin().size() > 1 || (ancTrx.itin().size() == 1 && groupCodes.size() > 1))
          formatOCGenericMsg(construct, "MORE AIR EXTRAS AVAILABLE - USE WPAE*T WITH QUALIFIERS");
      }
      else
        formatOCGenericMsg(construct, "MORE AIR EXTRAS AVAILABLE - USE WPAE WITH QUALIFIERS");
    }
  }
  _currentIndex = 1;
  if (ancTrx.getRequest()->isMultiTicketRequest())
    _currentIndex = _currentIndexForMT_AE;
  formatOCFeesGroupsForR7(ancTrx, construct, groupFeesUsagesVector, allFeesDisplayOnly, timeOutMax);

  if (allFeesDisplayOnly)
    formatOCTrailer(construct, "AL"); // ST0 - AL
}

void
AncillaryPricingResponseFormatter::processOCFeesInServiceGroup(
    const ServiceFeesGroup* sfg,
    AncillaryPricingTrx& ancTrx,
    const std::vector<ServiceGroup>& groupCodes,
    bool timeOutMax,
    uint16_t& feesCount,
    uint16_t& dispOnlyFeesCount,
    std::vector<PaxOCFees>& paxOcFees)
{
  TSE_ASSERT(sfg);
  if (IS_DEBUG_ENABLED(_logger))
  {
    std::ostringstream dbgMsg;
    dbgMsg << __LOG4CXX_FUNC__;
    dbgMsg << ": sfg={groupCode=" << sfg->groupCode();
    dbgMsg << ", state=" << sfg->stateStr();
    dbgMsg << ", ocFeesMap=[" << sfg->ocFeesMap().size() << "]{";
    for (auto farePathAndOcFees : sfg->ocFeesMap())
    {
      dbgMsg << farePathAndOcFees.first->paxType()->paxType();
      dbgMsg << "=>[" << farePathAndOcFees.second.size() << "],";
    }
    dbgMsg << "}}, paxOcFees.size()=" << paxOcFees.size();
    LOG4CXX_DEBUG(_logger, dbgMsg.str());
  }

  const ServiceGroup sfgGroupCode = sfg->groupCode();
  switch (sfg->state())
  {
  case ServiceFeesGroup::VALID:
  {
    if (timeOutMax &&
        (std::find(groupCodes.begin(), groupCodes.end(), sfgGroupCode) == groupCodes.end()))
    {
      // group code is NOT in the request
      LOG4CXX_DEBUG(
          _logger, __LOG4CXX_FUNC__ << "Group " << sfgGroupCode << " is not in request - skipping");
      ServiceFeesGroup* sfg1 = const_cast<ServiceFeesGroup*>(sfg);
      sfg1->state() = ServiceFeesGroup::EMPTY;
      break;
    }
    ServiceFeeUtil::createOCFeesUsages(*sfg, ancTrx, true);
    AncRequest* ancReq = static_cast<AncRequest*>(ancTrx.getRequest());

    paxOcFees = isOriginalOrderNeeded(sfg, ancTrx)
                    ? ServiceFeeUtil::getFees(*sfg, ancReq->paxType(currentItin()))
                    : ServiceFeeUtil::getSortedFees(*sfg, ancReq->paxType(currentItin()));

    // iterate fees
    std::vector<PaxOCFees>::iterator paxOcFeesIter = paxOcFees.begin();
    std::vector<PaxOCFees>::iterator paxOcFeesIterEnd = paxOcFees.end();
    for (; paxOcFeesIter != paxOcFeesIterEnd; ++paxOcFeesIter, ++feesCount)
    {
      if ((*paxOcFeesIter).fees()->isDisplayOnly() ||
          ((*paxOcFeesIter).fees()->optFee() &&
           ((*paxOcFeesIter).fees()->optFee()->notAvailNoChargeInd() == 'X')))
      {
        dispOnlyFeesCount++;
      }
    }
  }
  break;

  case ServiceFeesGroup::EMPTY:
  case ServiceFeesGroup::NOT_AVAILABLE:
    break;

  default:
    break;
  }

  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__ << " - exiting");
}

void
AncillaryPricingResponseFormatter::formatOCFees(AncillaryPricingTrx& ancTrx,
                                                XMLConstruct& construct,
                                                bool timeOutMax)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::formatOCFees() - entered");
  std::vector<ServiceGroup> groupCodes; // requested Group codes
  ancTrx.getOptions()->getGroupCodes(ancTrx.getOptions()->serviceGroupsVec(), groupCodes);

  uint16_t feesCount = 0;
  uint16_t dispOnlyFeesCount = 0;
  GroupFeesVector groupFeesVector;

  // iterate groups
  if (ancTrx.activationFlags().isAB240())
  {
    // Process the groups created by SERVICE_FEES using WP*AE mode,
    // but exclude all the MSG elements (see formatOCFeesGroupsForR7)
    _isWPDispAE = true;
  }

  std::vector<ServiceFeesGroup*>::const_iterator sfgIter = currentItin()->ocFeesGroup().begin();
  std::vector<ServiceFeesGroup*>::const_iterator sfgIterEnd = currentItin()->ocFeesGroup().end();
  for (; sfgIter != sfgIterEnd; ++sfgIter)
  {
    std::vector<PaxOCFees> paxOcFees;
    processOCFeesInServiceGroup(
        *sfgIter, ancTrx, groupCodes, timeOutMax, feesCount, dispOnlyFeesCount, paxOcFees);
    groupFeesVector.push_back(std::make_pair(*sfgIter, paxOcFees));
  }

  if (ancTrx.activationFlags().isAB240())
  {
    // Process all the groups created by SERVICE_FEES and exclude
    // them from the next processing step which will use M70 logic
    formatOCFeesGroupsForR7(
        ancTrx, construct, groupFeesVector, feesCount == dispOnlyFeesCount, timeOutMax);
    groupFeesVector.clear();
    _isWPDispAE = false;
  }

  if (!fallback::fixed::AB240_DecoupleServiceFeesAndFreeBag())
  {
    sfgIter = currentItin()->ocFeesGroupsFreeBag().begin();
    sfgIterEnd = currentItin()->ocFeesGroupsFreeBag().end();
    for (; sfgIter != sfgIterEnd; ++sfgIter)
    {
      std::vector<PaxOCFees> paxOcFees;
      processOCFeesInServiceGroup(
          *sfgIter, ancTrx, groupCodes, timeOutMax, feesCount, dispOnlyFeesCount, paxOcFees);
      groupFeesVector.push_back(std::make_pair(*sfgIter, paxOcFees));
    }
  }

  formatOCFeesGroups(ancTrx, construct, groupFeesVector, timeOutMax);
}

void
AncillaryPricingResponseFormatter::buildGroupHeader(const ServiceFeesGroup* sfg,
                                                    XMLConstruct& construct)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::buildGroupHeader() - entered");
  std::ostringstream outputLine;
  outputLine.setf(std::ios::left, std::ios::adjustfield);
  // replace character '(' and ')' characters with * character
  std::string grpDesc(sfg->groupDescription());
  replaceSpecialCharInDisplay(grpDesc);
  outputLine << sfg->groupCode() << "-" << std::setw(34) << grpDesc << "CXR SEG/CPA          FEE";

  prepareResponseText(outputLine.str(), construct);
}

void
AncillaryPricingResponseFormatter::formatOCFeesGroupsForR7(AncillaryPricingTrx& ancTrx,
                                                           XMLConstruct& construct,
                                                           const GroupFeesVector& groupFeesVector,
                                                           const bool allFeesDisplayOnly,
                                                           bool timeOutMax)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::formatOCFeesGroupsForR7() - entered");

  if (IS_DEBUG_ENABLED(_logger))
  {
    std::ostringstream dbgMsg;
    for (auto gf : groupFeesVector)
    {
      dbgMsg << gf.first->groupCode();
      dbgMsg << "=>[" << gf.second.size() << "]{";
      for (auto pf : gf.second)
      {
        dbgMsg << pf.paxType();
        dbgMsg << ",";
      }
      dbgMsg << "},";
    }
    LOG4CXX_DEBUG(_logger,
                  __LOG4CXX_FUNC__ << ": groupFeesVector=[" << groupFeesVector.size() << "]{"
                                   << dbgMsg.str() << "}");
  }

  bool dispValid = false;
  const bool mixedSegmentsInItin =
      !currentItin()->allSegsConfirmed() && !currentItin()->allSegsUnconfirmed();

  GroupFeesVector::const_iterator groupFeesVectorIter = groupFeesVector.begin();
  GroupFeesVector::const_iterator groupFeesVectorIterEnd = groupFeesVector.end();
  for (; groupFeesVectorIter != groupFeesVectorIterEnd && !_doNotProcess; ++groupFeesVectorIter)
  {
    if (shouldSkipCreatingOcgElement(ancTrx, groupFeesVectorIter->second))
      continue;

    const ServiceFeesGroup* sfg = (*groupFeesVectorIter).first;
    const ServiceGroup sfgGroupCode = sfg->groupCode();

    construct.openElement(xml2::OCGroupInfo); // OCG Open
    construct.addAttributeInteger(xml2::GenId, (*groupFeesVectorIter).second.size());
    construct.addAttribute(xml2::OCFeeGroupCode, sfgGroupCode);
    if (!timeOutMax)
    {
      if (sfg->state() == ServiceFeesGroup::EMPTY)
      {
        construct.addAttribute(xml2::OCGroupStatusCode, "NF"); // ST0 - NF
      }
      else if (sfg->state() == ServiceFeesGroup::NOT_AVAILABLE)
      {
        construct.addAttribute(xml2::OCGroupStatusCode, "NA"); // ST0 - NA
      }
      // PSS issue a msg for Inavlid Group code
    }

    std::vector<PaxOCFees>::const_iterator feesIter = (*groupFeesVectorIter).second.begin();
    std::vector<PaxOCFees>::const_iterator feesIterEnd = (*groupFeesVectorIter).second.end();
    if (feesIter != feesIterEnd)
    {
      if (sfg->state() == ServiceFeesGroup::VALID)
      {
        dispValid = true;
        // use a different construct so we can add status indicators to the group tag
        // once all fees are processed
        XMLConstruct feesConstruct;

        // format group header
        if (!ancTrx.activationFlags().isAB240()) // MSG elements are not required for AB240
          buildGroupHeader(sfg, feesConstruct);

        std::string st1Ind = "";
        // format individual OC fee lines
        feesIter = (*groupFeesVectorIter).second.begin();
        feesIterEnd = (*groupFeesVectorIter).second.end();
        for (; feesIter != feesIterEnd && !_doNotProcess; ++feesIter)
        {
          if (allFeesDisplayOnly)
          {
            // request is display only or all fees marked as display only
            if (!ancTrx.activationFlags().isAB240()) // MSG elements are not required for AB240
              formatOCFeesLineForR7(ancTrx, feesConstruct, *feesIter, 0, ' ');
            buildOSCData(ancTrx, feesConstruct, *feesIter);
          }
          else
          {
            const char footnoteChar = getFootnoteByHirarchyOrder(*(*feesIter).fees());
            if (footnoteChar != ' ')
              buildST1data(st1Ind, &footnoteChar);
            if (footnoteChar == '@' || footnoteChar == '*')
            {
              if (!ancTrx.activationFlags().isAB240()) // MSG elements are not required for AB240
                formatOCFeesLineForR7(ancTrx, feesConstruct, *feesIter, 0, footnoteChar);
              buildOSCData(ancTrx, feesConstruct, *feesIter);
            }
            else
            {
              if (!ancTrx.activationFlags().isAB240()) // MSG elements are not required for AB240
                formatOCFeesLineForR7(
                    ancTrx, feesConstruct, *feesIter, _currentIndex, footnoteChar);
              buildOSCData(ancTrx, feesConstruct, *feesIter, _currentIndex);
              ++_currentIndex;
            }
          }

          if (maxResponseReached())
            _doNotProcess = true;
        }
        if (!st1Ind.empty())
        {
          construct.addAttribute(xml2::OCGroupStatusInd, st1Ind); // Add attribute ST1
        }
        construct.addElementData(feesConstruct.getXMLData().c_str());
      }
    }
    construct.closeElement(); // OCG Close
    if (_doNotProcess)
      break;
  }
  if (mixedSegmentsInItin && dispValid)
    formatOCTrailer(construct, "UF"); // ST0 - UF
}

bool
AncillaryPricingResponseFormatter::isOCSData(const PaxOCFees& paxOCFees) const
{
  for (OCFeesUsage* ocFeesUsage : paxOCFees.fees()->ocfeeUsage())
  {
    if (ocFeesUsage->optFee())
      return true;
  }
  return false;
}

bool
AncillaryPricingResponseFormatter::isOCSData(const std::vector<PaxOCFees>& paxOCFeesVector) const
{
  for (PaxOCFees paxOCFees : paxOCFeesVector)
    if (isOCSData(paxOCFees))
      return true;

  return false;
}

int
AncillaryPricingResponseFormatter::getOCSNumber(const std::vector<PaxOCFees>& paxOCFeesVector) const
{
  int ocsNumber = 0;

  for (PaxOCFees paxOCFees : paxOCFeesVector)
    if (isOCSData(paxOCFees))
      ++ocsNumber;

  return ocsNumber;
}

void
AncillaryPricingResponseFormatter::formatOCFeesGroupsForR7(
    AncillaryPricingTrx& ancTrx,
    XMLConstruct& construct,
    const GroupFeesUsagesVector& groupFeesUsagesVector,
    const bool allFeesDisplayOnly,
    bool timeOutMax)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::formatOCFeesGroupsForR7() - entered");

  bool dispValid = false;
  const bool mixedSegmentsInItin =
      !currentItin()->allSegsConfirmed() && !currentItin()->allSegsUnconfirmed();

  GroupFeesUsagesVector::const_iterator groupFeesVectorIter = groupFeesUsagesVector.begin();
  GroupFeesUsagesVector::const_iterator groupFeesVectorIterEnd = groupFeesUsagesVector.end();
  for (; groupFeesVectorIter != groupFeesVectorIterEnd && !_doNotProcess; ++groupFeesVectorIter)
  {
    const ServiceFeesGroup* sfg = (*groupFeesVectorIter).first;
    const ServiceGroup sfgGroupCode = sfg->groupCode();

    construct.openElement(xml2::OCGroupInfo); // OCG Open
    construct.addAttributeInteger(xml2::GenId, (*groupFeesVectorIter).second.size());
    construct.addAttribute(xml2::OCFeeGroupCode, sfgGroupCode);
    if (!timeOutMax)
    {
      if (sfg->state() == ServiceFeesGroup::EMPTY)
      {
        construct.addAttribute(xml2::OCGroupStatusCode, "NF"); // ST0 - NF
      }
      else if (sfg->state() == ServiceFeesGroup::NOT_AVAILABLE)
      {
        construct.addAttribute(xml2::OCGroupStatusCode, "NA"); // ST0 - NA
      }
      // PSS issue a msg for Inavlid Group code
    }

    std::vector<PaxOCFeesUsages>::const_iterator feesIter = (*groupFeesVectorIter).second.begin();
    std::vector<PaxOCFeesUsages>::const_iterator feesIterEnd = (*groupFeesVectorIter).second.end();
    if (feesIter != feesIterEnd)
    {
      if (sfg->state() == ServiceFeesGroup::VALID)
      {
        dispValid = true;
        // use a different construct so we can add status indicators to the group tag
        // once all fees are processed
        XMLConstruct feesConstruct;

        // format group header
        if (!ancTrx.activationFlags().isAB240()) // MSG elements are not required for AB240
          buildGroupHeader(sfg, feesConstruct);

        std::string st1Ind = "";
        // format individual OC fee lines
        feesIter = (*groupFeesVectorIter).second.begin();
        feesIterEnd = (*groupFeesVectorIter).second.end();
        for (; feesIter != feesIterEnd && !_doNotProcess; ++feesIter)
        {
          if (allFeesDisplayOnly)
          {
            // request is display only or all fees marked as display only
            if (!ancTrx.activationFlags().isAB240()) // MSG elements are not required for AB240
              formatOCFeesLineForR7(ancTrx, feesConstruct, *feesIter, 0, ' ');
            buildOSCData(ancTrx, feesConstruct, *feesIter);
          }
          else
          {
            const char footnoteChar = getFootnoteByHirarchyOrder(*(*feesIter).fees());
            if (footnoteChar != ' ')
            {
              buildST1data(st1Ind, &footnoteChar);
            }
            if (footnoteChar == '@' || footnoteChar == '*')
            {
              if (!ancTrx.activationFlags().isAB240()) // MSG elements are not required for AB240
                formatOCFeesLineForR7(ancTrx, feesConstruct, *feesIter, 0, footnoteChar);
              buildOSCData(ancTrx, feesConstruct, *feesIter);
            }
            else
            {
              if (!ancTrx.activationFlags().isAB240()) // MSG elements are not required for AB240
                formatOCFeesLineForR7(
                    ancTrx, feesConstruct, *feesIter, _currentIndex, footnoteChar);
              buildOSCData(ancTrx, feesConstruct, *feesIter, _currentIndex);
              ++_currentIndex;
            }
          }

          if (maxResponseReached())
          {
            _doNotProcess = true;
          }
        }
        if (!st1Ind.empty())
        {
          construct.addAttribute(xml2::OCGroupStatusInd, st1Ind); // Add attribute ST1
        }
        construct.addElementData(feesConstruct.getXMLData().c_str());
      }
    }
    construct.closeElement(); // OCG Close
    if (_doNotProcess)
    {
      break;
    }
  }

  if (mixedSegmentsInItin && dispValid)
  {
    formatOCTrailer(construct, "UF"); // ST0 - UF
  }

  if (ancTrx.getRequest()->isMultiTicketRequest())
  {
    _currentIndexForMT_AE = _currentIndex;
  }
}

void
AncillaryPricingResponseFormatter::formatOCFeesGroups(AncillaryPricingTrx& ancTrx,
                                                      XMLConstruct& construct,
                                                      const GroupFeesVector& groupFeesVector,
                                                      bool timeOutMax)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::formatOCFeesGroups() - entered");

  if (IS_DEBUG_ENABLED(_logger))
    logGroupFeesVector(groupFeesVector);

  for (auto groupFees : groupFeesVector)
  {
    if (_doNotProcess)
      break;

    if (LIKELY(!shouldSkipCreatingOcgElement(ancTrx, groupFees.second)))
    {
      const ServiceFeesGroup* sfg = groupFees.first;
      const ServiceGroup sfgGroupCode = sfg->groupCode();
      LOG4CXX_DEBUG(_logger,
                    __LOG4CXX_FUNC__ << __LINE__ << "groupCode=" << sfgGroupCode << "["
                                     << groupFees.second.size() << "]");

      construct.openElement(xml2::OCGroupInfo); // OCG Open
      construct.addAttributeInteger(xml2::GenId, getOCSNumber(groupFees.second));
      construct.addAttribute(xml2::OCFeeGroupCode, sfgGroupCode);

      std::vector<PaxOCFees>::const_iterator feesIter = groupFees.second.begin();
      std::vector<PaxOCFees>::const_iterator feesIterEnd = groupFees.second.end();
      if (feesIter != feesIterEnd)
      {
        if (sfg->state() == ServiceFeesGroup::VALID)
        {
          if (isOCSData(groupFees.second))
            for (; feesIter != feesIterEnd && !_doNotProcess; ++feesIter)
            {
              if (isOCSData(*feesIter))
                buildOSCData(ancTrx, construct, *feesIter);
            }
          else
            prepareResponseText("NOT FOUND", construct);
        }
      }
      else if (sfg->state() == ServiceFeesGroup::EMPTY)
        prepareResponseText("NOT FOUND", construct);
      else if (sfg->state() == ServiceFeesGroup::NOT_AVAILABLE)
        prepareResponseText("NOT AVAILABLE", construct);
      else if (sfg->state() == ServiceFeesGroup::INVALID)
        prepareResponseText("NOT VALID", construct);

      construct.closeElement(); // OCG Close
    }
  }
}

void AncillaryPricingResponseFormatter::logGroupFeesVector(const GroupFeesVector& groupFeesVector)
{
  std::ostringstream dbgMsg;
  for (auto gf : groupFeesVector)
  {
    dbgMsg << gf.first->groupCode();
    dbgMsg << "=>[" << gf.second.size() << "]{";
    for (auto pf : gf.second)
    {
      dbgMsg << pf.paxType();
      dbgMsg << ",";
    }
    dbgMsg << "},";
  }
  LOG4CXX_DEBUG(_logger,
                __LOG4CXX_FUNC__ << ": groupFeesVector=[" << groupFeesVector.size() << "]{"
                                 << dbgMsg.str() << "}");
}

bool AncillaryPricingResponseFormatter::shouldSkipCreatingOcgElement(
  AncillaryPricingTrx& ancTrx, const std::vector<PaxOCFees>& paxOcFeesVector) const
{
  if (!ancTrx.isSecondCallForMonetaryDiscount())
    return false;
  else
    for (const auto& paxOcFees : paxOcFeesVector)
      if (paxOcFees.fees()->ocfeeUsage().size())
        return false;

  return true;
}

std::string
AncillaryPricingResponseFormatter::getPaxType(const PaxOCFees& paxOcFees)
{
  return paxOcFees.paxType();
}

std::string
AncillaryPricingResponseFormatter::getPaxType(const PaxOCFeesUsages& paxOcFees)
{
  return paxOcFees.paxType();
}

void
AncillaryPricingResponseFormatter::formatOCFeesLineForR7(AncillaryPricingTrx& ancTrx,
                                                         XMLConstruct& construct,
                                                         const PaxOCFees& paxOcFees,
                                                         const uint16_t index,
                                                         const char& indicator)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::formatOCFeesLineForR7() - entered");
  ServiceFeeUtil util(ancTrx);
  std::ostringstream paxOCFeeLine;

  paxOCFeeLine.setf(std::ios::left, std::ios::adjustfield);
  // we assume that indices are 1-based and 0 is a special value
  if (index)
  {
    paxOCFeeLine << std::setw(3) << index;
  }
  else
  {
    paxOCFeeLine << "-- ";
  }

  paxOCFeeLine << getPaxType(paxOcFees) << "-" << std::setw(30)
               << paxOcFees.fees()->subCodeInfo()->commercialName();
  paxOCFeeLine.unsetf(std::ios::left);
  paxOCFeeLine.setf(std::ios::right, std::ios::adjustfield);
  paxOCFeeLine << std::setw(3) << paxOcFees.fees()->carrierCode() << " ";
  paxOCFeeLine << std::setw(2) << currentItin()->segmentPnrOrder(paxOcFees.fees()->travelStart())
               << "-" << paxOcFees.fees()->travelStart()->origin()->loc()
               << paxOcFees.fees()->travelEnd()->destination()->loc();
  paxOCFeeLine.setf(std::ios::fixed, std::ios::floatfield);

  const DateTime& ticketingDate = ancTrx.ticketingDate();
  Money targetMoney = util.convertOCFeeCurrency(paxOcFees.fees());

  if (paxOcFees.fees()->optFee()->notAvailNoChargeInd() != 'X')
  {
    paxOCFeeLine.precision(targetMoney.noDec(ticketingDate));
    DataHandle dataHandle;
    OCFeesPrice* ocFeesPrice = OCFeesPrice::create(*paxOcFees.fees(), ancTrx, dataHandle);
    paxOCFeeLine << std::setw(11)
                 << ocFeesPrice->getFeeAmountInSellingCurrencyPlusTaxes(*paxOcFees.fees(),
                                                                        targetMoney);
  }
  else
  {
    paxOCFeeLine << std::setw(11) << "  NOT AVAIL";
  }

  paxOCFeeLine << " " << indicator << "\n";
  prepareResponseText(paxOCFeeLine.str(), construct);
}

void
AncillaryPricingResponseFormatter::formatOCFeesLineForR7(AncillaryPricingTrx& ancTrx,
                                                         XMLConstruct& construct,
                                                         const PaxOCFeesUsages& paxOcFees,
                                                         const uint16_t index,
                                                         const char& indicator)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::formatOCFeesLineForR7() - entered");

  std::ostringstream paxOCFeeLine;

  paxOCFeeLine.setf(std::ios::left, std::ios::adjustfield);
  // we assume that indices are 1-based and 0 is a special value
  if (index)
  {
    paxOCFeeLine << std::setw(3) << index;
  }
  else
  {
    paxOCFeeLine << "-- ";
  }

  paxOCFeeLine << getPaxType(paxOcFees) << "-" << std::setw(30)
               << getCommercialName(paxOcFees.fees());

  paxOCFeeLine.unsetf(std::ios::left);
  paxOCFeeLine.setf(std::ios::right, std::ios::adjustfield);
  paxOCFeeLine << std::setw(3) << paxOcFees.fees()->carrierCode() << " ";
  paxOCFeeLine << std::setw(2) << currentItin()->segmentPnrOrder(paxOcFees.fees()->travelStart())
               << "-" << paxOcFees.fees()->travelStart()->origin()->loc()
               << paxOcFees.fees()->travelEnd()->destination()->loc();
  paxOCFeeLine.setf(std::ios::fixed, std::ios::floatfield);

  const DateTime& ticketingDate = ancTrx.ticketingDate();
  ServiceFeeUtil util(ancTrx);
  Money targetMoney = util.convertOCFeeCurrency(*(paxOcFees.fees()));

  if (paxOcFees.fees()->optFee()->notAvailNoChargeInd() != 'X')
  {
    paxOCFeeLine.precision(targetMoney.noDec(ticketingDate));
    DataHandle dataHandle;
    OCFeesPrice* ocFeesPrice = OCFeesPrice::create(*paxOcFees.fees(), ancTrx, dataHandle);
    paxOCFeeLine << std::setw(11)
                 << ocFeesPrice->getFeeAmountInSellingCurrencyPlusTaxes(*paxOcFees.fees(),
                                                                        targetMoney);
  }
  else
  {
    paxOCFeeLine << std::setw(11) << "  NOT AVAIL";
  }

  paxOCFeeLine << " " << indicator << "\n";
  prepareResponseText(paxOCFeeLine.str(), construct);
}

void
AncillaryPricingResponseFormatter::buildOSCData(AncillaryPricingTrx& ancTrx,
                                                XMLConstruct& construct,
                                                const PaxOCFees& paxOcFees,
                                                const uint16_t index)
{
  if (UNLIKELY(shouldSkipCreatingOscElement(ancTrx, paxOcFees)))
    return;

  LOG4CXX_DEBUG(_logger,
                __LOG4CXX_FUNC__ << ": paxOcFees.fees()->ocfeeUsage().size()="
                                 << paxOcFees.fees()->ocfeeUsage().size() << ", index=" << index
                                 << " - entered");

  // Build all detailed information
  construct.openElement(xml2::OCS5Info); // OSC Open

  // SHI - need to add line number
  if (index && !ancTrx.activationFlags().isAB240())
  {
    construct.addAttributeInteger(xml2::ServiceLineNumber, index);
  }

  buildOSCCommonData(construct, paxOcFees.fees()->subCodeInfo());

  buildOSCOptionalData(ancTrx, construct, paxOcFees, index);

  if (ancTrx.activationFlags().isAB240())
  {
    _maxOCInResponse = maxNumberOfOCFeesAb240Key.getValue();
  }
  else if (_isWPDispAE || (_isWpAePostTicket && !_maxSizeAllowed))
  {
    _maxOCInResponse = maxNumberOfOCFeesKey.getValue();

    if (testOverrideMaxNumberOfFees.isValid(&ancTrx))
    {
      _maxOCInResponse = testOverrideMaxNumberOfFees.getValue(&ancTrx);
    }
  }

  if (fallback::fallbackSurroundOscWithOccForMonetaryDiscount(&ancTrx))
  {
    uint32_t i = 0;
    int32_t totalsize = 0;
    for (; i < paxOcFees.fees()->ocfeeUsage().size() && _curOCInResponse < _maxOCInResponse; ++i)
    {
      OCFeesUsage ocFeeUsage = *(paxOcFees.fees()->ocfeeUsage()[i]);
      XMLConstruct constructS7;
      buildS7OOSData(ancTrx, constructS7, paxOcFees, ocFeeUsage, index);

      if (_isWPDispAE || (_isWpAePostTicket && !_maxSizeAllowed))
        totalsize = (_maxTotalBuffSizeForWPDispAE - 20 - construct.getXMLData().size());
      else
        totalsize = (_maxTotalBuffSize - construct.getXMLData().size());

      int32_t constructS7size = constructS7.getXMLData().size();

      if (constructS7size > totalsize)
      {
        LOG4CXX_DEBUG(_logger,
                      __LOG4CXX_FUNC__ << "RESPONSE SIZE LIMIT EXCEEDED - PROCESSING STOPPED");
        _doNotProcess = true;
        break;
      }
      else
      {
        LOG4CXX_DEBUG(_logger,
                      __LOG4CXX_FUNC__ << "ADDING constructS7 number " << _curOCInResponse
                                       << ", size=" << constructS7.getXMLData().size()
                                       << ", totalsize=" << totalsize);
        construct.addElementData(constructS7.getXMLData().c_str(), constructS7.getXMLData().size());
      }

      _curOCInResponse++;
    }
    construct.closeElement(); // OSC close
    if (maxResponseReached() && i < paxOcFees.fees()->ocfeeUsage().size())
    {
      LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__ << __LINE__ << " _doNotProcess=true");
      _doNotProcess = true;
    }
  }
  else
  {
    buildOscContent(ancTrx, construct, paxOcFees, index);

    construct.closeElement(); // OSC close
    if (maxResponseReached() &&
        getOcFeesUsageCountInOsc(paxOcFees) < paxOcFees.fees()->ocfeeUsage().size())
    {
      LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__ << __LINE__ << " _doNotProcess=true");
      _doNotProcess = true;
    }
  }
}

bool
AncillaryPricingResponseFormatter::shouldSkipCreatingOscElement(AncillaryPricingTrx& ancTrx,
                                                                const PaxOCFees& paxOcFees) const
{
  if (ancTrx.isSecondCallForMonetaryDiscount())
    if (!paxOcFees.fees()->ocfeeUsage().size())
      return true;

  return false;
}

void
AncillaryPricingResponseFormatter::buildOscContent(AncillaryPricingTrx& ancTrx,
                                                   XMLConstruct& construct,
                                                   const PaxOCFees& paxOcFees,
                                                   const uint16_t index)
{
  if (ancTrx.activationFlags().isMonetaryDiscount())
  {
    auto ancIdToOcFeesUsage = getAncillaryIdentifierToOcFeesUsageMap(paxOcFees);
    for (auto& ocFeesUsageGroup : ancIdToOcFeesUsage)
      buildOccData(ancTrx, construct, paxOcFees, index, ocFeesUsageGroup);
  }
  else
  {
    for (unsigned i = 0; i < getOcFeesUsageCountInOsc(paxOcFees); ++i)
    {
      OCFeesUsage ocFeeUsage = *(paxOcFees.fees()->ocfeeUsage()[i]);
      bool responseSizeExceeded = buildOosElement(ancTrx, construct, paxOcFees, index, ocFeeUsage);
      if (responseSizeExceeded)
        break;
    }
  }
}

unsigned
AncillaryPricingResponseFormatter::getOcFeesUsageCountInOsc(const PaxOCFees& paxOcFees) const
{
  return std::min((unsigned)paxOcFees.fees()->ocfeeUsage().size(), (unsigned)_maxOCInResponse);
}

std::map<AncillaryIdentifier, std::vector<OCFeesUsage*>>
AncillaryPricingResponseFormatter::getAncillaryIdentifierToOcFeesUsageMap(
    const PaxOCFees& paxOcFees)
{
  std::map<AncillaryIdentifier, std::vector<OCFeesUsage*>> ancIdToOcFeesUsage;
  unsigned ocFeesUsageCount = getOcFeesUsageCountInOsc(paxOcFees);

  for (unsigned i = 0; i < ocFeesUsageCount; ++i)
  {
    OCFeesUsage* ocFeesUsage = paxOcFees.fees()->ocfeeUsage()[i];
    ancIdToOcFeesUsage[ocFeesUsage->getAncillaryPriceIdentifier().getIdentifier()].push_back(
        ocFeesUsage);
  }

  return ancIdToOcFeesUsage;
}

void
AncillaryPricingResponseFormatter::buildOccData(
    AncillaryPricingTrx& ancTrx,
    XMLConstruct& construct,
    const PaxOCFees& paxOcFees,
    const uint16_t index,
    std::map<AncillaryIdentifier, std::vector<OCFeesUsage*>>::value_type ocFeesUsageGroup)
{
  construct.openElement(xml2::OosCollection);
  construct.addAttribute(xml2::AncillaryIdentifier, ocFeesUsageGroup.first.getIdentifier());

  for (unsigned i = 0; i < ocFeesUsageGroup.second.size(); ++i)
  {
    OCFeesUsage ocFeeUsage = *(ocFeesUsageGroup.second[i]);
    bool responseSizeExceeded = buildOosElement(ancTrx, construct, paxOcFees, index, ocFeeUsage);
    if (responseSizeExceeded)
      break;
  }

  construct.closeElement(); // OCC close
}

bool
AncillaryPricingResponseFormatter::buildOosElement(AncillaryPricingTrx& ancTrx,
                                                   XMLConstruct& construct,
                                                   const PaxOCFees& paxOcFees,
                                                   const uint16_t index,
                                                   OCFeesUsage& ocFeesUsage)
{
  int32_t totalsize;
  XMLConstruct constructS7;
  buildS7OOSData(ancTrx, constructS7, paxOcFees, ocFeesUsage, index);

  if (_isWPDispAE || (_isWpAePostTicket && !_maxSizeAllowed))
    totalsize = (_maxTotalBuffSizeForWPDispAE - 20 - construct.getXMLData().size());
  else
    totalsize = (_maxTotalBuffSize - construct.getXMLData().size());

  int32_t constructS7size = constructS7.getXMLData().size();

  if (constructS7size > totalsize)
  {
    LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__ << "RESPONSE SIZE LIMIT EXCEEDED - PROCESSING STOPPED");
    _doNotProcess = true;
    return true;
  }
  else
  {
    LOG4CXX_DEBUG(_logger,
                  __LOG4CXX_FUNC__ << "ADDING constructS7 number " << _curOCInResponse
                                   << ", size=" << constructS7.getXMLData().size()
                                   << ", totalsize=" << totalsize);
    construct.addElementData(constructS7.getXMLData().c_str(), constructS7.getXMLData().size());
  }

  _curOCInResponse++;
  return false;
}

void
AncillaryPricingResponseFormatter::buildOSCData(AncillaryPricingTrx& ancTrx,
                                                XMLConstruct& construct,
                                                const PaxOCFeesUsages& paxOcFeesUsages,
                                                const uint16_t index)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__ << ": index=" << index);
  // Build all detailed information
  construct.openElement(xml2::OCS5Info); // OSC Open

  // SHI - need to add line number
  if (index && !ancTrx.activationFlags().isAB240())
  {
    construct.addAttributeInteger(xml2::ServiceLineNumber, index);
  }

  buildOSCCommonData(construct, *(paxOcFeesUsages.fees()));

  buildOSCOptionalData(ancTrx, construct, paxOcFeesUsages, index);

  if (ancTrx.activationFlags().isAB240())
  {
    _maxOCInResponse = maxNumberOfOCFeesAb240Key.getValue();
  }
  else if (_isWPDispAE || (_isWpAePostTicket && !_maxSizeAllowed))
  {
    _maxOCInResponse = maxNumberOfOCFeesKey.getValue();

    if (testOverrideMaxNumberOfFees.isValid(&ancTrx))
    {
      _maxOCInResponse = testOverrideMaxNumberOfFees.getValue(&ancTrx);
    }
  }

  XMLConstruct constructS7;
  buildS7OOSData(ancTrx, constructS7, paxOcFeesUsages, index);

  int32_t totalsize = 0;
  if (_isWPDispAE || (_isWpAePostTicket && !_maxSizeAllowed))
    totalsize = (_maxTotalBuffSizeForWPDispAE - 20 - construct.getXMLData().size());
  else
    totalsize = (_maxTotalBuffSize - construct.getXMLData().size());

  int32_t constructS7size = constructS7.getXMLData().size();
  _curOCInResponse++;
  if (constructS7size > totalsize)
    _doNotProcess = true;
  else
    construct.addElementData(constructS7.getXMLData().c_str(), constructS7.getXMLData().size());

  construct.closeElement(); // OSC close
  if (maxResponseReached())
    _doNotProcess = true;
}

void
AncillaryPricingResponseFormatter::buildOSCOptionalData(AncillaryPricingTrx& ancTrx,
                                                        XMLConstruct& construct,
                                                        const PaxOCFees& paxOcFees,
                                                        const uint16_t index)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::buildOSCOptionalData() - entered");

  if (_isWPDispAE || _isWpAePostTicket)
  {
    if (ancTrx.activationFlags().isAB240())
    {
      const char footnoteChar = getFootnoteByHirarchyOrder(*paxOcFees.fees());
      bool isDisplayOnly = paxOcFees.fees()->isDisplayOnly() || isOcFeesTrxDisplayOnly(ancTrx) ||
                           footnoteChar == '*' || footnoteChar == '@';
      construct.addAttributeChar(xml2::ServiceDisplayOnly, isDisplayOnly ? 'Y' : 'N');
    }
    else
    {
      if (paxOcFees.fees()->isDisplayOnly() || isOcFeesTrxDisplayOnly(ancTrx) || !index)
      {
        // tags to be added for display only fees SFD
        construct.addAttributeChar(xml2::ServiceDisplayOnly, 'Y');
        return;
      }
    }
  }

  bool addSchema2Elements = ancTrx.getRequest()->majorSchemaVersion() >= 2;
  if (ancTrx.activationFlags().isAB240())
    addSchema2Elements =
        (paxOcFees.fees()->subCodeInfo()->fltTktMerchInd() != FLIGHT_RELATED_SERVICE &&
         paxOcFees.fees()->subCodeInfo()->fltTktMerchInd() != PREPAID_BAGGAGE);

  buildOSCEmdSoftPassIndicator(ancTrx, construct, paxOcFees.fees());

  buildOSCCommonOptionalData(
      ancTrx, construct, paxOcFees.fees()->subCodeInfo(), addSchema2Elements);
}

void
AncillaryPricingResponseFormatter::buildOSCEmdSoftPassIndicator(const AncillaryPricingTrx& ancTrx,
                                                                XMLConstruct& construct,
                                                                const OCFees* ocfees) const
{
  if (ancTrx.activationFlags().isEmdForCharges() && ocfees)
  {
    if (ocfees->getEmdSoftPassChargeIndicator() == EmdSoftPassIndicator::EmdSoftPass)
      construct.addAttributeChar(xml2::EmdChargeInd, 'T');

    if (ocfees->getEmdSoftPassChargeIndicator() == EmdSoftPassIndicator::EmdPassOrNoEmdValidation)
      construct.addAttributeChar(xml2::EmdChargeInd, 'F');
  }
}

void
AncillaryPricingResponseFormatter::buildOSCOptionalData(AncillaryPricingTrx& ancTrx,
                                                        XMLConstruct& construct,
                                                        const PaxOCFeesUsages& paxOcFees,
                                                        const uint16_t index)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::buildOSCOptionalData() - entered");

  if (_isWPDispAE || _isWpAePostTicket)
  {
    if (ancTrx.activationFlags().isAB240())
    {
      const char footnoteChar = getFootnoteByHirarchyOrder(*paxOcFees.fees());
      bool isDisplayOnly = paxOcFees.fees()->isDisplayOnly() || isOcFeesTrxDisplayOnly(ancTrx) ||
                           footnoteChar == '*' || footnoteChar == '@';
      construct.addAttributeChar(xml2::ServiceDisplayOnly, isDisplayOnly ? 'Y' : 'N');
    }
    else
    {
      if (paxOcFees.fees()->isDisplayOnly() || isOcFeesTrxDisplayOnly(ancTrx) || !index)
      {
        // tags to be added for display only fees SFD
        construct.addAttributeChar(xml2::ServiceDisplayOnly, 'Y');
        return;
      }
    }
  }

  bool addSchema2Elements = ancTrx.getRequest()->majorSchemaVersion() >= 2;
  if (ancTrx.activationFlags().isAB240())
    addSchema2Elements =
        (paxOcFees.fees()->subCodeInfo()->fltTktMerchInd() != FLIGHT_RELATED_SERVICE &&
         paxOcFees.fees()->subCodeInfo()->fltTktMerchInd() != PREPAID_BAGGAGE);

  buildOSCEmdSoftPassIndicator(ancTrx, construct, paxOcFees.fees()->oCFees());

  buildOSCCommonOptionalData(
      ancTrx, construct, paxOcFees.fees()->subCodeInfo(), addSchema2Elements);
}

void
AncillaryPricingResponseFormatter::buildS7OOSData(AncillaryPricingTrx& ancTrx,
                                                  XMLConstruct& construct,
                                                  const PaxOCFees& paxOcFees,
                                                  OCFeesUsage& ocFees,
                                                  const uint16_t index)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::buildS7OOSData() - entered");
  // For multiple S7 - check if the s7 is 0
  if (ocFees.optFee() == nullptr)
    return;

  // Build all detailed information
  construct.openElement(xml2::OOSS7Info); // OOS Open
  addDisclosureMarkers(ancTrx, construct, ocFees);

  if (ocFees.optFee()->fltTktMerchInd() == PREPAID_BAGGAGE)
  {
    // N11 - SFM - Sector-Portion Ind
    construct.addAttributeChar(xml2::OOSSecPorInd, PREPAID_BAGGAGE);
  }
  else if (ocFees.optFee()->sectorPortionInd() != ' ')
  {
    // N11 - SFM - Sector-Portion Ind
    construct.addAttributeChar(xml2::OOSSecPorInd, ocFees.optFee()->sectorPortionInd());
  }

  // A01 - Send Origin Airport Code
  if (ocFees.matchedOriginAirport() != "")
    construct.addAttribute(xml2::OOSOrigAirportCode, ocFees.matchedOriginAirport());
  else
    construct.addAttribute(xml2::OOSOrigAirportCode, ocFees.travelStart()->origAirport());

  // A02 - Send Destination Airport Code
  if (ocFees.matchedDestinationAirport() != "")
    construct.addAttribute(xml2::OOSDestAirportCode, ocFees.matchedDestinationAirport());
  else
    construct.addAttribute(xml2::OOSDestAirportCode, ocFees.travelEnd()->destAirport());

  if (ancTrx.showBaggageTravelIndex() && ocFees.baggageTravelIndex())
  {
    construct.addAttributeInteger(xml2::BaggageTravelIndex, ocFees.baggageTravelIndex());
  }

  if (ocFees.isAnyS7SoftPass())
  {
    // PS1 - Soft Match Indicator
    construct.addAttributeBoolean(xml2::OOSSoftMatchInd, true);
  }

  AncRequest* req = dynamic_cast<AncRequest*>(ancTrx.getRequest());
  bool isM70 = false;

  if (req && req->ancRequestType() == AncRequest::M70Request)
    isM70 = true;

  bool isPadisAllowed = isM70;
  if (!fallback::fallbackAB240(&ancTrx))
  {
    isPadisAllowed = (isM70 || ancTrx.isAB240AncillaryRequest());
  }

  // For M70, R7 and R7 tuning
  // Check if fee is display only fees or for non M70 req check if fee is not display only through
  // historical or low fare
  // AX1 tag
  if (!ocFees.isDisplayOnly() && ocFees.optFee()->availabilityInd() == 'Y' &&
      ocFees.optFee()->notAvailNoChargeInd() != 'X')
  {
    if ((!isM70 && index && !isOcFeesTrxDisplayOnly(ancTrx)) || isM70)
      construct.addAttributeBoolean(xml2::AvailService, true);
  }

  // Tax excempt (PY0) indicator for EMD-S phase 2 (M70)
  if (TrxUtil::isEMDSPhase2Activated(ancTrx) && isM70)
  {
    if (ocFees.optFee()->taxExemptInd() == 'Y')
      construct.addAttributeChar(xml2::OOSTaxExemptInd, 'Y');
    if (ocFees.optFee()->taxExemptInd() == 'N')
      construct.addAttributeChar(xml2::OOSTaxExemptInd, 'N');
  }

  if (fallback::fallbackAddSequenceNumberInOOS(&ancTrx))
  {
    const Billing* billing = ancTrx.billing();
    const SubCodeInfo* subCodeInfo = ocFees.subCodeInfo();
    if (isPadisAllowed && billing && (billing->requestPath() != ACS_PO_ATSE_PATH) && subCodeInfo &&
        ("SA" == subCodeInfo->serviceGroup()))
    {
      construct.addAttributeUInteger(xml2::OptionalServicesSeqNo, ocFees.optFee()->seqNo());
      if (!ocFees.padisData().empty())
        construct.addAttributeUInteger(xml2::OosPadisNumber,
                                       ocFees.optFee()->upgrdServiceFeesResBkgDesigTblItemNo());
    }
  }
  else
  {
    construct.addAttributeUInteger(xml2::OptionalServicesSeqNo, ocFees.optFee()->seqNo());

    const Billing* billing = ancTrx.billing();
    const SubCodeInfo* subCodeInfo = ocFees.subCodeInfo();
    if (isPadisAllowed && billing && (billing->requestPath() != ACS_PO_ATSE_PATH) && subCodeInfo &&
        ("SA" == subCodeInfo->serviceGroup()))
    {
      if (!ocFees.padisData().empty())
        construct.addAttributeUInteger(xml2::OosPadisNumber,
                                       ocFees.optFee()->upgrdServiceFeesResBkgDesigTblItemNo());
    }
  }

  if (ancTrx.activationFlags().isMonetaryDiscount())
    if (ocFees.hasPriceModification())
      if (ocFees.getAncillaryPriceModifier()._identifier)
        construct.addAttribute(xml2::AncillaryPriceModificationIdentifier,
                               ocFees.getAncillaryPriceModifier()._identifier.get());

  // Build Segment Numbers for Portion of Travel
  buildSegments(ancTrx, construct, ocFees);

  // Build ticket coupon section
  buildSegmentsTickeCoupons(ancTrx, construct, paxOcFees);

  if (!allowanceServiceType(ocFees.subCodeInfo()) ||
      (ancTrx.activationFlags().isAB240() &&
       !fallback::fallbackDisplayPriceForBaggageAllowance(&ancTrx)))
  {
    // Build Passenger and Fare Information for all type except allowance
    buildSUMData(ancTrx, construct, paxOcFees, ocFees);
  }

  // AB240 doesn't require SFQ element when soft matches are not needed
  if (!ancTrx.activationFlags().isAB240() || (req && !(req->hardMatchIndicator())))
  {
    if (_isWPDispAE || _isWpAePostTicket)
      buildSFQDataR7(construct, ocFees, index);
    else
      buildSFQData(construct, ocFees);
  }

  buildDTEData(ancTrx, construct, ocFees, index);

  // Build Fee Application and Ticketing Information
  buildFATData(ancTrx, construct, ocFees, index);

  // Build table 198, 171 and 186 data
  buildS7OOSOptionalData(construct, ocFees);

  if (isPadisAllowed && !ocFees.padisData().empty())
  {
    buildPadisData(ancTrx, construct, ocFees);
  }

  // Build Tax Information
  // buildTAXData(construct, ocFees);  --> PlaceHolder

  const bool isExemptAllTaxes = ancTrx.getRequest()->isExemptAllTaxes();
  const bool isExemptSpecificTaxes = ancTrx.getRequest()->isExemptSpecificTaxes();
  const std::vector<std::string>& taxIdExempted = ancTrx.getRequest()->taxIdExempted();

  TaxFormatter taxFormatter(construct, isExemptAllTaxes, isExemptSpecificTaxes, taxIdExempted);
  taxFormatter.format(ocFees);

  if (ocFees.subCodeInfo()->fltTktMerchInd() == BAGGAGE_CHARGE ||
      (ancTrx.activationFlags().isAB240() &&
       ocFees.subCodeInfo()->fltTktMerchInd() == BAGGAGE_EMBARGO))
  {
    buildBGD(construct, ocFees);
  }
  else if (ocFees.subCodeInfo()->fltTktMerchInd() == BAGGAGE_ALLOWANCE ||
           (ancTrx.activationFlags().isAB240() &&
            ocFees.subCodeInfo()->fltTktMerchInd() == CARRY_ON_ALLOWANCE))
  {
    buildBGAData(ancTrx, construct, ocFees);
  }

  construct.closeElement(); // OOS Close
}

void
AncillaryPricingResponseFormatter::buildS7OOSData(AncillaryPricingTrx& ancTrx,
                                                  XMLConstruct& construct,
                                                  const PaxOCFeesUsages& paxOcFees,
                                                  const uint16_t index)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::buildS7OOSData() - entered");

  const OCFeesUsage ocFees = *(paxOcFees.fees());
  // For multiple S7 - check if the s7 is 0
  if (ocFees.optFee() == nullptr)
    return;

  // Build all detailed information
  construct.openElement(xml2::OOSS7Info); // OOS Open
  addDisclosureMarkers(ancTrx, construct, ocFees);

  if (ocFees.optFee()->fltTktMerchInd() == PREPAID_BAGGAGE)
  {
    // N11 - SFM - Sector-Portion Ind
    construct.addAttributeChar(xml2::OOSSecPorInd, PREPAID_BAGGAGE);
  }
  else if (ocFees.optFee()->sectorPortionInd() != ' ')
  {
    // N11 - SFM - Sector-Portion Ind
    construct.addAttributeChar(xml2::OOSSecPorInd, ocFees.optFee()->sectorPortionInd());
  }

  // A01 - Send Origin Airport Code
  if (ocFees.matchedOriginAirport() != "")
    construct.addAttribute(xml2::OOSOrigAirportCode, ocFees.matchedOriginAirport());
  else
    construct.addAttribute(xml2::OOSOrigAirportCode, ocFees.travelStart()->origAirport());

  // A02 - Send Destination Airport Code
  if (ocFees.matchedDestinationAirport() != "")
    construct.addAttribute(xml2::OOSDestAirportCode, ocFees.matchedDestinationAirport());
  else
    construct.addAttribute(xml2::OOSDestAirportCode, ocFees.travelEnd()->destAirport());

  if (ancTrx.showBaggageTravelIndex() && ocFees.baggageTravelIndex())
  {
    construct.addAttributeInteger(xml2::BaggageTravelIndex, ocFees.baggageTravelIndex());
  }

  if (ocFees.isAnyS7SoftPass())
  {
    // PS1 - Soft Match Indicator
    construct.addAttributeBoolean(xml2::OOSSoftMatchInd, true);
  }

  AncRequest* req = dynamic_cast<AncRequest*>(ancTrx.getRequest());
  bool isM70 = false;

  if (req && req->ancRequestType() == AncRequest::M70Request)
    isM70 = true;

  bool isPadisAllowed = false;
  if (!fallback::fallbackAB240(&ancTrx))
    isPadisAllowed = ancTrx.isAB240AncillaryRequest();

  // For M70, R7 and R7 tuning
  // Check if fee is display only fees or for non M70 req check if fee is not display only through
  // historical or low fare
  // AX1 tag
  if (!ocFees.isDisplayOnly() && ocFees.optFee()->availabilityInd() == 'Y' &&
      ocFees.optFee()->notAvailNoChargeInd() != 'X')
  {
    if ((!isM70 && index && !isOcFeesTrxDisplayOnly(ancTrx)) || isM70)
      construct.addAttributeBoolean(xml2::AvailService, true);
  }

  // Tax excempt (PY0) indicator for EMD-S phase 2 (M70)
  if (TrxUtil::isEMDSPhase2Activated(ancTrx) && isM70)
  {
    if (ocFees.optFee()->taxExemptInd() == 'Y')
      construct.addAttributeChar(xml2::OOSTaxExemptInd, 'Y');
    if (ocFees.optFee()->taxExemptInd() == 'N')
      construct.addAttributeChar(xml2::OOSTaxExemptInd, 'N');
  }

  const bool addPadisData =
      (ancTrx.isAddPadisData(isPadisAllowed, ocFees.subCodeInfo()->serviceGroup()) &&
       !ocFees.padisData().empty());

  if (fallback::fallbackAddSequenceNumberInOOS(&ancTrx))
  {
    if (addPadisData)
    {
      construct.addAttributeUInteger(xml2::OptionalServicesSeqNo, ocFees.optFee()->seqNo());
      construct.addAttributeUInteger(xml2::OosPadisNumber,
                                     ocFees.optFee()->upgrdServiceFeesResBkgDesigTblItemNo());
    }
  }
  else
  {
    construct.addAttributeUInteger(xml2::OptionalServicesSeqNo, ocFees.optFee()->seqNo());

    if (addPadisData)
      construct.addAttributeUInteger(xml2::OosPadisNumber,
                                     ocFees.optFee()->upgrdServiceFeesResBkgDesigTblItemNo());
  }

  // Build Segment Numbers for Portion of Travel
  buildSegments(ancTrx, construct, ocFees);

  // Build ticket coupon section
  buildSegmentsTickeCoupons(ancTrx, construct, paxOcFees);

  if (!allowanceServiceType(ocFees.subCodeInfo()) ||
      (ancTrx.activationFlags().isAB240() &&
       !fallback::fallbackDisplayPriceForBaggageAllowance(&ancTrx)))
  {
    // Build Passenger and Fare Information for all type except allowance
    buildSUMData(ancTrx, construct, paxOcFees);
  }

  // AB240 doesn't require SFQ element when soft matches are not needed
  if (!ancTrx.activationFlags().isAB240() || (req && !(req->hardMatchIndicator())))
  {
    if (_isWPDispAE || _isWpAePostTicket)
      buildSFQDataR7(construct, ocFees, index);
    else
      buildSFQData(construct, ocFees);
  }

  buildDTEData(ancTrx, construct, ocFees, index);

  // Build Fee Application and Ticketing Information
  buildFATData(ancTrx, construct, ocFees, index);

  // Build table 198, 171 and 186 data
  buildS7OOSOptionalData(construct, ocFees);

  if (addPadisData)
    buildPadisData(ancTrx, construct, ocFees);

  // Build Tax Information
  // buildTAXData(construct, ocFees);  --> PlaceHolder

  const bool isExemptAllTaxes = ancTrx.getRequest()->isExemptAllTaxes();
  const bool isExemptSpecificTaxes = ancTrx.getRequest()->isExemptSpecificTaxes();
  const std::vector<std::string>& taxIdExempted = ancTrx.getRequest()->taxIdExempted();

  TaxFormatter taxFormatter(construct, isExemptAllTaxes, isExemptSpecificTaxes, taxIdExempted);
  taxFormatter.format(ocFees);

  if (ocFees.subCodeInfo()->fltTktMerchInd() == BAGGAGE_CHARGE ||
      (ancTrx.activationFlags().isAB240() &&
       ocFees.subCodeInfo()->fltTktMerchInd() == BAGGAGE_EMBARGO))
  {
    buildBGD(construct, ocFees);
  }
  else if (ocFees.subCodeInfo()->fltTktMerchInd() == BAGGAGE_ALLOWANCE ||
           (ancTrx.activationFlags().isAB240() &&
            ocFees.subCodeInfo()->fltTktMerchInd() == CARRY_ON_ALLOWANCE))
  {
    buildBGAData(ancTrx, construct, ocFees);
  }

  construct.closeElement(); // OOS Close
}

void
AncillaryPricingResponseFormatter::buildSegmentsTickeCoupons(AncillaryPricingTrx& ancTrx,
                                                             XMLConstruct& construct,
                                                             const PaxOCFeesUsages& paxOcFees)
{
}

void
AncillaryPricingResponseFormatter::buildSegmentsTickeCoupons(AncillaryPricingTrx& ancTrx,
                                                             XMLConstruct& construct,
                                                             const PaxOCFees& paxOcFees)
{
}

void
AncillaryPricingResponseFormatter::addDisclosureMarkers(AncillaryPricingTrx& ancTrx,
                                                        XMLConstruct& construct,
                                                        const OCFeesUsage& ocFees)
{
  if (!ancTrx.activationFlags().isAB240() ||
      !ancTrx.getOptions()->isOcOrBaggageDataRequested(RequestedOcFeeGroup::DisclosureData))
    return;

  const BaggageTravel* baggageTravel =
      ocFees.baggageTravelIndex() > 0 &&
              ocFees.baggageTravelIndex() <= ocFees.farePath()->baggageTravels().size()
          ? ocFees.farePath()->baggageTravels()[ocFees.baggageTravelIndex() - 1]
          : nullptr;

  if (!baggageTravel)
    return;

  if (baggageTravel->_charges[0] && ocFees.optFee() == baggageTravel->_charges[0]->optFee())
  {
    construct.addAttributeChar(xml2::OosFirstDisclosureOccurrence, 'T');
  }

  if (baggageTravel->_charges[1] && ocFees.optFee() == baggageTravel->_charges[1]->optFee())
  {
    construct.addAttributeChar(xml2::OosSecondDisclosureOccurrence, 'T');
  }
}

void
AncillaryPricingResponseFormatter::buildSegments(AncillaryPricingTrx& ancTrx,
                                                 XMLConstruct& construct,
                                                 const OCFeesUsage& ocFees)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::buildSegments() - entered");
  if (ocFees.travelStart() == ocFees.travelEnd() &&
      ocFees.travelStart()->pnrSegment() != ARUNK_PNR_SEGMENT_ORDER)
  {
    if (ancTrx.showBaggageTravelIndex() && ocFees.baggageTravelIndex())
      buildQBT(construct, ocFees.travelStart()->pnrSegment());
    buildSegmentElement(construct, ocFees.travelStart()->pnrSegment());
    return;
  }

  std::vector<tse::TravelSeg*>::const_iterator segI =
      std::find_if(currentItin()->travelSeg().begin(),
                   currentItin()->travelSeg().end(),
                   FindPoint(ocFees.travelStart()));
  std::vector<tse::TravelSeg*>::const_iterator segE =
      std::find_if(currentItin()->travelSeg().begin(),
                   currentItin()->travelSeg().end(),
                   FindPoint(ocFees.travelEnd()));

  if (ancTrx.showBaggageTravelIndex() && ocFees.baggageTravelIndex())
  {
    std::vector<tse::TravelSeg*>::const_iterator segIter = segI;
    for (; segIter != currentItin()->travelSeg().end() && segIter != segE + 1; ++segIter)
      buildQBT(construct, (*segIter)->pnrSegment());
  }

  const bool isUSDotOneTimePerFlightPortionFee = currentItin()->getBaggageTripType().isUsDot() &&
                                                 ocFees.optFee() != nullptr &&
                                                 ocFees.optFee()->frequentFlyerMileageAppl() == '4';
  const bool isBaggageAllowance = ocFees.subCodeInfo()->fltTktMerchInd() == BAGGAGE_ALLOWANCE;
  const bool isBaggageCharge = ocFees.subCodeInfo()->fltTktMerchInd() == BAGGAGE_CHARGE;

  for (; segI != currentItin()->travelSeg().end() && segI != segE + 1; segI++)
  {
    if (((*segI)->pnrSegment() != ARUNK_PNR_SEGMENT_ORDER))
    {
      if ((ancTrx.activationFlags().isAB240() &&
           (isBaggageAllowance || isUSDotOneTimePerFlightPortionFee)) ||
          (!isBaggageAllowance && !isBaggageCharge) ||
          ((*segI)->checkedPortionOfTravelInd() == 'T'))
        buildSegmentElement(construct, (*segI)->pnrSegment());
    }
  }
}

void
AncillaryPricingResponseFormatter::buildQBT(XMLConstruct& construct, const int16_t& pnrSegment)
{
  if (pnrSegment != ARUNK_PNR_SEGMENT_ORDER)
  {
    char tmpBuf[10];
    sprintf(tmpBuf, "%02d", pnrSegment);
    construct.openElement(xml2::SegIdInBaggageTravel);
    construct.addElementData(tmpBuf);
    construct.closeElement();
  }
}

void
AncillaryPricingResponseFormatter::buildSegmentElement(XMLConstruct& construct,
                                                       const int16_t& pnrSegment)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::buildSegmentElement() - entered");
  if (pnrSegment != ARUNK_PNR_SEGMENT_ORDER)
  {
    char tmpBuf[10];
    sprintf(tmpBuf, "%02d", pnrSegment);
    construct.openElement(xml2::OOSSectorNumber);
    construct.addElementData(tmpBuf);
    construct.closeElement();
  }
}

void
AncillaryPricingResponseFormatter::buildS7OOSOptionalData(XMLConstruct& construct,
                                                          const OCFeesUsage& ocFees)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::buildS7OOSOptionalData() - entered");
  if (ocFees.softMatchS7Status().isSet(OCFees::S7_RBD_SOFT) && !ocFees.softMatchRBDT198().empty())
  {
    // Build RBD Information - Table 198
    buildRBDData(construct, ocFees);
  }

  if (ocFees.softMatchS7Status().isSet(OCFees::S7_RESULTING_FARE_SOFT) &&
      !ocFees.softMatchResultingFareClassT171().empty())
  {
    // Build Carrier Resulting Fare Class Information - Table 171
    buildFCLData(construct, ocFees);
  }

  if (ocFees.softMatchS7Status().isSet(OCFees::S7_CARRIER_FLIGHT_SOFT) &&
      !ocFees.softMatchCarrierFlightT186().empty())
  {
    // Build Tax Carrier Flight Information - Table 186
    buildCFTData(construct, ocFees);
  }
  // TODO in this place we have "CFT" element
}

bool
AncillaryPricingResponseFormatter::isC52ActiveForAB240(AncillaryPricingTrx& ancTrx) const
{
  return ancTrx.activationFlags().isAB240() &&
         !fallback::fallbackAddC52ForRecordsWithNoChargeAB240(&ancTrx);
}

void
AncillaryPricingResponseFormatter::buildSUMData(AncillaryPricingTrx& ancTrx,
                                                XMLConstruct& construct,
                                                const PaxOCFees& paxOcFees,
                                                OCFeesUsage& ocFees)
{
  if (!fallback::fallbackAddC52ForRecordsWithNoCharge(&ancTrx) || isC52ActiveForAB240(ancTrx))
    insertPriceDataToSum(ancTrx, construct, paxOcFees, ocFees);
  else
    buildSUMData_old_implementation(ancTrx, construct, paxOcFees, ocFees);
}

void
AncillaryPricingResponseFormatter::insertPriceDataToSum(AncillaryPricingTrx& ancTrx,
                                                        XMLConstruct& construct,
                                                        const PaxOCFees& paxOcFees,
                                                        OCFeesUsage& ocFees)
{
  tse::AncillaryPricingResponseSumDataFiller sumDataFiller(
      &ancTrx, &construct, &paxOcFees, &ocFees, this);
  sumDataFiller.fillSumElement();
}

// remove with fallbackAddC52ForRecordsWithNoCharge
void
AncillaryPricingResponseFormatter::buildSUMData_old_implementation(AncillaryPricingTrx& ancTrx,
                                                                   XMLConstruct& construct,
                                                                   const PaxOCFees& paxOcFees,
                                                                   OCFeesUsage& ocFees)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::buildSUMData() - entered");

  ServiceFeeUtil util(ancTrx);
  Money equivPrice("");
  MoneyAmount feeAmount = ocFees.feeAmount();

  const bool isFlightRelatedService =
      ocFees.subCodeInfo()->fltTktMerchInd() == FLIGHT_RELATED_SERVICE;
  const bool isPrepaidBaggage = ocFees.subCodeInfo()->fltTktMerchInd() == PREPAID_BAGGAGE;
  const bool isBaggageRequest =
      ancTrx.getRequest()->majorSchemaVersion() >= 2 &&
      ServiceFeeUtil::checkServiceGroupForAcs(ocFees.subCodeInfo()->serviceGroup());

  if (ancTrx.activationFlags().isAB240() && (isPrepaidBaggage || isFlightRelatedService))
  {
    equivPrice = util.convertOCFeeCurrency(ocFees);
  }
  else if (isBaggageRequest)
  {
    equivPrice = util.convertBaggageFeeCurrency(ocFees);
    if (ServiceFeeUtil::isFeeFarePercentage(*(ocFees.optFee())))
    {
      // first convert the currency, next round the original amount
      CurrencyRoundingUtil roundingUtil;
      roundingUtil.round(feeAmount, ocFees.feeCurrency(), ancTrx);
    }
  }
  else
  {
    equivPrice = util.convertOCFeeCurrency(ocFees);
  }

  const DateTime& ticketingDate = ancTrx.ticketingDate();

  // Build Passenger and Fare Information
  construct.openElement(xml2::SUMPsgrFareInfo); // SUM Open

  // B70 - SHF - Passenger Type Code
  construct.addAttribute(xml2::PsgrTypeCode, paxOcFees.paxType());

  CurrencyNoDec currencyNoDecForC51;

  DataHandle dataHandle;
  OCFeesPrice* ocFeesPrice = OCFeesPrice::create(ocFees, ancTrx, dataHandle);
  CurrencyCode currency;
  const MoneyAmount basePrice = ocFeesPrice->getBasePrice(ocFees, feeAmount);

  if (ocFees.feeCurrency() != "")
  {
    // C51 - SFA - Base Price
    currencyNoDecForC51 = ocFees.feeNoDec();
    construct.addAttributeDouble(xml2::SUMBasePrice, basePrice, currencyNoDecForC51);

    // C5A - SFB - Base Currency
    currency = ocFees.feeCurrency();
    construct.addAttribute(xml2::SUMBaseCurrencyCode, currency);
  }
  else // If base price currency is blank use equivalent currency code for base price decimal
  // calculation, C51 - SFA - Base Price
  {
    currencyNoDecForC51 = equivPrice.noDec(ticketingDate);
    construct.addAttributeDouble(xml2::SUMBasePrice, basePrice, currencyNoDecForC51);

    // SFC, SFH - Equivalent Price and Equivalent Currency
    // PNR needs this currency code when base price is 0
    currency = equivPrice.code();
    construct.addAttribute(xml2::SUMEquiCurCode, currency);
  }

  bool isC52avaliable = false;
  MoneyAmount equivalentBasePrice = 0.0;
  CurrencyNoDec currencyNoDecForC52 = 0;
  if (ocFees.feeCurrency() != "" && ocFees.feeCurrency() != equivPrice.code())
  {
    // C52 - SFC  - Equivalent Base Price
    equivalentBasePrice = ocFeesPrice->getEquivalentBasePrice(ocFees, equivPrice.value());
    currencyNoDecForC52 = equivPrice.noDec(ticketingDate);
    construct.addAttributeDouble(xml2::SUMEquiBasePrice, equivalentBasePrice, currencyNoDecForC52);

    // SFC, SFH - Equivalent Price and Equivalent Currency
    currency = equivPrice.code();
    construct.addAttribute(xml2::SUMEquiCurCode, currency);
    isC52avaliable = true;
  }

  // N21 - SFI Tax Indicator
  if (ocFees.optFee()->taxInclInd() != ' ')
    construct.addAttributeChar(xml2::TaxInd, ocFees.optFee()->taxInclInd());

  if (isApplyAMTaxLogic(ancTrx, ocFees))
    applyAMTaxLogic(ancTrx,
                    ocFees,
                    isC52avaliable,
                    basePrice,
                    equivalentBasePrice,
                    currencyNoDecForC51,
                    currencyNoDecForC52,
                    currency);

  // C50 - SFE - Total Price - Place Holder
  if (!isC52avaliable && isBaggageRequest)
    construct.addAttributeDouble(
        xml2::ServiceTotalPrice4OC,
        ocFeesPrice->getFeeAmountInSellingCurrencyPlusTaxes(ocFees, feeAmount),
        currencyNoDecForC51);
  else
    construct.addAttributeDouble(
        xml2::ServiceTotalPrice4OC,
        ocFeesPrice->getFeeAmountInSellingCurrencyPlusTaxes(ocFees, equivPrice),
        equivPrice.noDec(ticketingDate));
  construct.closeElement(); // SUM Close
}

void
AncillaryPricingResponseFormatter::buildSUMData(AncillaryPricingTrx& ancTrx,
                                                XMLConstruct& construct,
                                                const PaxOCFeesUsages& paxOcFees)
{
  if (!fallback::fallbackAddC52ForRecordsWithNoCharge(&ancTrx) || isC52ActiveForAB240(ancTrx))
    insertPriceDataToSum(ancTrx, construct, paxOcFees);
  else
    buildSUMData_old_implementation(ancTrx, construct, paxOcFees);
}

void
AncillaryPricingResponseFormatter::insertPriceDataToSum(AncillaryPricingTrx& ancTrx,
                                                        XMLConstruct& construct,
                                                        const PaxOCFeesUsages& paxOcFees)
{
  tse::AncillaryPricingResponseSumDataFiller sumDataFiller(&ancTrx, &construct, &paxOcFees, this);
  sumDataFiller.fillSumElement();
}

// remove with fallbackAddC52ForRecordsWithNoCharge
void
AncillaryPricingResponseFormatter::buildSUMData_old_implementation(AncillaryPricingTrx& ancTrx,
                                                                   XMLConstruct& construct,
                                                                   const PaxOCFeesUsages& paxOcFees)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::buildSUMData() - entered");

  OCFeesUsage ocFees = *(paxOcFees.fees());
  ServiceFeeUtil util(ancTrx);
  Money equivPrice("");
  MoneyAmount feeAmount = ocFees.feeAmount();

  const bool isFlightRelatedService =
      ocFees.subCodeInfo()->fltTktMerchInd() == FLIGHT_RELATED_SERVICE;
  const bool isPrepaidBaggage = ocFees.subCodeInfo()->fltTktMerchInd() == PREPAID_BAGGAGE;

  if (ancTrx.activationFlags().isAB240() && (isFlightRelatedService || isPrepaidBaggage))
  {
    equivPrice = util.convertOCFeeCurrency(ocFees);
  }
  else if (ancTrx.getRequest()->majorSchemaVersion() >= 2 &&
           ServiceFeeUtil::checkServiceGroupForAcs(ocFees.subCodeInfo()->serviceGroup()))
  {
    equivPrice = util.convertBaggageFeeCurrency(ocFees);
    if (ServiceFeeUtil::isFeeFarePercentage(*(ocFees.optFee())))
    {
      // first convert the currency, next round the original amount
      CurrencyRoundingUtil roundingUtil;
      roundingUtil.round(feeAmount, ocFees.feeCurrency(), ancTrx);
    }
  }
  else
  {
    equivPrice = util.convertOCFeeCurrency(ocFees);
  }

  const DateTime& ticketingDate = ancTrx.ticketingDate();

  // Build Passenger and Fare Information
  construct.openElement(xml2::SUMPsgrFareInfo); // SUM Open

  // B70 - SHF - Passenger Type Code
  construct.addAttribute(xml2::PsgrTypeCode, paxOcFees.paxType());

  DataHandle dataHandle;
  OCFeesPrice* ocFeesPrice = OCFeesPrice::create(ocFees, ancTrx, dataHandle);
  CurrencyCode currency;
  CurrencyNoDec currencyNoDecForC51;
  MoneyAmount basePrice = feeAmount;

  if (ocFees.feeCurrency() != "")
  {
    // C51 - SFA - Base Price
    construct.addAttributeDouble(
        xml2::SUMBasePrice, ocFeesPrice->getBasePrice(ocFees, feeAmount), ocFees.feeNoDec());

    // C5A - SFB - Base Currency
    construct.addAttribute(xml2::SUMBaseCurrencyCode, ocFees.feeCurrency());
    currency = ocFees.feeCurrency();
    currencyNoDecForC51 = ocFees.feeNoDec();
  }
  else // If base price currency is blank use equivalent currency code for base price decimal
  // calculation, C51 - SFA - Base Price
  {
    currencyNoDecForC51 = equivPrice.noDec(ticketingDate);
    construct.addAttributeDouble(
        xml2::SUMBasePrice, ocFeesPrice->getBasePrice(ocFees, feeAmount), currencyNoDecForC51);

    // SFC, SFH - Equivalent Price and Equivalent Currency
    // PNR needs this currency code when base price is 0
    construct.addAttribute(xml2::SUMEquiCurCode, equivPrice.code());
    currency = equivPrice.code();
  }

  bool isC52avaliable = false;
  MoneyAmount equivalentBasePrice = 0.0;
  CurrencyNoDec currencyNoDecForC52 = 0;

  if (ocFees.feeCurrency() != "" && ocFees.feeCurrency() != equivPrice.code())
  {
    // C52 - SFC  - Equivalent Base Price
    currencyNoDecForC52 = equivPrice.noDec(ticketingDate);
    construct.addAttributeDouble(xml2::SUMEquiBasePrice,
                                 ocFeesPrice->getEquivalentBasePrice(ocFees, equivPrice.value()),
                                 currencyNoDecForC52);

    // SFC, SFH - Equivalent Price and Equivalent Currency
    construct.addAttribute(xml2::SUMEquiCurCode, equivPrice.code());
    currency = equivPrice.code();
    equivalentBasePrice = equivPrice.value();
    isC52avaliable = true;
  }

  // N21 - SFI Tax Indicator
  if (ocFees.optFee()->taxInclInd() != ' ')
    construct.addAttributeChar(xml2::TaxInd, ocFees.optFee()->taxInclInd());

  if (isApplyAMTaxLogic(ancTrx, ocFees))
    applyAMTaxLogic(ancTrx,
                    ocFees,
                    isC52avaliable,
                    basePrice,
                    equivalentBasePrice,
                    currencyNoDecForC51,
                    currencyNoDecForC52,
                    currency);

  // C50 - SFE - Total Price - Place Holder
  construct.addAttributeDouble(
      xml2::ServiceTotalPrice4OC,
      ocFeesPrice->getFeeAmountInSellingCurrencyPlusTaxes(ocFees, equivPrice),
      equivPrice.noDec(ticketingDate));

  construct.closeElement(); // SUM Close
}

void
AncillaryPricingResponseFormatter::buildSFQData(XMLConstruct& construct, const OCFeesUsage& ocFees)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::buildSFQData() - entered");
  // Build all SFQ detailed information
  construct.openElement(xml2::FeeQualifyInfo); // SFQ Open

  // N1A - Cabin
  if (ocFees.softMatchS7Status().isSet(OCFees::S7_CABIN_SOFT) && ocFees.optFee()->cabin() != ' ')
  {
    construct.addAttributeChar(xml2::SFQCabin, ocFees.optFee()->cabin());
  }

  if (ocFees.optFee()->tourCode() != EMPTY_STRING())
  { // SHC - Tour Code
    construct.addAttribute(xml2::ServiceTourCode, ocFees.optFee()->tourCode());
  }

  if (ocFees.softMatchS7Status().isSet(OCFees::S7_RULETARIFF_SOFT) &&
      ocFees.optFee()->ruleTariff() != 0)
  {
    // Q0W - Rule Tariff
    construct.addAttributeShort(xml2::SFQRuleTariff, ocFees.optFee()->ruleTariff());
  }

  if (ocFees.softMatchS7Status().isSet(OCFees::S7_RULE_SOFT) && ocFees.optFee()->rule() != "")
  {
    // SHP - Rule
    construct.addAttribute(xml2::SFQRule, ocFees.optFee()->rule());
  }

  if (ocFees.optFee()->dayOfWeek() != "")
  { // SHQ - Day of the week
    construct.addAttribute(xml2::SFQDayOfWeek, ocFees.optFee()->dayOfWeek());
  }

  if (ocFees.softMatchS7Status().isSet(OCFees::S7_EQUIPMENT_SOFT) &&
      ocFees.optFee()->equipmentCode() != "")
  {
    // SHR - Equipment Code
    construct.addAttribute(xml2::SFQEquipmentCode, ocFees.optFee()->equipmentCode());
  }

  if (ocFees.softMatchS7Status().isSet(OCFees::S7_TIME_SOFT))
    buildSTOData(construct, ocFees);

  construct.closeElement(); // SFQ Close
}

void
AncillaryPricingResponseFormatter::buildSFQDataR7(XMLConstruct& construct,
                                                  const OCFeesUsage& ocFees,
                                                  const uint16_t index)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::buildSFQDataR7() - entered");
  // Build SFQ
  if (!ocFees.isDisplayOnly() && index && ocFees.optFee()->tourCode() != "")
  {
    construct.openElement(xml2::FeeQualifyInfo); // SFQ Open
    // SHC - Tour Code
    construct.addAttribute(xml2::ServiceTourCode, ocFees.optFee()->tourCode());
    construct.closeElement(); // SFQ Close
  }
}

void
AncillaryPricingResponseFormatter::buildSTOData(XMLConstruct& construct, const OCFeesUsage& ocFees)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::buildSTOData() - entered");
  // Build all STO detailed information
  construct.openElement(xml2::StopOverInfo); // STO Open

  if (ocFees.optFee()->stopCnxDestInd() != ' ')
  {
    // N12 - Stopover - Connection - Destination Indicator
    construct.addAttributeChar(xml2::StpOvrConxDestInd, ocFees.optFee()->stopCnxDestInd());
  }

  if (ocFees.optFee()->stopoverTime() != "")
  {
    // Q01 - Stopover Time
    construct.addAttribute(xml2::StvrTime, ocFees.optFee()->stopoverTime());
  }

  if (ocFees.optFee()->stopoverUnit() != ' ')
  {
    // N13 - Stopover Time Unit
    construct.addAttributeChar(xml2::StvrTimeUnit, ocFees.optFee()->stopoverUnit());
  }

  construct.closeElement(); // STO Close
}

void
AncillaryPricingResponseFormatter::buildDTEData(AncillaryPricingTrx& ancTrx,
                                                XMLConstruct& construct,
                                                const OCFeesUsage& ocFees,
                                                const uint16_t index)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::buildDTEData() - entered");
  // Build Date and Time Information
  construct.openElement(xml2::DateTimeInfo); // DTE Open

  // D01 - SFZ - Travel Dates Effective
  processTicketEffDate(construct, ocFees);

  // D02 - SHA - Latest Travel Date Permitted
  processTicketDiscDate(construct, ocFees);

  if (!fallback::fallbackAlways_display_purchase_by_date_for_AB240(&ancTrx) &&
      ancTrx.activationFlags().isAB240())
  {
    // D03 - SHM - Purchase By Date
    processPurchaseByDateWPDispAE(ancTrx, construct, ocFees);
  }
  else
  {
    if (_isWPDispAE || _isWpAePostTicket)
    {
      if (!ocFees.isDisplayOnly() && index)
      {
        // D03 - SHM - Purchase By Date
        processPurchaseByDateWPDispAE(ancTrx, construct, ocFees);
      }
    }
    else if (!ocFees.optFee()->advPurchPeriod().empty() || !ocFees.optFee()->advPurchUnit().empty())
    {
      processPurchaseByDate(construct, ocFees);
    }
  }

  if (ocFees.softMatchS7Status().isSet(OCFees::S7_TIME_SOFT))
  {
    // Q11 - Start Time
    construct.addAttributeInteger(xml2::StartTime, ocFees.optFee()->startTime());

    // Q12 - Stop Time
    construct.addAttributeInteger(xml2::StopTime, ocFees.optFee()->stopTime());
  }

  construct.closeElement(); // DTE Close
}

void
AncillaryPricingResponseFormatter::buildFATData(AncillaryPricingTrx& ancTrx,
                                                XMLConstruct& construct,
                                                const OCFeesUsage& ocFees,
                                                const uint16_t index)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::buildFATData() - entered");
  // Build Fee Application and Ticketing Information
  construct.openElement(xml2::FeeApplTktInfo); // FAT Open

  bool displayOnlyFlagForR7 =
      (_isWPDispAE || _isWpAePostTicket) && (ocFees.isDisplayOnly() || !index);

  if ((!displayOnlyFlagForR7 || ancTrx.activationFlags().isAB240()) &&
      ocFees.optFee()->frequentFlyerMileageAppl() != ' ')
  { // N41 - SHG - Fee Application Ind
    construct.addAttributeChar(xml2::FeeApplInd, ocFees.optFee()->frequentFlyerMileageAppl());
  }

  // P01 - SFR - Fee is not Guaranteed
  if (ocFees.softMatchS7Status().value() != 0 ||
      (!displayOnlyFlagForR7 && (!ocFees.isFeeGuaranteed() || ancillariesNonGuarantee())))
  {
    construct.addAttributeBoolean(xml2::FeeGuranInd, true);
  }

  if ((!displayOnlyFlagForR7 || ancTrx.activationFlags().isAB240()) &&
      ocFees.optFee()->advPurchTktIssue() != ' ')
  { // N42 - SFX - Simultaneous Ticket Indicator
    construct.addAttributeChar(xml2::SimulTktInd, ocFees.optFee()->advPurchTktIssue());
  }

  if (ocFees.optFee()->notAvailNoChargeInd() != ' ')
  { // N43 - SNN - No Charge or Not Available Indicator
    construct.addAttributeChar(xml2::NoChrgNotAvailInd, ocFees.optFee()->notAvailNoChargeInd());
  }

  if ((!displayOnlyFlagForR7 || ancTrx.activationFlags().isAB240()) &&
      ocFees.optFee()->formOfFeeRefundInd() != ' ')
  { // N44 - SFP - Form of Refund
    construct.addAttributeChar(xml2::FormOfRefund, ocFees.optFee()->formOfFeeRefundInd());
  }

  if ((ocFees.softMatchS7Status().isSet(OCFees::S7_FREQFLYER_SOFT) ||
       ((_isWPDispAE || _isWpAePostTicket) && (!ocFees.isDisplayOnly() || index)) ||
       checkFFStatus(ocFees)) &&
      ocFees.optFee()->frequentFlyerStatus() != 0)
  { // Q7D - SHB - FQTV carrier filed tier level
    construct.addAttributeShort(xml2::FQTVCxrFiledTierLvl, ocFees.optFee()->frequentFlyerStatus());
  }

  if ((!displayOnlyFlagForR7 || ancTrx.activationFlags().isAB240()) &&
      ocFees.optFee()->refundReissueInd() != ' ')
  { // N45 - SFL - Refund Reissue Indicator
    construct.addAttributeChar(xml2::RefundReissueInd, ocFees.optFee()->refundReissueInd());
  }

  if ((!displayOnlyFlagForR7 || ancTrx.activationFlags().isAB240()) &&
      ocFees.optFee()->commissionInd() != ' ')
  { // P03 - SFL - Commision Indicator
    construct.addAttributeChar(xml2::CommisionInd, ocFees.optFee()->commissionInd());
  }

  if ((!displayOnlyFlagForR7 || ancTrx.activationFlags().isAB240()) &&
      ocFees.optFee()->interlineInd() != ' ')
  { // P04 - SFL - Interline Indicator
    construct.addAttributeChar(xml2::InterlineInd, ocFees.optFee()->interlineInd());
  }

  if (ancTrx.showBaggageTravelIndex() && (ocFees.optFee()->fltTktMerchInd() == BAGGAGE_CHARGE ||
                                          ocFees.optFee()->fltTktMerchInd() == BAGGAGE_ALLOWANCE))
    construct.addAttributeInteger(xml2::OptionalServicesSeqNo, ocFees.optFee()->seqNo());

  //[EMD] add code for Interline Indicator here if required

  construct.closeElement(); // FAT Close
}

void
AncillaryPricingResponseFormatter::buildRBDData(XMLConstruct& construct, const OCFeesUsage& ocFees)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::buildRBDData() - entered");
  // Build RBD Information - Table 198

  std::vector<SvcFeesResBkgDesigInfo*>::const_iterator resBKGInfoIter =
      ocFees.softMatchRBDT198().begin();
  std::vector<SvcFeesResBkgDesigInfo*>::const_iterator resBKGInfoEndIter =
      ocFees.softMatchRBDT198().end();

  for (; resBKGInfoIter != resBKGInfoEndIter; resBKGInfoIter++)
  {
    construct.openElement(xml2::RBDInfo); // RBD Open
    SvcFeesResBkgDesigInfo* resBKGInfo = (*resBKGInfoIter);

    // N51 - Marketing Operating Indicator
    construct.addAttributeChar(xml2::MktOptInd, resBKGInfo->mkgOperInd());

    // B51 - RBD Carrier
    construct.addAttribute(xml2::RBDCarrier, resBKGInfo->carrier());

    // Booking Codes
    buildBKCData(construct, resBKGInfo->bookingCode1());
    buildBKCData(construct, resBKGInfo->bookingCode2());
    buildBKCData(construct, resBKGInfo->bookingCode3());
    buildBKCData(construct, resBKGInfo->bookingCode4());
    buildBKCData(construct, resBKGInfo->bookingCode5());
    construct.closeElement(); // RBD Close
  }
}

void
AncillaryPricingResponseFormatter::buildPadisData(AncillaryPricingTrx& ancTrx,
                                                  XMLConstruct& construct,
                                                  const OCFeesUsage& ocFees)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::buildPadisData() - entered");

  construct.openElement(xml2::PspPadisInfo);

  const std::vector<SvcFeesResBkgDesigInfo*>& padisData = ocFees.padisData();
  for (const SvcFeesResBkgDesigInfo* padis : padisData)
    buildUpc(ancTrx, construct, *padis, ocFees);

  construct.closeElement();
}

void
AncillaryPricingResponseFormatter::buildUpc(AncillaryPricingTrx& ancTrx,
                                            XMLConstruct& construct,
                                            const SvcFeesResBkgDesigInfo& padis,
                                            const OCFeesUsage& ocFeesUsage)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::buildUpc() - entered");

  construct.openElement(xml2::UpcPadisInfo);
  construct.addAttributeInteger(xml2::UpcPadisSequence, padis.seqNo());

  std::set<BookingCode> padisCodeSet;
  ServiceFeeUtil::fill(padisCodeSet, &padis);

  std::map<BookingCode, std::string> padisCodeAbbreviationMap;
  std::map<BookingCode, std::string> padisCodeDescriptionMap;

  const FarePath* farePath = ocFeesUsage.farePath();

  if (farePath && farePath->itin())
  {
    ServiceFeeUtil::getTranslatedPadisDescription(ancTrx,
                                                  ocFeesUsage.carrierCode(),
                                                  farePath->itin()->travelDate(),
                                                  padis,
                                                  padisCodeDescriptionMap,
                                                  padisCodeAbbreviationMap);
  }

  for (const BookingCode padisCode : padisCodeSet)
    buildPds(construct, padisCode, padisCodeAbbreviationMap, padisCodeDescriptionMap);

  construct.closeElement();
}

void
AncillaryPricingResponseFormatter::buildPds(
    XMLConstruct& construct,
    const BookingCode& padisCode,
    std::map<BookingCode, std::string>& padisCodeAbbreviationMap,
    std::map<BookingCode, std::string>& padisCodeDescriptionMap)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::buildPdc() - entered");

  construct.openElement(xml2::PdsPadisItem); // PDS Open
  construct.addAttribute(xml2::PdsPadisCode, padisCode);
  const std::string padisCodeAbbreviation = getPadisCodeString(padisCode, padisCodeAbbreviationMap);
  construct.addAttribute(xml2::PdsPadisAbbreviation, padisCodeAbbreviation);
  const std::string padisCodeDescription = getPadisCodeString(padisCode, padisCodeDescriptionMap);
  construct.addAttribute(xml2::PdsPadisDescription, padisCodeDescription);
  construct.closeElement(); // PDS Close
}

std::string
AncillaryPricingResponseFormatter::getPadisCodeString(
    const BookingCode& padisCode, std::map<BookingCode, std::string>& padisCodeMap)
{
  return padisCodeMap.find(padisCode) == padisCodeMap.end() ? "" : padisCodeMap[padisCode];
}

void
AncillaryPricingResponseFormatter::buildBKCData(XMLConstruct& construct, const BookingCode& bkCode)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::buildBKCData() - entered");
  if (bkCode.empty())
    return;

  construct.openElement(xml2::BookingCodeType); // BKC Open
  construct.addElementData(bkCode.c_str());
  construct.closeElement(); // BKC End
}

void
AncillaryPricingResponseFormatter::buildFCLData(XMLConstruct& construct, const OCFeesUsage& ocFees)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::buildFCLData() - entered");
  // Build Carrier Resulting Fare Class Information - Table 171

  std::vector<SvcFeesCxrResultingFCLInfo*>::const_iterator cxrRsltFCLIter =
      ocFees.softMatchResultingFareClassT171().begin();
  std::vector<SvcFeesCxrResultingFCLInfo*>::const_iterator cxrRsltFCLEndIter =
      ocFees.softMatchResultingFareClassT171().end();

  for (; cxrRsltFCLIter != cxrRsltFCLEndIter; cxrRsltFCLIter++)
  {
    construct.openElement(xml2::CarrierFareClassInfo); // FCL Open
    SvcFeesCxrResultingFCLInfo* cxrRsltFCL = (*cxrRsltFCLIter);

    // B61 - FCL Carrier
    construct.addAttribute(xml2::FCLCarrier, cxrRsltFCL->carrier());

    // BJ0 - Resulting Fare Class or Ticket Designator
    construct.addAttribute(xml2::ResultFCLOrTKTDesig.c_str(), cxrRsltFCL->resultingFCL().c_str());

    // S53 - FCL Fare Type
    construct.addAttribute(xml2::FCLFareType, cxrRsltFCL->fareType());
    construct.closeElement(); // FCL Close
  }
}

void
AncillaryPricingResponseFormatter::buildCFTData(XMLConstruct& construct, const OCFeesUsage& ocFees)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::buildCFTData() - entered");
  // Build Tax Carrier Flight Information - Table 186

  std::vector<CarrierFlightSeg*>::const_iterator cxrFlightIter =
      ocFees.softMatchCarrierFlightT186().begin();
  std::vector<CarrierFlightSeg*>::const_iterator cxrFlightEndIter =
      ocFees.softMatchCarrierFlightT186().end();

  for (; cxrFlightIter != cxrFlightEndIter; cxrFlightIter++)
  {
    construct.openElement(xml2::CarrierFlightInfo); // CFT Open

    CarrierFlightSeg* cfs = (*cxrFlightIter);

    // B71 - Marketing Carrier
    construct.addAttribute(xml2::MKtCarrier, cfs->marketingCarrier());

    // B72 - Operating Carrier
    construct.addAttribute(xml2::OprtCarrier, cfs->operatingCarrier());

    if (cfs->flt1() != -1)
    {
      // Q0B - Flight Number 1
      construct.addAttributeShort(xml2::FlightNum1, cfs->flt1());
    }

    if (cfs->flt2() != 0)
    {
      // Q0C - Flight Number 2
      construct.addAttributeShort(xml2::FlightNum2, cfs->flt2());
    }
    construct.closeElement(); // CFT Close
  }
}

void
AncillaryPricingResponseFormatter::buildBGD(XMLConstruct& construct, const OCFeesUsage& ocFees)
{
  construct.openElement(xml2::BaggageBGD); // BGD Open
  construct.addAttributeShort(xml2::BaggageOC1, ocFees.optFee()->baggageOccurrenceFirstPc());
  construct.addAttributeShort(xml2::BaggageOC2, ocFees.optFee()->baggageOccurrenceLastPc());

  if (ocFees.optFee()->baggageWeight() != -1 && (ocFees.optFee()->baggageWeightUnit()) != BLANK)
  {
    construct.addAttributeShort(xml2::BaggageSizeWeightLimit, ocFees.optFee()->baggageWeight());
    construct.addAttributeChar(xml2::BaggageSizeWeightUnitType,
                               convertWeightUnit(ocFees.optFee()->baggageWeightUnit()));
  }

  construct.closeElement();
}

void
AncillaryPricingResponseFormatter::buildBGAData(AncillaryPricingTrx& ancTrx,
                                                XMLConstruct& construct,
                                                const OCFeesUsage& ocFees)
{
  construct.openElement(xml2::BaggageBGA); // BGA Open

  if (ocFees.optFee()->baggageWeight() != -1 && (ocFees.optFee()->baggageWeightUnit()) != BLANK)
  {
    construct.addAttributeShort(xml2::BaggageSizeWeightLimit, ocFees.optFee()->baggageWeight());
    construct.addAttributeChar(xml2::BaggageSizeWeightUnitType,
                               convertWeightUnit(ocFees.optFee()->baggageWeightUnit()));
  }

  if (ocFees.optFee()->freeBaggagePcs() != NO_BAGGAGE_PCS)
    construct.addAttributeShort(xml2::BaggageBPC, ocFees.optFee()->freeBaggagePcs());

  if (ocFees.optFee()->freeBaggagePcs() != NO_BAGGAGE_PCS)
  {
    for (const OCFees::BaggageItemProperty& baggageItemProperty : ocFees.getBaggageProperty())
      if (!baggageItemProperty.isFreeText())
        buildITRData(ancTrx, construct, baggageItemProperty);

    for (const OCFees::BaggageItemProperty& baggageItemProperty : ocFees.getBaggageProperty())
      if (baggageItemProperty.isFreeText())
        prepareMessage(construct, Message::TYPE_GENERAL, 0, baggageItemProperty.getFreeText());
  }

  construct.closeElement(); // BGA Close
}

void
AncillaryPricingResponseFormatter::buildITRData(
    AncillaryPricingTrx& ancTrx,
    XMLConstruct& construct,
    const OCFees::BaggageItemProperty& baggageItemProperty)
{
  construct.openElement(xml2::BaggageITR);
  construct.addAttributeInteger(xml2::GenId, baggageItemProperty.getNumber());
  buildITTData(ancTrx, construct, baggageItemProperty.getSubCode());
  construct.closeElement();
}

void
AncillaryPricingResponseFormatter::buildOSCCommonData(XMLConstruct& construct,
                                                      const SubCodeInfo* subCodeInfo)
{
  // SHK - RFIC Subcode
  construct.addAttribute(xml2::ServiceRFICSubCode, subCodeInfo->serviceSubTypeCode());

  // SFF - Commercial name
  construct.addAttribute(xml2::ServiceCommercialName, subCodeInfo->commercialName());

  // SFV - Vendor Code
  construct.addAttribute(xml2::ServiceVendorCode, subCodeInfo->vendor());

  // B01 - SFK - Carrier Code
  construct.addAttribute(xml2::S5CarrierCode, subCodeInfo->carrier());
}

// @@@ It's for the OCFeesUsages (both methods should stay, the old one is for baggage)
void
AncillaryPricingResponseFormatter::buildOSCCommonData(XMLConstruct& construct,
                                                      const OCFeesUsage& ocFees)
{
  // SHK - RFIC Subcode
  construct.addAttribute(xml2::ServiceRFICSubCode, ocFees.subCodeInfo()->serviceSubTypeCode());

  // SFF - Commercial name
  construct.addAttribute(xml2::ServiceCommercialName, getCommercialName(&ocFees));

  // SFV - Vendor Code
  construct.addAttribute(xml2::ServiceVendorCode, ocFees.subCodeInfo()->vendor());

  // B01 - SFK - Carrier Code
  construct.addAttribute(xml2::S5CarrierCode, ocFees.subCodeInfo()->carrier());
}
//

void
AncillaryPricingResponseFormatter::buildITTData(AncillaryPricingTrx& ancTrx,
                                                XMLConstruct& construct,
                                                const SubCodeInfo* subCodeInfo)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::buildITTData() - entered");

  construct.openElement(xml2::BaggageITT); // OSC Open

  buildOSCCommonData(construct, subCodeInfo);
  buildOSCCommonOptionalData(ancTrx, construct, subCodeInfo);

  construct.closeElement(); // OSC close
}

void
AncillaryPricingResponseFormatter::buildOSCCommonOptionalData(AncillaryPricingTrx& ancTrx,
                                                              XMLConstruct& construct,
                                                              const SubCodeInfo* subCodeInfo,
                                                              bool addSchema2Elements)
{
  LOG4CXX_DEBUG(_logger,
                "AncillaryPricingResponseFormatter::buildOSCCommonOptionalData() - entered");

  if (subCodeInfo->rfiCode() != BLANK)
  { // N01 - SFG - RFIC
    construct.addAttributeChar(xml2::S5RFICCode, subCodeInfo->rfiCode());
  }

  if (subCodeInfo->ssrCode() != EMPTY_STRING())
  { // SHL - SSR code
    construct.addAttribute(xml2::ServiceSSRCode, subCodeInfo->ssrCode());
  }

  // N02 - SFJ - EMD Type
  construct.addAttributeChar(xml2::S5EMDType, subCodeInfo->emdType());

  if (subCodeInfo->bookingInd() != EMPTY_STRING())
  { // SFN - Booking Ind
    construct.addAttribute(xml2::ServiceBooking, subCodeInfo->bookingInd());
  }

  if (subCodeInfo->ssimCode() != BLANK)
  { // N03 - SFW - SSIM Code
    construct.addAttributeChar(xml2::S5SSIMCode, subCodeInfo->ssimCode());
  }

  AncRequest* req = dynamic_cast<AncRequest*>(ancTrx.getRequest());

  if (TrxUtil::isEMDSPhase2Activated(ancTrx) && req &&
      req->ancRequestType() == AncRequest::M70Request)
  {
    // PR0 - Consumption at issuance indicator
    if (subCodeInfo->consumptionInd() != BLANK)
    {
      construct.addAttributeChar(xml2::S5Consumption, subCodeInfo->consumptionInd());
    }
  }

  if (!ancTrx.activationFlags().isAB240())
  {
    if (_isWPDispAE || _isWpAePostTicket)
      construct.addAttributeChar(xml2::ServiceDisplayOnly, 'N');
  }

  if (subCodeInfo->serviceSubGroup() != EMPTY_STRING())
  {
    construct.addAttribute(xml2::AncillaryServiceSubGroup, subCodeInfo->serviceSubGroup());
  }

  const ServicesSubGroup* servicesSubGroup = ancTrx.dataHandle().getServicesSubGroup(
      subCodeInfo->serviceGroup(), subCodeInfo->serviceSubGroup());

  if (servicesSubGroup != nullptr && servicesSubGroup->definition() != EMPTY_STRING())
  {
    construct.addAttribute(xml2::AncillaryServiceSubGroupDescription,
                           servicesSubGroup->definition());
  }

  if (subCodeInfo->fltTktMerchInd() != BLANK)
  {
    construct.addAttributeChar(xml2::AncillaryServiceType, subCodeInfo->fltTktMerchInd());
  }

  if (addSchema2Elements)
  {
    buildBIRData(ancTrx, construct, subCodeInfo);
    buildSDCData(ancTrx, construct, subCodeInfo);
    buildMSGData(ancTrx, construct, subCodeInfo);
  }
}

void
AncillaryPricingResponseFormatter::buildBIRData(AncillaryPricingTrx& ancTrx,
                                                XMLConstruct& construct,
                                                const SubCodeInfo* subCode)
{
  if (subCode)
  {
    if (!subCode->description1().empty())
    {
      buildBIRData(construct, subCode->description1());

      if (!subCode->description2().empty())
      {
        buildBIRData(construct, subCode->description2());
      }
    }
  }
}

void
AncillaryPricingResponseFormatter::buildBIRData(XMLConstruct& construct,
                                                const ServiceGroupDescription& description)
{
  for (BaggageSizeWeightDescription& baggageSizeWeight : _baggageSizeWeightRestrictions)
    if (baggageSizeWeight._serviceDescription == description)
      buildBIRData(construct, baggageSizeWeight);
}

void
AncillaryPricingResponseFormatter::buildBIRData(
    XMLConstruct& construct,
    const AncillaryPricingResponseFormatter::BaggageSizeWeightDescription& sizeWeightDescription)
{
  construct.openElement(xml2::BaggageBIR);

  if (sizeWeightDescription._baggageSizeWeightLimit != NO_BAGGAGE_SIZE_WEIGHT &&
      sizeWeightDescription._baggageSizeWeightUnitType != 0 &&
      sizeWeightDescription._baggageSizeWeightLimitType != 0)
  {
    std::ostringstream ss;
    ss << sizeWeightDescription._baggageSizeWeightLimit;
    construct.addAttribute(xml2::BaggageSizeWeightLimit, ss.str());
    construct.addAttributeChar(xml2::BaggageSizeWeightUnitType,
                               sizeWeightDescription._baggageSizeWeightUnitType);
    construct.addAttributeChar(xml2::BaggageSizeWeightLimitType,
                               sizeWeightDescription._baggageSizeWeightLimitType);
  }
  construct.closeElement();
}

void
AncillaryPricingResponseFormatter::buildSDCData(AncillaryPricingTrx& ancTrx,
                                                XMLConstruct& construct,
                                                const SubCodeInfo* subCode)
{
  if (subCode)
  {
    if (!subCode->description1().empty())
    {
      const ServicesDescription* description1 =
          ancTrx.dataHandle().getServicesDescription(subCode->description1());
      if (description1)
      {
        construct.openElement(xml2::BaggageSDC);
        construct.addAttribute(xml2::ServiceDescriptionDC1, subCode->description1());
        construct.addAttribute(xml2::ServiceDescriptionD00, description1->description());
        construct.closeElement();

        if (!subCode->description2().empty())
        {
          const ServicesDescription* description2 =
              ancTrx.dataHandle().getServicesDescription(subCode->description2());
          if (description2)
          {
            construct.openElement(xml2::BaggageSDC);
            construct.addAttribute(xml2::ServiceDescriptionDC1, subCode->description2());
            construct.addAttribute(xml2::ServiceDescriptionD00, description2->description());
            construct.closeElement();
          }
        }
      }
    }
  }
}

void
AncillaryPricingResponseFormatter::buildMSGData(AncillaryPricingTrx& ancTrx,
                                                XMLConstruct& construct,
                                                const SubCodeInfo* subCode)
{
  if (subCode)
  {
    const TaxText* taxText =
        ancTrx.dataHandle().getTaxText(subCode->vendor(), subCode->taxTextTblItemNo());
    if (taxText)
    {
      boost::regex expression("^//(\\d{2})/(\\w{3})$");
      boost::cmatch what;
      for (std::string txtMsg : taxText->txtMsgs())
        if (!boost::regex_match(txtMsg.c_str(), what, expression))
          prepareMessage(construct, Message::TYPE_GENERAL, 0, txtMsg);
    }
  }
}

void
AncillaryPricingResponseFormatter::processTicketEffDate(XMLConstruct& construct,
                                                        const OCFeesUsage& ocFees)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::processTicketEffDate() - entered");
  // D01 - SFZ - Travel Dates Effective
  std::ostringstream temp;
  std::ostringstream effYear;
  std::ostringstream effMonth;
  std::ostringstream effDay;

  if (ServiceFeeUtil::isStartDateSpecified(*(ocFees.optFee())))
  {
    temp << std::setw(4) << std::setfill('0') << ocFees.optFee()->tvlStartYear() << "-"
         << std::setw(2) << std::setfill('0') << ocFees.optFee()->tvlStartMonth() << "-"
         << std::setw(2) << std::setfill('0') << ocFees.optFee()->tvlStartDay();
  }
  else if (ocFees.optFee()->ticketEffDate().isValid())
  {
    temp << ocFees.optFee()->ticketEffDate().dateToSqlString();
  }
  else
  {
    effYear << ocFees.optFee()->ticketEffDate().year();
    effMonth << std::setw(2) << std::setfill('0')
             << ((unsigned short)ocFees.optFee()->ticketEffDate().date().month());
    effDay << std::setw(2) << std::setfill('0') << ocFees.optFee()->ticketEffDate().date().day();

    temp << effYear.str() << "-" << effMonth.str() << "-" << effDay.str();
  }
  construct.addAttribute(xml2::TvlDtEffDt, temp.str());
}

void
AncillaryPricingResponseFormatter::processTicketEffDateForR7(XMLConstruct& construct,
                                                             const OCFeesUsage& ocFees)
{
  LOG4CXX_DEBUG(_logger,
                "AncillaryPricingResponseFormatter::processTicketEffDateForR7() - entered");
  // D01 - SFZ - Travel Dates Effective For R7
  std::ostringstream temp;
  std::ostringstream effYear;
  std::ostringstream effMonth;
  std::ostringstream effDay;

  if (ocFees.optFee()->ticketEffDate().isValid() || ocFees.optFee()->ticketEffDate().isEmptyDate())
  {
    effYear << ocFees.optFee()->ticketEffDate().year();
    effMonth << std::setw(2) << std::setfill('0')
             << ((unsigned short)ocFees.optFee()->ticketEffDate().date().month());
    effDay << std::setw(2) << std::setfill('0') << ocFees.optFee()->ticketEffDate().date().day();

    temp << effYear.str().substr(2, 2) << "/" << effMonth.str() << "/" << effDay.str();
  }
  else
  {
    effYear << std::setw(2) << std::setfill('0') << ocFees.optFee()->tvlStartYear();
    effMonth << std::setw(2) << std::setfill('0') << ocFees.optFee()->tvlStartMonth();
    effDay << std::setw(2) << std::setfill('0') << ocFees.optFee()->tvlStartDay();

    temp << effYear.str() << "/" << effMonth.str() << "/" << effDay.str();
  }
  construct.addAttribute(xml2::TvlDtEffDt, temp.str());
}

void
AncillaryPricingResponseFormatter::processTicketDiscDate(XMLConstruct& construct,
                                                         const OCFeesUsage& ocFees)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::processTicketDiscDate() - entered");
  //  D02 - SHA - Travel Dates Discontinue
  std::ostringstream temp;

  if (ocFees.optFee()->ticketDiscDate().isValid())
  {
    temp << ocFees.optFee()->ticketDiscDate().dateToSqlString();
  }
  else if (ocFees.optFee()->ticketDiscDate().isInfinity())
  {
    temp << "9999-12-31";
  }
  else
    temp << std::setw(4) << std::setfill('0') << ocFees.optFee()->tvlStopYear() << "-"
         << std::setw(2) << std::setfill('0') << ocFees.optFee()->tvlStopMonth() << "-"
         << std::setw(2) << std::setfill('0') << ocFees.optFee()->tvlStopDay();

  construct.addAttribute(xml2::LtstTvlDtPermt, temp.str());
}

void
AncillaryPricingResponseFormatter::processTicketDiscDateForR7(XMLConstruct& construct,
                                                              const OCFeesUsage& ocFees)
{
  LOG4CXX_DEBUG(_logger,
                "AncillaryPricingResponseFormatter::processTicketDiscDateForR7() - entered");
  //  D02 - SHA - Travel Dates Discontinue For R7
  std::ostringstream temp;
  std::ostringstream discYear;
  std::ostringstream discMonth;
  std::ostringstream discDay;

  if (ocFees.optFee()->ticketDiscDate().isValid())
  {
    discYear << ocFees.optFee()->ticketDiscDate().year();
    discMonth << std::setw(2) << std::setfill('0')
              << ((unsigned short)ocFees.optFee()->ticketDiscDate().date().month());
    discDay << std::setw(2) << std::setfill('0') << ocFees.optFee()->ticketDiscDate().date().day();

    temp << discYear.str().substr(2, 2) << "/" << discMonth.str() << "/" << discDay.str();
  }
  else if (ocFees.optFee()->ticketDiscDate().isInfinity())
  {
    temp << "99/12/31";
  }
  else
    temp << std::setw(2) << std::setfill('0') << ocFees.optFee()->tvlStopYear() << "/"
         << std::setw(2) << std::setfill('0') << ocFees.optFee()->tvlStopMonth() << "/"
         << std::setw(2) << std::setfill('0') << ocFees.optFee()->tvlStopDay();

  construct.addAttribute(xml2::LtstTvlDtPermt, temp.str());
}

void
AncillaryPricingResponseFormatter::processPurchaseByDate(XMLConstruct& construct,
                                                         const OCFeesUsage& ocFees)
{
  LOG4CXX_DEBUG(_logger, "AncillaryPricingResponseFormatter::processPurchaseByDate() - entered");

  std::ostringstream temp;
  if (ocFees.purchaseByDate().isValid())
    temp << ocFees.purchaseByDate().dateToSqlString();
  else
    temp << ocFees.purchaseByDate().date();

  construct.addAttribute(xml2::PurByDate, temp.str());
}

void
AncillaryPricingResponseFormatter::processPurchaseByDateWPDispAE(AncillaryPricingTrx& ancTrx,
                                                                 XMLConstruct& construct,
                                                                 const OCFeesUsage& ocFees)
{
  LOG4CXX_DEBUG(_logger,
                "AncillaryPricingResponseFormatter::processPurchaseByDateWPDispAE() - entered");
  DateTime time = calculatePurchaseByDate(ancTrx);
  construct.addAttribute(xml2::PurByDate, time.dateToString(YYYYMMDD, "-").c_str());
}

bool
AncillaryPricingResponseFormatter::allowanceServiceType(const SubCodeInfo* codeInfo) const
{
  return (codeInfo->serviceSubTypeCode().equalToConst("0DF") &&
          codeInfo->fltTktMerchInd() == BAGGAGE_ALLOWANCE);
}

bool
AncillaryPricingResponseFormatter::checkFFStatus(const OCFeesUsage& ocFees)
{
  return (ocFees.optFee()->serviceSubTypeCode().equalToConst("0DG"));
}

Indicator
AncillaryPricingResponseFormatter::convertWeightUnit(const Indicator& unit) const
{
  return (unit == 'K') ? unit : 'L';
}

bool
AncillaryPricingResponseFormatter::isOriginalOrderNeeded(const ServiceFeesGroup* sfg,
                                                         const AncillaryPricingTrx& ancTrx) const
{
  return FreeBaggageUtil::isSortNeeded(&ancTrx) && sfg->groupCode().equalToConst("BG");
}

bool
AncillaryPricingResponseFormatter::isApplyAMTaxLogic(AncillaryPricingTrx& ancTrx,
                                                     const OCFeesUsage& ocFees) const
{
  if (ocFees.subCodeInfo()->fltTktMerchInd() == BAGGAGE_CHARGE &&
      !fallback::fallbackAMChargesTaxes(&ancTrx) && ancTrx.activationFlags().isAB240() &&
      ancTrx.billing() && ancTrx.billing()->partitionID() == CARRIER_AM &&
      ocFees.optFee()->taxInclInd() == ' ')
    return true;

  return false;
}

void
AncillaryPricingResponseFormatter::getAmount(bool isC52avaliable,
                                             MoneyAmount basePrice,
                                             MoneyAmount equivalentBasePrice,
                                             CurrencyNoDec currencyNoDecForC51,
                                             CurrencyNoDec currencyNoDecForC52,
                                             MoneyAmount& actualBasePrice,
                                             uint16_t& numberOfDec)
{
  if (isC52avaliable)
  {
    actualBasePrice = equivalentBasePrice;
    numberOfDec = currencyNoDecForC52;
  }
  else
  {
    actualBasePrice = basePrice;
    numberOfDec = currencyNoDecForC51;
  }
}

void
AncillaryPricingResponseFormatter::applyAMTaxLogic(AncillaryPricingTrx& ancTrx,
                                                   OCFeesUsage& ocFees,
                                                   bool isC52avaliable,
                                                   MoneyAmount basePrice,
                                                   MoneyAmount equivalentBasePrice,
                                                   CurrencyNoDec currencyNoDecForC51,
                                                   CurrencyNoDec currencyNoDecForC52,
                                                   const CurrencyCode& currency)
{
  TSE_ASSERT(!_nation.empty());

  MoneyAmount actualBasePrice = 0.0;
  uint16_t numberOfDec = 0;
  getAmount(isC52avaliable,
            basePrice,
            equivalentBasePrice,
            currencyNoDecForC51,
            currencyNoDecForC52,
            actualBasePrice,
            numberOfDec);

  if (!ancTrx.getAmVatTaxRatesOnCharges())
    ancTrx.loadAmVatTaxRatesOnCharges();

  TSE_ASSERT(ancTrx.getAmVatTaxRatesOnCharges());

  const AmVatTaxRatesOnCharges::AmVatTaxRate* amVatTaxRatePtr =
      ancTrx.getAmVatTaxRatesOnCharges()->getAmVatTaxRate(_nation);
  if (!amVatTaxRatePtr)
  {
    LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__ << ": AM Taxes not applicable for POS: " << _nation);
    return;
  }

  OCFees::TaxItem taxItem;
  taxItem.setTaxCode(amVatTaxRatePtr->getTaxCode());

  MoneyAmount taxAmount = (actualBasePrice * amVatTaxRatePtr->getTaxRate()) / 100;

  Money money(taxAmount, currency);
  roundOBFeeCurrency(ancTrx, money);

  taxItem.setTaxAmount(money.value());

  taxItem.setNumberOfDec(numberOfDec);
  ocFees.oCFees()->addTax(taxItem, ocFees.getSegIndex());
}

} // namespace
