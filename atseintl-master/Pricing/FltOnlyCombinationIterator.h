#pragma once

#include <vector>

namespace tse
{
class ShoppingTrx;

class FltOnlyCombinationIterator
{
public:
  FltOnlyCombinationIterator(ShoppingTrx& trx, const std::vector<int>& dimensions);
  bool next(std::vector<int>& comb);

private:
  struct Combination
  {
    Combination();
    std::vector<int> indices_;
    void calcScore(const ShoppingTrx& trx);
    int score_;
  };
  void setupCombinations(std::vector<int>& indices, int remain, std::size_t leg);
  bool createBucket();
  struct CombinationComp
  {
    bool operator()(const Combination& first, const Combination& second) const
    {
      return first.score_ < second.score_;
    }
  };
  ShoppingTrx& trx_;
  std::vector<int> dimensions_;
  std::vector<Combination> bucket_;
  std::size_t pointer_;
  int bucketVal_;
};
} // tse
