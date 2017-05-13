/*---------------------------------------------------------------------------
 *  Copyright Sabre 2015
 *    The copyright to the computer program(s) herein
 *    is the property of Sabre.
 *    The program(s) may be used and/or copied only with
 *    the written permission of Sabre or in accordance
 *    with the terms and conditions stipulated in the
 *    agreement/contract under which the program(s)
 *    have been supplied.
 *-------------------------------------------------------------------------*/
#pragma once

#include "DataModel/FareUsage.h"

#include <cstdint>
#include <vector>
namespace tse
{
class FarePath;
class FareUsage;
class PaxTypeFare;

class ValidationResultsCleaner
{
  class CategoriesReverterGuard
  {
    PaxTypeFare& paxTypeFare;
    std::vector<std::pair<uint16_t, bool>> categoriesWithStatus;

  public:
    void addCategoryToRevert(uint16_t category);

    CategoriesReverterGuard(PaxTypeFare& ptf) : paxTypeFare(ptf) {}
    ~CategoriesReverterGuard();
  };

public:
  explicit ValidationResultsCleaner(FareUsage& fareUsage)
    : _guard(*fareUsage.paxTypeFare()), _fareUsage(fareUsage)
  {
  }

  static void clearFpRuleValidationResults(const std::vector<uint16_t>& categories, FarePath& fp);
  static void clearCat12Surcharges(FarePath& farePath);

  bool needRevalidation(const std::vector<uint16_t>& catSequence, FarePath& farePath);

private:
  void clearFuRuleValidationResults(const uint16_t category, FarePath& farePath);
  CategoriesReverterGuard _guard;
  FareUsage& _fareUsage;
};
}
