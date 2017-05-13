//-------------------------------------------------------------------------------------------------
//
//  File:        SOLItinGroups.h
//  Created:     Sep 1, 2011
//  Authors:     Artur de Sousa Rocha
//
//  Description: Itinerary groupings for SOL Carnival project
//
//  Copyright Sabre 2011
//
//               The copyright to the computer program(s) herein
//               is the property of Sabre.
//               The program(s) may be used and/or copied only with
//               the written permission of Sabre or in accordance
//               with the terms and conditions stipulated in the
//               agreement/contract under which the program(s)
//               have been supplied.
//
//-------------------------------------------------------------------------------------------------

#pragma once

#include "Common/Assert.h"
#include "Common/Money.h"

#include <boost/none.hpp>
#include <boost/optional.hpp>

#include <algorithm>
#include <vector>

#include <tr1/array>

namespace tse
{
class Itin;

class SOLItinGroups
{
public:
  enum GroupType
  {
    ORIGINAL = 0,
    INTERLINE_BY_CXR_AND_LEG,
    INTERLINE_BY_CXR_GROUPING,
    ONLINE_BY_LEG,
    ONLINE_BY_DOM_INT_GROUPING,
    GROUP_TYPE_MAX
  };

  class ItinGroup : public std::vector<Itin*>
  {
  public:
    typedef boost::optional<Money> MoneyNullable; // if Itin cannot be priced

    /**
     * set per Itin price, this info is used to calculate total and diagnostics only, as for now
     */
    void setItinPrice(size_t itinIndex, MoneyNullable price)
    {
      setItinAttr<MoneyNullable, &ItinAttr::price>(itinIndex, price);
    }

    /**
     * get per Itin price, this info is used to calculate total and diagnostics only, as for now
     */
    MoneyNullable getItinPrice(size_t itinIndex) const
    {
      return getItinAttr<MoneyNullable, &ItinAttr::price>(itinIndex);
    }

    /**
     * set per Itin farecalc line, this info will be used for diagnostics only
     *
     * As far each sub-Itin is SOLO Carnival is associated with only one FareMarket,
     * thus we can allow here this 1-to-1 relation.
     */
    void setItinFarecalcLine(size_t itinIndex, const std::string& val)
    {
      setItinAttr<std::string, &ItinAttr::farecalcLine>(itinIndex, val);
    }

    /**
     * get per Itin farecalc line, this info will be used for diagnostics only
     *
     * As far each sub-Itin is SOLO Carnival is associated with only one FareMarket,
     * thus we can allow here this 1-to-1 relation.
     */
    std::string getItinFarecalcLine(size_t itinIndex) const
    {
      return getItinAttr<std::string, &ItinAttr::farecalcLine>(itinIndex);
    }

    /**
     * get sum of per Itin prices
     *
     * @return no result, if at least one of:
     * <ul>
     * <li>is not set;</li>
     * <li>at least one Itin cannot be priced;</li>
     * <li>ItinGroup is empty</li>
     * <ul>
     */
    MoneyNullable getTotalPrice() const { return _totalPrice; }

    /**
     * set sum of per Itin prices
     *
     * @param totalPrice must be nullMoney() if at one Itin cannot be prices or ItinGroup is empty
     */
    void setTotalPrice(MoneyNullable totalPrice) { _totalPrice = totalPrice; }

  private:
    struct ItinAttr
    {
      MoneyNullable price;
      std::string farecalcLine;
    };
    typedef std::vector<ItinAttr> ItinAttrVec;

    ItinAttrVec _itinAttrVec;
    MoneyNullable _totalPrice;

    template <class T, T ItinAttr::*F>
    void setItinAttr(size_t itinIndex, T attrValue)
    {
      TSE_ASSERT(itinIndex < size() || !"invalid index");

      _itinAttrVec.resize(this->size());
      _itinAttrVec[itinIndex].*F = attrValue;
    }

    template <class T, T ItinAttr::*F>
    T getItinAttr(size_t itinIndex) const
    {
      TSE_ASSERT(itinIndex < size() || !"invalid index");

      if (_itinAttrVec.empty())
      {
        return T(); // return default value of T, which will be either boost::none or empty
                    // std::string
      }
      else
      {
        TSE_ASSERT(_itinAttrVec.size() == size() || !"vector size discrepancy");

        return _itinAttrVec[itinIndex].*F;
      }
    }
  };

  class ItinGroupVec : public std::tr1::array<ItinGroup*, GROUP_TYPE_MAX>
  {
  public:
    ItinGroupVec() { std::fill(begin(), end(), (ItinGroup*)nullptr); }
  };

  SOLItinGroups() {} // Only for DataHandle!
  virtual ~SOLItinGroups() {}

  // Access

  ItinGroupVec& itinGroups() { return _itinGroups; }
  const ItinGroupVec& itinGroups() const { return _itinGroups; }

  /**
   * Mark the cheapest group. This member data field is set from
   * @see SoloCarnivalFCUtil::assignPriceToSolItinGroupsMapItems()
   */
  void markCheapestItinGroup(boost::optional<GroupType> groupType)
  {
    _cheapestItinGroup = groupType;
  }

  /**
   * Returns appropriate member data field value.
   *
   * @see markCheapestItinGroup()
   */
  boost::optional<GroupType> getCheapestItinGroup() const { return _cheapestItinGroup; }

  /**
   * Returns the group of given type.
   * Don't pass GROUP_TYPE_MAX.
   */
  const ItinGroup& getItinGroup(GroupType type) const
  {
    return *_itinGroups.at(static_cast<size_t>(type));
  }

protected:
  ItinGroupVec _itinGroups;
  boost::optional<GroupType> _cheapestItinGroup;
};

} // tse namespace

