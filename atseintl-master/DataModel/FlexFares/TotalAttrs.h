//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------

#pragma once

#include "Common/CabinType.h"
#include "Common/TseConsts.h"
#include "DataModel/FlexFares/AttrComplexStats.h"
#include "DataModel/FlexFares/Types.h"

#include <tuple>

namespace tse
{

namespace flexFares
{

class TotalAttrs
{
private:
  typedef std::tuple<AttrComplexStats<std::string>, // corpIds
                       AttrComplexStats<std::string>, // accCodes;
                       GroupsIds, // public fares
                       GroupsIds, // private fares
                       GroupsIds, // passenger type
                       GroupsIds, // no adv purchase
                       GroupsIds, // no penalties
                       GroupsIds> // no min/max stay
      TotalAttrsContainer;

  TotalAttrsContainer _groups;
  bool _matchEmptyAccCode = false;
  TariffType _tariffType = TariffType::Unknown;

public:
  const TariffType getTariffType() const { return _tariffType; }
  void setTariffType(TariffType tariffType) { _tariffType = tariffType; }

  void setMatchEmptyAccCode(const bool& value) { _matchEmptyAccCode = value; }
  bool matchEmptyAccCode() const { return _matchEmptyAccCode; }
  // WARNING: this method should not be used for CORP_IDS and ACC_CODES
  // see specializations in cpp file
  template <Attribute attribute>
  void addGroup(const GroupId index)
  {
    std::get<attribute>(_groups).insert(index);
  }

  void addCorpId(const std::string& corpId, const GroupId index)
  {
    std::get<CORP_IDS>(_groups).insert(corpId, index);
  }

  void addAccCode(const std::string& accCode, const GroupId index)
  {
    std::get<ACC_CODES>(_groups).insert(accCode, index);
  }

  template <Attribute attribute>
  bool isValidationNeeded() const
  {
    return !std::get<attribute>(_groups).empty();
  }

  // WARNING: this method should not be used for CORP_IDS and ACC_CODES
  // see specializations in cpp file
  template <Attribute attribute>
  bool areAllGroupsValid(const GroupsIds& validGroups) const
  {
    std::vector<GroupId> notValidatedGroups;
    std::set_difference(std::get<attribute>(_groups).begin(),
                        std::get<attribute>(_groups).end(),
                        validGroups.begin(),
                        validGroups.end(),
                        notValidatedGroups.begin());

    return notValidatedGroups.empty();
  }

  template <Attribute attribute>
  decltype(auto) getAllGroups() const
  {
    return std::get<attribute>(_groups);
  }
};

} // flexFares
} // tse

