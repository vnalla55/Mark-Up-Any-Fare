//-----------------------------------------------------------------------------------------------
//     (C) 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//
//    --------------------------------------------------------------------------------------------

#pragma once

#include "Common/TsePrimitiveTypes.h"
#include "Common/Utils/Pprint.h"
#include "DBAccess/CategoryRuleItemInfo.h"

namespace tse
{

class CombinabilityRuleItemInfo
{
public:
  uint32_t itemcat() const { return _common.itemcat(); }
  uint32_t orderNo() const { return _common.orderNo(); }
  CategoryRuleItemInfo::LogicalOperators relationalInd() const { return _common.relationalInd(); }
  Indicator inOutInd() const { return _common.inOutInd(); }
  Indicator directionality() const { return _common.directionality(); }
  uint32_t itemNo() const { return _common.itemNo(); }

  Indicator textonlyInd() const { return _textonlyInd; }
  Indicator eoervalueInd() const { return _eoervalueInd; }
  Indicator eoeallsegInd() const { return _eoeallsegInd; }
  Indicator sameCarrierInd() const { return _sameCarrierInd; }
  Indicator sameRuleTariffInd() const { return _sameRuleTariffInd; }
  Indicator sameFareInd() const { return _sameFareInd; }

  const CategoryRuleItemInfo& categoryRuleItemInfo() const { return _common; }

  void setItemcat(uint32_t itemcat) { _common.setItemcat(itemcat); }
  void setOrderNo(uint32_t orderNo) { _common.setOrderNo(orderNo); }
  void setRelationalInd(CategoryRuleItemInfo::LogicalOperators relationalInd) { _common.setRelationalInd(relationalInd); }
  void setInOutInd(Indicator inOutInd) { _common.setInOutInd(inOutInd); }
  void setDirectionality(Indicator directionality) { _common.setDirectionality(directionality); }
  void setItemNo(uint32_t itemNo) { _common.setItemNo(itemNo); }

  void setTextonlyInd(Indicator textonlyInd) { _textonlyInd = textonlyInd; }
  void setEoervalueInd(Indicator eoervalueInd) { _eoervalueInd = eoervalueInd; }
  void setEoeallsegInd(Indicator eoeallsegInd) { _eoeallsegInd = eoeallsegInd; }
  void setSameCarrierInd(Indicator sameCarrierInd) { _sameCarrierInd = sameCarrierInd; }
  void setSameRuleTariffInd(Indicator sameRuleTariffInd) { _sameRuleTariffInd = sameRuleTariffInd; }
  void setSameFareInd(Indicator sameFareInd) { _sameFareInd = sameFareInd; }

  bool operator==(const CombinabilityRuleItemInfo& rhs) const
  {
    return (_common == rhs._common) &&
           (_textonlyInd == rhs._textonlyInd) &&
           (_eoervalueInd == rhs._eoervalueInd) &&
           (_eoeallsegInd == rhs._eoeallsegInd) &&
           (_sameCarrierInd == rhs._sameCarrierInd) &&
           (_sameRuleTariffInd == rhs._sameRuleTariffInd) &&
           (_sameFareInd == rhs._sameFareInd);
  }

  static void dummyData(CombinabilityRuleItemInfo& info)
  {
    CategoryRuleItemInfo::dummyData(info._common);
    info._textonlyInd = 'Z';
    info._eoervalueInd = 'Y';
    info._eoeallsegInd = 'X';
    info._sameCarrierInd = 'W';
    info._sameRuleTariffInd = 'V';
    info._sameFareInd = 'U';
  }

  WBuffer& write(WBuffer& os, size_t* memSize) const
  {
    if (memSize)
    {
      *memSize += sizeof(CombinabilityRuleItemInfo);
    }
    return convert(os, this);
  }

  RBuffer& read(RBuffer& is) { return convert(is, this); }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _common);
    FLATTENIZE(archive, _textonlyInd);
    FLATTENIZE(archive, _eoervalueInd);
    FLATTENIZE(archive, _eoeallsegInd);
    FLATTENIZE(archive, _sameCarrierInd);
    FLATTENIZE(archive, _sameRuleTariffInd);
    FLATTENIZE(archive, _sameFareInd);
  }

  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return buffer & ptr->_common & ptr->_textonlyInd & ptr->_eoervalueInd &
           ptr->_eoeallsegInd & ptr->_sameCarrierInd & ptr->_sameRuleTariffInd &
           ptr->_sameFareInd;
  }

private:

  CategoryRuleItemInfo _common;
  Indicator _textonlyInd = ' ';
  Indicator _eoervalueInd = ' ';
  Indicator _eoeallsegInd = ' ';
  Indicator _sameCarrierInd = ' ';
  Indicator _sameRuleTariffInd = ' ';
  Indicator _sameFareInd = ' ';
};

inline void pprint_impl(std::ostream& out, const CombinabilityRuleItemInfo& c)
{
  out << "<";
  c.categoryRuleItemInfo().tostream(out);
  out << ", " << c.textonlyInd();
  out << ", " << c.eoervalueInd();
  out << ", " << c.eoeallsegInd();
  out << ", " << c.sameCarrierInd();
  out << ", " << c.sameRuleTariffInd();
  out << ", " << c.sameFareInd();
  out << ">";
}

inline std::size_t hash_value(const CombinabilityRuleItemInfo& c)
{
  std::size_t seed = tools::calc_hash(c.categoryRuleItemInfo());
  boost::hash_combine(seed, c.textonlyInd());
  boost::hash_combine(seed, c.eoervalueInd());
  boost::hash_combine(seed, c.eoeallsegInd());
  boost::hash_combine(seed, c.sameCarrierInd());
  boost::hash_combine(seed, c.sameRuleTariffInd());
  boost::hash_combine(seed, c.sameFareInd());
  return seed;
}

inline size_t deep_sizeof_impl(const CombinabilityRuleItemInfo& s)
{
  return sizeof(s);
}

} // namespace tse

