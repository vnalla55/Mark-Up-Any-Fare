// -------------------------------------------------------------------
//
//  Copyright (C) Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
// -------------------------------------------------------------------

#pragma once

#include "Common/Assert.h"
#include "DataModel/NegPaxTypeFareData.h"
#include "DataModel/NegPaxTypeFareDataComparator.h"
#include "DataModel/NegPTFBucket.h"


#include <vector>

namespace tse
{
template <typename Comparator, typename Bucket = NegPTFBucket>
class NegPTFBucketContainer
{
  friend class NegPTFBucketContainerTest;

public:
  typedef std::vector<Bucket> InnerVec;
  typedef typename InnerVec::iterator iterator;
  typedef typename InnerVec::const_iterator const_iterator;

  void insert(const NegPaxTypeFareData& fareData)
  {
    if(!fareData.validatingCxr.empty())
    {
      for (Bucket& bucket : _buckets)
      {
        if (_comparator.areSameButValCxrNot(bucket.getRepresentative(), fareData))
        {
          bucket.getRepresentative().ptf->validatingCarriers().push_back(fareData.validatingCxr);
          return;
        }
        for (NegPTFBucket::R3NegFares& r3NegFares : bucket)
        {
          for (NegPaxTypeFareData& negPtfData : r3NegFares)
          {
            if (_comparator.areSameButValCxrNot(negPtfData, fareData))
            {
              bucket.addCxrToTypeFare(negPtfData, fareData);
              return;
            }
          }
        }
      }
    }

    for (Bucket& bucket : _buckets)
    {
      if (_comparator.areEquivalent(bucket.getRepresentative(), fareData))
      {
        bucket.insert(fareData);
        return;
      }
    }
    _buckets.resize(_buckets.size() + 1);
    _buckets.back().insert(fareData);
  }

  size_t size() const { return _buckets.size(); }
  bool empty() const { return _buckets.empty(); }
  iterator begin() { return _buckets.begin(); }
  iterator end() { return _buckets.end(); }
  const Bucket& operator[](const size_t& i) const { return _buckets[i]; }

  Comparator& getComparator() { return _comparator; }

  template <typename Validator>
  void collectValidNegFares(Validator validator, std::set<PaxTypeFare*>& result) const;

private:
  Comparator _comparator;
  InnerVec _buckets;
};

template <typename Comparator, typename Bucket>
template <typename Validator>
void
NegPTFBucketContainer<Comparator, Bucket>::collectValidNegFares(Validator validator,
                                                                std::set<PaxTypeFare*>& result)
    const
{
  result.clear();
  std::set<PaxTypeFare*> partialResult;
  for (const Bucket& bucket : _buckets)
  {
    bucket.collectValidNegFares(validator, partialResult);
    result.insert(partialResult.begin(), partialResult.end());
  }
}
}

