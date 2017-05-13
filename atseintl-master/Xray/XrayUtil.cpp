// ----------------------------------------------------------------
//
//   Copyright Sabre 2016
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#include "Xray/XrayUtil.h"

namespace tse
{
namespace xray
{
DynamicConfigurableFlagOn
xrayEnabledCfg("XRAY", "FEATURE_ENABLE", false);
ConfigurableValue<std::string>
xrayServerAddrCfg("XRAY", "SERVER_ADDRESS");
ConfigurableValue<uint16_t>
xrayServerPortCfg("XRAY", "SERVER_PORT");
ConfigurableValue<std::string>
authServerAddrCfg("XRAY", "AUTH_SERVER_ADDRESS");
ConfigurableValue<uint16_t>
authServerPortCfg("XRAY", "AUTH_SERVER_PORT");
ConfigurableValue<std::string>
authUserCfg("XRAY", "AUTH_USER");
ConfigurableValue<std::string>
authPassCfg("XRAY", "AUTH_PASS");
ConfigurableValue<bool>
directConnection("XRAY", "DIRECT_CONNECTION", false);

namespace asyncjsoncontainer
{
ConfigurableValue<uint32_t>
maxNumberOfMessages("XRAY", "MAX_NUMBER_OF_MESSAGES", 10000);
ConfigurableValue<uint32_t>
containerThreadSendTimeout("XRAY", "CONTAINER_THREAD_SEND_TIMEOUT", 3600);
}
}
}
