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

#pragma once

#include "Pricing/Shopping/FiltersAndPipes/GeneratingFilter.h"
#include "Pricing/Shopping/FiltersAndPipes/IFilterObserver.h"
#include "Pricing/Shopping/FiltersAndPipes/IGenerator.h"
#include "Pricing/Shopping/FiltersAndPipes/IPredicate.h"
#include "Pricing/Shopping/Utils/ShoppingUtilTypes.h"
#include "Pricing/Shopping/Utils/SopCartesianGenerator.h"

#include <boost/utility.hpp>

#include <string>
#include <vector>

namespace tse
{

class ShoppingTrx;

namespace utils
{

// Generates Flight Only Solutions, filtering SOP
// combinations according to criteria:
// a) minimum connect time
// b) cabin class validity
// c) interline ticketing agreement

// Use the constructor without parameters
// and use addPredicate to install custom predicates
// for own criteria
class FosGenerator : public IGenerator<SopCombination>, boost::noncopyable
{

public:
  // Creates a generator without predicates.
  // Use addPredicate to add custom predicates.
  FosGenerator(ShoppingTrx& trx);

  void addPredicate(IPredicate<SopCombination>* predicate, const std::string& predicateName);

  void setNumberOfLegs(unsigned int legs);
  unsigned int getNumberOfLegs() const;

  void addSop(unsigned int legId, uint32_t sopId);
  unsigned int getSopsNumberOnLeg(unsigned int legId) const;

  SopCombination next() override;

  // Adds an observer notified about failed combinations
  void addObserver(IFilterObserver<SopCombination>* observer);

private:
  ShoppingTrx& _trx;
  SopCartesianGenerator _generator;
  GeneratingFilter<SopCombination> _filter;
};

} // namespace utils

} // namespace tse

