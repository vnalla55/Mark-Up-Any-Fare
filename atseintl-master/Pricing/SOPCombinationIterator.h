
#include <vector>

#include <stdint.h>

namespace tse
{
class ShoppingTrx;
class FareUsageMatrixMap;

class SOPCombinationIterator
{
public:
  SOPCombinationIterator(ShoppingTrx& trx, const FareUsageMatrixMap& mapping);
  ~SOPCombinationIterator() {}

  bool next(std::vector<int>& combination);

private:
  struct Combination
  {
    Combination();

    std::vector<int> indices_;
    std::size_t score1_, score2_;
    inline void calcScore(const ShoppingTrx& trx);
  };
  struct CombinationComp
  {
    bool operator()(const Combination* first, const Combination* second) const
    {
      if (first->score1_ < second->score1_)
      {
        return true;
      }
      else if (first->score1_ > second->score1_)
      {
        return false;
      }
      return first->score2_ < second->score2_;
    }
  };

  void setupCombinations(std::vector<int>& indices,
                         std::size_t leg,
                         const uint64_t& abortCheckInterval,
                         uint64_t& cnt);
  ShoppingTrx& trx_;
  std::vector<int> dimensions_;
  std::vector<Combination*> combinations_;
  std::size_t pointer_;
};
} // tse
