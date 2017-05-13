#pragma once

#include "Common/TseCodeTypes.h"
#include "DataModel/FareMarket.h"


namespace tse
{
class FareMarket;
class Loc;
}

namespace tse
{
namespace shpq
{


// MergedFMKey - unique key merged fare markets
// For each O&D and carrier is created one merged fare market
class MergedFMKey
{
public:
  explicit MergedFMKey(const FareMarket*);

  const Loc* origin() const { return _origin; }
  const Loc* destination() const { return _destination; }
  const CarrierCode& carrier() const { return _carrier; }
  const FareMarket::SOL_FM_TYPE fmType() const { return _fmType; }

private:
  const Loc* _origin;
  const Loc* _destination;
  CarrierCode _carrier;
  FareMarket::SOL_FM_TYPE _fmType;
};

struct MFMKeyLess
{
  bool operator()(const MergedFMKey& lhp, const MergedFMKey& rhp) const;
};

// Key value to find unique fare market path (FMPath) across Solution Patterns
// Unique values: O&D and Cxr for each Merged Fare Market from FMPath
class SoloFMPathKey
{
  typedef std::vector<MergedFMKey> MFMKeyVector;

public:
  typedef MFMKeyVector::const_iterator const_iterator;
  SoloFMPathKey();
  SoloFMPathKey(const FareMarket*);
  SoloFMPathKey(const FareMarket*, const FareMarket*);

  void addFareMarket(const FareMarket*);
  void clear() { _mfmKeys.clear(); }
  const_iterator begin() const { return _mfmKeys.begin(); }
  const_iterator end() const { return _mfmKeys.end(); }

private:
  MFMKeyVector _mfmKeys;
};

struct SoloFMPathKeyLess
{
  bool operator()(const SoloFMPathKey&, const SoloFMPathKey&) const;
};
}
} // namespace tse::shpq

