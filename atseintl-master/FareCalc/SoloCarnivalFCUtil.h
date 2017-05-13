//----------------------------------------------------------------------------
//  File:        SoloCarnivalFCUtil.h
//  Created:     2011-09-20
//
//  Description: Solo Carnival Shopping Fare Calc utility class
//
//  Updates:
//
//  Copyright Sabre 2011
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

#include "Common/Money.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/SOLItinGroups.h"

#include <boost/none.hpp>
#include <boost/optional.hpp>

namespace tse
{

class Itin;
class PricingTrx;

class SoloCarnivalFCUtil
{
public:
  SoloCarnivalFCUtil(PricingTrx& trx) : _trx(trx) {}

  /**
   * assign prices to all Itin/ItinGroup inhabitants of in _trx.solItinGroupsMap in-place,
   * invokes SOLItinGroups::markCheapestItinGroup() on each of SOLItinGroups
   *
   * also assigns farecalc line for each sub-Itin
   */
  void assignPriceToSolItinGroupsMapItems();

private:
  typedef SOLItinGroups::ItinGroup ItinGroup;

  PricingTrx& _trx;

  /**
   * Calculates total amount the same way, as it is done in
   *
   * XMLShoppingResponse::generateITNbody(),
   * XMLShoppingResponse::generateTOT()
   *
   * used in
   * @see assignPriceToSolItinGroupsMapItems()
   *
   * @return no result, if cannot calculate Itin price at this stage
   */
  boost::optional<Money> calcTotalAmount(const Itin&);

  /**
   * get sum of per Itin prices, Itin price is taken from ItinGroup::getItinPrice()
   *
   * used in
   * @see assignPriceToSolItinGroupsMapItems()
   *
   * @return no result, if at least one of at least one Itin is not priced or ItinGroup is empty
   */
  boost::optional<Money> calcTotalAmount(const SOLItinGroups::ItinGroup&) const;

  /**
   * find cheapest, ItinGroup price is taken from ItinGroup::getTotalPrice()
   *
   * used in
   * @see assignPriceToSolItinGroupsMapItems()
   *
   * @return no result, if none of ItinGroup can be priced
   */
  boost::optional<SOLItinGroups::GroupType>
  findCheapestItinGroup(const SOLItinGroups::ItinGroupVec&) const;

  /**
   * get farecalc line on Itin
   *
   * As far each sub-Itin is SOLO Carnival is associated with only one FareMarket,
   * thus we can use Itin-FareMarker 1-to-1 relation here.
   *
   * used in
   * @see assignPriceToSolItinGroupsMapItems()
   *
   * @return empty string on error
   */
  std::string getFareCalcLine(const Itin&);

  /**
   * @throws exception of conversion fails, the exception is to be caught within the class scope
   */
  MoneyAmount getAmountInCurrency(const MoneyAmount sourceAmount,
                                  const CurrencyCode& sourceCurrency,
                                  const CurrencyCode& targetCurrency);

}; // class SoloCarnivalFCUtil

} // namespace tse

