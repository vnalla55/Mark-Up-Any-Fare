// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#pragma once

#include "DataModel/Common/Types.h"
#include "DomainDataObjects/Geo.h"
#include "Rules/PaymentDetail.h"

namespace tax
{

class TaxPointIterator
{

public:
  TaxPointIterator(Geo const& geo) : _geo(&geo)
  {
    if (geo.isDeparture())
      _isReversed = false;
    else if (geo.isArrival())
      _isReversed = true;
    else
      throw std::logic_error("TaxPointIterator called with an unknown tax point tag.");
  }

  bool isValid() const { return _geo != nullptr; }

  TaxPointIterator& decrement(const bool ticketedOnly)
  {
    const Geo* geo = nullptr;
    if (_isReversed)
    {
      geo = doubleNext(_geo);

      if (ticketedOnly)
      {
        while (geo && geo->isUnticketed())
          geo = doubleNext(geo);
      }
    }
    else
    {
      geo = doublePrev(_geo);
      if (ticketedOnly)
      {
        while (geo && geo->isUnticketed())
          geo = doublePrev(geo);
      }
    }
    _geo = geo;
    return *this;
  }

  Geo const* getGeo() const { return _geo; }

private:
  bool _isReversed;
  const Geo* _geo;
  const Geo* doubleNext(const Geo* geo)
  {
    if (geo == nullptr)
      return nullptr;
    if (geo->next() == nullptr)
      return nullptr;
    return geo->next()->next();
  }
  const Geo* doublePrev(const Geo* geo)
  {
    if (geo == nullptr)
      return nullptr;
    if (geo->prev() == nullptr)
      return nullptr;
    return geo->prev()->prev();
  }
};

class TaxPointUtil
{
public:
  TaxPointUtil(const PaymentDetail& pd) : _paymentDetail(pd) {}

  bool checkIfMatchesStopoverTagCondition(type::Loc2StopoverTag loc2StopoverTag,
                                          type::Index index,
                                          type::Index furthestIndex) const
  {
    if (loc2StopoverTag == type::Loc2StopoverTag::Stopover)
    {
      return _paymentDetail.isStopover(index);
    }
    else if (loc2StopoverTag == type::Loc2StopoverTag::FareBreak)
    {
      return _paymentDetail.isFareBreak(index);
    }
    else if (loc2StopoverTag == type::Loc2StopoverTag::Furthest)
    {
      return (furthestIndex == index);
    }

    return false;
  }

  bool isStop(Geo const& geo) const { return _paymentDetail.isStopover(geo.id()); }

  type::Index findNearestFareBreak(int direction, size_t geosSize) const
  {
    for (type::Index i = _paymentDetail.getTaxPointLoc1().id() + direction; i < geosSize;
         i += direction)
    {
      if (_paymentDetail.isFareBreak(i))
      {
        return i;
      }
    }

    return type::Index((direction == 1) ? geosSize - 1 : 0);
  }

private:
  const PaymentDetail& _paymentDetail;
};

class TaxPointUtils
{
public:
  typedef std::pair<type::Index, type::Index> TRange;
  TaxPointUtils(const TaxPointsProperties& properties) : _properties(properties) {}

  bool isBeginOrEnd(type::Index id, const TRange& range) const
  {
    return id == range.first || id == range.second;
  }

  bool isOpen(type::Index id, const TRange& range) const
  {
    return _properties[id].isOpen && !isBeginOrEnd(id, range);
  }

  bool isStopover(type::Index id) const
  {
    const TaxPointProperties& properties = _properties[id];
    return properties.isFirst || properties.isLast ||
           properties.isSurfaceStopover() ||
           (properties.isTimeStopover && *properties.isTimeStopover) ||
           properties.isExtendedStopover;
  }

  bool isFareBreak(type::Index id, const TRange& range) const
  {
    return _properties[id].isFareBreak || isBeginOrEnd(id, range);
  }

  bool isFeeBreak(type::Index id, const TRange& range) const
  {
    return isBeginOrEnd(id, range);
  }

private:
  const TaxPointsProperties& _properties;
};
}

