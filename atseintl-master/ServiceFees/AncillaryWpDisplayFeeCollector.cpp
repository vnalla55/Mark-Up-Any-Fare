//  Copyright Sabre 2011
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
#include "ServiceFees/AncillaryWpDisplayFeeCollector.h"

#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "Common/TseEnums.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/AncRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/TrxAborter.h"
#include "DBAccess/Customer.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag878Collector.h"
#include "Diagnostic/Diag879Collector.h"
#include "ServiceFees/AncillaryWpDisplayValidator.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/ServiceFeesGroup.h"


namespace tse
{
using namespace std;
FALLBACK_DECL(reworkTrxAborter);
static Logger
logger("atseintl.ServiceFees.AncillaryWpDisplayFeeCollector");

AncillaryWpDisplayFeeCollector::AncillaryWpDisplayFeeCollector(PricingTrx& trx)
  : OptionalFeeCollector(trx)
{
}

AncillaryWpDisplayFeeCollector::~AncillaryWpDisplayFeeCollector() {}

void
AncillaryWpDisplayFeeCollector::process()
{
  LOG4CXX_DEBUG(logger, "Entered AncillaryWpDisplayFeeCollector::process()");

  createPseudoPricingSolution();

  try
  {
    if (fallback::reworkTrxAborter(&_trx))
      checkTrxAborted(_trx);
    else
      _trx.checkTrxAborted();
  }
  catch (const ErrorResponseException& e)
  {
    if (e.code() == ErrorResponseException::ANCILLARY_TIME_OUT_R7)
    {
      std::string newMsg = "";
      AncillaryPricingTrx& ancPrcTrx = static_cast<AncillaryPricingTrx&>(_trx);
      AncRequest* req = dynamic_cast<AncRequest*>(ancPrcTrx.getRequest());
      if (req && req->isPostTktRequest())
      {
        if (ancPrcTrx.itin().size() == 1 &&
            (req->paxType(ancPrcTrx.itin().front()).size() == 1 ||
             (req->paxType(ancPrcTrx.itin().front()).size() > 1 && samePaxType(ancPrcTrx))))
          newMsg += "MAX AIR EXTRAS EXCEEDED/USE AE SVC QUALIFIER";
        else
          newMsg += "MAX AIR EXTRAS EXCEEDED/USE AE SVC QUALIFIER OR TKT SELECT";
      }
      else
        newMsg += "AIR EXTRAS MAY APPLY - USE WPAE WITH SERVICE QUALIFIER";

      throw ErrorResponseException(e.code(), newMsg.c_str());
    }
    throw;
  }

  if (_trx.diagnostic().isActive())
  {
    DiagnosticTypes diagType = _trx.diagnostic().diagnosticType();
    if (diagType == Diagnostic881)
    {
      LOG4CXX_DEBUG(logger, "Leaving AncillaryWpDisplayFeeCollector::process() - Diag 881 done");
      return;
    }
  }

  collect();
  if (_trx.diagnostic().isActive())
    checkDiag878And879();

  LOG4CXX_DEBUG(logger, "Leaving AncillaryWpDisplayFeeCollector::process() - OC Fee done");
}

void
AncillaryWpDisplayFeeCollector::checkDiag878And879() const
{

  if (_trx.diagnostic().diagnosticType() == Diagnostic878)
  {
    DCFactory* factory = DCFactory::instance();
    Diag878Collector* diagPtr = dynamic_cast<Diag878Collector*>(factory->create(_trx));

    diagPtr->enable(Diagnostic878);
    diagPtr->activate();

    (*diagPtr) << _trx;

    diagPtr->flushMsg();
  }
  else if (_trx.diagnostic().diagnosticType() == Diagnostic879)
  {
    DCFactory* factory = DCFactory::instance();
    Diag879Collector* diagPtr = dynamic_cast<Diag879Collector*>(factory->create(_trx));

    diagPtr->enable(Diagnostic879);
    diagPtr->activate();

    (*diagPtr) << _trx;

    diagPtr->flushMsg();
  }
}

void
AncillaryWpDisplayFeeCollector::processSubCodes(ServiceFeesGroup& srvFeesGrp,
                                                const CarrierCode& candCarrier,
                                                int unitNo,
                                                bool isOneCarrier) const
{
  OcValidationContext ctx(_trx, *_farePath->itin(), *_farePath->paxType(), _farePath);
  AncillaryWpDisplayValidator ancillaryWpDisplayValidator(
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
      ancillaryWpDisplayValidator, srvFeesGrp, candCarrier, unitNo, isOneCarrier);
}

bool
AncillaryWpDisplayFeeCollector::samePaxType(AncillaryPricingTrx& ancTrx)
{
  Itin* itin = ancTrx.itin().front();
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
AncillaryWpDisplayFeeCollector::multiThreadSliceAndDice(
    std::vector<std::pair<std::vector<TravelSeg*>::const_iterator,
                          std::vector<TravelSeg*>::const_iterator> >& routes,
    int unitNo)
{
  multiThreadSliceAndDiceImpl<AncillaryWpDisplayFeeCollector>(routes, unitNo);
}
} // namespace
