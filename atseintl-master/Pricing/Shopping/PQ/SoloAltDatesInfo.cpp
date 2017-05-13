#include "Pricing/Shopping/PQ/SoloAltDatesInfo.h"

#include "Common/ShoppingAltDateUtil.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{
namespace shpq
{
SoloAltDatesInfo::SoloAltDatesInfo(ShoppingTrx& trx)
  : _trx(trx),
    _altDates(trx.isAltDates()),
    _displayDiag941All(trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "ALL")
{
}

DiagCollector& operator<<(DiagCollector& dc, const SoloAltDatesInfo& soloAltDatesInfo)
{
  if (!soloAltDatesInfo._altDates || !soloAltDatesInfo._displayDiag941All ||
      soloAltDatesInfo._altDatePairsSolutionsFound.empty())
  {
    return dc;
  }

  dc << "Alternate dates results:\n";
  typedef std::map<DatePair, size_t>::value_type DatePairAndSolNum;
  for (const DatePairAndSolNum& stat : soloAltDatesInfo._altDatePairsSolutionsFound)
  {
    const DatePair& datePair = stat.first;
    dc << "Added " << stat.second << " solution(s)"
       << " for DatePair(" << datePair.first.dateToIsoExtendedString() << ", "
       << datePair.second.dateToIsoExtendedString() << ")\n";
  }

  return dc;
}
}
} // namespace tse::shpq
