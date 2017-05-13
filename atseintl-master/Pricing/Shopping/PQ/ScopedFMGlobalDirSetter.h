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

#include "Common/TseEnums.h"

#include <memory>

namespace tse
{
class ShoppingTrx;
class FareMarket;

class ScopedFMGlobalDirSetter;
typedef std::shared_ptr<ScopedFMGlobalDirSetter> ScopedFMGlobalDirSetterPtr;

/*
 * The purpose of this class is to set a correct global direction in fare market in a particular
 * scope.
 * It will store old fare market global direction and restore it during destruction.
 */
class ScopedFMGlobalDirSetter
{
public:
  ScopedFMGlobalDirSetter(ShoppingTrx* trx,
                          FareMarket* fareMarket,
                          const GlobalDirection* globalDir = nullptr);
  ~ScopedFMGlobalDirSetter();

private:
  void set(const GlobalDirection* globalDirection);

private:
  ShoppingTrx* _trx;
  GlobalDirection _globalDirection;
  FareMarket* _fareMarket;
};
}

