//----------------------------------------------------------------------------
//
//  File:  BrandingServiceCaller.cpp
//  Description: See BrandingServiceCaller.h file
//  Created:     FEB 24, 2008
//  Authors:     Mauricio Dantas
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

#include "BrandingService/BrandingServiceCaller.h"

#include "Common/BrandingUtil.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/ElapseTimeWatch.h"
#include "Common/ErrorResponseException.h"
#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "Common/TSELatencyData.h"
#include "Common/TseSrvStats.h"
#include "Common/XMLConstruct.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PricingTrx.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag891Collector.h"
#include "Diagnostic/Diagnostic.h"
#include "Xform/CustomXMLParser/IXMLUtils.h"

#define S8_DIAG_REQUEST_HEADER "S8BRAND XML REQUEST"
#define S8_DIAG_RESPONSE_HEADER "S8BRAND XML RESPONSE"
#define CBAS_DIAG_REQUEST_HEADER "ABCC XML REQUEST"
#define CBAS_DIAG_RESPONSE_HEADER "ABCC XML RESPONSE"

namespace tse
{
FALLBACK_DECL(fallbackDebugOverrideBrandingServiceResponses);

namespace
{
class CurlInitializer
{
public:
  CurlInitializer() { curl_global_init(CURL_GLOBAL_ALL); }

  ~CurlInitializer() { curl_global_cleanup(); }
};

static CurlInitializer&
getBSCaller()
{
  static CurlInitializer caller;
  return caller;
}
ConfigurableValue<std::string>
s8URL("S8_BRAND_SVC", "URL");
ConfigurableValue<int16_t>
s8BrandSvcTimeout("S8_BRAND_SVC", "TIMEOUT", 1);
ConfigurableValue<std::string>
brandURL("ABCC_SVC", "BRAND_URL");
ConfigurableValue<int16_t>
brandSvcTimeout("ABCC_SVC", "BRAND_SVC_TIMEOUT");
}

static Logger
logger("atseintl.FareDisplay.BrandingServiceCaller");

template <BrandDataSource source>
BrandingServiceCaller<source>::BrandingServiceCaller(PricingTrx& trx)
  : _trx(trx)
{
  getBSCaller();
}

template <BrandDataSource source>
bool
BrandingServiceCaller<source>::callBranding(std::string& request, std::string& response)
{
  ElapseTimeWatch etw;
  etw.start();

  LOG4CXX_DEBUG(logger, "Request= " << request);

  char curlErrorBuffer[CURL_ERROR_SIZE + 1];
  curlErrorBuffer[CURL_ERROR_SIZE] = 0;

  int16_t svcTimeoutInSeconds =
      (source == S8) ? s8BrandSvcTimeout.getValue() : brandSvcTimeout.getValue();
  std::string brandingUrl = (source == S8) ? s8URL.getValue() : brandURL.getValue();
  const std::string xrayLlsBrandName = (source == S8) ? "S8BRAND" : "CBASBRAND";

  if (brandingUrl.empty())
    return false;

  CURL* handle = curl_easy_init();

  if (!handle)
  {
    etw.stop();

    if (source == S8)
      TseSrvStats::recordS8BRANDCall(false, 0, 0, etw.elapsedTime(), _trx);
    else
      TseSrvStats::recordABCCCall(false, 0, 0, etw.elapsedTime(), _trx);

    return false;
  }

  if (curl_easy_setopt(handle, CURLOPT_ERRORBUFFER, curlErrorBuffer) != CURLE_OK)
    return cleanCurl(handle);

  if (curl_easy_setopt(handle, CURLOPT_VERBOSE, 0) != CURLE_OK)
    return cleanCurl(handle);

  if (curl_easy_setopt(handle, CURLOPT_HEADER, 0) != CURLE_OK)
    return cleanCurl(handle);

  if (curl_easy_setopt(handle, CURLOPT_TIMEOUT, svcTimeoutInSeconds) != CURLE_OK)
    return cleanCurl(handle);

  if (curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 1) != CURLE_OK)
    return cleanCurl(handle);

  if (curl_easy_setopt(handle, CURLOPT_NOSIGNAL, 1) != CURLE_OK)
    return cleanCurl(handle);

  if (curl_easy_setopt(handle, CURLOPT_POSTFIELDS, request.c_str()) != CURLE_OK)
    return cleanCurl(handle);

  if (curl_easy_setopt(handle, CURLOPT_URL, brandingUrl.c_str()) != CURLE_OK)
    return cleanCurl(handle);

