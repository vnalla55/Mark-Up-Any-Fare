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

#include "Diagnostic/FPCpuTimeMeter.h"

#include "Diagnostic/DiagCollector.h"

#include <algorithm>
#include <iomanip>

namespace tse
{

DiagCollector& operator<<(DiagCollector& dc, const FPCpuTimeMeter& stat)
{
  if (stat.isBlank())
    return dc;

  static const std::string TITLE_1 = "FARE PATH COMBINATION COUNT",
                           TITLE_2 = "DIVERSITY MODEL CALLS", TITLE_3 = "CPU TIME%",
                           TITLE_4 = "DM CALLS RATIO", TITLE_5 = "CPU TIME RATIO",
                           SEPARATOR = " | ";

  dc << TITLE_1 << SEPARATOR << TITLE_2 << SEPARATOR << TITLE_3 << SEPARATOR << TITLE_4 << SEPARATOR
     << TITLE_5 << "\n";

  for (const FPCpuTimeMeter::CpuMetricPerFPMap::value_type& fpStat : stat._map)
  {
    dc << std::setw(TITLE_1.size()) << std::right << fpStat.first._numCombinations;

    dc << SEPARATOR;
    dc << std::setw(TITLE_2.size()) << std::right << fpStat.second._numCalls;

    const double cpuRatio = fpStat.second._stat.scalar() / stat._totalCpuTime.scalar();
    const double dmCallsRatio = (double)fpStat.second._numCalls / stat._totalNumCalls;

    dc << SEPARATOR;
    dc << std::right << std::fixed << std::setprecision(2)
       << std::setw(std::max<int>(5, TITLE_3.size() - 2)) << (cpuRatio * 100.) << " %";

    dc << SEPARATOR;
    {
      const int barLength = static_cast<int>(dmCallsRatio * TITLE_4.size() + /*round*/ 0.5);
      if (barLength)
        dc << std::setfill('.') << std::right << std::setw(barLength) << '.';

      // padding for the next column
      dc << std::setfill(' ') << std::setw(TITLE_4.size() - barLength) << ' ';
    }

    dc << SEPARATOR;
    {
      const int barLength = static_cast<int>(cpuRatio * TITLE_5.size() + /*round*/ 0.5);
      if (barLength)
        dc << std::setfill('.') << std::right << std::setw(barLength) << '.';

      dc << std::setfill(' '); // restore previous state
    }

    dc << "\n";
  }

  return dc;
}

} // ns tse
