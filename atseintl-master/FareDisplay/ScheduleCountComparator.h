#pragma once

#include "Common/TseCodeTypes.h"
#include "FareDisplay/Comparator.h"
#include "FareDisplay/FDConsts.h"

namespace tse
{

class PaxTypeFare;
class FlightCount;

class ScheduleCountComparator : public Comparator
{
public:
  Comparator::Result compare(const PaxTypeFare& l, const PaxTypeFare& r) override;

private:
  std::vector<FlightCount*> _scheduleCounts;

  void prepare(const FareDisplayTrx& trx) override;

  Comparator::Result compareScheduleCounts(const CarrierCode& l, const CarrierCode& r);

  Comparator::Result compareIndividualCounts(uint16_t l, uint16_t r);

  const FlightCount* getFlightCount(const CarrierCode& cxr) const;
};

} // namespace tse

