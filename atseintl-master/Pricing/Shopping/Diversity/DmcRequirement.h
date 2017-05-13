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

#include "Common/TsePrimitiveTypes.h"

#include <iostream>
#include <vector>

#include <stdint.h>

namespace tse
{
class Diversity;
class ItinStatistic;
class ShoppingTrx;
class DiagCollector;

class DmcRequirement
{
public:
  enum
  {
    NEED_NONSTOPS = 0x001,
    NEED_NONSTOPS_CARRIERS = 0x002,
    NEED_ADDITIONAL_NONSTOPS = 0x004,
    NEED_CARRIERS = 0x008,
    NEED_CUSTOM = 0x010,
    NEED_JUNK = 0x020,
    NEED_LUXURY = 0x040,
    NEED_UGLY = 0x080,
    NEED_GOLD = 0x100,
    NEED_RC_ONLINES = 0x200,
    NEED_IBF_DIRECTS = 0x400,   // USED differnt than NS flag to have a correct priorities sequence
    NEED_OUTBOUNDS = 0x800,
    NEED_INBOUNDS = 0x1000, // JUNK < RC_ONLINES < NEED_IBF_DIRECTS < NEED_OUTBOUNDS/INBOUNDS for IBF sorting
    REQUIREMENTS_COUNT = 13,
    NEED_NONSTOPS_MASK = 0x07
  };
  typedef int Value;

  struct Printer
  {
    Value _rbits;
    friend std::ostream& operator<<(std::ostream& dc, const Printer& manip);
  };
  static Printer print(Value rbits)
  {
    Printer pr = { rbits };
    return pr;
  }

  struct SopInfosStatistics
  {
    SopInfosStatistics(unsigned legSize, MoneyAmount pScore, Value pStatus);

    std::vector<int32_t> minimumFlightTimeMinutes;
    MoneyAmount score;
    Value status;
  };
};

class DmcRequirementsSharedContext
{
public:
  DmcRequirementsSharedContext(Diversity& diversity,
                               const ItinStatistic& stats,
                               DiagCollector* dc,
                               const ShoppingTrx& trx)
    : _diversity(diversity), _stats(stats), _dc(dc), _trx(trx)
  {
  }

  virtual ~DmcRequirementsSharedContext() {}

  virtual void printRequirements(bool bucketsOnly) = 0;
  virtual void printCarriersRequirements(bool directOnly = false) = 0;

  Diversity& _diversity;
  const ItinStatistic& _stats;
  DiagCollector* const _dc;
  const ShoppingTrx& _trx;
};

} // ns tse

