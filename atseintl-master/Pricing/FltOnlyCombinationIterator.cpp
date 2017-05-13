#include "Pricing/FltOnlyCombinationIterator.h"

#include "DataModel/ShoppingTrx.h"


#include <iostream>

namespace tse
{
FltOnlyCombinationIterator::FltOnlyCombinationIterator(ShoppingTrx& trx,
                                                       const std::vector<int>& dimensions)
  : trx_(trx), pointer_(0), bucketVal_(-1)
{
  dimensions_ = dimensions;
}

bool
FltOnlyCombinationIterator::next(std::vector<int>& comb)
{
  if (bucket_.size() == pointer_)
  {
    ++bucketVal_;
    pointer_ = 0;
    if (!createBucket())
    {
      return false;
    }
    // std::cerr << "bucketVal_=" << bucketVal_ << std::endl;
  }
  comb.swap(bucket_[pointer_++].indices_);
  return true;
}

void
FltOnlyCombinationIterator::setupCombinations(std::vector<int>& indices, int remain, size_t leg)
{
  if (dimensions_.size() - 1 == leg) // last
  {
    if (remain < dimensions_[leg])
    {
      indices[leg] = remain;
      Combination combination;
      size_t numlegs(dimensions_.size());
      combination.indices_.resize(numlegs);
      for (size_t i = 0; i < numlegs; ++i)
        combination.indices_[i] = indices[i];

      bucket_.push_back(combination);
    }
  }
  else
  {
    for (; indices[leg] < std::min(dimensions_[leg], remain + 1); ++indices[leg])
      setupCombinations(indices, remain - indices[leg], leg + 1);
  }
  indices[leg] = 0;
}

bool
FltOnlyCombinationIterator::createBucket()
{
  bucket_.clear();
  size_t numlegs(dimensions_.size());
  if (LIKELY(numlegs > 0))
  {
    std::vector<int> indices(numlegs);
    setupCombinations(indices, bucketVal_, 0);
    CombinationComp comp;
    for (Combination& combination : bucket_)
      combination.calcScore(trx_);

    std::sort(bucket_.begin(), bucket_.end(), comp);
  }
  return !bucket_.empty();
}

FltOnlyCombinationIterator::Combination::Combination() : score_(0) {}

void
FltOnlyCombinationIterator::Combination::calcScore(const ShoppingTrx& trx)
{
  int leg(0);
  for (int idx : indices_)
  {
    score_ += trx.legs()[leg].sop()[idx].itin()->travelSeg().size();
    ++leg;
  }
}
} // tse
