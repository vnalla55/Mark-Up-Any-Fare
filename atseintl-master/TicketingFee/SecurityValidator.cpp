//-------------------------------------------------------------------
//  Copyright Sabre 2009
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

#include "TicketingFee/SecurityValidator.h"

#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "Common/TseUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/SvcFeesSecurityInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/SvcFeesDiagCollector.h"
#include "Rules/RuleUtil.h"


namespace tse
{

static Logger
logger("atseintl.TicketingFee.SecurityValidator");

const Indicator SecurityValidator::MUST_BE_TVL_AGENCY;
const LocTypeCode SecurityValidator::AIRLINE_SPECIFIC_X;
const LocTypeCode SecurityValidator::AIRLINE_SPECIFIC_V;
const LocTypeCode SecurityValidator::AIRLINE_SPECIFIC_A;
const LocTypeCode SecurityValidator::ERSP_NUMBER;
const LocTypeCode SecurityValidator::LNIATA_NUMBER;

SecurityValidator::SecurityValidator(PricingTrx& trx,
                                     const std::vector<TravelSeg*>::const_iterator segI,
                                     const std::vector<TravelSeg*>::const_iterator segIE)
  : _trx(trx), _diag(nullptr), _segI(segI), _segIE(segIE)
{
}

SecurityValidator::~SecurityValidator() {}

bool
SecurityValidator::validate(
    int s4SeqNo, int t183ItemNo, bool& view, VendorCode vc, SvcFeesDiagCollector* dc)
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": s4SeqNo=" << s4SeqNo << ", t183ItemNo=" << t183ItemNo
                << ", view=" << view << ", vc=" << vc);
  const std::vector<SvcFeesSecurityInfo*>& securityInfo = getSecurityInfo(vc, t183ItemNo);

  createDiag(s4SeqNo, t183ItemNo);
  if (UNLIKELY(securityInfo.empty()))
  {
    emptyMsg(vc);
    if (dc && _diag)
      *dc << _diag->str();
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " returning false");
    return false;
  }
  StatusT183Security returnCode = PASS_T183;

  std::vector<SvcFeesSecurityInfo*>::const_iterator secI = securityInfo.begin();
  const std::vector<SvcFeesSecurityInfo*>::const_iterator secEnd = securityInfo.end();
  for (; secI != secEnd; ++secI)
  {
    returnCode = validateSequence(*secI, view);
    detailDiag(*secI, returnCode);
    if (returnCode == PASS_T183)
    {
      if (UNLIKELY(dc && _diag))
        *dc << _diag->str();
      LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " returning true");
      return true;
    }
  }
  if (UNLIKELY(dc && _diag))
    *dc << _diag->str();
  else
    endDiag();
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " returning false");
  return false;
}

StatusT183Security
SecurityValidator::validateSequence(const SvcFeesSecurityInfo* secInfo, bool& view) const
{
  TSE_ASSERT(secInfo);
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__);
  if (UNLIKELY(!checkTvlAgency(secInfo)))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_TVL_AGENCY");
    return FAIL_TVL_AGENCY;
  }

  if (!checkGds(secInfo))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_CXR_GDS");
    return FAIL_CXR_GDS;
  }

  if (UNLIKELY(!checkDutyCode(secInfo)))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_DUTY");
    return FAIL_DUTY;
  }

  if (!checkLoc(secInfo))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_GEO");
    return FAIL_GEO;
  }

  if (!checkCode(secInfo))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_TYPE_CODE");
    return FAIL_TYPE_CODE;
  }

  if (secInfo->viewBookTktInd() == '2')
    view = true;

  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning PASS_T183");
  return PASS_T183;
}

const std::vector<SvcFeesSecurityInfo*>&
SecurityValidator::getSecurityInfo(VendorCode vc, int itemNo)
{
  return _trx.dataHandle().getSvcFeesSecurity(vc, itemNo);
}

