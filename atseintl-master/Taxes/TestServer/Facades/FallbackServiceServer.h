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

#include "AtpcoTaxes/ServiceInterfaces/FallbackService.h"

namespace tse
{
class Trx;
#ifndef ATPCO_FALLBACKS_DEFS
#define ATPCO_FALLBACKS_DEFS

#define ATPCO_FALLBACK_DEF(_name_)      \
  namespace fallback                    \
  {                                     \
  bool _name_(const tse::Trx* /*trx*/)  \
  {                                     \
    return false;                       \
  }                                     \
  }

#define ATPCO_FIXEDFALLBACK_DEF(_name_) \
  namespace fallback                    \
  {                                     \
  namespace fixed                       \
  {                                     \
  bool _name_()                         \
  {                                     \
    return false;                       \
  }                                     \
  }                                     \
  }

#endif
}

namespace tax
{

class FallbackServiceServer : public FallbackService
{
public:
  FallbackServiceServer() {}
  ~FallbackServiceServer() override {}

  bool isSet(const std::function<bool(const tse::Trx*)>& fallbackFunc) const override;
  bool isSet(const std::function<bool()>& fallbackFunc) const override;
};
}
