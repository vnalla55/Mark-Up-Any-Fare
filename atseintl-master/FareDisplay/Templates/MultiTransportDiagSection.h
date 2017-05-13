//-------------------------------------------------------------------
//
//  Copyright Sabre 2005
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#pragma once

#include "Common/MultiTransportMarkets.h"
#include "Common/SmallBitSet.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/FareDisplayTrx.h"
#include "FareDisplay/Templates/Section.h"

#include <map>
#include <vector>

namespace tse
{
class FareMarket;

class MultiTransportDiagSection : public Section
{
private:
  class MultiTransportMapElem
  {
  protected:
    enum MultiTransportFareSetsFlag
    {
      MTT_isPublished = 0x01,
      MTT_isConstructed = 0x02,
      MTT_returnedPublished = 0x04,
      MTT_returnedConstructed = 0x08,
      MTT_returnedCreated = 0x10
    };
    typedef SmallBitSet<uint8_t, MultiTransportFareSetsFlag> MultiTransportFareSetsFlags;

  public:
    // Main constructor
    MultiTransportMapElem(const LocCode& orig, const LocCode& dest)
      : _origin(orig), _destination(dest)
    {
    }

    const LocCode& origin() const { return _origin; }
    const LocCode& destination() const { return _destination; }
    static LocCode& requestedOrigin() { return _requestedOrigin; }
    static LocCode& requestedDestination() { return _requestedDestination; }

    bool operator==(const MultiTransportMapElem& key) const
    {
      // compare key class members
      return ((key._origin == _origin) && (key._destination == _destination));
    }

    bool operator<(const MultiTransportMapElem& sec) const
    {
      if (*this == sec)
        return false;
      else if (_origin == _requestedOrigin && _destination == _requestedDestination)
        return true;
      else if (sec._origin == _requestedOrigin && sec._destination == _requestedDestination)
        return false;
      // group by origin, destination
      else if (_origin < sec._origin)
        return true;
      else if (_origin > sec._origin)
        return false;
      else if (_destination < sec._destination)
        return true;
      return false;
    }

    bool isPublishedFare() const { return _multiTransportSet.isSet(MTT_isPublished); }
    bool isReturnedPublishedFare() const { return _multiTransportSet.isSet(MTT_returnedPublished); }
    bool isConstructedFare() const { return _multiTransportSet.isSet(MTT_isConstructed); }
    bool isReturnedConstructedFare() const
    {
      return _multiTransportSet.isSet(MTT_returnedConstructed);
    }
    bool isReturnedCreated() const { return _multiTransportSet.isSet(MTT_returnedCreated); }

    void setPublishedFare(bool val = true) { _multiTransportSet.set(MTT_isPublished, val); }
    void setReturnedPublishedFare(bool val = true)
    {
      _multiTransportSet.set(MTT_returnedPublished, val);
    }
    void setConstructedFare(bool val = true) { _multiTransportSet.set(MTT_isConstructed, val); }
    void setReturnedConstrucedFare(bool val = true)
    {
      _multiTransportSet.set(MTT_returnedConstructed, val);
    }
    void setReturnedCreatedFare(bool val = true)
    {
      _multiTransportSet.set(MTT_returnedCreated, val);
    }

    std::string& locOriginType() { return _locOriginType; }
    std::string& locDestinationType() { return _locDestinationType; }

    std::string isFare()
    {
      if (isPublishedFare() || isConstructedFare())
        return "Y";
      return "N";
    }

    std::string isReturnedFare()
    {
      if (isReturnedPublishedFare() || isReturnedConstructedFare() || isReturnedCreated())
        return "Y";
      return "N";
    }

  private:
    LocCode _origin;
    LocCode _destination;
    std::string _locOriginType;
    std::string _locDestinationType;
    MultiTransportFareSetsFlags _multiTransportSet;

    static LocCode _requestedOrigin;
    static LocCode _requestedDestination;
  };

public:
  MultiTransportDiagSection(FareDisplayTrx& trx) : Section(trx) { _trx.response().clear(); }

  void buildDisplay() override;

protected:
  bool initMap();
  void buildHeader();
  void buildRequestLine(const FareMarket& fm) const;
  bool isMarketHasPublishedFare(const MultiTransportMarkets::Market& market, const FareMarket& fm);
  MultiTransportMapElem* getMultiTransportMapElem(const CarrierCode& carrier,
                                                  const LocCode& origin,
                                                  const LocCode& destination);
  MultiTransportMapElem* putMultiTransportMapElem(const CarrierCode& carrier,
                                                  const LocCode& origin,
                                                  const LocCode& destination);

private:
  std::map<CarrierCode, std::set<MultiTransportMapElem>> _multiTransportMap;
};
} // namespace tse
