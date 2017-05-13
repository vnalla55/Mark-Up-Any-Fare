#pragma once

#include "Common/TseCodeTypes.h"
#include "DataModel/Trx.h"

#include <vector>

namespace tse
{
class Loc;
class PaxType;
class YQYRFees;

namespace YQYR
{
class YQYRClassifier;
class YQYRFilters;
class DiagCollectorShopping;
struct DiagStream
{
public:
  ~DiagStream() { _trx.diagnostic().insertDiagMsg(_oss.str()); }
  DiagStream(const DiagStream& other) : _trx(other._trx) {}

  std::ostream& operator<<(const char* msg)
  {
    _oss << msg;
    return _oss;
  }

  std::ostream& operator<<(const uint32_t value)
  {
    _oss << value;
    return _oss;
  }

  std::ostream& operator<<(std::ostream& (*pf)(std::ostream&)) { return pf(_oss); }

private:
  friend class DiagCollectorShopping;

  DiagStream(Trx& trx) : _trx(trx) {}

  Trx& _trx;
  std::ostringstream _oss;
};

struct DiagCollectorShopping
{
public:
  DiagCollectorShopping(Trx& trx) : _trx(trx) {}
  DiagStream diagStream() { return DiagStream(_trx); }

private:
  Trx& _trx;
};

class YQYRBucket
{
public:
  YQYRBucket(const Loc* furthestPoint) : _furthestPoint(furthestPoint) {}
  YQYRBucket(const Loc* furthestPoint, const PaxType* paxType)
    : _furthestPoint(furthestPoint), _paxType(paxType)
  {
  }

  const Loc* getFurthestPoint() const { return _furthestPoint; }
  const PaxType* getPaxType() const { return _paxType; }
  const std::vector<const YQYRFees*>& getApplicableRecords() const { return _recordsApplicable; }

  void add(const YQYRFees* fee) { _recordsApplicable.push_back(fee); }

  template <class FeeAppl>
  void processFees(const CarrierCode &carrier, FeeAppl& application) const
  {
    if (_recordsApplicable.empty() || !_furthestPoint)
    {
      if (UNLIKELY(application.getDc()))
      {
        DiagStream stream(application.getDc()->diagStream());
        stream << " - NO RECORDS APPLICABLE OR FURTHEST POINT IS NOT SET FOR THE REQUESTED BUCKET"
               << std::endl;
      }
      return;
    }

    application.matchFees(carrier, _recordsApplicable);
  }

private:
  YQYRBucket() : _furthestPoint(nullptr) {}

  const Loc* _furthestPoint;
  const PaxType* _paxType = nullptr;
  std::vector<const YQYRFees*> _recordsApplicable;
};

struct FeeStorage
{
  FeeStorage(const std::vector<YQYRBucket>& originalBuckets)
  {
    _buckets.reserve(originalBuckets.size());
    for (const YQYRBucket& bucket : originalBuckets)
      _buckets.push_back(YQYRBucket(bucket));
  }

  void addFee(const YQYRFees* fee, const YQYRClassifier& classifier, DiagCollectorShopping* dc);

  void copyFees(const FeeStorage& source, const YQYRFilters& filters, DiagCollectorShopping* dc);

  void copyFees(const FeeStorage& source,
                const YQYRFilters& filters,
                const uint32_t bucket,
                DiagCollectorShopping* dc);

  template <class FeeAppl>
  void
  processBucket(const uint16_t bucketIndex, const CarrierCode& carrier, FeeAppl& application) const
  {
    const YQYRBucket& bucket(_buckets[bucketIndex]);
    bucket.processFees(carrier, application);
  }

private:
  std::vector<YQYRBucket> _buckets;
};

struct CarrierStorage
{
  bool isInitialized() const { return !_feesPerCode.empty(); }

  std::vector<FeeStorage> _feesPerCode;
};
}
}
