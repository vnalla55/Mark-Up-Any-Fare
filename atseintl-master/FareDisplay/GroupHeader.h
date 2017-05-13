//-------------------------------------------------------------------
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

#include "DataModel/FareDisplayResponse.h"
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
class FareDisplayResponse;

class GroupHeader
{
  friend class GroupHeaderTest;

public:
  GroupHeader(FareDisplayTrx& trx);

  /**
   * Invokes the main data handle call to collect information from the parent table.
   */
  void setGroupHeaderInfo(std::vector<Group*>&);
  void setMultiTransportHeader(std::vector<Group*>&, std::vector<Group::GroupType>&);
  void setBrandHeader();
  void setS8BrandHeader();
  void setCabinHeader();

private:
  bool _isInternational;
  bool _isSameCityPair;

  FareDisplayTrx& _trx;

  void setCxrHeader();
  void setPaxHeader(std::vector<Group*>&, std::vector<Group::GroupType>&);
  void setGlobalHeader(std::vector<Group*>&, std::vector<Group::GroupType>&);
  void setCabinHeader(std::vector<Group*>&, std::vector<Group::GroupType>&);

  struct EqualByGrpType : public std::unary_function<Group, bool>
  {
    EqualByGrpType(Group::GroupType type) : _type(type) {}
    ~EqualByGrpType() {}

  public:
    bool operator()(const Group* g) const
    {
      if (g == nullptr)
        return false;
      else
        return g->groupType() == _type;
    }

  private:
    Group::GroupType _type;
  };

  static const uint16_t MORE_THAN_ONE = 1;
  bool isMultipleGlobal();
};

} // namespace tse
