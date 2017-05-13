//-------------------------------------------------------------------
//
//  Authors:     Michał Ożga
//
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------


#pragma once

#include "Common/FallbackUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseEnums.h"

namespace tse
{
class PricingTrx;

template <typename BSCaller>
class ConfigurableBrandingServiceCaller
{
public:
  ConfigurableBrandingServiceCaller(PricingTrx& trx) : _trx(trx)
  {
    _responseOverride = TrxUtil::getOverrideBsResponse(trx);
  }

  std::string callBranding(std::string& request);
  StatusBrandingService getStatusBrandingService() const { return _status; }

private:
  std::string normalBranding(std::string& request);
  std::string overridenBranding(std::string& request);

  PricingTrx& _trx;
  std::string _responseOverride;
  StatusBrandingService _status = StatusBrandingService::NO_BS_ERROR;
};

template <typename BSCaller>
std::string ConfigurableBrandingServiceCaller<BSCaller>::callBranding(std::string& request)
{
  if (LIKELY( "N" == _responseOverride))
  {
    return normalBranding(request);
  }
  else
  {
    return overridenBranding(request);
  }
}

template <typename BSCaller>
std::string ConfigurableBrandingServiceCaller<BSCaller>::normalBranding(std::string& request)
{
  BSCaller brandingServiceCaller(_trx);
  std::string response;
  brandingServiceCaller.callBranding(request, response);
  _status = brandingServiceCaller.getStatusBrandingService();
  return response;
}

template <typename BSCaller>
std::string ConfigurableBrandingServiceCaller<BSCaller>::overridenBranding(std::string& request)
{
  BSCaller brandingServiceCaller(_trx);
  if (_responseOverride == "BS_NO_BRANDS_FOUND")
  {
    _status = StatusBrandingService::BS_UNAVAILABLE;
    _responseOverride.clear();
  }

  brandingServiceCaller.printDiagnostic(request, _responseOverride, _status);
  return _responseOverride;
}

} //namespace tse
