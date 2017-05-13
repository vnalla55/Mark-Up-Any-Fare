#pragma once

namespace tse
{

class PassPredicate : public Predicate
{
public:
  PassPredicate() : Predicate("PassPredicate") {};

  virtual PredicateReturnType
  test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx) override
  {
    PredicateReturnType retval;
    retval.valid = PASS;
    return retval;
  }

  std::string toString(int level = 0) const override
  {
    std::string s = printTabs(level);
    s += "Pass\n";
    return s;
  }
};

} // namespace tse

