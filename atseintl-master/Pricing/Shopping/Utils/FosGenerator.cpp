//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "Pricing/Shopping/Utils/FosGenerator.h"

#include "Common/Assert.h"
#include "Pricing/Shopping/FiltersAndPipes/NamedPredicateWrapper.h"

namespace tse
{

namespace utils
{

FosGenerator::FosGenerator(ShoppingTrx& trx) : _trx(trx), _filter(_generator) {}

void
FosGenerator::addPredicate(IPredicate<SopCombination>* predicate, const std::string& predicateName)
{
  _filter.addPredicate(wrapPredicateWithName(predicate, predicateName, _trx));
}

void
FosGenerator::setNumberOfLegs(unsigned int legs)
{
  _generator.setNumberOfLegs(legs);
}

unsigned int
FosGenerator::getNumberOfLegs() const
{
  return _generator.getNumberOfLegs();
}

void
FosGenerator::addSop(unsigned int legId, uint32_t sopId)
{
  _generator.addSop(legId, sopId);
}

unsigned int
FosGenerator::getSopsNumberOnLeg(unsigned int legId) const
{
  return _generator.getSopsOnLeg(legId).size();
}

SopCombination
FosGenerator::next()
{
  return _filter.next(_trx);
}

void
FosGenerator::addObserver(IFilterObserver<SopCombination>* observer)
{
  TSE_ASSERT(observer != nullptr);
  _filter.addObserver(observer);
}

} // namespace utils

} // namespace tse
