//----------------------------------------------------------------------------
// 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
// ----------------------------------------------------------------------------

#pragma once

#include "DBAccess/AddonFareInfo.h"
#include "DBAccess/AddonFareInfoFactory.h"
#include "DBAccess/Flattenizable.h"

#include <set>

namespace tse
{

class DataHandle;

class SITAAddonFareInfo : public AddonFareInfo
{
public:
  // construction / destruction
  // ============ = ===========

  SITAAddonFareInfo()
    : _baseMPMInd(' '),
      _throughMPMInd(' '),
      _ruleTariff(0),
      _ruleExcludeInd(' '),
      _gatewayZone(0),
      _interiorZone(0),
      _globalClassFlag(' '),
      _tariffFamily(' '),
      _fareQualInd(' ')
  {
  }
  virtual ~SITAAddonFareInfo() {};
  const eAddonFareInfoType objectType() const override { return eSITAAddonFareInfo; }

  /**
   * This methods obtains a new FareInfo pointer from
   * the data handle and populates it to be 'equal'
   * to the current object
   *
   * @param dataHandle
   *
   * @return new object
   */
  AddonFareInfo* clone(DataHandle& dataHandle) const override;

  /**
   * This methods populates a given FareInfo to be
   * 'equal' to the current object
   *
   * @param FareInfo - object to populate
   */
  void clone(AddonFareInfo& cloneObj) const override;

  // accessors
  // =========

  RoutingNumber& baseFareRouting() { return _baseFareRouting; }
  const RoutingNumber& baseFareRouting() const { return _baseFareRouting; }

  RoutingNumber& thruFareRouting() { return _thruFareRouting; }
  const RoutingNumber& thruFareRouting() const { return _thruFareRouting; }

  Indicator& baseMPMInd() { return _baseMPMInd; }
  const Indicator& baseMPMInd() const { return _baseMPMInd; }

  Indicator& throughMPMInd() { return _throughMPMInd; }
  const Indicator& throughMPMInd() const { return _throughMPMInd; }

  TariffNumber& ruleTariff() { return _ruleTariff; }
  const TariffNumber& ruleTariff() const { return _ruleTariff; }

  RuleNumber& throughRule() { return _throughRule; }
  const RuleNumber& throughRule() const { return _throughRule; }

  Indicator& ruleExcludeInd() { return _ruleExcludeInd; }
  const Indicator& ruleExcludeInd() const { return _ruleExcludeInd; }

  AddonZone& gatewayZone() { return _gatewayZone; }
  const AddonZone& gatewayZone() const { return _gatewayZone; }

  AddonZone& interiorZone() { return _interiorZone; }
  const AddonZone& interiorZone() const { return _interiorZone; }

  Indicator& globalClassFlag() { return _globalClassFlag; }
  const Indicator& globalClassFlag() const { return _globalClassFlag; }

  Indicator& tariffFamily() { return _tariffFamily; }
  const Indicator& tariffFamily() const { return _tariffFamily; }

  Indicator& fareQualInd() { return _fareQualInd; }
  const Indicator& fareQualInd() const { return _fareQualInd; }

  std::set<Indicator>& fareQualCodes() { return _fareQualCodes; }
  const std::set<Indicator>& fareQualCodes() const { return _fareQualCodes; }

  std::set<DBEClass>& dbeClasses() { return _dbeClasses; }
  const std::set<DBEClass>& dbeClasses() const { return _dbeClasses; }

  std::set<RuleNumber>& rules() { return _rules; }
  const std::set<RuleNumber>& rules() const { return _rules; }

