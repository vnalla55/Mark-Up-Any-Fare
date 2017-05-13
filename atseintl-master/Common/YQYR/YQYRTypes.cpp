#include "Common/YQYR/YQYRTypes.h"

#include "Common/Assert.h"
#include "Common/YQYR/YQYRFilters.h"
#include "DBAccess/YQYRFees.h"
#include "DBAccess/YQYRFeesNonConcur.h"

namespace tse
{
namespace YQYR
{
void
FeeStorage::addFee(const YQYRFees* fee, const YQYRClassifier& classifier, DiagCollectorShopping* dc)
{
  classifier.classify(_buckets, fee, dc);
}

void
FeeStorage::copyFees(const FeeStorage& source,
                     const YQYRFilters& filters,
                     DiagCollectorShopping* dc)
{
  TSE_ASSERT(source._buckets.size() == _buckets.size());

  for (size_t i = 0; i < source._buckets.size(); ++i)
  {
    for (const YQYRFees* fee : source._buckets[i].getApplicableRecords())
    {
      if (filters.isFilteredOut(fee, dc))
        continue;

      _buckets[i].add(fee);
    }
  }
}

void
FeeStorage::copyFees(const FeeStorage& source,
                     const YQYRFilters& filters,
                     const uint32_t bucket,
                     DiagCollectorShopping* dc)
{
  TSE_ASSERT(source._buckets.size() == _buckets.size() && bucket < _buckets.size());

  for (const YQYRFees* fee : source._buckets[bucket].getApplicableRecords())
  {
    if (filters.isFilteredOut(fee, dc))
      continue;

    _buckets[bucket].add(fee);
  }
}
}
}
