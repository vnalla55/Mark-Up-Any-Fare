// vim:ts=2:sts=2:sw=2:cin:et
// -------------------------------------------------------------------
//
//! \author       Robert Luberda
//! \date         25-11-2011
//! \file         SoloTrxData.cpp
//! \brief
//!
//!  Copyright (C) Sabre 2011
//!
//!          The copyright to the computer program(s) herein
//!          is the property of Sabre.
//!          The program(s) may be used and/or copied only with
//!          the written permission of Sabre or in accordance
//!          with the terms and conditions stipulated in the
//!          agreement/contract under which the program(s)
//!          have been supplied.
//
// -------------------------------------------------------------------
#include "Pricing/Shopping/PQ/SoloTrxData.h"

#include "Common/Assert.h"
#include "Common/Config/ConfigurableValue.h"
#include "Diagnostic/DCFactory.h"
#include "Pricing/PricingOrchestrator.h"

namespace tse
{
namespace
{
ConfigurableValue<bool>
crTuning1("SHOPPING_DIVERSITY", "CR_TUNING_1", true);
ConfigurableValue<bool>
crTuning2("SHOPPING_DIVERSITY", "CR_TUNING_2", true);
ConfigurableValue<bool>
crTuningTag2("SHOPPING_DIVERSITY", "CR_TUNING_TAG2", true);
}
namespace shpq
{
SoloTrxData::SoloTrxData(ShoppingTrx& shoppingTrx, PricingOrchestrator& pricingOrchestrator)
  : _shoppingTrx(shoppingTrx),
    _factoriesConfig(pricingOrchestrator.getFactoriesConfig()),
    _paxFPFBaseData(pricingOrchestrator.getFactoriesConfig()),
    _puFactoryWrapper(shoppingTrx, pricingOrchestrator),
    _diagCollector(DCFactory::instance()->create(shoppingTrx)),
    _shoppingDC(DCFactory::instance()->create(shoppingTrx)),
    _crTuning1(crTuning1.getValue()),
    _crTuning2(crTuning2.getValue()),
    _crTuningTag2(crTuningTag2.getValue())
{
  TSE_ASSERT(_diagCollector);
}

SoloTrxData::~SoloTrxData()
{
  _diagCollector->flushMsg();
  _shoppingDC->flushMsg();
}

DiagCollector*
SoloTrxData::getShoppingDC(DiagnosticTypes dt) const
{
  if (getShoppingTrx().diagnostic().diagnosticType() == dt)
    return _shoppingDC;
  return nullptr;
}

void
SoloTrxData::setUpDiags(const bool enable)
{
  const DiagnosticTypes& diagType((getTrx().diagnostic().diagnosticType()));

  switch (diagType)
  {
  // pricing diags
  case Diagnostic600:
  case Diagnostic601:
  case Diagnostic603:
  case Diagnostic605:
  case Diagnostic661:
    setUpDiag(getDiagCollector(), enable);
    break;

  // shopping diags
  case Diagnostic941:
  case Diagnostic942:
  {
    setUpDiag(getShoppingDC(), enable);
    break;
  }
  default:
    break;
  }
}

void
SoloTrxData::setUpDiag(DiagCollector& dc, bool enable)
{
  if (enable)
  {
    dc.enable(getTrx().diagnostic().diagnosticType());
    dc.printHeader();
  }
  else
  {
    dc.flushMsg();
    dc.disable(getTrx().diagnostic().diagnosticType());
  }
}
}
}
