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
#include "Pricing/Shopping/IBF/AllDirectOptionsRepresented.h"
#include "Pricing/Shopping/IBF/AllOnlinesForCarrier.h"
#include "Pricing/Shopping/IBF/AllSopsRepresented.h"
#include "Pricing/Shopping/IBF/AllSOPsOnSpecifiedLeg.h"
#include "Pricing/Shopping/IBF/EarlierOptionsAreBetter.h"
#include "Pricing/Shopping/IBF/MctCombinableOptionsAreBetter.h"
#include "Pricing/Shopping/IBF/ScheduleRepeatLimit.h"
#include "Pricing/Shopping/IBF/SopCountingAllSopsRepresented.h"
#include "Pricing/Shopping/IBF/SopUsageTracker.h"
#include "Pricing/Shopping/Swapper/LockingAppraiser.h"
#include "Pricing/Shopping/Swapper/Swapper.h"
#include "Pricing/Shopping/Utils/ILogger.h"
#include "Pricing/Shopping/Utils/ShoppingUtilTypes.h"

#include <string>

namespace tse
{

class ShoppingTrx;
class LessConnectionsIsBetter;

// Tracks how IBF requirements are fulfilled
// during the options generation phase of PO queue,
// managing content of the result set
//
// Options are collected in a buffer and sorted from most
// to least valuable. If necessary, some least valuable options
// are removed so that the required number of options is not
// surpassed.
//
// Behaviour of the tracker:
// - all direct FOS options are kept in the result set
// - requirements are tracked:
//
//    * all (valid) SOPs covered
//
//      (tracker has knowledge about all valid SOPs in transaction.
//      If each such SOP appears in the response, the requirement
//      is fulfilled)
//
//    * requested carrier online options preferred
//      for remaining options
//
//      (tracker is given an estimate for the number of
//      RC onlines in the result set. After the estimate is met,
//      the requirement is considered fulfilled)
//
// - if two options are equally valuable in terms of
//  requirements above, the one that came earlier
//  from the queue is preferred, since assumed cheaper.
class IbfRequirementsTracker
{
public:
  using ImplSwapper = swp::Swapper<utils::SopCombination>;
  using AddResponse = ImplSwapper::AddResponse;

  IbfRequirementsTracker(const ShoppingTrx& trx,
                         unsigned int requestedNrOfSolutions,
                         const std::string& requestingCarrierCode,
                         utils::ILogger* logger = nullptr);

  // Enables Schedule Repeat Limit requirement
  void enableSrl(unsigned int srlValue);

  // Sets the desired *total* number of RC Online options
  // (must be <= requestedNrOfSolutions)
  void setRCOnlinesDesiredCount(unsigned int desired);

  // Informs requirements tracker about a new FOS solution
  void newDirectFosSolution(const utils::SopCombination& comb);

  // Informs requirements tracker about a new queue solution
  //
  // Returns: same as for Swapper
  AddResponse newQueueSolution(const utils::SopCombination& comb);




  // Returns the number of RC Online options
  // in the contained result set of options
  unsigned int getRcOnlinesCount() const
  {
    return _allOnlinesForCarrier->getCollectedOptionsCount();
  }

  // Returns the requested number of solutions,
  // as set in the constructor.
  unsigned int getRequestedNbrOfSolutions() const { return _swapper.getCapacity(); }

  unsigned int getSolutionsCount() const { return _swapper.getSize(); }

  // Tells if the requested number of solutions
  // has been collected
  bool hasRequestedNbrOfSolutions() const { return _swapper.isFull(); }

  // Tells if requirements:
  // "all SOPs represented"
  // "all direct options present"
  // "RC onlines preferred"
  // are met.
  bool areAllRequirementsMet() const
  {
    if (!isAllSopsRepresentedSatisfied())
    {
      return false;
    }

    if (!isAllDirectOptionsPresentSatisfied())
    {
      return false;
    }

    if (!isAllOnlinesForCarrierSatisfied())
    {
      return false;
    }

    return true;
  }

  bool isAllSopsRepresentedSatisfied() const { return _allSopsRepresented.isSatisfied(); }

  bool isAllDirectOptionsPresentSatisfied() const {return _allDirectOptionsRepresented.isSatisfied(); }

  bool isAllOnlinesForCarrierSatisfied() const { return _allOnlinesForCarrier->isSatisfied(); }

  bool isSrlSatisfied() const
  {
    if (nullptr == _scheduleRepeatLimit)
    {
      return true;
    }
    return _scheduleRepeatLimit->isSatisfied();
  }


  std::string swapperToString() const;

  ImplSwapper& getSwapper();


  void addDirectSopForTracking(unsigned int legId, unsigned int sopId)
  {
    _allDirectOptionsRepresented.addDirectSopForTracking(legId, sopId);
  }

  void calculateDirectFosTargetCount()
  {
    _allDirectOptionsRepresented.setTargetCount(
        std::min(_allDirectOptionsRepresented.getAllSopCombinationsCount(),
                 _swapper.getCapacity()));
  }

  void addSopForUsageTracking(unsigned int legId, uint32_t sopId)
  {
    _sopUsageTracker.addSopForTracking(legId, sopId);
  }

  void informSrlAppraiserOnSopCounts()
  {
    if (_scheduleRepeatLimit)
    {
      const SopUsageTracker::SopCountPerLeg sopCountPerLeg =
          _sopUsageTracker.getTrackedSopCountPerLeg();
      for (const auto& elem : sopCountPerLeg)
      {
        _scheduleRepeatLimit->reportSopCount(elem.first, elem.second);
      }
    }
  }

  void setLegIdToTrack(uint16_t legIdToTrack)
  {
    _allSOPsOnSpecifiedLeg.setLegIdToTrack(legIdToTrack);
  }


  SopUsageTracker& getSopUsageTracker()
  {
    return _sopUsageTracker;
  }

private:
  void prepareAppraisersForIBF();
  void prepareAppraisersForEXC();

  static const int LESS_CONNECTION_SATISFY_PERCENT;

  static const int HIGHEST_PLUS_PRIORITY;
  static const int HIGHEST_PRIORITY;
  static const int HIGH_PRIORITY;
  static const int MEDIUM_PLUS_PRIORITY;
  static const int MEDIUM_PRIORITY;
  static const int LOW_PRIORITY;
  static const int LOWEST_PRIORITY;

  const ShoppingTrx& _trx;

  SopUsageTracker _sopUsageTracker;

  swp::LockingAppraiser<utils::SopCombination> _lockingAppraiser;
  AllSopsRepresented _allSopsRepresented;
  AllSOPsOnSpecifiedLeg _allSOPsOnSpecifiedLeg;
  SopCountingAllSopsRepresented _newAllSopsRepresented;
  AllDirectOptionsRepresented<> _allDirectOptionsRepresented;
  MctCombinableOptionsAreBetter _mctOptionsAreBetter;
  AllOnlinesForCarrier* _allOnlinesForCarrier = nullptr;
  LessConnectionsIsBetter* _lessConnectionsIsBetter = nullptr;
  EarlierOptionsAreBetter _earlierOptionsAreBetter;
  ScheduleRepeatLimit* _scheduleRepeatLimit = nullptr;

  ImplSwapper _swapper;
  utils::ILogger* _logger;
};

} // namespace tse

