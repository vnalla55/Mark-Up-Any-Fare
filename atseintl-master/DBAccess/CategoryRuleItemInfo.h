//-------------------------------------------------------------------
//
//  File:    CategoryRuleItemInfo.h
//  Authors:    Devapriya SenGupta
//
//  Copyright Sabre 2004
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
#include "Common/TsePrimitiveTypes.h"
#include "Common/Utils/CommonUtils.h"
#include "Common/Utils/Pprint.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

#include <boost/functional/hash.hpp>
#include <iostream>
#include <vector>

namespace tse
{

/**
 * @class CategoryRuleItemInfo
 *
 * @brief Defines a wrapper for a Record 2 segment.
 */
class CategoryRuleItemInfo
{
public:
  enum LogicalOperators
  {
    IF,
    THEN,
    OR,
    ELSE,
    AND
  };

  uint32_t itemcat() const { return _itemcat; }
  CategoryRuleItemInfo::LogicalOperators relationalInd() const { return _relationalInd; }
  uint32_t orderNo() const { return _orderNo; }
  Indicator inOutInd() const { return _inOutInd; }
  Indicator directionality() const { return _directionality; }
  uint32_t itemNo() const { return _itemNo; }

  const CategoryRuleItemInfo& categoryRuleItemInfo() const { return *this; }

  void setItemcat(uint32_t itemcat) { _itemcat = itemcat; }
  void setOrderNo(uint32_t orderNo) { _orderNo = orderNo; }
  void setRelationalInd(CategoryRuleItemInfo::LogicalOperators relationalInd) { _relationalInd = relationalInd; }
  void setInOutInd(Indicator inOutInd) { _inOutInd = inOutInd; }
  void setDirectionality(Indicator directionality) { _directionality = directionality; }
  void setItemNo(uint32_t itemNo) { _itemNo = itemNo; }

  bool operator==(const CategoryRuleItemInfo& rhs) const
  {
    return (_itemcat == rhs._itemcat) &&
           (_orderNo == rhs._orderNo) &&
           (_relationalInd == rhs._relationalInd) &&
           (_inOutInd == rhs._inOutInd) &&
           (_directionality == rhs._directionality) &&
           (_itemNo == rhs._itemNo);
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _itemcat);
    FLATTENIZE(archive, _orderNo);
    FLATTENIZE(archive, _relationalInd);
    FLATTENIZE(archive, _inOutInd);
    FLATTENIZE(archive, _directionality);
    FLATTENIZE(archive, _itemNo);
  }

  WBuffer& write(WBuffer& os, size_t* memSize) const
  {
    if (memSize)
    {
      *memSize += sizeof(CategoryRuleItemInfo);
    }
    return convert(os, this);
  }

  RBuffer& read(RBuffer& is) { return convert(is, this); }

  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return buffer & ptr->_itemcat & ptr->_orderNo & ptr->_relationalInd &
           ptr->_inOutInd & ptr->_directionality & ptr->_itemNo;
  }

  static void dummyData(CategoryRuleItemInfo& c)
  {
    c._itemcat = 101;
    c._orderNo = 102;
    c._relationalInd = CategoryRuleItemInfo::AND;
    c._inOutInd = 'A';
    c._directionality = 'B';
    c._itemNo = 104;
  }

  void tostream(std::ostream& out) const
  {
    out << itemcat() << ", ";
    out << orderNo() << ", ";
    out << relationalInd() << ", ";
    out << inOutInd() << ", ";
    out << directionality() << ", ";
    out << itemNo();
  }

private:

  uint32_t _itemcat = 0;
  uint32_t _orderNo = 0;
  LogicalOperators _relationalInd = IF;
  Indicator _inOutInd = ' ';
  Indicator _directionality = ' ';
  uint32_t _itemNo = 0;
};

inline std::size_t hash_value(const CategoryRuleItemInfo& c)
{
  std::size_t seed = 0;
  boost::hash_combine(seed, c.itemcat());
  boost::hash_combine(seed, c.orderNo());
  boost::hash_combine(seed, c.relationalInd());
  boost::hash_combine(seed, c.inOutInd());
  boost::hash_combine(seed, c.directionality());
  boost::hash_combine(seed, c.itemNo());
  return seed;
}


inline void pprint_impl(std::ostream& out, const CategoryRuleItemInfo& c)
{
  out << "<";
  c.tostream(out);
  out << ">";
}

inline std::string tostring(const CategoryRuleItemInfo::LogicalOperators& op)
{
  switch (op)
  {
  case CategoryRuleItemInfo::IF:
    return "IF";
  case CategoryRuleItemInfo::THEN:
    return "THEN";
  case CategoryRuleItemInfo::OR:
    return "OR";
  case CategoryRuleItemInfo::ELSE:
    return "ELSE";
  case CategoryRuleItemInfo::AND:
    return "AND";
  default:
    return "UNKNOWN CategoryRuleItemInfo::LogicalOperators";
  }
}

inline CategoryRuleItemInfo::LogicalOperators logicalOpFromChar(char c)
{
  switch (c)
  {
  case '=':
    return CategoryRuleItemInfo::THEN;
  case '/':
    return CategoryRuleItemInfo::OR;
  case '&':
    return CategoryRuleItemInfo::AND;
  case '*':
    return CategoryRuleItemInfo::ELSE;
  case ':':
    return CategoryRuleItemInfo::IF;
  default:
    TSE_ASSERT(!"Bad character");
    return CategoryRuleItemInfo::IF;
  }
}

inline size_t deep_sizeof_impl(const CategoryRuleItemInfo& s)
{
  return sizeof(s);
}

} // tse namespace

FLATTENIZABLE_PRIMITIVE(CategoryRuleItemInfo::LogicalOperators);
