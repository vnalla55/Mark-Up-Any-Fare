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

#include "Pricing/Shopping/FiltersAndPipes/ICollector.h"
#include "Pricing/Shopping/Utils/ShoppingUtilTypes.h" // SopCandidate

#include <boost/utility.hpp>

#include <vector>

namespace tse
{

class ShoppingTrx;

namespace utils
{

// Iterates over SOPs in given trx and sends
// info about each encountered SOP to registered
// collectors.
class SopDistributor : boost::noncopyable
{
public:
  // Register a SOP collector
  void addCollector(ICollector<SopCandidate>* collector);

  // Iterate over SOPs from transaction trx
  // and send information about each SOP
  // to all registered collectors.
  void distributeSops(const ShoppingTrx& trx) const;

private:
  typedef ICollector<SopCandidate> ISopCollector;

  void callCollectorsWithSopCandidate(const SopCandidate& candidate) const;

  std::vector<ISopCollector*> _registeredCollectors;
};

} // namespace utils

} // namespace tse

