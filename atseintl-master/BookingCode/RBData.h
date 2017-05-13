#pragma once

#include "Common/TseCodeTypes.h"

namespace tse
{

/* RBData holds all the information that is required to get rule tex for RB entry */
class AirSeg;
class RBDataItem;

class RBData
{

public:
  /** Gets Booking Code Seqnuence Number**/
  bool getData();

  /**adds each Item to the vector, one item is directly mapped to one key for RTG.*/
  void addItem(RBDataItem* item)
  {
    if (item)
      _rbItems.push_back(item);
  }

  bool hasNext(uint16_t count) { return (_rbItems.size() > count); }

  void setBookingCodes(std::vector<BookingCode>& bookingCodes);
  const std::vector<BookingCode>& getBookingCodes() const { return _bookingCodes; }

  const std::vector<RBDataItem*>& rbItems() const { return _rbItems; }
  std::vector<RBDataItem*>& rbItems() { return _rbItems; }
  void setLastSegmentConditional(bool lastSegConditional)
  {
    _lastSegmentConditional = lastSegConditional;
  }

  bool isLastSegmentConditional() const { return _lastSegmentConditional; }

  void setSecondary(bool secondary) { _secondary = secondary; }

  bool isSecondary() const { return _secondary; }

  AirSeg* airSeg() { return _airSeg; }
  void setAirSeg(AirSeg* airSeg) { _airSeg = airSeg; }

  void setSecondaryCityPairSameAsPrime(bool secondaryCityPairSameAsPrime)
  {
    _secondaryCityPairSameAsPrime = secondaryCityPairSameAsPrime;
  }

  bool isSecondaryCityPairSameAsPrime() const { return _secondaryCityPairSameAsPrime; }

  void setCarrierMatchedTable990(bool carrierMatchedTable990)
  {
    _carrierMatchedTable990 = carrierMatchedTable990;
  }

  bool isCarrierMatchedTable990() const { return _carrierMatchedTable990; }

private:
  bool _lastSegmentConditional = false;
  std::vector<RBDataItem*> _rbItems;
  std::vector<BookingCode> _bookingCodes;
  bool _secondary = false;
  bool _secondaryCityPairSameAsPrime = false;
  bool _carrierMatchedTable990 = false;
  AirSeg* _airSeg = nullptr;
};
}
