#pragma once

#include <vector>

namespace tse
{
class FarePath;
class PricingUnit;
class FareUsage;

class FareUsageIter
{
public:
  typedef std::vector<FareUsage*>::iterator iterator;
  typedef std::vector<FareUsage*>::const_iterator const_iterator;

  FareUsageIter(const FarePath& fp) : _fp(fp) { init(fp); }
  FareUsageIter(const FarePath& fp, const std::vector<PricingUnit*>& sideTripPu) : _fp(fp)
  {
    init(sideTripPu);
  }

  iterator begin() { return _fuv.begin(); }
  iterator end() { return _fuv.end(); }

  const std::vector<FareUsage*>& fareUsages() { return _fuv; }

private:
  void init(const FarePath& fp);
  void init(const std::vector<PricingUnit*>& sideTrip);
  void init(std::vector<FareUsage*>& fus);

private:
  const FarePath& _fp;
  std::vector<FareUsage*> _fuv;
};

} // namespace tse

