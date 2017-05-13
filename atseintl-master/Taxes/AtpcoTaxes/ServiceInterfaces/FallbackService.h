// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#pragma once

#include <functional>

namespace tse
{
class Trx;
#ifndef ATPCO_FALLBACKS
#define ATPCO_FALLBACKS


#ifdef CONFIG_HIERARCHY_REFACTOR

#define ATPCO_FALLBACK_DECL(_name_)      \
  namespace fallback                     \
  {                                      \
  bool _name_##_old(const Trx* trx);     \
  inline bool _name_(const Trx* trx) { return _name_##_old(trx); } \
  }

#define ATPCO_FIXEDFALLBACK_DECL(_name_) \
  namespace fallback                     \
  {                                      \
  namespace fixed                        \
  {                                      \
  bool _name_##_old();                   \
  inline bool _name_() { return _name_##_old(); } \
  }                                      \
  }

#else

#define ATPCO_FALLBACK_DECL(_name_)      \
  namespace fallback                     \
  {                                      \
  bool _name_(const Trx* trx);           \
  }

#define ATPCO_FIXEDFALLBACK_DECL(_name_) \
  namespace fallback                     \
  {                                      \
  namespace fixed                        \
  {                                      \
  bool _name_();                         \
  }                                      \
  }

#endif

#endif
}

namespace tax
{

class FallbackService
{
public:
  FallbackService() {}
  virtual ~FallbackService() {}

  virtual bool isSet(const std::function<bool(const tse::Trx*)>& fallbackFunc) const = 0;
  virtual bool isSet(const std::function<bool()>& fallbackFunc) const = 0;
};
}

