//-------------------------------------------------------------------
//
//  File:        GroupingDataRetriever.h
//  Created:     June 24 , 2005
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

#include "Common/FDCustomerRetriever.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayResponse.h"
#include "DBAccess/FareDisplaySort.h"
#include "FareDisplay/FDConsts.h"
#include "FareDisplay/Group.h"


namespace tse
{

/**
*   @class GroupingDataRetriever
*   Responssible of collecting all grouping and sorting date from the database and translate them
*   to a common interface, a.k.a Group
*/

class FareDisplayTrx;
class FareDisplayRequest;
class FareDisplayResponse;

class GroupingDataRetriever : public FDCustomerRetriever
{
  friend class GroupingDataRetrieverTest;
  friend class Grouping_RetrieveSort;

public:
  GroupingDataRetriever(FareDisplayTrx& trx);
  virtual ~GroupingDataRetriever() {};
  /**
   * Invokes the main data handle call to collect information from the parent table.
   */
  void getGroupAndSortPref(std::vector<Group*>& groups);
  virtual bool retrieve() override
  {
    return false;
  };

protected:
  Indicator _domIntlAppl;
  Indicator _fareDisplayType;
  std::vector<FareDisplaySort*> _parentTableData;

  // all db access for one set of records
  virtual bool retrieveData(const Indicator& userApplType,
                            const UserApplCode& userAppl,
                            const Indicator& pseudoCityType,
                            const PseudoCityCode& pseudoCity,
                            const TJRGroup& tjrGroup) override;

  /**
   * Performs sanity checks on the SortData and creates a
   * Group for each specific SortData.
   */
  bool createGroup(FareDisplaySort&, Group&);

  /**
   * Reads the data record of the sort data and selects the group type
   * and sort order for that group.
   */
  Group::GroupType getGroupType(const FareDisplaySort& sortOption, Group&);

  virtual bool initializeGroups(std::vector<Group*>& groups);

  virtual void setHeader(std::vector<Group*>&);

  /**
   * Functor to order sort data by sequence number.
   */
  struct LessBySeqNo : public std::binary_function<FareDisplaySort, FareDisplaySort, bool>
  {
    bool operator()(const FareDisplaySort* l, const FareDisplaySort* r) const
    {
      if (l == nullptr)
        return true;
      if (r == nullptr)
        return false;
      return (l->seqno() < r->seqno());
    }
  };

  struct EqualByGrpType : public std::unary_function<Group, bool>
  {
    EqualByGrpType(Group::GroupType type) : _type(type) {}
    ~EqualByGrpType() {}

  public:
    bool operator()(const Group* g) const
    {
      if (g == nullptr)
        return false;
      return g->groupType() == _type;
    }

  private:
    Group::GroupType _type;
  };
}; // class GroupingDataRetriever
} // namespace tse

