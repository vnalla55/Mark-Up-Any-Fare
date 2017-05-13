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
//-------------------------------------------------------------------
#include "ServiceFees/AncillaryPricingFeeCollector.h"

#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/ServiceFeeUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseEnums.h"
#include "DataModel/Agent.h"
#include "DataModel/AncRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/TrxAborter.h"
#include "DBAccess/Customer.h"
#include "ServiceFees/AncillaryPricingValidator.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/ServiceFeesGroup.h"

namespace tse
{
using namespace std;
FALLBACK_DECL(reworkTrxAborter);
static Logger
logger("atseintl.ServiceFees.AncillaryPricingFeeCollector");

AncillaryPricingFeeCollector::AncillaryPricingFeeCollector(PricingTrx& trx)
  : OptionalFeeCollector(trx)
{
}

AncillaryPricingFeeCollector::~AncillaryPricingFeeCollector() {}

void
AncillaryPricingFeeCollector::process()
{
  LOG4CXX_DEBUG(logger, "Entered AncillaryPricingFeeCollector::process()");

  createPseudoPricingSolution();

  if (fallback::reworkTrxAborter(&_trx))
    checkTrxAborted(_trx);
  else
    _trx.checkTrxAborted();

  if (_trx.diagnostic().isActive())
  {
    DiagnosticTypes diagType = _trx.diagnostic().diagnosticType();
    if (diagType == Diagnostic881)
    {
      LOG4CXX_DEBUG(logger, "Leaving AncillaryPricingFeeCollector::process() - Diag 881 done");
      return;
    }
  }

  collect();
  LOG4CXX_DEBUG(logger, "Leaving AncillaryPricingFeeCollector::process() - OC Fee done");
}

bool
AncillaryPricingFeeCollector::isAgencyActive()
{
  return true;
}

void
AncillaryPricingFeeCollector::processSubCodes(ServiceFeesGroup& srvFeesGrp,
                                              const CarrierCode& candCarrier,
                                              int unitNo,
                                              bool isOneCarrier) const
{
  OcValidationContext ctx(_trx, *_farePath->itin(), *_farePath->paxType(), _farePath);
  AncillaryPricingValidator ancillaryPricingValidator(
      ctx,
      _beginsOfUOT[unitNo],
      _beginsOfUOT[unitNo + 1],
      _beginsOfLargestUOT[2],
      _ts2ss,
      _isInternational.getValForKey(_farePath->itin()),
      isOneCarrier,
      getOperatingMarketingInd(),
      diag877());
  OptionalFeeCollector::processSubCodes(
      ancillaryPricingValidator, srvFeesGrp, candCarrier, unitNo, isOneCarrier);
}

void
AncillaryPricingFeeCollector::printActiveCxrGroupHeader() const
{

  if (_trx.billing()->requestPath() == AEBSO_PO_ATSE_PATH && TrxUtil::isRequestFromAS(_trx))
  {
    OptionalFeeCollector::printActiveCxrGroupHeader();
  }
}
bool
AncillaryPricingFeeCollector::processCarrierMMRecords()
{

  if (_trx.billing()->requestPath() == AEBSO_PO_ATSE_PATH && TrxUtil::isRequestFromAS(_trx))
  {
    if (_trx.itin().front()->ocFeesGroup().empty())
      return false;
    return OptionalFeeCollector::processCarrierMMRecords();
  }
  std::vector<tse::CarrierCode> cxrCodes;
  if (!collectOperMktCxr(cxrCodes))
    return false;

  if (_operatingCxr[0].empty() && _operatingCxr[1].empty() && _marketingCxr[0].empty() &&
      _marketingCxr[1].empty())
  {
    printNoMerchActiveCxrGroup();
    return false;
  }
  return true;
}

bool
AncillaryPricingFeeCollector::validMerchCxrGroup(const CarrierCode& carrier,
                                                 const ServiceGroup& srvGroup) const
{
  if (_trx.billing()->requestPath() == AEBSO_PO_ATSE_PATH && TrxUtil::isRequestFromAS(_trx))
  {
    return OptionalFeeCollector::validMerchCxrGroup(carrier, srvGroup);
  }
  return true;
}

bool
AncillaryPricingFeeCollector::checkAllSegsConfirmed(std::vector<TravelSeg*>::const_iterator begin,
                                                    std::vector<TravelSeg*>::const_iterator end,
                                                    bool doDiag) const
{
  _farePath->itin()->allSegsConfirmed() = true;
  _farePath->itin()->allSegsUnconfirmed() = false;

  return true;
}

void
AncillaryPricingFeeCollector::updateServiceFeesDisplayOnlyState(OCFees* ocFee)
{
}
void
AncillaryPricingFeeCollector::getGroupCodesNotProcessedForTicketingDate(
    std::vector<ServiceGroup>& grValid,
    std::vector<ServiceGroup>& grNotValidForTicketDate,
    std::vector<ServiceGroup>& grNotProcessedForTicketDate)
{
}
bool
AncillaryPricingFeeCollector::shouldProcessAllGroups() const
{
  return false;
}
UserApplCode
AncillaryPricingFeeCollector::getUserApplCode() const
{
  std::string crs = SABRE_USER;
  if (_trx.getRequest()->ticketingAgent()->cxrCode() == AXESS_MULTIHOST_ID)
    crs = AXESS_USER;
  else if (_trx.getRequest()->ticketingAgent()->cxrCode() == ABACUS_MULTIHOST_ID)
    crs = ABACUS_USER;
  else if (_trx.getRequest()->ticketingAgent()->cxrCode() == INFINI_MULTIHOST_ID)
    crs = INFINI_USER;
  return crs;
}

void
AncillaryPricingFeeCollector::updateServiceFeesGroupState(ServiceFeesGroup* sfG)
{
  if (sfG->ocFeesMap().empty())
    sfG->state() = ServiceFeesGroup::EMPTY;
}

void
AncillaryPricingFeeCollector::addInvalidGroups(Itin& itin,
                                               const std::vector<ServiceGroup>& grNA,
                                               const std::vector<ServiceGroup>& grNP,
                                               const std::vector<ServiceGroup>& grNV)
{
  if (!grNA.empty())
    addNotAvailableGroups(itin, grNA, ServiceFeesGroup::NOT_AVAILABLE);

  if (!grNV.empty())
    addNotAvailableGroups(itin, grNV, ServiceFeesGroup::INVALID);
}
int
AncillaryPricingFeeCollector::getNumberOfSegments(std::vector<TravelSeg*>::const_iterator firstSeg,
                                                  std::vector<TravelSeg*>::const_iterator endSeg)
    const
{
  int numberOfArunks = 1;
  for (; firstSeg != endSeg; ++firstSeg)
  {
    if (!dynamic_cast<AirSeg*>(*firstSeg))
      ++numberOfArunks;
  }

  return numberOfArunks;
}

void
AncillaryPricingFeeCollector::getAllActiveGroupCodes(std::set<ServiceGroup>& gValid)
{

  if (_trx.billing()->requestPath() == AEBSO_PO_ATSE_PATH && TrxUtil::isRequestFromAS(_trx))
  {
    static const Indicator crs = MULTIHOST_USER_APPL;
    UserApplCode userCode = _trx.billing()->partitionID();

    static const std::string application = "PRICING";
    getAllActiveGroupCodesForApplication(gValid, crs, userCode, application);
  }
  else
  {
    OptionalFeeCollector::getAllActiveGroupCodes(gValid);
  }

  LOG4CXX_DEBUG(logger, "Leaving AncillaryPricingFeeCollector::getAllActiveGroupCodes(): collect "
                         "all active Group codes");
}

void
AncillaryPricingFeeCollector::getAllActiveGroupCodes(std::vector<ServiceGroup>& gValid)
{

  if (_trx.billing()->requestPath() == AEBSO_PO_ATSE_PATH && TrxUtil::isRequestFromAS(_trx))
  {
    Indicator crs = MULTIHOST_USER_APPL;
    UserApplCode userCode = _trx.billing()->partitionID();

    std::string application = "PRICING";
    getAllActiveGroupCodesForApplication(gValid, crs, userCode, application);
  }
  else
  {
    OptionalFeeCollector::getAllActiveGroupCodes(gValid);
  }

  LOG4CXX_DEBUG(logger, "Leaving AncillaryPricingFeeCollector::getAllActiveGroupCodes(): collect "
                         "all active Group codes");
}

ServiceFeesGroup::FindSolution
AncillaryPricingFeeCollector::getProcessingMethod(ServiceFeesGroup& srvGroup)
{
  return srvGroup.getFindSolutionForSubCodeForAncillaryPricing();
}

void
AncillaryPricingFeeCollector::multiThreadSliceAndDice(
    std::vector<std::pair<std::vector<TravelSeg*>::const_iterator,
                          std::vector<TravelSeg*>::const_iterator> >& routes,
    int unitNo)
{
  multiThreadSliceAndDiceImpl<AncillaryPricingFeeCollector>(routes, unitNo);
}

} // namespace
