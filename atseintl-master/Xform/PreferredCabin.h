#pragma once

#include "Common/CabinType.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

#include <tuple>
#include <vector>


namespace tse
{
class Cabin;
class PricingTrx;

typedef std::tuple<BookingCode, CarrierCode, LocCode, LocCode> BkkNode;

class Logger;

class PreferredCabin
{
public:
  PreferredCabin();

  void selectCabin();

  void addBkkNode(const BookingCode&,  const CarrierCode&, const LocCode&, const LocCode&);
  void setPreferredCabinInd(Indicator preferredCabin);
  const CabinType& getPreferredCabin() const;

  Indicator getBkkPreferredCabinInd() const;
  void setBkkPreferredCabinInd(Indicator bkkPreferredCabinInd);

  void setup(PricingTrx* trx);

  void setClass(Indicator ch);
  void setClassFromAlphaNum(char ch);
  void setLegId(const uint32_t& lid) { _legId = lid; }
protected:
  virtual const Cabin* getCabin(TravelSeg* tvl);

private:
  PricingTrx* _trx;
  std::vector<BkkNode> _bkkNodes;

  CabinType _preferredCabin;
  Indicator _bkkPreferredCabinInd;
  uint32_t  _legId;

  static Logger _logger;
};
}
