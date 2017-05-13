// vim:ts=2:sts=2:sw=2:cin:et
// -------------------------------------------------------------------
//
//! \author       Robert Luberda
//! \date         22-11-2011
//! \file         SoloTrxData.h
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

#pragma once

#include "DataModel/ShoppingTrx.h"
#include "Pricing/PaxFPFBaseData.h"
#include "Pricing/Shopping/PQ/SoloPQItemManager.h"
#include "Pricing/Shopping/PQ/SoloPUFactoryWrapper.h"

namespace tse
{
class FactoriesConfig;
class PricingOrchestrator;
class DiagCollector;
class PaxFPFBaseData;
}

namespace tse
{
namespace shpq
{

class SoloTrxData
{
public:
  SoloTrxData(ShoppingTrx& shoppingTrx, PricingOrchestrator& pricingOrchestrator);

  ~SoloTrxData();

  Trx& getTrx() const { return _shoppingTrx; }
  ShoppingTrx& getShoppingTrx() const { return _shoppingTrx; }

  SoloPUFactoryWrapper& getPUFactoryWrapper() { return _puFactoryWrapper; }

  const FactoriesConfig& getFactoriesConfig() const { return _factoriesConfig; }

  PaxFPFBaseData& getPaxFPFBaseData() { return _paxFPFBaseData; }

  DiagCollector& getDiagCollector() const { return *_diagCollector; }
  DiagCollector& getShoppingDC() const { return *_shoppingDC; }
  DiagCollector* getShoppingDC(DiagnosticTypes dt) const;
  void setUpDiags(const bool enable);

  SoloPQItemManager& getPQItemManager() { return _pqItemManager; }

  bool isCRTuning1Active() const { return _crTuning1; }
  bool isCRTuning2Active() const { return _crTuning2; }
  bool isCRTuningTag2Active() const { return _crTuningTag2; }

private:
  void setUpDiag(DiagCollector& dc, bool enable);

  ShoppingTrx& _shoppingTrx;
  const FactoriesConfig& _factoriesConfig;
  PaxFPFBaseData _paxFPFBaseData;
  SoloPUFactoryWrapper _puFactoryWrapper;
  DiagCollector* _diagCollector;
  DiagCollector* _shoppingDC;
  SoloPQItemManager _pqItemManager;
  bool _crTuning1;
  bool _crTuning2;
  bool _crTuningTag2;
};

} /* namespace shpq */
} /* namespace tse */
