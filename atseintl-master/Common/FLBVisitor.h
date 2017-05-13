// ----------------------------------------------------------------
//
//   Copyright Sabre 2012
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
#pragma once

#include "DataModel/FareMarket.h"

#include <cstddef>
#include <cstdint>

namespace tse
{
class FarePath;
class PaxTypeFare;
class ShoppingTrx;

class FLBVisitor
{
public:
  enum WhatNext
  { CONTINUE,
    NEXT_PTF,
    STOP };
  /**
   * In spite of flbIdx is passed, FLBVisitor must check if PTF has empty flight bitmap
   * (because of invalid fare or flight independent validation has not been called yet)
   */
  virtual WhatNext visit(PaxTypeFare& ptf,
                         uint16_t legIndex,
                         size_t flbIdx,
                         uint32_t carrierKey,
                         const SOPUsages& sopUsages) = 0;
  virtual ~FLBVisitor() {}

  static void apply(ShoppingTrx& trx, FarePath& fp, FLBVisitor& visitor);
};

} // ns tse

