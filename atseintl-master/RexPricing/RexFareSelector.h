//-------------------------------------------------------------------
//
//  File:        RexFareSelector.h
//  Created:     April 16, 2007
//  Authors:     Artur Krezel
//
//  Updates:
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

#pragma once

#include "Common/Logger.h"
#include "Common/TsePrimitiveTypes.h"
#include "RexPricing/PreSelectedFaresStore.h"
#include "RexPricing/RexFareSelectorBasicStrategy.h"
#include "RexPricing/RexFareSelectorDiscountedStrategy.h"
#include "RexPricing/RexFareSelectorHipStrategy.h"
#include "RexPricing/RexFareSelectorVarianceStrategy.h"
#include "RexPricing/RexFareSelectorVCTRStrategy.h"

#include <set>

namespace tse
{
class Diag23XCollector;

class RexFareSelector
{
  friend class RexFareSelectorTest;

public:
  RexFareSelector(RexBaseTrx& trx);

  virtual ~RexFareSelector() = default;

  static uint32_t getActiveThreads();

  void process();

  void retrieveVCTRTask(FareCompInfo& fareCompInfo);

private:
  enum class Strategy
  { VCTR,
    BASIC,
    VARIANCE,
    DISCOUNTED,
    HIP };

  RexBaseTrx& _trx;
  const uint16_t _subjectCategory;

  void invalidateClones(const FareMarket& originalFm,
                        std::vector<FareMarket*>::const_iterator dupFmIter,
                        const std::vector<FareMarket*>::const_iterator exchFmIterEnd);
  void selectSecondaryFareMarket();
  bool isSecondaryFareMarket(const FareCompInfo& fci) const;

  Strategy chooseStrategy(FareCompInfo& fc) const;
  const RexFareSelectorStrategy& getSelector(Strategy strategy) const;

  static Logger _logger;

  void processAllFareCompInfos();

  Service* getServicePointer(const std::string& serviceName) const;

  virtual bool runFareCollector(Service& fareCollector) const;

  void displayDiagnostics();
  void displayFareCompInfo(Diag23XCollector& diag, FareCompInfo& fc);
  Diag23XCollector* getActiveDiag();

  void initializeMatchedFare(PaxTypeFare* ptf);

  void setCalculationCurrencyNoDecForExcItin() const;

  bool checkIfNewItinHasDiscount();

  void processDateSequence();

  void storeVCTR();
  void restoreVCTR();

  void updateFCAmount(FareCompInfo& fc);

  bool _lastChanceForVariance;

  bool _discountOnNewItinFound;

  PreSelectedFaresStore _preSelectedFares;

  const RexFareSelectorVCTRStrategy _selectorVCTR;
  const RexFareSelectorBasicStrategy _selectorBasic;
  const RexFareSelectorVarianceStrategy _selectorVariance;
  const RexFareSelectorDiscountedStrategy _selectorDiscounted;
  const RexFareSelectorHipStrategy _selectorHip;

  bool _areAllFCsProcessed;
  std::vector<std::pair<VCTR, bool>> _vctrStorage;
};

} // namespace tse
