#ifndef REXFARESELECTORSTRATEGY_TEST_UTILS_H
#define REXFARESELECTORSTRATEGY_TEST_UTILS_H

#include "RexPricing/PaxTypeFareWrapper.h"

namespace tse
{

inline std::ostream& operator<<(std::ostream& out, std::vector<PaxTypeFareWrapper>::iterator it)
{
  return out << &(*it);
}

inline std::ostream& operator<<(std::ostream& os, const PaxTypeFareWrapper& wrp)
{
  return os << wrp.get();
}

inline bool operator==(const PaxTypeFareWrapper& a, const PaxTypeFareWrapper& b)
{
  return a.get() == b.get();
}

} // tse

#endif
