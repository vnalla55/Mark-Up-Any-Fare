#include "Pricing/SOPCombinationIterator.h"

#include "DataModel/ShoppingTrx.h"
#include "Pricing/FareUsageMatrixMap.h"
#include "Pricing/PricingUtil.h"


namespace tse
{

SOPCombinationIterator::SOPCombinationIterator(ShoppingTrx& trx, const FareUsageMatrixMap& mapping)
  : trx_(trx), pointer_(0)
{
  dimensions_ = mapping.getDimensions();
  if (!dimensions_.empty())
  {
    uint64_t sz(1), abortCheckInterval = TrxUtil::abortCheckInterval(trx);
    for (int dim : dimensions_)
      sz *= dim;

    combinations_.reserve(sz);

    std::vector<int> indices(dimensions_.size());

    sz = 0;
    setupCombinations(indices, 0, abortCheckInterval, sz);

    bool maxNbrCombset(false);
    // reset combinations

    sz = 0;
    for (Combination* combination : combinations_)
    {
      if (UNLIKELY(++sz == abortCheckInterval))
      {
        PricingUtil::checkTrxAborted(trx, 0, 0, maxNbrCombset);
        sz = 0;
      }

      std::vector<int> sops;
      mapping.getSops(combination->indices_, sops);
      combination->indices_.swap(sops);
      combination->calcScore(trx);
    }

    CombinationComp comp;
    std::sort(combinations_.begin(), combinations_.end(), comp);
  }
}

void
SOPCombinationIterator::setupCombinations(std::vector<int>& indices,
                                          size_t leg,
                                          const uint64_t& abortCheckInterval,
                                          uint64_t& cnt)
{
  if (dimensions_.size() - 1 == leg) // last
  {
    bool maxNbrCombset(false);
    for (; indices[leg] < dimensions_[leg]; ++indices[leg])
    {
      Combination* combination(nullptr);
      trx_.dataHandle().get(combination);
      if (LIKELY(combination != nullptr))
      {
        size_t numlegs(dimensions_.size());
        combination->indices_.resize(numlegs);
        for (size_t i = 0; i < numlegs; ++i)
          combination->indices_[i] = indices[i];

        combinations_.push_back(combination);

        if (UNLIKELY(++cnt == abortCheckInterval))
        {
          PricingUtil::checkTrxAborted(trx_, 0, 0, maxNbrCombset);
          cnt = 0;
        }
      }
    }
  }
  else
  {
    for (; indices[leg] < dimensions_[leg]; ++indices[leg])
      setupCombinations(indices, leg + 1, abortCheckInterval, cnt);
  }
  indices[leg] = 0;
}

SOPCombinationIterator::Combination::Combination() : score1_(0), score2_(0) {}

inline void
SOPCombinationIterator::Combination::calcScore(const ShoppingTrx& trx)
{
  for (int idx : indices_)
    score1_ += idx;

  int leg(0);
  for (int idx : indices_)
  {
    score2_ += trx.legs()[leg].sop()[idx].itin()->travelSeg().size();
    ++leg;
  }
}

bool
SOPCombinationIterator::next(std::vector<int>& combination)
{
  if (pointer_ < combinations_.size())
  {
    if (combination.empty())
    {
      combination.resize(combinations_[pointer_]->indices_.size());
    }
    // std::cerr << "combination: { ";
    for (size_t i = 0; i < combination.size(); ++i)
    {
      combination[i] = combinations_[pointer_]->indices_[i];
      // std::cerr << combination[i] << ' ';
    }
    pointer_++;
    // std::cerr << '}' << std::endl;
    return true;
  }
  return false;
}
} // tse
