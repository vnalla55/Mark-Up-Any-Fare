// ----------------------------------------------------------------
//
//   Copyright Sabre 2011
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

#include <memory>

namespace tse
{

class ShoppingTrx;

namespace shpq
{

class SoloFmPath;

class SoloFmPathOrchestrator
{
public:
  typedef std::shared_ptr<SoloFmPath> SoloFmPathPtr;

  SoloFmPathOrchestrator(ShoppingTrx& trx) : _trx(trx) {}
  void process(SoloFmPathPtr);

private:
  ShoppingTrx& _trx;
};
}
} // namespace tse::shpq

