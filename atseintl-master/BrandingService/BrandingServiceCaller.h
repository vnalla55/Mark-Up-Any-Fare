//-------------------------------------------------------------------
//
//  File:        BrandDataRetriever.h
//  Created:     January 25, 2008
//  Authors:     Mauricio Dantas
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

#include "Common/TseEnums.h"

#include <string>
#include <cstdint>
#include <curl.h>

namespace tse
{

class PricingTrx;
class Diag890Collector;
class Diag891Collector;
class Diagnostic;
enum BrandingMessageFormat : unsigned int;

enum BrandDataSource
{ CBAS,
  S8
};

template<BrandDataSource source = CBAS>
class BrandingServiceCaller
{
  friend class S8BrandingServiceCallerTest;
  friend class CbasBrandingServiceCallerTest;

public:
  BrandingServiceCaller(PricingTrx& trx);
  virtual ~BrandingServiceCaller() = default;

  enum BrandingCallErrorCode
  { NO_ERROR = 0,
    NO_ENDPOINT,
    CURL_INIT_ERROR,
    NETWORK_ERROR,
    SERVICE_UNAVAILABLE,
    SERVICE_SPECIFIC,
    RESPONSE_PARSING_ERROR };

  bool callBranding(std::string& data, std::string& response);
  StatusBrandingService getStatusBrandingService() const { return _status; }
  void printDiagnostic(const std::string& request,
                       const std::string& response,
                       StatusBrandingService status);

private:
  static size_t writeData(void* buffer, size_t size, size_t nmemb, void* userp);


  void diagReportError(int16_t& svcTimeout,
                       std::string& brandingUrl,
                       CURLcode curlCode,
                       char curlErrorBuffer[]);

  void printFareDisplayDiagnostic(const std::string& request,
                                  const std::string& response,
                                  StatusBrandingService status);
  void printPricingShoppingDiagnostic(const std::string& request,
                                      const std::string& response,
                                      StatusBrandingService status);
  BrandingMessageFormat getFormatOfDiagnosticMessage(Diagnostic& diag);

  bool cleanCurl(CURL* curlHandle);

  bool createDiag890();
  bool createDiag();
  bool displayXML(const std::string& xml,
                  const std::string& diagHeader,
                  StatusBrandingService status = StatusBrandingService::NO_BS_ERROR);

  std::string _response;
  PricingTrx& _trx;
  StatusBrandingService _status = StatusBrandingService::NO_BS_ERROR;
  Diag890Collector* _diag890 = nullptr;
  Diag891Collector* _diag891 = nullptr;
  bool _flushDiagMsg = true;
};

} // namespace tse
