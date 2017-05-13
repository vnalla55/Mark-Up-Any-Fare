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

#pragma once

#ifdef CONFIG_HIERARCHY_REFACTOR

#include "Common/Config/DynamicConfigurableFallbackValue.h"
#include "Common/Config/FallbackValue.h"

#define FALLBACK_DECL(_name_)                                                                      \
  namespace fallback                                                                               \
  {                                                                                                \
  namespace value                                                                                  \
  {                                                                                                \
  extern DynamicConfigurableFallbackValue _name_;                                                  \
  }                                                                                                \
  inline bool _name_(const Trx* trx) { return value::_name_.get(trx); }                            \
  }

#define FIXEDFALLBACK_DECL(_name_)                                                                 \
  namespace fallback                                                                               \
  {                                                                                                \
  namespace fixed                                                                                  \
  {                                                                                                \
  namespace value                                                                                  \
  {                                                                                                \
  extern FallbackValue _name_;                                                                     \
  }                                                                                                \
  inline bool _name_() { return value::_name_.get(); }                                             \
  }                                                                                                \
  }

#else

#define FALLBACK_DECL(_name_)                                                                      \
  namespace fallback                                                                               \
  {                                                                                                \
  [[gnu::pure]] bool _name_(const Trx* trx);                                                       \
  }

#define FIXEDFALLBACK_DECL(_name_)                                                                 \
  namespace fallback                                                                               \
  {                                                                                                \
  namespace fixed                                                                                  \
  {                                                                                                \
  [[gnu::pure]] bool _name_();                                                                     \
  }                                                                                                \
  }

#endif
