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

#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/FOS/DetailedSop.h"
#include "Pricing/Shopping/FOS/FosBaseGenerator.h"
#include "Pricing/Shopping/FOS/FosTypes.h"
#include "Pricing/Shopping/Utils/FosGenerator.h"

#include <list>

namespace tse
{
namespace fos
{
class SolFosGenerator : public FosBaseGenerator
{
public:
  SolFosGenerator(ShoppingTrx& trx,
                  FosFilterComposite& fosFilterComposite,
                  Diag910Collector* dc910 = nullptr)
    : FosBaseGenerator(trx, fosFilterComposite, dc910), _interlineGenerator(nullptr)
  {
  }
  virtual ~SolFosGenerator() {}

  void initGenerators() override;
  void addPredicates() override;

private:
  utils::FosGenerator* _interlineGenerator;
};

} // namespace fos
} // namespace tse
