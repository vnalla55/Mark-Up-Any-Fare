//-------------------------------------------------------------------
//
//  File:        AltPricingTrx.cpp
//
//  Description: Fare Display Transaction object
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
//-------------------------------------------------------------------
#include "DataModel/AltPricingTrx.h"

#include "Common/AltPricingUtil.h"
#include "Common/Logger.h"
#include "Common/OBFeesUtils.h"
#include "DataModel/Agent.h"
#include "DataModel/FarePath.h"
#include "Diagnostic/DiagTools.h"
#include "Xform/AltPricingResponseFormatter.h"
#include "Xform/PricingResponseFormatter.h"
#include "Xform/XMLConvertUtils.h"

namespace tse
{
static Logger
logger("atseintl.DataModel.AltPricingTrx");

bool
AltPricingTrx::process(Service& srv)
{
  if (WP == altTrxType())
  {
    PricingTrx& pricingTrx(*this);
    return srv.process(pricingTrx);
  }
  return srv.process(*this);
}

bool
AltPricingTrx::isSingleMatch() const
{
  return (!OBFeesUtils::fallbackObFeesWPA(this) &&
          !_itin.empty() &&
          !_itin.front()->farePath().empty() &&
          PricingTrx::WPA == altTrxType() &&
          (_request->ticketingAgent()->abacusUser() ||
          _request->ticketingAgent()->infiniUser()) &&
          paxType().size() == _itin.front()->farePath().size() &&
          !AltPricingUtil::isXMrequest(*this) &&
          [] (const std::vector<FarePath*>& farePaths) -> bool
          {
            for (auto farePath : farePaths)
              if (!farePath->processed())
                return false;
            return true;
          } (_itin.front()->farePath()));
}

bool
AltPricingTrx::convert(std::string& response)
{
  std::string tmpResponse;
  XMLConvertUtils::tracking(*this);
  if (!taxRequestToBeReturnedAsResponse().empty())
  {
    response = taxRequestToBeReturnedAsResponse();
    return true;
  }
  LOG4CXX_DEBUG(logger, "Doing PricingTrx response");
  Diagnostic& diag = diagnostic();
  if (fareCalcCollector().empty())
  {
    LOG4CXX_WARN(logger, "Pricing Response Items are Missing");
  }

  FareCalcCollector* fareCalcCollector = nullptr;
  if ((diag.diagnosticType() == DiagnosticNone || diag.diagnosticType() == Diagnostic855) &&
      !_fareCalcCollector.empty())
  {
    fareCalcCollector = _fareCalcCollector.front();
  }

  bool forDisplay = displayOnly();

  tmpResponse = diag.toString();

  if (diag.diagnosticType() == Diagnostic854 &&
      (diag.diagParamIsSet(Diagnostic::DISPLAY_DETAIL, Diagnostic::TOPLINE_METRICS) ||
       diag.diagParamIsSet(Diagnostic::DISPLAY_DETAIL, Diagnostic::FULL_METRICS)))
  {
    tmpResponse += "\n\n" + utils::getMetricsInfo(*this);
  }

  if (diag.diagnosticType() != DiagnosticNone && tmpResponse.length() == 0)
  {
    char tmpBuf[512];
    sprintf(tmpBuf, "DIAGNOSTIC %d RETURNED NO DATA", diag.diagnosticType());
    tmpResponse.insert(0, tmpBuf);
  }

  if (altTrxType() == AltPricingTrx::WP)
  {
    if (isRfbListOfSolutionEnabled() || activationFlags().isSearchForBrandsPricing())
    {
      AltPricingResponseFormatter formatter;
      response = formatter.formatResponse(tmpResponse, forDisplay, *this, fareCalcCollector);
    }
    else
    {
      PricingResponseFormatter formatter;
      response = formatter.formatResponse(tmpResponse, forDisplay, *this, fareCalcCollector);
    }
  }
  else if (isPriceSelectionEntry())
  {
    response = wtfrFollowingWp() ? XMLConvertUtils::formatWtfrResponse(this)
                                 : XMLConvertUtils::formatWpaWtfrResponse(this);
    LOG4CXX_INFO(logger, "response: " << response);
  }
  else
  {
    AltPricingResponseFormatter formatter;
    if (getOptions()->returnAllData() == GDS)
    { // Currently,  GDS(SDS) is not supported and send error.
      response = formatter.formatResponse("NO FARES/RBD/CARRIER",
                                          false,
                                          *this,
                                          nullptr,
                                          ErrorResponseException::NO_FARES_RBD_CARRIER);
    }
    else
    {
      response = formatter.formatResponse(tmpResponse, forDisplay, *this, fareCalcCollector);
    }
  }

  return true;
}

} // tse namespace