  if (curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, &BrandingServiceCaller::writeData) !=
      CURLE_OK)
    return cleanCurl(handle);

  if (curl_easy_setopt(handle, CURLOPT_WRITEDATA, this) != CURLE_OK)
  {
    etw.stop();

    if (source == S8)
      TseSrvStats::recordS8BRANDCall(false, request.size(), 0, etw.elapsedTime(), _trx);
    else
      TseSrvStats::recordABCCCall(false, request.size(), 0, etw.elapsedTime(), _trx);

    return cleanCurl(handle);
  }

  CURLcode curlCode;
  {
    TSELatencyData metrics(_trx, (source == S8) ? "S8 BRAND QUERY" : "CBAS BRAND QUERY");
    // This function will add the request XML data send it to Brand service and get the response
    // back.
    curlCode = curl_easy_perform(handle);
  }

  if (curlCode != CURLE_OK)
  {
    etw.stop();

    if (source == S8)
      TseSrvStats::recordS8BRANDCall(
          false, request.size(), response.size(), etw.elapsedTime(), _trx);
    else
      TseSrvStats::recordABCCCall(false, request.size(), response.size(), etw.elapsedTime(), _trx);

    _status = StatusBrandingService::BS_UNAVAILABLE;
    printDiagnostic(request, response, _status);

    diagReportError(svcTimeoutInSeconds, brandingUrl, curlCode, curlErrorBuffer);
    curl_easy_cleanup(handle);
    return false;
  }

  curl_easy_cleanup(handle);

  response = _response;
  etw.stop();

  if (const auto jsonMessage = _trx.getXrayJsonMessage())
    jsonMessage->setLlsNetworkTime(xrayLlsBrandName,
                                   etw.elapsedTime() * xray::MILLISECONDS_IN_SECOND);

  if (source == S8)
    TseSrvStats::recordS8BRANDCall(true, request.size(), response.size(), etw.elapsedTime(), _trx);
  else
    TseSrvStats::recordABCCCall(true, request.size(), response.size(), etw.elapsedTime(), _trx);

  LOG4CXX_DEBUG(logger, "Response=" << response.c_str());

  if (source == S8)
    IXMLUtils::stripnamespaces(response);

  printDiagnostic(request, response, _status);
  LOG4CXX_INFO(logger, " Leaving BrandingService.BrandingServiceCaller ");

  return true;
}

template <BrandDataSource source>
size_t
BrandingServiceCaller<source>::writeData(void* buffer, size_t size, size_t nmemb, void* userp)
{
  size_t chunkSize = size * nmemb;
  BrandingServiceCaller* pBsc = (BrandingServiceCaller*)userp;
  pBsc->_response.append((char*)buffer, chunkSize);
  return chunkSize;
}

template <BrandDataSource source>
void
BrandingServiceCaller<source>::diagReportError(int16_t& svcTimeout,
                                               std::string& brandingUrl,
                                               CURLcode curlCode,
                                               char curlErrorBuffer[])
{
  LOG4CXX_ERROR(logger,
                "BRANDING SERVICE ERROR: \n"
                    << "CURL communication error: " << curlErrorBuffer
                    << " when trying to call service at: " << brandingUrl << " with timeout of "
                    << svcTimeout << " curl code: " << curlCode);
}

template <BrandDataSource source>
bool
BrandingServiceCaller<source>::displayXML(const std::string& xml,
                                          const std::string& diagHeader,
                                          StatusBrandingService status)
{
  if (!(_trx.getTrxType() == PricingTrx::PRICING_TRX || _trx.getTrxType() == PricingTrx::MIP_TRX ||
        _trx.getTrxType() == PricingTrx::IS_TRX))
    return true;

  if (_trx.diagnostic().diagnosticType() == Diagnostic890)
  {
    if (!createDiag890())
      return true;
  }
  else if (_trx.diagnostic().diagnosticType() == Diagnostic891)
  {
    if (!createDiag())
      return true;
  }
  // display diagnostic and stop processing

  if (fallback::fallbackDebugOverrideBrandingServiceResponses(&_trx))
  {
    bool formatResponse = !(_trx.diagnostic().diagParamMapItem("DD") == "XML");
    if (_trx.diagnostic().diagnosticType() == Diagnostic890)
      _diag890->displayXML_old(xml, diagHeader, formatResponse, status);
    else if (_trx.diagnostic().diagnosticType() == Diagnostic891)
      _diag891->displayXML_old(xml, diagHeader, formatResponse, status);
  }
  else
  {
    if (_trx.diagnostic().diagnosticType() == Diagnostic890)
    {
      _diag890->displayXML(
          xml, diagHeader, getFormatOfDiagnosticMessage(_trx.diagnostic()), status);
    }
    else if (_trx.diagnostic().diagnosticType() == Diagnostic891)
    {
      _diag891->displayXML(
          xml, diagHeader, getFormatOfDiagnosticMessage(_trx.diagnostic()), status);
    }
  }

  return false;
}

