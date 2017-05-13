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

#include "DomainDataObjects/Loc.h"
#include "DomainDataObjects/FlightUsage.h"

namespace tax
{
class Geo;
class Flight;

namespace LocUtil
{
bool
isMultiairport(const Geo& first, const Geo& second);
}

class Geo
{
public:
  Geo(void);
  ~Geo(void);

  Loc const& loc() const { return _loc; }

  Loc& loc() { return _loc; }

  const type::AirportCode& locCode() const { return _loc.code(); }

  const type::CityCode& cityCode() const { return _loc.cityCode(); }

  const type::Nation& getNation() const { return _loc.nation(); }

  type::Index& id() { return _id; }

  const type::Index& id() const { return _id; }

  type::UnticketedTransfer const& unticketedTransfer() const { return _unticketedTransfer; }

  type::UnticketedTransfer& unticketedTransfer() { return _unticketedTransfer; }

  bool isUnticketed() const { return _unticketedTransfer == type::UnticketedTransfer::Yes; }

  bool isFirst() const { return _id == 0; }
  bool isLast() const { return _isLast; }
  bool isDeparture() const { return _loc.tag() == type::TaxPointTag::Departure; }
  bool isArrival() const { return _loc.tag() == type::TaxPointTag::Arrival; }
  type::Index getFlightRefId() const { return _id / 2; }

  const Flight* getFlight(const std::vector<FlightUsage>& flightUsages) const
  {
    return flightUsages[getFlightRefId()].flight();
  }

  const FlightUsage* getFlightUsage(const std::vector<FlightUsage>& flightUsages) const
  {
    return &flightUsages[getFlightRefId()];
  }

  void setPrev(Geo* prev) { _prev = prev; }

  void setNext(Geo* next) { _next = next; }

  const Geo* prev() const { return _prev; }

  const Geo* next() const { return _next; }

  const Geo* samePointDifferentTag() const
  {
    if (isDeparture())
    {
      if (_prev && _prev->cityCode() == cityCode())
        return _prev;
      else
        return nullptr;
    }
    else
    {
      if (_next && _next->cityCode() == cityCode())
        return _next;
      else
        return nullptr;
    }
  }

  std::ostream& print(std::ostream& out, int indentLevel = 0, char indentChar = ' ') const;

  void makeLast();

private:
  type::Index _id;
  Loc _loc;
  type::UnticketedTransfer _unticketedTransfer;
  bool _isLast;
  Geo* _prev;
  Geo* _next;
};
}