  virtual bool operator==(const SITAAddonFareInfo& rhs) const
  {
    return ((AddonFareInfo::operator==(rhs)) && (_baseFareRouting == rhs._baseFareRouting) &&
            (_thruFareRouting == rhs._thruFareRouting) && (_baseMPMInd == rhs._baseMPMInd) &&
            (_throughMPMInd == rhs._throughMPMInd) && (_ruleTariff == rhs._ruleTariff) &&
            (_throughRule == rhs._throughRule) && (_ruleExcludeInd == rhs._ruleExcludeInd) &&
            (_gatewayZone == rhs._gatewayZone) && (_interiorZone == rhs._interiorZone) &&
            (_globalClassFlag == rhs._globalClassFlag) && (_tariffFamily == rhs._tariffFamily) &&
            (_fareQualInd == rhs._fareQualInd) && (_fareQualCodes == rhs._fareQualCodes) &&
            (_dbeClasses == rhs._dbeClasses) && (_rules == rhs._rules));
  }

  static void dummyData(SITAAddonFareInfo& obj)
  {
    obj._baseFareRouting = "ABCD";
    obj._thruFareRouting = "EFGH";
    obj._baseMPMInd = 'I';
    obj._throughMPMInd = 'J';
    obj._ruleTariff = 1;
    obj._throughRule = "KLMN";
    obj._ruleExcludeInd = 'O';
    obj._gatewayZone = 2;
    obj._interiorZone = 3;
    obj._globalClassFlag = 'Q';
    obj._tariffFamily = 'R';
    obj._fareQualInd = 'S';

    obj._fareQualCodes.insert('T');
    obj._fareQualCodes.insert('U');

    obj._dbeClasses.insert("VWX");
    obj._dbeClasses.insert("YZa");

    obj._rules.insert("bcde");
    obj._rules.insert("fghi");
  }

  WBuffer& write(WBuffer& os, size_t* memSize) const override
  {
    os.write(static_cast<boost::uint8_t>(eSITAAddonFareInfo));
    if (memSize)
    {
      *memSize += sizeof(SITAAddonFareInfo);
    }
    return convert(os, this);
  }

  RBuffer& read(RBuffer& is) override { return convert(is, this); }

protected:
  RoutingNumber _baseFareRouting;
  RoutingNumber _thruFareRouting;
  Indicator _baseMPMInd;
  Indicator _throughMPMInd;
  TariffNumber _ruleTariff;
  RuleNumber _throughRule;
  Indicator _ruleExcludeInd;
  AddonZone _gatewayZone;
  AddonZone _interiorZone;
  Indicator _globalClassFlag;
  Indicator _tariffFamily;
  Indicator _fareQualInd;
  std::set<Indicator> _fareQualCodes;
  std::set<DBEClass> _dbeClasses;
  std::set<RuleNumber> _rules;

public:
  void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, AddonFareInfo);
    FLATTENIZE(archive, _baseFareRouting);
    FLATTENIZE(archive, _thruFareRouting);
    FLATTENIZE(archive, _baseMPMInd);
    FLATTENIZE(archive, _throughMPMInd);
    FLATTENIZE(archive, _ruleTariff);
    FLATTENIZE(archive, _throughRule);
    FLATTENIZE(archive, _ruleExcludeInd);
    FLATTENIZE(archive, _gatewayZone);
    FLATTENIZE(archive, _interiorZone);
    FLATTENIZE(archive, _globalClassFlag);
    FLATTENIZE(archive, _tariffFamily);
    FLATTENIZE(archive, _fareQualInd);
    FLATTENIZE(archive, _fareQualCodes);
    FLATTENIZE(archive, _dbeClasses);
    FLATTENIZE(archive, _rules);
  }

protected:
  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return AddonFareInfo::convert(buffer, ptr) & ptr->_baseFareRouting & ptr->_thruFareRouting &
           ptr->_baseMPMInd & ptr->_throughMPMInd & ptr->_ruleTariff & ptr->_throughRule &
           ptr->_ruleExcludeInd & ptr->_gatewayZone & ptr->_interiorZone & ptr->_globalClassFlag &
           ptr->_tariffFamily & ptr->_fareQualInd & ptr->_fareQualCodes & ptr->_dbeClasses &
           ptr->_rules;
  }
}; // class SITAAddonFareInfo

} // namespace tse
