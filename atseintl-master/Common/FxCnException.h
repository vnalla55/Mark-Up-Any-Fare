#pragma once

#include <boost/regex_fwd.hpp>

#include <vector>

namespace tse
{

class PricingTrx;
class Itin;
class TravelSeg;

class FxCnException
{
public:
  FxCnException(PricingTrx& trx, const Itin& itin) : _trx(trx), _itin(itin) {}
  ~FxCnException() {}

  /**
   * Check wether FareX Chinese Exception applies
   **/
  bool operator()();

  bool checkThruMarket(const std::vector<TravelSeg*>& tvlSeg);
  bool checkThruFare();

  bool checkSingleFareBasis(const PricingTrx& trx) const;

private:
  unsigned int countPatternMatch(const char* str, const boost::regex& regex) const;

private:
  PricingTrx& _trx;
  const Itin& _itin;
};
}
