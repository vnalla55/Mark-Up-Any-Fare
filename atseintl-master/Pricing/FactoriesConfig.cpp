// vim:ts=2:sts=2:sw=2:cin:et
// -------------------------------------------------------------------
//
//! \author       Robert Luberda
//! \date         18-11-2011
//! \file         FactoriesConfig.cpp
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

#include "Pricing/FactoriesConfig.h"

#include "Common/Config/ConfigurableValue.h"

#include <string>

namespace tse
{
namespace
{
ConfigurableValue<int32_t>
multiPaxShortCktTimeOutCfg("PRICING_SVC", "MULTI_PAX_SHORT_CKT_TIMEOUT", 30);
ConfigurableValue<bool>
puScopeValEnabledCfg("PRICING_SVC", "PU_SCOPE_VALIDATION_ENABLED", true);
ConfigurableValue<bool>
eoeTuningEnabledCfg("PRICING_SVC", "EOE_TUNING_ENABLED", true);
ConfigurableValue<bool>
searchAlwaysLowToHighCfg("PRICING_SVC", "SEARCH_ALWAYS_LOW_TO_HIGH", false);
ConfigurableValue<bool>
enableCxrFareSearchTuningCfg("PRICING_SVC", "ENABLE_CXR_FARE_SEARCH_TUNING", true);
ConfigurableValue<uint32_t>
maxNbrCombMsgThresholdCfg("PRICING_SVC", "MAX_NBR_COMBO_MSG_THRESHOLD", 5000);
ConfigurableValue<int32_t>
plusUpPushBackThresholdCfg("PRICING_SVC", "PLUS_UP_PUSH_BACK_THRESHOLD", 100);
ConfigurableValue<int32_t>
plusUpPushBackMaxCfg("PRICING_SVC", "PLUS_UP_PUSH_BACK_MAX", 10000);
ConfigurableValue<int32_t>
globalPushBackMaxCfg("PRICING_SVC", "PER_PAX_TYPE_GLOBAL_PUSH_BACK_MAX", -1);
ConfigurableValue<bool>
avoidStaticObjectPoolCfg("PRICING_SVC", "AVOID_STATIC_OBJECT_POOL", true);
}
// for unit tests only
FactoriesConfig::FactoriesConfig()
  : _maxNbrCombMsgThreshold(5000),
    _multiPaxShortCktTimeOut(30),
    _plusUpPushBackThreshold(100),
    _plusUpPushBackMax(10000),
    _globalPushBackMax(-1),
    _puScopeValidationEnabled(true),
    _eoeTuningEnabled(true),
    _searchAlwaysLowToHigh(false),
    _enableCxrFareSearchTuning(true),
    _avoidStaticObjectPool(true)
{
}

FactoriesConfig::FactoriesConfig(const tse::ConfigMan& config)
  : _maxNbrCombMsgThreshold(5000),
    _multiPaxShortCktTimeOut(30),
    _plusUpPushBackThreshold(100),
    _plusUpPushBackMax(10000),
    _globalPushBackMax(-1),
    _puScopeValidationEnabled(true),
    _eoeTuningEnabled(true),
    _searchAlwaysLowToHigh(false),
    _enableCxrFareSearchTuning(true),
    _avoidStaticObjectPool(true)
{
  _multiPaxShortCktTimeOut = multiPaxShortCktTimeOutCfg.getValue();
  _puScopeValidationEnabled = puScopeValEnabledCfg.getValue();
  _eoeTuningEnabled = eoeTuningEnabledCfg.getValue();
  // If this param is 'Y', search low to high combination and if it is not CXR-Fare combo
  // then see if there is any valid CXR-fare combination to over ride this one.
  // Otherwise, arrange the fare in a way that CXR-fare combo are search first to make sure
  // carrier fare prefenrece is honored.
  //
  _searchAlwaysLowToHigh = searchAlwaysLowToHighCfg.getValue();
  _enableCxrFareSearchTuning = enableCxrFareSearchTuningCfg.getValue();
  _maxNbrCombMsgThreshold = maxNbrCombMsgThresholdCfg.getValue();
  _plusUpPushBackThreshold = plusUpPushBackThresholdCfg.getValue();
  _plusUpPushBackMax = plusUpPushBackMaxCfg.getValue();
  _globalPushBackMax = globalPushBackMaxCfg.getValue();
  _avoidStaticObjectPool = avoidStaticObjectPoolCfg.getValue();
}

} /* namespace tse */
