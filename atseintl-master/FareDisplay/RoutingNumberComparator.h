#pragma once

#include "DataModel/PaxTypeFare.h"
#include "FareDisplay/Comparator.h"
#include "FareDisplay/FDConsts.h"

namespace tse
{
class FareDisplayTrx;

class RoutingNumberComparator : public Comparator
{
public:
  Comparator::Result compare(const PaxTypeFare& l, const PaxTypeFare& r) override;
  void prepare(const FareDisplayTrx& trx) override;

private:
  class RoutingNumbers
  {
  public:
    RoutingNumbers(const PaxTypeFare& fare) : _routingNumber(fare.routingNumber())
    {
      if (fare.hasConstructedRouting())
      {
        _origAddonRtgNum = fare.origAddonRouting();
        _destAddonRtgNum = fare.destAddonRouting();
      }
    }

    RoutingNumber _routingNumber;
    RoutingNumber _origAddonRtgNum;
    RoutingNumber _destAddonRtgNum;

    bool operator<(const RoutingNumbers& key) const
    {
      if (this->_routingNumber < key._routingNumber)
        return true;
      if (this->_routingNumber > key._routingNumber)
        return false;
      if (this->_origAddonRtgNum < key._origAddonRtgNum)
        return true;
      if (this->_origAddonRtgNum > key._origAddonRtgNum)
        return false;
      if (this->_destAddonRtgNum < key._destAddonRtgNum)
        return true;
      if (this->_destAddonRtgNum > key._destAddonRtgNum)
        return false;
      return false;
    }

    bool operator>(const RoutingNumbers& key) const
    {
      if (this->_routingNumber > key._routingNumber)
        return true;
      if (this->_routingNumber < key._routingNumber)
        return false;
      if (this->_origAddonRtgNum > key._origAddonRtgNum)
        return true;
      if (this->_origAddonRtgNum < key._origAddonRtgNum)
        return false;
      if (this->_destAddonRtgNum > key._destAddonRtgNum)
        return true;
      if (this->_destAddonRtgNum < key._destAddonRtgNum)
        return false;
      return false;
    }
  };

  bool _isInternational = false;
  Comparator::Result compareAllRtgNos(const PaxTypeFare& l, const PaxTypeFare& r);
};

} // namespace tse

