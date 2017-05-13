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

#include "Taxes/AtpcoTaxes/ServiceInterfaces/FallbackService.h"

namespace tse
{
class Trx;

class FallbackServiceV2 : public tax::FallbackService
{
public:
  FallbackServiceV2(const Trx& trx) : _trx(trx) {}
  FallbackServiceV2(const FallbackServiceV2&) = delete;
  FallbackServiceV2& operator=(const FallbackServiceV2&) = delete;
  ~FallbackServiceV2() override {}

  bool isSet(const std::function<bool(const tse::Trx*)>& fallbackFunc) const override;
  bool isSet(const std::function<bool()>& fallbackFunc) const override;

private:
  const Trx& _trx;
};

} // namespace tse

