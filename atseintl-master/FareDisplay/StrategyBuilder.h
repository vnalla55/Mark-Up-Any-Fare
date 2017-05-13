//-------------------------------------------------------------------
//  File:        StrategyBuilder.h
//  Created:     April 1, 2005
//  Authors:     Abu
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "FareDisplay/Group.h"


namespace tse
{
class GroupingStrategy;
class FareDisplayTrx;

/**
*   @class StrategyBuilder
*   StrategyBuilder builds the appropriate Strategy to build the FareGroups.
*/
class StrategyBuilder
{
  friend class StrategyBuilderTest;

public:
  StrategyBuilder();
  virtual ~StrategyBuilder() {};

  /**
   * build the correct startegy that implements the group/sort"
   */
  GroupingStrategy* buildStrategy(FareDisplayTrx&) const;

private:

  /**
   * Invokes the Cabin Group for one or multiple Inclusion codes.
   */
  void invokeCabinGroup(FareDisplayTrx& trx, std::vector<Group*>& groups) const;

  /**
   * Invokes the GroupingDataRetriever class to get all Grouping and Sorting Data.
   */
  bool getGroupingData(FareDisplayTrx& trx, std::vector<Group*>& groups) const;

  /**
   * Invokes the BrandDataRetriever class to get all Brand Data.
   */
  bool getBrandData(FareDisplayTrx& trx, std::vector<Group*>& groups) const;

  /**
   * selects the appropriate strategy
   */
  bool selectStrategy(FareDisplayTrx& trx, GroupingStrategy*& strategy) const;

  /**
   * apply sort override using geo-graphical prererence.
   */
  bool applySortOverride(FareDisplayTrx& trx) const;
};

} // namespace tse

