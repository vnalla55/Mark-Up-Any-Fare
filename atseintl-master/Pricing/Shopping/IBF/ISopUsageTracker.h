//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2014
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

#include "Pricing/Shopping/Utils/ShoppingUtilTypes.h"

#include <map>
#include <string>

namespace tse
{

class ISopUsageCounter
{
public:
  typedef std::map<unsigned int, unsigned int> SopUsages;

  virtual unsigned int getUsageCount(unsigned int legId, uint32_t sopId) const = 0;
  virtual SopUsages getSopUsagesOnLeg(unsigned int legId) const = 0;
  virtual ~ISopUsageCounter() {}
};

class ISopUsageTracker
{
public:
  virtual bool combinationAdded(const utils::SopCombination& combination) = 0;
  virtual void combinationRemoved(const utils::SopCombination& combination) = 0;

  // Returns the number of SOPs tracked
  virtual unsigned int getNbrOfSops() const = 0;

  // Returns the number of SOPs not used in any combination
  virtual unsigned int getNbrOfUnusedSops() const = 0;

  //return the number of SOPs not used on single leg
  virtual unsigned int getNbrOfUnusedSopsOnLeg(unsigned int legId) const = 0;

  // Returns the string representation of the tracker
  virtual std::string toString() const = 0;

  virtual ~ISopUsageTracker() {}
};


} // namespace tse


