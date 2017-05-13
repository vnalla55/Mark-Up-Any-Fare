//-------------------------------------------------------------------
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//-------------------------------------------------------------------
#pragma once

#include <map>
#include <vector>

namespace tse
{
class DataHandle;
class FarePath;
class Itin;
class PricingUnit;

class FarePathCopier
{
public:
  FarePathCopier(DataHandle& dataHandle) : _dataHandle(dataHandle) {}

  FarePath* getDuplicate(const FarePath&);

protected:
  FarePath* duplicateBase(const FarePath&);
  std::vector<FarePath*> duplicateFarePathsForGSA(const FarePath& farePath, Itin& similar);
  void fixMainTripSideTripLink(const std::map<PricingUnit*, PricingUnit*>& stPUreplacement,
                               const FarePath& sourceFarePath,
                               FarePath& resFP) const;
  std::vector<PricingUnit*>
  fixSideTripPUVector(const std::map<PricingUnit*, PricingUnit*>& stPUreplacement,
                      const std::vector<PricingUnit*>& sourceVect) const;

private:
  DataHandle& _dataHandle;
};
}

