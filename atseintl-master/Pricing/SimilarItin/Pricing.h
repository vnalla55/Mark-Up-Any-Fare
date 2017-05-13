/*----------------------------------------------------------------------------
 *  Copyright Sabre 2015
 *
 *     The copyright to the computer program(s) herein
 *     is the property of Sabre.
 *     The program(s) may be used and/or copied only with
 *     the written permission of Sabre or in accordance
 *     with the terms and conditions stipulated in the
 *     agreement/contract under which the program(s)
 *     have been supplied.
 *-------------------------------------------------------------------------*/
#pragma once

#include <vector>

namespace tse
{
class FPPQItem;
class GroupFarePath;
class GroupFarePathFactory;

namespace similaritin
{
struct Context;

template <typename D>
class Pricing
{
public:
  Pricing(Context& context, D& diagnostic) : _context(context), _diagnostic(diagnostic) {}
  void price(GroupFarePathFactory& gfpf, const std::vector<FPPQItem*>& groupFPath);

private:
  void priceWithMother(GroupFarePathFactory&);

  Context& _context;
  D& _diagnostic;
};
}
} // tse namespace
