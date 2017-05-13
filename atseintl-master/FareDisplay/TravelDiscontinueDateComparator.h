#pragma once

#include "FareDisplay/Comparator.h"
#include "FareDisplay/FDConsts.h"

namespace tse
{

class PaxTypeFare;

class TravelDiscontinueDateComparator : public Comparator
{
public:
  Comparator::Result compare(const PaxTypeFare& l, const PaxTypeFare& r) override;
  void prepare(const FareDisplayTrx& trx) override;

private:
  const DateTime& getDate(const PaxTypeFare& p) const;
  bool hasCat14(const PaxTypeFare& p) const;
  Comparator::Result compareDate(const PaxTypeFare& l, const PaxTypeFare& r) const;
  Comparator::Result compareTravelTicketInfo(const bool& l, const bool& r) const;
  DateTime _ticketingDate;
  DateTime _emptyDate;
  uint16_t diffTime(const DateTime& tvlDiscDate) const;
};

} // namespace tse

