//----------------------------------------------------------------------------
//
//  Copyright Sabre 2011
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

#ifndef TEST_FALLBACK_UTIL_H
#define TEST_FALLBACK_UTIL_H

#include "Common/Config/DynamicConfigurableFallbackValue.h"
#include "Common/Config/FallbackValue.h"

#define FALLBACKVALUE_DECL(_name_)                                                                 \
  namespace fallback                                                                               \
  {                                                                                                \
  namespace value                                                                                  \
  {                                                                                                \
  extern DynamicConfigurableFallbackValue _name_;                                                  \
  }                                                                                                \
  }

#define FIXEDFALLBACKVALUE_DECL(_name_)                                                            \
  namespace fallback                                                                               \
  {                                                                                                \
  namespace fixed                                                                                  \
  {                                                                                                \
  namespace value                                                                                  \
  {                                                                                                \
  extern FallbackValue _name_;                                                                     \
  }                                                                                                \
  }                                                                                                \
  }

#endif
