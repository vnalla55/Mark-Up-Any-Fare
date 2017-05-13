//----------------------------------------------------------------------------
//  File:        Diag914Collector.C
//
//  Copyright Sabre 2004
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

#include "Diagnostic/Diag914Collector.h"

#include "Common/DateTime.h"
#include "Common/ShoppingAltDateUtil.h"
#include "DataModel/ShoppingTrx.h"

namespace tse
{
Diag914Collector&
Diag914Collector::operator<<(const ShoppingTrx& trx)
{
  *this << "DATE PAIRS\n";
  outputAltDates(trx);
  outputDurations(trx);
  return *this;
}

void
Diag914Collector::outputAltDates(const ShoppingTrx& trx)
{
  if (!trx.isAltDates())
    *this << "NO ALTERNATE DATES" << std::endl;

  PricingTrx::AltDatePairs::const_iterator datePairIter = trx.altDatePairs().begin();

  for (; datePairIter != trx.altDatePairs().end(); ++datePairIter)
  {
    *this << datePairIter->first.first.dateToString(MMDDYY, "") << "   "
          << datePairIter->first.second.dateToString(MMDDYY, "") << std::endl;
  }
}

void
Diag914Collector::outputDurations(const ShoppingTrx& trx)
{
  if (!trx.isAltDates() || trx.durationAltDatePairs().empty())
    return;

  *this << "MAIN DURATION :" << ShoppingAltDateUtil::getNoOfDays(trx.mainDuration()) << std::endl;
  PricingTrx::DurationAltDatePairs::const_iterator iter = trx.durationAltDatePairs().begin();

  for (; iter != trx.durationAltDatePairs().end(); ++iter)
  {
    *this << "DURATION :" << ShoppingAltDateUtil::getNoOfDays(iter->first) << std::endl;
    PricingTrx::AltDatePairs::const_iterator altDatePairsIter = iter->second.begin();

    for (; altDatePairsIter != iter->second.end(); ++altDatePairsIter)
    {
      *this << altDatePairsIter->first.first.dateToString(MMDDYY, "") << "   "
            << altDatePairsIter->first.second.dateToString(MMDDYY, "") << std::endl;
    }
  }
}
}
