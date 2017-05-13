#pragma once

#include "Common/TseCodeTypes.h"

#include <boost/unordered_map.hpp>

#include <cassert>
#include <vector>

#include <stdint.h>

namespace tse
{

class FarePathFactoryFailedPricingUnits
{
  typedef boost::unordered_map<uint32_t, std::vector<CarrierCode> > Indexes;

public:
  FarePathFactoryFailedPricingUnits() : _empty(true) {}

  void initialize(std::size_t puSize)
  {
    puIndexes_.clear();
    puIndexes_.resize(puSize);
  }

  bool empty() const { return _empty; }

  void setFailed(uint32_t puIndex, uint32_t pricingUnitIndex)
  {
     setFailed(puIndex, CarrierCode(), pricingUnitIndex);
  }

  void setFailed(uint32_t puIndex, CarrierCode cxr, uint32_t pricingUnitIndex)
  {
    assert(puIndex < puIndexes_.size());
    Indexes& indexes = puIndexes_[puIndex];

    if(cxr.empty())
       indexes.insert(std::make_pair(pricingUnitIndex,std::vector<CarrierCode>()));
    else
    {
       Indexes::iterator it = indexes.find(pricingUnitIndex);
       if(it != indexes.end())
       {
          it->second.push_back(cxr);
       }
       else
       {
         indexes.insert(std::make_pair(pricingUnitIndex,std::vector<CarrierCode>(1,cxr)));
       }
    }
    _empty = false;
  }

  bool isFailed(uint32_t puIndex, uint32_t pricingUnitIndex) const
  {
     std::vector<CarrierCode> dummyParam;
     return isFailed(puIndex, dummyParam, dummyParam, pricingUnitIndex);
  }

  bool isFailed(uint32_t puIndex,
                const std::vector<CarrierCode>& valCxrs,
                std::vector<CarrierCode>& failedValCxr,
                uint32_t pricingUnitIndex) const
  {
    assert(puIndex < puIndexes_.size());
    const Indexes& indexes = puIndexes_[puIndex];
    if(valCxrs.empty())
    {
      return indexes.find(pricingUnitIndex) != indexes.end();
    }
    else
    {
       Indexes::const_iterator it = indexes.find(pricingUnitIndex);
       if(it != indexes.end())
       {
         if((*it).second.empty())
         {
           return false;
         }

         bool failed = true;
         for (CarrierCode cxr : valCxrs)
         {
           if(std::find((*it).second.begin(), (*it).second.end(), cxr) == (*it).second.end())
           {
             failed = false;
           }
           else
           {
             failedValCxr.push_back(cxr);
           }
         }
         return failed;
       }
    }
    return false;
  }

  void clear()
  {
    for (auto& indexes : puIndexes_)
    {
      indexes.clear();
    }

    _empty = true;
  }

private:
  FarePathFactoryFailedPricingUnits(const FarePathFactoryFailedPricingUnits&);
  void operator=(const FarePathFactoryFailedPricingUnits&);

private:
  bool _empty;
  std::vector<Indexes> puIndexes_;
};

} // namespace tse

