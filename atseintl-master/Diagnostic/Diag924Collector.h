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

#include "Diagnostic/DiagCollector.h"

namespace tse
{
class PaxTypeFare;
class ShoppingTrx;

namespace shpq
{
class DirFMPathList;
class SoloFmPath;
}

class Diag924Collector : public DiagCollector
{
public:
  Diag924Collector() : _shoppingTrx(nullptr) {}
  explicit Diag924Collector(Diagnostic& root);

  ShoppingTrx*& shoppingTrx() { return _shoppingTrx; }

  Diag924Collector& operator<<(const shpq::SoloFmPath&);

private:
  Diag924Collector& operator<<(const shpq::DirFMPathList&);
  Diag924Collector& operator<<(const PaxTypeFare&) override;

private:
  ShoppingTrx* _shoppingTrx;
};

} // namespace tse

