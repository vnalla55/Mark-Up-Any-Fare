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

#include "Common/Assert.h"
#include "Pricing/Shopping/Utils/ShoppingUtilTypes.h"

#include <vector>

namespace tse
{

namespace utils
{

class SopCombinationDeconstructor
{
public:
  typedef SopCombination ItemType;
  typedef SopEntry PartType;
  typedef std::vector<PartType> PartVector;

  void insertItem(const SopCombination& comb)
  {
    _parts.clear();
    for (unsigned int i = 0; i < comb.size(); ++i)
    {
      _parts.push_back(SopEntry(i, comb[i]));
    }
  }

  const PartVector& getItemParts() const { return _parts; }

private:
  PartVector _parts;
};

} // namespace utils

} // namespace tse

