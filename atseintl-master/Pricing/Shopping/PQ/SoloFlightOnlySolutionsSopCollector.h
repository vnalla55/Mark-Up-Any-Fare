// ----------------------------------------------------------------
//
//   Copyright Sabre 2012
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/PQ/SoloFlightOnlySolutionsDataTypes.h"

#include <vector>

namespace tse
{
class TravelSeg;

namespace fos
{

class SOPCollections;
class SOPDetails;

class SoloFlightOnlySolutionsSopCollector
{
public:
  SoloFlightOnlySolutionsSopCollector(ShoppingTrx& trx) : _trx(trx) {}
  void collectApplicableSopsAltDates(SOPCollections& collection);

private:
  CarrierCode
  calculateGovCarrier(uint32_t legIdx, std::vector<TravelSeg*>& tvlSegments, TravelSeg* lastSegment);

  SOPDetails createCandidate(uint32_t legIdx, std::vector<std::vector<TravelSeg*>>& tvlSegments);
  SOPDetails createCandidate(uint32_t legIdx, TravelSeg* tvlSegment);

  void addSop(SOPCollections& collection,
              uint32_t legIdx,
              uint32_t sopIdx,
              const DateTime& date,
              const CarrierCode& sopGovCarrier,
              SOPDetails details);

  void processSopAltDates(SOPCollections& collection,
                          uint32_t legIdx,
                          uint32_t sopIdx,
                          ShoppingTrx::SchedulingOption& sop);

private:
  ShoppingTrx& _trx;
};

} // namespace fos
} // namespace tse
