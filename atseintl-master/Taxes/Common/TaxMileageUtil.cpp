//---------------------------------------------------------------------------
//  Copyright Sabre 2014
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

#include "Taxes/Common/TaxMileageUtil.h"

#include "Common/Assert.h"
#include "Common/DateTime.h"
#include "Common/TseUtil.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Mileage.h"
#include "DBAccess/MileageSubstitution.h"

namespace tse
{
namespace
{
class MatchesGlobalDirection : public std::unary_function<Mileage*, bool>
{
public:
  MatchesGlobalDirection(const GlobalDirection& globalDir) : _globalDir(globalDir) {}

  bool operator()(const Mileage* const mileage) const { return mileage->globaldir() == _globalDir; }

private:
  const GlobalDirection _globalDir;
};

class MileagesComparator
{
public:
  MileagesComparator(const GlobalDirection& globalDir) : _matcher(globalDir) {}

  bool operator()(tse::Mileage* mileage1, tse::Mileage* mileage2) const
  {
    TSE_ASSERT(mileage1 != nullptr && mileage2 != nullptr);
    if (!_matcher(mileage1))
      return true;
    if (!_matcher(mileage2))
      return false;

    return (mileage1->mileage() < mileage2->mileage());
  }

private:
  MatchesGlobalDirection _matcher;
};
} // anonymous namespace

uint32_t
TaxMileageUtil::getDistance(const Loc& origin,
                            const Loc& destination,
                            const GlobalDirection globalDirection,
                            const DateTime& dateTime,
                            DataHandle& dataHandle)
{
  LocCode city1 = origin.city().empty() ? origin.loc() : origin.city();
  LocCode city2 = destination.city().empty() ? destination.loc() : destination.city();
  boost::optional<uint32_t> mileage =
      getMileageWithSubstitution(city1, city2, globalDirection, dateTime, TPM, dataHandle);
  if (mileage)
    return *mileage;

  mileage = getMileageWithSubstitution(city1, city2, globalDirection, dateTime, MPM, dataHandle);
  if (mileage)
    return (*mileage) * 5 / 6;

  return TseUtil::greatCircleMiles(origin, destination);
}

uint32_t
TaxMileageUtil::getGCMDistance(const Loc& origin,
                               const Loc& destination)
{
  return TseUtil::greatCircleMiles(origin, destination);
}

boost::optional<uint32_t>
TaxMileageUtil::getMileageWithSubstitution(const tse::LocCode& city1,
                                           const tse::LocCode& city2,
                                           const GlobalDirection& globalDir,
                                           const DateTime& dateTime,
                                           const tse::Indicator mileageType,
                                           DataHandle& dataHandle)
{
  boost::optional<uint32_t> mileage =
      getMileage(city1, city2, globalDir, dateTime, mileageType, dataHandle);
  if (mileage)
    return mileage;

  const MileageSubstitution* city1Subst = dataHandle.getMileageSubstitution(city1, dateTime);

  if (city1Subst)
  {
    mileage = getMileage(
        city1Subst->publishedLoc(), city2, globalDir, dateTime, mileageType, dataHandle);
    if (mileage)
      return mileage;
  }

  const MileageSubstitution* city2Subst = dataHandle.getMileageSubstitution(city2, dateTime);

  if (city2Subst)
  {
    mileage = getMileage(
        city1, city2Subst->publishedLoc(), globalDir, dateTime, mileageType, dataHandle);
    if (mileage)
      return mileage;
  }

  if (city1Subst && city2Subst)
  {
    mileage = getMileage(city1Subst->publishedLoc(),
                         city2Subst->publishedLoc(),
                         globalDir,
                         dateTime,
                         mileageType,
                         dataHandle);
    if (mileage)
      return mileage;
  }

  return boost::none;
}

boost::optional<uint32_t>
TaxMileageUtil::getMileage(const LocCode& city1,
                           const LocCode& city2,
                           const GlobalDirection& globalDir,
                           const DateTime& dateTime,
                           const Indicator mileageType,
                           DataHandle& dataHandle)
{
  const std::vector<Mileage*>& mileages =
      dataHandle.getMileage(city1, city2, dateTime, mileageType);

  if (mileages.empty())
    return boost::none;

  if (mileages.size() == 1)
    return mileages.front()->mileage();

  MileagesComparator comparator(globalDir);
  return (*std::max_element(mileages.begin(), mileages.end(), comparator))->mileage();
}
}

