//----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include <cstddef>
#include <cstdint>

namespace tse
{
namespace Memory
{
extern bool _managerEnabled;
extern uint32_t _updatePeriod;
extern uint32_t _initialCapacity;
extern size_t _globalThreshold;
extern size_t _criticalThreshold;
extern size_t _trxThreshold;
extern size_t _fpStorageCheckPeriod;
extern size_t _dhCheckPeriod;
extern uint32_t _monitorUpdatePeriod;
extern uint32_t _monitorUpdatePeriodGranularity;
extern size_t _criticalFreeRssWatermark;
extern size_t _softFreeRssWatermark;
extern bool _changesFallback;

namespace
{
const bool& managerEnabled = _managerEnabled;
const uint32_t& updatePeriod = _updatePeriod;
const uint32_t& initialCapacity = _initialCapacity;
const size_t& globalThreshold = _globalThreshold;
const size_t& criticalThreshold = _criticalThreshold;
const size_t& trxThreshold = _trxThreshold;
const size_t& fpStorageCheckPeriod = _fpStorageCheckPeriod;
const size_t& dhCheckPeriod = _dhCheckPeriod;
const uint32_t& monitorUpdatePeriod = _monitorUpdatePeriod;
const uint32_t& monitorUpdatePeriodGranularity = _monitorUpdatePeriodGranularity;
const size_t& criticalFreeRssWatermark = _criticalFreeRssWatermark;
const size_t& softFreeRssWatermark = _softFreeRssWatermark;
const bool& changesFallback = _changesFallback;
}

void configure();
}
}