bool
SecurityValidator::checkTvlAgency(const SvcFeesSecurityInfo* secInfo) const
{
  LOG4CXX_DEBUG(logger, "Entered SecurityValidator::checkTvlAgency()");
  if (secInfo->travelAgencyInd() == MUST_BE_TVL_AGENCY)
  {
    if (UNLIKELY(_trx.getRequest()->ticketingAgent()->tvlAgencyPCC().empty()))
      return false;
  }
  return true;
}

bool
SecurityValidator::checkGds(const SvcFeesSecurityInfo* secInfo) const
{
  if (secInfo->carrierGdsCode().empty())
    return true;

  if (secInfo->carrierGdsCode()[0] == '1')
  {
    return (secInfo->carrierGdsCode() == RuleConst::SABRE1S ||
	   (secInfo->carrierGdsCode() == RuleConst::SABRE1B
	       && _trx.getRequest()->ticketingAgent()->abacusUser()) ||
	   (secInfo->carrierGdsCode() == RuleConst::SABRE1J
	       && _trx.getRequest()->ticketingAgent()->axessUser())  ||
	   (secInfo->carrierGdsCode() == RuleConst::SABRE1F
	       && _trx.getRequest()->ticketingAgent()->infiniUser()));
  }

  if (LIKELY(!_trx.getRequest()->ticketingAgent()->tvlAgencyPCC().empty()))
    return false;

  return secInfo->carrierGdsCode() == _trx.billing()->partitionID();
}

bool
SecurityValidator::checkDutyCode(const SvcFeesSecurityInfo* secInfo) const
{
  LOG4CXX_DEBUG(logger, "Entered SecurityValidator::checkDutyCode()");
  if (UNLIKELY(!secInfo->carrierGdsCode().empty() && !secInfo->dutyFunctionCode().empty()))
  {
    if (!RuleUtil::validateDutyFunctionCode(_trx.getRequest()->ticketingAgent(),
                                            secInfo->dutyFunctionCode()))
      return false;
  }
  return true;
}

bool
SecurityValidator::checkLoc(const SvcFeesSecurityInfo* secInfo) const
{
  LOG4CXX_DEBUG(logger, "Entered SecurityValidator::checkLoc()");
  if (!secInfo->loc().loc().empty())
  {
    const Loc* agentLocation = getLocation();
    if (UNLIKELY(!agentLocation))
      return false;
    return isInLoc(secInfo, *agentLocation);
  }
  return true;
}

const Loc*
SecurityValidator::getLocation() const
{
  if (!_trx.getRequest()->ticketPointOverride().empty())
    return locationOverride(_trx.getRequest()->ticketPointOverride());

  return _trx.getRequest()->ticketingAgent()->agentLocation();
}

const Loc*
SecurityValidator::locationOverride(const LocCode& loc) const
{
  return _trx.dataHandle().getLoc(loc, _trx.ticketingDate());
}

bool
SecurityValidator::isInLoc(const SvcFeesSecurityInfo* secInfo, const Loc& loc) const
{
  return LocUtil::isInLoc(loc,
                          secInfo->loc().locType(),
                          secInfo->loc().loc(),
                          ATPCO_VENDOR_CODE,
                          RESERVED,
                          LocUtil::OTHER,
                          GeoTravelType::International,
                          EMPTY_STRING(),
                          _trx.ticketingDate());
}

