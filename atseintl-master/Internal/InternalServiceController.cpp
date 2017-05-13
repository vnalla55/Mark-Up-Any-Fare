//-------------------------------------------------------------------
//
//  File:        InternalServiceController.cpp
//  Created:     July 5, 2004
//  Authors:     Mike Carroll
//
//  Description:
//
//
//  Copyright Sabre 2004
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

#include "Internal/InternalServiceController.h"

#include "Common/CustomerActivationUtil.h"
#include "Common/Global.h"
#include "Common/GoverningCarrier.h"
#include "Common/Logger.h"
#include "Common/RBDByCabinUtil.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/CurrencyTrx.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/MileageTrx.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag187Collector.h"
#include "Diagnostic/Diag188Collector.h"
#include "Diagnostic/Diag194Collector.h"
#include "Diagnostic/Diag198Collector.h"
#include "Diagnostic/Diag199Collector.h"
#include "Diagnostic/Diag997Collector.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{
log4cxx::LoggerPtr
InternalServiceController::_logger(
    log4cxx::Logger::getLogger("atseintl.Internal.InternalServiceController"));

//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------
InternalServiceController::InternalServiceController() : _dcFactory(DCFactory::instance()) {}

//----------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------
InternalServiceController::~InternalServiceController() {}

//----------------------------------------------------------------------------
//  process()
//----------------------------------------------------------------------------
bool
InternalServiceController::process(PricingTrx& trx)
{
  LOG4CXX_INFO(_logger, "Entered InternalController");

  Diagnostic& trxDiag = trx.diagnostic();

  AncillaryPricingTrx* ancTrx = dynamic_cast<AncillaryPricingTrx*>(&trx);
  if (trxDiag.diagnosticType() == UpperBound)
  {
    if (ancTrx)
      trxDiag.insertDiagMsg(trxDiag.masterAncillaryPricingDiagnostic());
    else
      trxDiag.insertDiagMsg(trxDiag.masterPricingDiagnostic());
  }

  if (trxDiag.diagnosticType() == Diagnostic187  &&
      trx.diagnostic().diagParamMapItem(Diagnostic::RBD_ALL) == "LRBD")
    handleDiagnostic187(trx);
  if (trxDiag.diagnosticType() == Diagnostic188)
    handleDiagnostic188(trx);
  else if (trxDiag.diagnosticType() == Diagnostic193)
    handleDiagnostic193(trx);
  else if (trxDiag.diagnosticType() == Diagnostic194)
    handleDiagnostic194(trx);
  else if (trxDiag.diagnosticType() == Diagnostic198)
    handleDiagnostic198(trx);
  else if (trxDiag.diagnosticType() == Diagnostic199)
    handleDiagnostic199(trx);
  else if (trxDiag.diagnosticType() == Diagnostic997)
    handleDiagnostic997(trx);

  return true;
}

void
InternalServiceController::handleDiagnostic187(PricingTrx& trx)
{
  Diag187Collector* diagCol = dynamic_cast<Diag187Collector*>(_dcFactory->create(trx));
  diagCol->enable(Diagnostic187);
  RBDByCabinUtil rbdUtil(trx, NO_CALL, diagCol);
  rbdUtil.invoke187Diagnostic();
  diagCol->flushMsg();
}

void
InternalServiceController::handleDiagnostic188(PricingTrx& trx)
{
  Diag188Collector* diagCol = dynamic_cast<Diag188Collector*>(_dcFactory->create(trx));
  diagCol->enable(Diagnostic188);
  (*diagCol) << trx;
  diagCol->flushMsg();
}

void
InternalServiceController::handleDiagnostic193(PricingTrx& trx)
{
  const CarrierCode* cc = nullptr;
  if (trx.getOptions()->isRtw())
  {
    GoverningCarrier govCxr(&trx);
    FareMarket* rtwFm = trx.itin().front()->fareMarket().front();
    govCxr.processRtw(*rtwFm);
    cc = &rtwFm->governingCarrier();
  }

  DiagCollector* diagCol = dynamic_cast<DiagCollector*>(_dcFactory->create(trx));
  diagCol->enable(Diagnostic193);
  CustomerActivationUtil caUtil(trx, cc);
  caUtil.print193Diagnostic(diagCol);
  diagCol->flushMsg();
}

void
InternalServiceController::handleDiagnostic194(PricingTrx& trx)
{
  Diag194Collector* diagCol = dynamic_cast<Diag194Collector*>(_dcFactory->create(trx));
  diagCol->enable(Diagnostic194);

  if (trx.excTrxType() == PricingTrx::PORT_EXC_TRX)
    (*diagCol) << static_cast<const ExchangePricingTrx&>(trx);

  else if (trx.excTrxType() == PricingTrx::AR_EXC_TRX)
    (*diagCol) << static_cast<const RexPricingTrx&>(trx);

  else if (trx.excTrxType() == PricingTrx::AF_EXC_TRX)
    (*diagCol) << static_cast<const RefundPricingTrx&>(trx);

  else
    (*diagCol) << "***** DIAGNOSTIC 194 PERTAINS TO EXCHANGE/REX TRANSACTIONS ONLY *****\n";

  diagCol->flushMsg();
}

void
InternalServiceController::handleDiagnostic198(PricingTrx& trx)
{
  trx.loadAmVatTaxRatesOnCharges();

  Diag198Collector* diagCol = dynamic_cast<Diag198Collector*>(_dcFactory->create(trx));
  diagCol->enable(Diagnostic198);
  (*diagCol) << trx;
  diagCol->flushMsg();
}

void
InternalServiceController::handleDiagnostic199(PricingTrx& trx)
{
  Diag199Collector* diagCol = dynamic_cast<Diag199Collector*>(_dcFactory->create(trx));
  diagCol->enable(Diagnostic199);
  (*diagCol) << trx;
  diagCol->flushMsg();
}

void
InternalServiceController::handleDiagnostic997(PricingTrx& trx)
{
  Diag997Collector* diagCol = dynamic_cast<Diag997Collector*>(_dcFactory->create(trx));
  diagCol->enable(Diagnostic997);
  (*diagCol) << trx;
  diagCol->flushMsg();
}
} // tse
