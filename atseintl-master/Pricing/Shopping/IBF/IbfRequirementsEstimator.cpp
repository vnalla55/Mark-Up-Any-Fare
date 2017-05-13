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

#include "Pricing/Shopping/IBF/IbfRequirementsEstimator.h"

#include "Common/Assert.h"
#include "Pricing/Shopping/Utils/PrettyPrint.h"

#include <algorithm>
#include <sstream>

namespace tse
{

IbfRequirementsEstimator::IbfRequirementsEstimator()
  : _reqSolutionsCount(0),
    _directFosCount(UNDEFINED),
    _rcoDirectFosCount(UNDEFINED),
    _legsCount(0),
    _optionsCountToCoverAllSops(UNDEFINED),
    _rcoMax(UNDEFINED),
    _remainingRco(UNDEFINED)
{
}

void
IbfRequirementsEstimator::setRequiredSolutionsCount(unsigned int reqSolutionsCount)
{
  TSE_ASSERT(reqSolutionsCount > 0);
  TSE_ASSERT(reqSolutionsCount != UNDEFINED);
  _reqSolutionsCount = reqSolutionsCount;
}

void
IbfRequirementsEstimator::setDirectFosCount(unsigned int directFosCount)
{
  TSE_ASSERT(directFosCount != UNDEFINED);
  _directFosCount = directFosCount;
}

void
IbfRequirementsEstimator::setRcoDirectFosCount(unsigned int rcoDirectFosCount)
{
  TSE_ASSERT(rcoDirectFosCount != UNDEFINED);
  _rcoDirectFosCount = rcoDirectFosCount;
}

void
IbfRequirementsEstimator::setLegsCount(unsigned int legsCount)
{
  TSE_ASSERT(legsCount > 0);
  _legsCount = legsCount;
  // If expanding capacity, set new elements
  // to UNDEFINED to mark that new information needs
  // filling up
  _unusedSopsPerLeg.resize(legsCount, UNDEFINED);
  _rcoSopsCountPerLeg.resize(legsCount, UNDEFINED);
}

void
IbfRequirementsEstimator::setUnusedSopsCount(unsigned int legId, unsigned int unusedSops)
{
  TSE_ASSERT(legId < _unusedSopsPerLeg.size());
  TSE_ASSERT(unusedSops != UNDEFINED);
  _unusedSopsPerLeg[legId] = unusedSops;
}

void
IbfRequirementsEstimator::setRcoSopsCount(unsigned int legId, unsigned int rcoSopsCount)
{
  TSE_ASSERT(legId < _rcoSopsCountPerLeg.size());
  TSE_ASSERT(rcoSopsCount != UNDEFINED);
  _rcoSopsCountPerLeg[legId] = rcoSopsCount;
}

void
IbfRequirementsEstimator::estimateRemainingRcoCount()
{
  checkIfAllDataSupplied();
  checkIfDataValid();
  TSE_ASSERT(_unusedSopsPerLeg.size() == _rcoSopsCountPerLeg.size());

  _optionsCountToCoverAllSops = estimateOptionsCountToCoverAllSops();
  _rcoMax = estimateRcoMax();
  _remainingRco = rRcoCountActualEstimate();
}

unsigned int
IbfRequirementsEstimator::getEstimatedRemainingRco() const
{
  // Check if calculated
  TSE_ASSERT(_remainingRco != UNDEFINED);
  return _remainingRco;
}

unsigned int
IbfRequirementsEstimator::getEstimatedOptionsCountToCoverAllSops() const
{
  // Check if calculated
  TSE_ASSERT(_optionsCountToCoverAllSops != UNDEFINED);
  return _optionsCountToCoverAllSops;
}

unsigned int
IbfRequirementsEstimator::getEstimatedRcoMax() const
{
  // Check if calculated
  TSE_ASSERT(_rcoMax != UNDEFINED);
  return _rcoMax;
}

void
IbfRequirementsEstimator::checkIfAllDataSupplied() const
{
  TSE_ASSERT(_reqSolutionsCount != UNDEFINED);
  TSE_ASSERT(_directFosCount != UNDEFINED);
  TSE_ASSERT(_rcoDirectFosCount != UNDEFINED);
  TSE_ASSERT(_legsCount != 0);

  for (auto& elem : _unusedSopsPerLeg)
  {
    TSE_ASSERT(elem != UNDEFINED);
  }

  for (auto& elem : _rcoSopsCountPerLeg)
  {
    TSE_ASSERT(elem != UNDEFINED);
  }
}

void
IbfRequirementsEstimator::checkIfDataValid() const
{
  // DF <= Q
  TSE_ASSERT(_directFosCount <= _reqSolutionsCount);

  // DFrco <= DF
  TSE_ASSERT(_rcoDirectFosCount <= _directFosCount);
}

unsigned int
IbfRequirementsEstimator::estimateOptionsCountToCoverAllSops() const
{
  if (_unusedSopsPerLeg.empty())
  {
    // fallback to a safe value
    return 0;
  }

  return *(std::max_element(_unusedSopsPerLeg.begin(), _unusedSopsPerLeg.end()));
}

unsigned int
IbfRequirementsEstimator::estimateRcoMax() const
{
  unsigned int rcoMax = 1;
  for (auto& elem : _rcoSopsCountPerLeg)
  {
    rcoMax *= elem;
  }
  return rcoMax;
}

unsigned int
IbfRequirementsEstimator::rRcoCountActualEstimate() const
{
  // spaceConsuming = DF + AS
  const unsigned int spaceConsuming = _directFosCount + _optionsCountToCoverAllSops;

  unsigned int spaceLeft; // Q - DF - AS
  if (spaceConsuming > _reqSolutionsCount)
  {
    spaceLeft = 0;
  }
  else
  {
    spaceLeft = _reqSolutionsCount - spaceConsuming;
  }

  TSE_ASSERT(_rcoDirectFosCount <= _rcoMax);
  unsigned int rcoPossible = _rcoMax - _rcoDirectFosCount; // RcoMax - DFrco

  if (spaceLeft <= rcoPossible)
  {
    return spaceLeft;
  }
  return rcoPossible;
}

std::string
IbfRequirementsEstimator::toString() const
{
  using namespace std;
  using namespace tse::utils;

  ostringstream out;
  out << "*** IbfRequirementsEstimator ***" << endl;
  out << "------------------------------------------------------------" << endl;
  out << endl;
  BlockMaker b(50);
  BlockMaker s(8);
  out << "INPUT VALUES" << endl;
  out << "------------------------------------------------------------" << endl;
  out << b("# required solutions") << _reqSolutionsCount << endl;
  out << b("# generated direct FOS solutions") << _directFosCount << endl;
  out << b("# direct FOS solutions being RC onlines") << _rcoDirectFosCount << endl;
  out << endl;
  out << "# unused SOPs per leg:" << endl;
  out << s("Leg") << s("# unused SOPs") << endl;
  for (unsigned int i = 0; i < _unusedSopsPerLeg.size(); ++i)
  {
    out << s(i) << _unusedSopsPerLeg[i] << endl;
  }
  out << endl;
  out << "# requested carrier online sops per leg:" << endl;
  out << s("Leg") << s("# RCO SOPs") << endl;
  for (unsigned int i = 0; i < _rcoSopsCountPerLeg.size(); ++i)
  {
    out << s(i) << _rcoSopsCountPerLeg[i] << endl;
  }
  out << endl;
  out << "ESTIMATED VALUES" << endl;
  out << "------------------------------------------------------------" << endl;
  out << b("# options to meet 'All SOPs represented'") << _optionsCountToCoverAllSops
      << " = max(" << _unusedSopsPerLeg << ")" << endl;
  out << b("maximum nbr of possible RC Online options") << _rcoMax
      << " = product of (" << _rcoSopsCountPerLeg << ")" << endl;
  out << b("# REMAINING REQUESTED CARRIER ONLINE SOLUTIONS") << _remainingRco
      << " = min(" << _reqSolutionsCount << " - " << _directFosCount << " - " << _optionsCountToCoverAllSops
      << ", " << _rcoMax << " - " << _rcoDirectFosCount << ")" << endl;
  out << "------------------------------------------------------------" << endl;
  return out.str();
}

std::ostream& operator<<(std::ostream& out, const IbfRequirementsEstimator& e)
{
  out << e.toString();
  return out;
}

} // namespace tse
