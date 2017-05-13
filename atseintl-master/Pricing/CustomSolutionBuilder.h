/*
 * CustomSolutionBuilder.h
 *
 *  Created on: March 3, 2013
 *      Author: Masud Khan
 *
 * The copyright to the computer program(s) herein
 * is the property of Sabre.
 * The program(s) may be used and/or copied only with
 * the written permission of Sabre or in accordance
 * with the terms and conditions stipulated in the
 * agreement/contract under which the program(s)
 * have been supplied.
 *
 */

#pragma once

#include "Pricing/GroupFarePath.h"

namespace tse
{
class ShoppingPQ;
class ShoppingTrx;

class CustomSolutionBuilder
{
public:

  CustomSolutionBuilder(ShoppingTrx& trx) : _trx(trx) {}

  void getAdditionalSolutionsFromExistingGFP();
  void getAdditionalSolutionsFromQueue();
  void getFlightOnlySolutions();

private:
  typedef std::pair<ShoppingPQ*, GroupFarePath*> GFPInPQ;
  struct SortByTotalNUCAmount
  {
    bool operator()(const GFPInPQ lhs, const GFPInPQ rhs)
    {
      GroupFarePath* gfp1 = lhs.second, *gfp2 = rhs.second;
      return gfp1->getTotalNUCAmount() < gfp2->getTotalNUCAmount();
    }
  };
  typedef std::multiset<GFPInPQ, SortByTotalNUCAmount> SortedGfpSet;

  bool isMoreCustomSolutionsNeeded();

private:
  ShoppingTrx& _trx;
};
}

