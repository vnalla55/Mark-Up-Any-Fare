#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "FareDisplay/Comparator.h"
#include "FareDisplay/FDConsts.h"

namespace tse
{

class PaxTypeFare;
class FDSSorting;

class FareBasisComparator : public Comparator
{
public:
  friend class FareBasisComparatorTest;

  Comparator::Result compare(const PaxTypeFare& l, const PaxTypeFare& r) override;

  void prepare(const FareDisplayTrx& trx) override;

  FDSSorting*& fdsItem() { return _fdsItem; }
  const FDSSorting* fdsItem() const { return _fdsItem; }

private:
  Comparator::Result compareFareBasis(const FareClassCode& l, const FareClassCode& r, uint16_t);
  Comparator::Result
  resolveSizeInequality(const FareClassCode& l, const FareClassCode& r, uint16_t stepNo);

  Comparator::Result result(uint16_t lhs, uint16_t rhs, const Indicator& sortType);
  uint16_t priority(const FareClassCode& code, uint16_t stepNo);
  uint16_t priority(const std::string& str, const char value = '*');
  bool lessBySize(const FareClassCode& l, const FareClassCode& r, uint16_t stepNo);
  const std::string& getCharLists(uint16_t stepNo);
  const Indicator sortType(uint16_t stepNo);
  FDSSorting* _fdsItem = nullptr;
};

} // namespace tse

