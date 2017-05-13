// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
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

#include "Pricing/Shopping/FOS/FosBaseGenerator.h"

#include <list>

namespace tse
{
class ShoppingTrx;
class FosFilterComposite;

namespace fos
{

class AdditionalDirectFosGenerator : public FosBaseGenerator
{
public:
  AdditionalDirectFosGenerator(ShoppingTrx& trx,
                               FosFilterComposite& fosFilterComposite,
                               Diag910Collector* dc910 = nullptr)
    : FosBaseGenerator(trx, fosFilterComposite, dc910)
  {
  }
  virtual ~AdditionalDirectFosGenerator() {}

  void initGenerators() override;
  void addPredicates() override;
};

} // namespace fos
} // namespace tse
