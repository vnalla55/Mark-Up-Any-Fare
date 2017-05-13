//----------------------------------------------------------------------------
//  Description: Global TSE Resources
//
//  Copyright Sabre 2013
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

/* Call this macro to log missing or invalid config value
 * usage: CONFIG_MAN_LOG_KEY_ERROR(logger, "SECTION", "KEY", "VALUE")
 */
#define CONFIG_MAN_LOG_KEY_ERROR(logger, key, section)                                             \
  LOG4CXX_ERROR((logger),                                                                          \
                __PRETTY_FUNCTION__ << " - missing " << (section) << "\\" << (key)                 \
                                    << " config value, using default instead");

#define CONFIG_MAN_LOG_SECTION_ERROR(logger, section)                                              \
  LOG4CXX_ERROR((logger), __PRETTY_FUNCTION__ << " - missing " << (section) << " config section");

