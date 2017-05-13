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

#include "Common/TseEnums.h"
#include "DataModel/FlexFares/TotalAttrs.h"
#include "DataModel/FlexFares/Types.h"

namespace tse
{
namespace flexFares
{

class GroupsFilter
{
public:
  GroupsFilter(const TotalAttrs& totalAttrs) : _totalAttrs(totalAttrs) {}

  void filterOutInvalidGroups(const ValidationStatusPtr status, GroupsIds& result) const;
  bool filterOutInvalidGroups(const ValidationStatusPtr status, GroupId& result);

protected:
  template <Attribute attribute>
  void filterOut(const ValidationStatusPtr status, GroupsIds& result) const;

  template <Attribute attribute>
  void filterOutComplexAttr(const ValidationStatusPtr status, GroupsIds& result) const;

  template <Attribute attribute>
  bool filterOut(const ValidationStatusPtr status, GroupId& result);

  template <Attribute attribute>
  bool filterOutComplexAttr(const ValidationStatusPtr status, GroupId& result);

  bool isValid(bool valid) const { return valid; }

  bool isValid(Record3ReturnTypes result) const
  {
    return (result == tse::SOFTPASS || result == tse::PASS || result == tse::SKIP);
  }

  bool isValidNew(bool valid) { return valid; }

  bool isValidNew(Record3ReturnTypes result)
  {
    return (result == tse::SOFTPASS || result == tse::PASS || result == tse::SKIP);
  }

  void removeInvalidGroups(const GroupsIds& invalidGroups, GroupsIds& result) const;

private:
  const TotalAttrs& _totalAttrs;
};
}
}

