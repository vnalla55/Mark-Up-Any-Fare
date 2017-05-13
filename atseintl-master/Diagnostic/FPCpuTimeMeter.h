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

#include <map>
#include <stdint.h>
#include <tr1/tuple>

#include "Common/TseConsts.h"

namespace tse
{
class DiagCollector;

namespace detail
{
struct CpuTimeMetric final
{
  double _wallTime = 0;
  double _userTime = 0;
  double _sysTime = 0;

  CpuTimeMetric() = default;
  CpuTimeMetric(double w, double u, double s) noexcept : _wallTime(w), _userTime(u), _sysTime(s) {}

  double scalar() const noexcept { return _wallTime; }

  CpuTimeMetric& operator+=(const CpuTimeMetric& rhs) noexcept
  {
    _wallTime += rhs._wallTime;
    _userTime += rhs._userTime;
    _sysTime += rhs._sysTime;
    return *this;
  }
};

struct CpuTimeMeter final
{
  CpuTimeMetric _stat;
  uint64_t _numCalls = 0;

  CpuTimeMeter() = default;

  void addStat(const CpuTimeMetric& stat) noexcept
  {
    _stat += stat;
    ++_numCalls;
  }
};

} // ns detail

class FPCpuTimeMeter final
{
public:
  using CpuTimeMetric = detail::CpuTimeMetric;

  FPCpuTimeMeter() = default;

  void addStat(size_t fpKey, size_t fpTotalCombNum, const CpuTimeMetric& stat)
  {
    Key k = { fpTotalCombNum, fpKey };
    _map[k].addStat(stat);

    _totalCpuTime += stat;
    ++_totalNumCalls;
  }

  bool isBlank() const { return (_totalCpuTime.scalar() < EPSILON); }

  friend DiagCollector& operator<<(DiagCollector& dc, const FPCpuTimeMeter& stat);

private:
  struct Key
  {
    size_t _numCombinations;
    size_t _fpKey;

    bool operator<(const Key& rhs) const
    {
      return std::tr1::tie(_numCombinations, _fpKey) <
             std::tr1::tie(rhs._numCombinations, rhs._fpKey);
    }
  };

  // CPU metrics per FP
  using CpuMetricPerFPMap = std::map<Key, detail::CpuTimeMeter>;

  CpuMetricPerFPMap _map;
  CpuTimeMetric _totalCpuTime;
  uint64_t _totalNumCalls = 0;
};

} // ns tse

