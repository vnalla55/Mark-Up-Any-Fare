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
#include "Service/RefundOrchestrator.h"

#include "Common/TrxUtil.h"
#include "DataModel/RefundPricingTrx.h"
#include "Diagnostic/DiagManager.h"
#include "Service/TransactionOrchestrator.h"

namespace tse
{

static Logger
logger("atseintl.Service.TransactionOrchestrator");

bool
TransactionOrchestrator::RefundOrchestrator::processExcItin(uint64_t services)
{
  LOG4CXX_DEBUG(logger, "Process(RefundPricingTrx&) - EXC part");

  _refundTrx.trxPhase() = RexBaseTrx::REPRICE_EXCITIN_PHASE;

  if (!_refundTrx.arePenaltiesAndFCsEqualToSumFromFareCalc() && (services & PRICING))
    services |= FARE_VALIDATOR;

  bool result = _to.invokeServices(_refundTrx, services);

  if (result && (services & PRICING))
  {
    _refundTrx.trxPhase() = RexBaseTrx::MATCH_EXC_RULE_PHASE;
    return _to.invokeServices(_refundTrx, REX_FARE_SELECTOR);
  }

  return result;
}

bool
TransactionOrchestrator::RefundOrchestrator::processNewItin(uint64_t services)
{
  LOG4CXX_DEBUG(logger, "Process(RefundPricingTrx&) - NEW part");

  _refundTrx.getRequest()->lowFareRequested() = 'Y';

  _refundTrx.trxPhase() = RexBaseTrx::PRICE_NEWITIN_PHASE;
  _refundTrx.setAnalyzingExcItin(false);

  if (_refundTrx.fullRefund())
  {
    if(TrxUtil::isAutomatedRefundCat33Enabled(_refundTrx))
    {
      return _to.invokeServices(_refundTrx, TAXES);
    }
    else
    {
      return true;
    }
  }

  return _to.invokeServices(_refundTrx, services);
}

bool
TransactionOrchestrator::RefundOrchestrator::process()
{
  _refundTrx.setAnalyzingExcItin(true);

  if (!_to.invokeServices(_refundTrx, ITIN_ANALYZER))
    return false;

  DiagnosticTypes diagType = _refundTrx.diagnostic().diagnosticType();
  DiagnosticQualifier qualifier = determineDiagnosticQualifier();

  switch (qualifier)
  {
  case DQ_NONE:
    return noDiagnosticProcess();
  case DQ_ITEXC:
  {
    _refundTrx.diagnostic().activate();
    if (internalDiagnostic())
    {
      _refundTrx.setAnalyzingExcItin(false);
      return processExcItin(INTERNAL);
    }

    if (diagType == Diagnostic233)
      return processExcItin(FARE_COLLECTOR | REX_FARE_SELECTOR);

    return processExcItin(TransactionOrchestrator::getExcItinServices(_refundTrx));
  }
  case DQ_ITALL:
  {
    _refundTrx.diagnostic().activate();
    if(TrxUtil::isAutomatedRefundCat33Enabled(_refundTrx))
    {
      return (processExcItin(FARE_COLLECTOR | REX_FARE_SELECTOR | PRICING | TAXES) &&
              processNewItin(FARE_COLLECTOR | FARE_VALIDATOR | PRICING | TAXES | FARE_CALC));
    }
    else
    {
      return (processExcItin(FARE_COLLECTOR | REX_FARE_SELECTOR | PRICING) &&
              processNewItin(FARE_COLLECTOR | FARE_VALIDATOR | PRICING | TAXES | FARE_CALC));
    }
  }
  case DQ_ITNEW:
  {
    _refundTrx.diagnostic().deActivate();
    bool result = processExcItin(FARE_COLLECTOR | REX_FARE_SELECTOR | PRICING);

    _refundTrx.diagnostic().activate();
    return result && processNewItin(FARE_COLLECTOR | FARE_VALIDATOR | PRICING | TAXES | FARE_CALC);
  }
  case DQ_ITEFT:
  {
    _refundTrx.diagnostic().deActivate();
    try
    {
      return (processExcItin(FARE_COLLECTOR | REX_FARE_SELECTOR | PRICING) &&
              processNewItin(FARE_COLLECTOR | FARE_VALIDATOR | PRICING));
    }
    catch (const ErrorResponseException& e)
    {
      if (isEnforceRedirection(e))
      {
        _refundTrx.diagnostic().activate();
        return _to.processEftItin(_refundTrx,
                                  _to.getServicesForDiagnostic(diagType, _refundTrx), true);
      }
      throw;
    }
    break;
  }
  default:
    break;
  }

  // should never get this
  return false;
}

bool
TransactionOrchestrator::RefundOrchestrator::isEnforceRedirection(const ErrorResponseException& e)
    const
{
  return (!_refundTrx.fullRefund() && !_refundTrx.secondaryExcReqType().empty() &&
          (e.code() == ErrorResponseException::REFUND_RULES_FAIL ||
           e.code() == ErrorResponseException::UNABLE_TO_MATCH_REFUND_RULES));
}

bool
TransactionOrchestrator::RefundOrchestrator::noDiagnosticProcess()
{
  uint64_t services = FARE_COLLECTOR | FARE_VALIDATOR | PRICING | TAXES | FARE_CALC;
  try
  {
    return processExcItin(TransactionOrchestrator::getExcItinServices(_refundTrx)) &&
        processNewItin(services);
  }
  catch (const ErrorResponseException& e)
  {
    if (isEnforceRedirection(e))
    {
      _refundTrx.redirectReasonError() = e;
      _to.processEftItin(_refundTrx, ITIN_ANALYZER | services, false);
    }
    throw;
  }

  return true;
}

bool
TransactionOrchestrator::RefundOrchestrator::internalDiagnostic() const
{
  const DiagnosticTypes& type = _refundTrx.diagnostic().diagnosticType();
  return type >= INTERNAL_DIAG_RANGE_BEGIN && type <= INTERNAL_DIAG_RANGE_END;
}

TransactionOrchestrator::DiagnosticQualifier
TransactionOrchestrator::RefundOrchestrator::determineDiagnosticQualifier() const
{
  if (_refundTrx.diagnostic().diagnosticType() == DiagnosticNone)
    return DQ_NONE;

  const std::string& diagItin = _refundTrx.diagnostic().diagParamMapItem(Diagnostic::ITIN_TYPE);

  if (diagItin == "RED")
    return DQ_ITEFT;

  switch (_refundTrx.diagnostic().diagnosticType())
  {
  case Diagnostic194:
  case Diagnostic233:
  case Diagnostic333:
  case Diagnostic688:
    return DQ_ITEXC;
  case Diagnostic689:
    return _refundTrx.fullRefund() ? DQ_ITEXC : DQ_ITNEW;
  case Diagnostic690:
    if (_refundTrx.fullRefund())
      return DQ_ITEXC;
    break;
  default:
    ;
  }

  if (internalDiagnostic())
    return DQ_ITEXC;

  if (diagItin == "EXC")
    return DQ_ITEXC;

  if (diagItin == "ALL")
    return DQ_ITALL;

  return DQ_ITNEW;
}
}
