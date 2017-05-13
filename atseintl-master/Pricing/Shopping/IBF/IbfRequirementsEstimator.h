
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
#include <iostream>
#include <string>
#include <vector>

namespace tse
{

// This class estimates the expected number of remaining
// RC Online options which have to be collected from
// the Pricing Orchestrator queue after the stage of
// generating initial direct FOS solutions
//
// This class comes to play after initial direct
// FOS solutions and before PO queue processing
//
// At this point, the known values are:
// * Q:     the total required number of solutions
// * DF:    the number of generated direct FOS solutions (DF <= Q)
// * DFrco: the number of RC onlines amongst direct FOS solutions (DFrco <= DF)
// * the number of unused SOPs on each leg
// * the number of RC online SOPs on each leg
//
// We estimate following values:
// * AS:     the estimated number of options to cover all
//           SOPs which are still not represented.
//
//           The formula:
//           AS = max(the number of unused SOPs on leg i for i in [0..legs number])
//
// * RcoMax: the estimated maximum number of RC Online options
//           that we can generate
//
//           The formula:
//           RcoMax = multiply(the number of RC SOPs on leg i for i in [0..legs number])
//
// From this, we calculate the expected number of remaining
// requested carrier online solutions RRco as:
// RRco = min(RcoMax - DFrco, Q - DF - AS)
class IbfRequirementsEstimator
{
public:
  IbfRequirementsEstimator();

  // Sets the total required number of solutions (Q)
  // reqSolutionsCount should be positive
  void setRequiredSolutionsCount(unsigned int reqSolutionsCount);

  // Sets the number of generated direct FOS solutions (DF)
  void setDirectFosCount(unsigned int directFosCount);

  // Sets the number of RC onlines amongst direct FOS solutions (DFrco)
  void setRcoDirectFosCount(unsigned int rcoDirectFosCount);

  // Sets the number of legs in the current transaction
  // legsCount should be positive
  void setLegsCount(unsigned int legsCount);

  // Sets the number of unused SOPs on a particular leg
  void setUnusedSopsCount(unsigned int legId, unsigned int unusedSops);

  // Sets the number of RC online SOPs on a particular leg
  void setRcoSopsCount(unsigned int legId, unsigned int rcoSopsCount);

  // Estimates the value of RRco:
  // number of remaining requested carrier online solutions.
  //
  // During estimation process, also AS and RcoMax are calculated
  // and stored. All three values can be read using appropriate getters.
  void estimateRemainingRcoCount();

  // Returns the estimated number of remaining
  // requested carrier online solutions (RRco)
  unsigned int getEstimatedRemainingRco() const;

  // Returns the estimated number of options to cover all
  // SOPs which are still not represented (AS)
  unsigned int getEstimatedOptionsCountToCoverAllSops() const;

  // Returns the estimated maximum number of RC Online options
  // that we can generate (RcoMax)
  unsigned int getEstimatedRcoMax() const;

  std::string toString() const;

private:
  enum
  {
    UNDEFINED = static_cast<unsigned int>(-1)
  };

  void checkIfAllDataSupplied() const;
  void checkIfDataValid() const;
  unsigned int estimateOptionsCountToCoverAllSops() const;
  unsigned int estimateRcoMax() const;
  unsigned int rRcoCountActualEstimate() const;

  // Set by user
  unsigned int _reqSolutionsCount;
  unsigned int _directFosCount;
  unsigned int _rcoDirectFosCount;
  unsigned int _legsCount;
  std::vector<unsigned int> _unusedSopsPerLeg;
  std::vector<unsigned int> _rcoSopsCountPerLeg;

  // Estimated
  unsigned int _optionsCountToCoverAllSops;
  unsigned int _rcoMax;
  unsigned int _remainingRco;
};

std::ostream& operator<<(std::ostream& out, const IbfRequirementsEstimator& e);

} // namespace tse

