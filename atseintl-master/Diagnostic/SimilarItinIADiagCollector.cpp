//----------------------------------------------------------------------------
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//----------------------------------------------------------------------------
#include "Diagnostic/SimilarItinIADiagCollector.h"

namespace tse
{
void
SimilarItinIADiagCollector::printSimilarItinData(const std::vector<Itin*>& itins)
{
  for (const Itin* itin : itins)
  {
    DiagCollector& dc = *this;
    if (!itin->getSimilarItins().empty())
    {
      dc << "MOTHER ITIN NO " << itin->itinNum() << " SUMMARY:" << std::endl;
      for (const SimilarItinData& data : itin->getSimilarItins())
      {
        dc << "SIMILAR ITIN DATA FOR ITIN: " << data.itin->itinNum() << std::endl;
        printSimilarItinData(data);
      }
    }
  }
}

void
SimilarItinIADiagCollector::printSimilarItinData(const SimilarItinData& data)
{
  DiagCollector& dc = *this;
  for (const auto& value : data.fareMarketData)
  {
    dc << "FARE MARKET:" << std::endl << *value.first << std::endl;
    dc << "EQUIVALENT TRAVEL SEGMENTS:" << std::endl;

    const size_t size = value.second.travelSegments.size();
    for (size_t i = 0; i < size; ++i)
    {
      dc << *value.second.travelSegments[i];
      printAvailability(value.second.classOfService[i]);
    }
    dc << std::endl;
  }
}

DiagCollector&
SimilarItinIADiagCollector::operator<<(const TravelSeg& ts)
{
  DiagCollector& dc = *this;
  dc << ts.origin()->loc();
  if (ts.isAir())
  {
    const AirSeg& airSeg = static_cast<const AirSeg&>(ts);
    dc << " " << airSeg.carrier() << airSeg.flightNumber();
  }
  dc << " " << ts.destination()->loc() << std::endl;
  return *this;
}

void
SimilarItinIADiagCollector::printAvailability(const std::vector<ClassOfService>& cosVec)
{
  DiagCollector& dc = *this;
  for (const ClassOfService& cos : cosVec)
    dc << cos.bookingCode() << cos.numSeats() << " ";

  dc << std::endl;
}
}
