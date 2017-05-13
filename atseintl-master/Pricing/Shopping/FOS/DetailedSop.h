// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
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

#include "Pricing/Shopping/FOS/FosTypes.h"

#include <set>

namespace tse
{
class DataHandle;
class TravelSeg;

namespace fos
{

class DetailedSop
{
  friend class FosGeneratorMock;

public:
  DetailedSop(uint32_t legId, uint32_t sopId);
  ~DetailedSop() {}

  uint32_t getLegId() const { return _legId; }
  uint32_t getSopId() const { return _sopId; }

  const SopDetailsPtrVec& getSopDetailsVec() const { return _sopDetailsVec; }
  void addDetail(SopDetails* details) { _sopDetailsVec.push_back(details); }

  bool operator<(const DetailedSop& other) const;
  bool operator==(const DetailedSop& other) const
  {
    return _legId == other.getLegId() && _sopId == other.getSopId();
  }

  void generateSopDetails(DataHandle& dataHandle, std::vector<TravelSeg*>& trvSegs, uint32_t legId);

private:
  CarrierCode calculateGovCarrier(uint32_t legIdx,
                                  const std::vector<TravelSeg*>& tvlSegments,
                                  const TravelSeg& lastSegment) const;

  uint32_t _legId;
  uint32_t _sopId;
  SopDetailsPtrVec _sopDetailsVec;
};

typedef std::set<DetailedSop> SopsWithDetailsSet;

} // fos
} // tse
