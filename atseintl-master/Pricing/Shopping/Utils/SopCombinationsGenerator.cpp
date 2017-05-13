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

#include "Pricing/Shopping/Utils/SopCombinationsGenerator.h"

#include "Common/Assert.h"
#include "Pricing/Shopping/Utils/PrettyPrint.h"
#include "Pricing/Shopping/Utils/StreamFormat.h"

#include <sstream>

namespace tse
{

namespace utils
{

void SopBank::setNumberOfLegs(unsigned int legs)
{
  TSE_ASSERT(legs > 0);
  _sops.resize(legs);
}

unsigned int SopBank::getNumberOfLegs() const
{
  return _sops.size();
}

void SopBank::addSop(unsigned int legId, uint32_t sopId)
{
  TSE_ASSERT(legId < _sops.size());
  _sops[legId].push_back(sopId);
}

// Returns a list of SOPs on the given leg
SopCombination& SopBank::getSopsOnLeg(unsigned int legId)
{
  TSE_ASSERT(legId < _sops.size());
  return _sops[legId];
}

std::string SopBank::toString() const
{
  std::ostringstream out;
  out << _sops;
  return out.str();
}


void BaseSopCombinationsGenerator::setNumberOfLegs(unsigned int legs)
{
  _userInputSops->setNumberOfLegs(legs);
}

unsigned int BaseSopCombinationsGenerator::getNumberOfLegs() const
{
  return _userInputSops->getNumberOfLegs();
}

void BaseSopCombinationsGenerator::addSop(unsigned int legId, uint32_t sopId)
{
  _userInputSops->addSop(legId, sopId);
}

// Returns a list of SOPs on the given leg
const SopCombination& BaseSopCombinationsGenerator::getSopsOnLeg(unsigned int legId) const
{
  return _userInputSops->getSopsOnLeg(legId);
}

void BaseSopCombinationsGenerator::manualInit()
{
  if (!_isInitialized)
  {
    initialize();
    _isInitialized = true;
  }
}

SopCombination BaseSopCombinationsGenerator::next()
{
  // Transfer SOPs to the generator at the first call
  manualInit();
  return nextElement();
}


void formatSopCombinationsGenerator(std::ostream& out,
    const ISopCombinationsGenerator& g)
{
  BlockMaker b(8);
  out << b("Leg") << b("#SOPs") << "SOPs" << std::endl;
  for (unsigned int i = 0; i < g.getNumberOfLegs(); ++i)
  {
    out << b(i) << b(g.getSopsOnLeg(i).size()) << b(g.getSopsOnLeg(i)) << std::endl;
  }
}


std::ostream& operator<<(std::ostream& out,
    const ISopCombinationsGenerator& g)
{
  out << tse::utils::format(g, formatSopCombinationsGenerator);
  return out;
}


} // namespace utils

} // namespace tse
