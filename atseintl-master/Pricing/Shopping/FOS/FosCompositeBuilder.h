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


#include <set>

namespace tse
{
class ShoppingTrx;
class Diag910Collector;

namespace fos
{

class FosTaskScope;
class FosBaseGenerator;
class FosStatistic;
class FosValidatorComposite;
class FosFilterComposite;
class FosMatrixInserter;

class FosCompositeBuilder
{
public:
  FosCompositeBuilder(ShoppingTrx& trx, Diag910Collector* dc910) : _trx(trx), _dc910(dc910) {}

  void buildFosFilterComposite(const FosTaskScope& task, FosFilterComposite& filterComposite);

  void buildFosValidatorComposite(const FosTaskScope& task,
                                  FosBaseGenerator& generator,
                                  FosStatistic& stats,
                                  FosValidatorComposite& validatorComposite);

  FosMatrixInserter* createFosMatrixInserter(const FosTaskScope& task);

private:
  ShoppingTrx& _trx;
  Diag910Collector* _dc910;
};

} // fos
} // tse
