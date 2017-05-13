//----------------------------------------------------------------------------
//  Copyright Sabre 2014
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
#include "Xform/TaxNewShoppingRequestHandler.h"

#include "DataModel/Billing.h"
#include "DataModel/TaxTrx.h"

namespace tse
{

TaxNewShoppingRequestHandler::TaxNewShoppingRequestHandler(Trx*& trx) : _trx(trx) {}

TaxNewShoppingRequestHandler::~TaxNewShoppingRequestHandler() {}

void
TaxNewShoppingRequestHandler::parse(DataHandle& dataHandle, const std::string& /*content*/)
{
  TaxTrx* taxTrx = dataHandle.create<TaxTrx>();

  taxTrx->requestType() = NEW_OTA_REQUEST;
  taxTrx->otaXmlVersion() = "1.0.0";
  taxTrx->setShoppingPath(true);
  taxTrx->otaRequestRootElementType() = "TAX";

  Billing* billing = nullptr;
  taxTrx->dataHandle().get(billing);
  taxTrx->billing() = billing;

  taxTrx->billing()->parentServiceName() = taxTrx->billing()->getServiceName(Billing::SVC_TAX);
  taxTrx->billing()->updateTransactionIds(taxTrx->transactionId());
  taxTrx->billing()->updateServiceNames(Billing::SVC_TAX);

  _trx = taxTrx;
}

} // namespace tse