bool
SecurityValidator::checkCode(const SvcFeesSecurityInfo* secInfo) const
{
  LOG4CXX_DEBUG(logger, "Entered SecurityValidator::checkCode()");
  if (secInfo->code().locType() == RuleConst::BLANK)
    return true;

  switch (secInfo->code().locType())
  {
    case AIRLINE_SPECIFIC_A:
      return (secInfo->code().loc() == _trx.getRequest()->ticketingAgent()->airlineChannelCode());
      break;

    case ERSP_NUMBER:
    case LNIATA_NUMBER:
      return false;
      break;
    case AIRLINE_SPECIFIC_X:
      return processAirlineSpecificX(secInfo);
      break;

    case AIRLINE_SPECIFIC_V:
      return (!secInfo->carrierGdsCode().empty() && secInfo->carrierGdsCode()[0] != '1' &&
              (secInfo->code().loc() == _trx.getRequest()->ticketingAgent()->airlineDept() ||
              secInfo->code().loc() == _trx.getRequest()->ticketingAgent()->officeDesignator()));
      break;

    default:
      if (secInfo->code().locType() == RuleConst::TRAVEL_AGENCY)
        return secInfo->code().loc() == _trx.getRequest()->ticketingAgent()->tvlAgencyPCC();
      else if (LIKELY(secInfo->code().locType() == RuleConst::IATA_TVL_AGENCY_NO))
        return secInfo->code().loc() == _trx.getRequest()->ticketingAgent()->tvlAgencyIATA();
      break;
  }

  return false;
}

void
SecurityValidator::createDiag(int s4SeqNo, int t183ItemNo)
{
  LOG4CXX_DEBUG(logger, "Entered SecurityValidator::createDiag()");

  if (LIKELY(!shouldCreateDiag()))
    return;

  const std::string& diagSQ = _trx.diagnostic().diagParamMapItem(Diagnostic::SEQ_NUMBER);
  if (!diagSQ.empty())
  {
    int inputSeqNumber = atoi(diagSQ.c_str());
    if (inputSeqNumber != s4SeqNo)
      return;
  }
  if (!_diag)
  {
    DiagCollector* diagCollector = DCFactory::instance()->create(_trx);
    _diag = dynamic_cast<SvcFeesDiagCollector*>(diagCollector);
    if (_diag == nullptr)
      return;
    _diag->activate();
  }
  if (_diag && shouldCollectDiag())
    _diag->printSecurityTable183Header(t183ItemNo);
}

bool
SecurityValidator::matchFareMarketInRequest() const
{
  std::map<std::string, std::string>::iterator endD = _trx.diagnostic().diagParamMap().end();
  std::map<std::string, std::string>::iterator beginD =
      _trx.diagnostic().diagParamMap().find(Diagnostic::FARE_MARKET);

  std::string specifiedFM("");

  // procees /FMDFWLON
  if (beginD != endD)
  {
    specifiedFM = (*beginD).second;
    if (!(specifiedFM.empty()))
    {
      const std::string boardCity(specifiedFM.substr(0, 3));
      const std::string offCity(specifiedFM.substr(3, 3));

      if (((boardCity != (*_segI)->boardMultiCity()) && (boardCity != (*_segI)->origin()->loc())) ||
          ((offCity != (*(_segIE - 1))->offMultiCity()) &&
           (offCity != (*(_segIE - 1))->destination()->loc())))
        return false;
    }
  }
  return true;
}

void
SecurityValidator::detailDiag(const SvcFeesSecurityInfo* feeSec, const StatusT183Security status)
{
  if (UNLIKELY(_diag && shouldCollectDiag()))
  {
    _diag->printSecurityTable183Info(feeSec, status);
  }
}

void
SecurityValidator::emptyMsg(VendorCode vc)
{
  if (!_diag)
    return;

  _diag->printSecurityTable183EmptyMsg(vc);
}

void
SecurityValidator::endDiag()
{
  if (UNLIKELY(_diag))
    _diag->flushMsg();
}

bool
SecurityValidator::shouldCreateDiag() const
{
  DiagnosticTypes diagType = _trx.diagnostic().diagnosticType();

  if (LIKELY(!_trx.diagnostic().isActive() || (diagType != Diagnostic870 && diagType != Diagnostic877)))
    return false;

  if (_trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) != "INFO")
    return false;

  return true;
}

bool
SecurityValidator::shouldCollectDiag() const
{
  return matchFareMarketInRequest();
}

bool
SecurityValidator::processAirlineSpecificX(const SvcFeesSecurityInfo* secInfo) const
{
  return false;
}

} // tse