template <BrandDataSource source>
bool
BrandingServiceCaller<source>::createDiag()
{
  if (!_trx.diagnostic().isActive())
    return false;

  DiagnosticTypes diagType = _trx.diagnostic().diagnosticType();
  if (diagType == Diagnostic891)
  {
    DiagCollector* diag = DCFactory::instance()->create(_trx);

    if (diag == nullptr)
      return false;
    diag->enable(Diagnostic891);
    _diag891 = dynamic_cast<Diag891Collector*>(diag);
    return true;
  }
  return false;
}

template <BrandDataSource source>
bool
BrandingServiceCaller<source>::createDiag890()
{
  if (!_trx.diagnostic().isActive())
    return false;

  DiagnosticTypes diagType = _trx.diagnostic().diagnosticType();
  if (diagType == Diagnostic890)
  {
    DiagCollector* diag = DCFactory::instance()->create(_trx);

    if (diag == nullptr)
      return false;
    diag->enable(Diagnostic890);
    _diag890 = dynamic_cast<Diag890Collector*>(diag);
    return true;
  }
  return false;
}

template <BrandDataSource source>
void
BrandingServiceCaller<source>::printFareDisplayDiagnostic(const std::string& request,
                                                          const std::string& response,
                                                          StatusBrandingService status)
{
  std::string reqParam = (source == S8) ? "S8BRANDREQ" : "ABCCREQ";
  std::string resParam = (source == S8) ? "S8BRANDRES" : "ABCCRES";
  FareDisplayTrx& fdTrx = static_cast<FareDisplayTrx&>(_trx);
  if (FareDisplayUtil::isXMLDiagRequested(fdTrx, reqParam))
  {
    std::string diagReqHeader = (source == S8) ? S8_DIAG_REQUEST_HEADER : CBAS_DIAG_REQUEST_HEADER;
    FareDisplayUtil::displayXMLDiag(fdTrx, request, diagReqHeader, status);
  }
  else if (FareDisplayUtil::isXMLDiagRequested(fdTrx, resParam))
  {
    std::string diagResHeader =
        (source == S8) ? S8_DIAG_RESPONSE_HEADER : CBAS_DIAG_RESPONSE_HEADER;
    FareDisplayUtil::displayXMLDiag(fdTrx, response, diagResHeader, status);
  }
}

template <BrandDataSource source>
void
BrandingServiceCaller<source>::printPricingShoppingDiagnostic(const std::string& request,
                                                              const std::string& response,
                                                              StatusBrandingService status)
{
  if (_trx.diagnostic().diagnosticType() == Diagnostic890 &&
      !displayXML(
          request, (source == S8) ? S8_DIAG_REQUEST_HEADER : CBAS_DIAG_REQUEST_HEADER, status))
  {
    if (_flushDiagMsg)
      _diag890->flushMsg();
  }
  else if (_trx.diagnostic().diagnosticType() == Diagnostic891 &&
           !displayXML(response,
                       (source == S8) ? S8_DIAG_RESPONSE_HEADER : CBAS_DIAG_RESPONSE_HEADER,
                       status))
  {
    if (_flushDiagMsg)
      _diag891->flushMsg();
  }
}

template <BrandDataSource source>
void
BrandingServiceCaller<source>::printDiagnostic(const std::string& request,
                                               const std::string& response,
                                               StatusBrandingService status)
{
  if (_trx.getTrxType() == PricingTrx::FAREDISPLAY_TRX)
    printFareDisplayDiagnostic(request, response, status);
  else if (source == S8)
    printPricingShoppingDiagnostic(request, response, status);
}

template <BrandDataSource source>
BrandingMessageFormat
BrandingServiceCaller<source>::getFormatOfDiagnosticMessage(Diagnostic& diag)
{
  if (diag.diagParamMapItem(Diagnostic::BRANDING_SERVICE_FORMAT) == Diagnostic::CDATA_XML)
  {
    return BrandingMessageFormat::BS_CDATA_SECTION;
  }
  else if (diag.diagParamMapItem("DD") == "XML")
  {
    return BrandingMessageFormat::BS_NO_FORMAT;
  }
  return BrandingMessageFormat::BS_GREEN_SCREEN_FORMAT;
}

template <BrandDataSource source>
bool
BrandingServiceCaller<source>::cleanCurl(CURL* curlHandle)
{
  curl_easy_cleanup(curlHandle);
  return false;
}

template class BrandingServiceCaller<CBAS>;
template class BrandingServiceCaller<S8>;
} // namespace tse
