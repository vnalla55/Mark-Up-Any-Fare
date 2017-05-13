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
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/FlexFares/Types.h"

#include <map>
#include <set>

namespace tse
{
class MaxPenaltyInfo;

namespace flexFares
{

struct GroupAttrs
{
  enum Restrictions
  {
    ALL_RESTRICTIONS = 0,
    NO_ADVANCE_PURCHASE = 1,
    NO_PENALTIES = 2,
    NO_MIN_MAX_STAY = 4,
    NO_RESTRICTIONS = 7
  };

  PaxTypeCode paxTypeCode;
  CabinType requestedCabin;
  std::set<std::string> corpIds;
  std::set<std::string> accCodes;
  bool publicFares;
  bool privateFares;
  bool flexFareXCIndicator;
  JumpCabinLogic ffgJumpCabinLogic;
  bool isFlexFareGroup;
  char flexFareXOFares;
  MaxPenaltyInfo* ffgMaxPenaltyInfo;
  unsigned char restrictions;

  GroupAttrs()
    : paxTypeCode(ADULT), publicFares(false), privateFares(false), flexFareXCIndicator(false),
      ffgJumpCabinLogic(JumpCabinLogic::ENABLED), isFlexFareGroup(false),flexFareXOFares('F'),
      ffgMaxPenaltyInfo(nullptr), restrictions(ALL_RESTRICTIONS)
  {
    requestedCabin.setUnknownClass();
  }

  bool arePublicFaresRequired() const { return publicFares; }
  bool arePrivateFaresRequired() const { return privateFares; }
  bool isNoAdvancePurchaseRequired() const { return restrictions & NO_ADVANCE_PURCHASE; }
  bool isNoPenaltiesRequired() const { return restrictions & NO_PENALTIES; }
  bool isNoMinMaxStayRequired() const { return restrictions & NO_MIN_MAX_STAY; }
};

class GroupsData : private std::map<GroupId, GroupAttrs>
{
public:
  using std::map<GroupId, GroupAttrs>::const_iterator;
  using std::map<GroupId, GroupAttrs>::iterator;
  using std::map<GroupId, GroupAttrs>::begin;
  using std::map<GroupId, GroupAttrs>::end;
  using std::map<GroupId, GroupAttrs>::value_type;

  const uint16_t getSize() const { return static_cast<uint16_t>(size()); }

  void createNewGroup(const GroupId& index = 0) { insert(value_type(index, GroupAttrs())); }

  const PaxTypeCode& getPaxTypeCode(const GroupId index = 0) const { return at(index).paxTypeCode; }
  void setPaxTypeCode(PaxTypeCode ptc, const GroupId index = 0) { at(index).paxTypeCode = ptc; }

  const CabinType& getRequestedCabin(const GroupId index = 0) const
  {
    return at(index).requestedCabin;
  }
  void setRequestedCabin(Indicator cabin, const GroupId index = 0)
  {
    at(index).requestedCabin.setClass(cabin);
  }
  void setRequestedCabinFromAlphaNum(char alphaNum, const GroupId index = 0)
  {
    at(index).requestedCabin.setClassFromAlphaNum(alphaNum);
  }

  const std::set<std::string>& getCorpIds(const GroupId index = 0) const
  {
    return at(index).corpIds;
  }
  void addCorpId(std::string corpId, const GroupId index) { at(index).corpIds.insert(corpId); }

  const std::set<std::string>& getAccCodes(const GroupId index = 0) const
  {
    return at(index).accCodes;
  }
  void addAccCode(std::string accCode, const GroupId index) { at(index).accCodes.insert(accCode); }

  void setPublicFares(bool v, const GroupId index = 0) { at(index).publicFares = v; }
  bool arePublicFaresRequired(const GroupId index = 0) const
  {
    return at(index).arePublicFaresRequired();
  }

  void setPrivateFares(bool v, const GroupId index = 0) { at(index).privateFares = v; }
  bool arePrivateFaresRequired(const GroupId index = 0) const
  {
    return at(index).arePrivateFaresRequired();
  }

  void requireNoAdvancePurchase(const GroupId index = 0)
  {
    at(index).restrictions |= GroupAttrs::NO_ADVANCE_PURCHASE;
  }
  bool isNoAdvancePurchaseRequired(const GroupId index = 0) const
  {
    return at(index).isNoAdvancePurchaseRequired();
  }

  void requireNoPenalties(const GroupId index = 0)
  {
    at(index).restrictions |= GroupAttrs::NO_PENALTIES;
  }
  bool isNoPenaltiesRequired(const GroupId index = 0) const
  {
    return at(index).isNoPenaltiesRequired();
  }

  void requireNoMinMaxStay(const GroupId index = 0)
  {
    at(index).restrictions |= GroupAttrs::NO_MIN_MAX_STAY;
  }
  bool isNoMinMaxStayRequired(const GroupId index = 0) const
  {
    return at(index).isNoMinMaxStayRequired();
  }

  void requireNoRestrictions(const GroupId index = 0)
  {
    at(index).restrictions |= GroupAttrs::NO_RESTRICTIONS;
  }

  void setFlexFareXCIndicatorStatus(bool v, const GroupId index = 0) { at(index).flexFareXCIndicator = v; }
  bool isFlexFareXCIndicatorON(const GroupId index = 0) const { return at(index).flexFareXCIndicator;}

  void setFlexFareGroupStatus(bool v, const GroupId index = 0) { at(index).isFlexFareGroup = v; }
  bool isFlexFareGroup(const GroupId index = 0) const { return at(index).isFlexFareGroup;}

  void setFlexFareXOFares(char v, const GroupId index = 0) { at(index).flexFareXOFares = v; }
  char getFlexFareXOFares(const GroupId index = 0) const { return at(index).flexFareXOFares;}

  void setFFGJumpCabinLogic(JumpCabinLogic v, const GroupId index = 0) { at(index).ffgJumpCabinLogic = v; }
  JumpCabinLogic getFFGJumpCabinLogic(const GroupId index = 0) const { return at(index).ffgJumpCabinLogic;}

  void setFFGMaxPenaltyInfo(MaxPenaltyInfo* v, const GroupId index = 0) { at(index).ffgMaxPenaltyInfo = v; }
  MaxPenaltyInfo* getFFGMaxPenaltyInfo(const GroupId index = 0) const { return at(index).ffgMaxPenaltyInfo;}

private:
  const GroupAttrs& at(const GroupId index) const
  {
    const_iterator ffg = find(index);
    if (ffg != end())
      return ffg->second;

    static const GroupAttrs empty;
    return empty;
  }
  GroupAttrs& at(const GroupId index) { return (*this)[index]; }
};

} // flexFares
} // tse

