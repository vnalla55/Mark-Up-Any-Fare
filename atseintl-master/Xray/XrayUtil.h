//----------------------------------------------------------------------------
//
//  File:               XRayUtil.h
//  Description:        Utilities for Xray
//  Created:            07/29/2016
//  Authors:            Grzegorz Ryniak
//
//  Copyright Sabre 2016
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

#include "Common/Config/ConfigurableValue.h"
#include "Common/Config/DynamicConfigurableFlag.h"

namespace tse
{
namespace xray
{
extern DynamicConfigurableFlagOn xrayEnabledCfg;
extern ConfigurableValue<std::string> xrayServerAddrCfg;
extern ConfigurableValue<uint16_t> xrayServerPortCfg;
extern ConfigurableValue<std::string> authServerAddrCfg;
extern ConfigurableValue<uint16_t> authServerPortCfg;
extern ConfigurableValue<std::string> authUserCfg;
extern ConfigurableValue<std::string> authPassCfg;
extern ConfigurableValue<bool> directConnection;

namespace asyncjsoncontainer
{
extern ConfigurableValue<uint32_t> maxNumberOfMessages;
extern ConfigurableValue<uint32_t> containerThreadSendTimeout;
}
}
}
