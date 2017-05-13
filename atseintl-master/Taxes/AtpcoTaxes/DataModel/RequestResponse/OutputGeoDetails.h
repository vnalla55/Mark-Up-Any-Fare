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

#include <boost/optional.hpp>

#include "DataModel/Common/Types.h"

namespace tax
{

class OutputGeoDetails
{
public:
  OutputGeoDetails()
    : _unticketedPoint(false),
      _taxPointIndexBegin(std::numeric_limits<type::Index>::max()),
      _taxPointIndexEnd(std::numeric_limits<type::Index>::max())
  {
  }

  bool operator==(const OutputGeoDetails& r) const
  {
    return (_taxPointIndexBegin == r.taxPointIndexBegin() &&
            _taxPointIndexEnd == r.taxPointIndexEnd());
  }

  const bool& unticketedPoint() const { return _unticketedPoint; }
  bool& unticketedPoint() { return _unticketedPoint; }

  const type::Index& taxPointIndexBegin() const { return _taxPointIndexBegin; }
  type::Index& taxPointIndexBegin() { return _taxPointIndexBegin; }

  const type::Index& taxPointIndexEnd() const { return _taxPointIndexEnd; }
  type::Index& taxPointIndexEnd() { return _taxPointIndexEnd; }

  const boost::optional<type::LocCode>& taxPointLoc1() const { return _taxPointLoc1; }
  boost::optional<type::LocCode>& taxPointLoc1() { return _taxPointLoc1; }

  const boost::optional<type::LocCode>& taxPointLoc2() const { return _taxPointLoc2; }
  boost::optional<type::LocCode>& taxPointLoc2() { return _taxPointLoc2; }

  const boost::optional<type::LocCode>& taxPointLoc3() const { return _taxPointLoc3; }
  boost::optional<type::LocCode>& taxPointLoc3() { return _taxPointLoc3; }

  const boost::optional<std::string>& journeyLoc1() const { return _journeyLoc1; }
  boost::optional<std::string>& journeyLoc1() { return _journeyLoc1; }

  const boost::optional<std::string>& journeyLoc2() const { return _journeyLoc2; }
  boost::optional<std::string>& journeyLoc2() { return _journeyLoc2; }

  const std::string& pointOfSaleLoc() const { return _pointOfSaleLoc; }
  std::string& pointOfSaleLoc() { return _pointOfSaleLoc; }

  const boost::optional<std::string>& pointOfTicketingLoc() const { return _pointOfTicketingLoc; }
  boost::optional<std::string>& pointOfTicketingLoc() { return _pointOfTicketingLoc; }

private:
  bool _unticketedPoint;
  type::Index _taxPointIndexBegin;
  type::Index _taxPointIndexEnd;
  boost::optional<type::LocCode> _taxPointLoc1;
  boost::optional<type::LocCode> _taxPointLoc2;
  boost::optional<type::LocCode> _taxPointLoc3;
  boost::optional<std::string> _journeyLoc1;
  boost::optional<std::string> _journeyLoc2;
  std::string _pointOfSaleLoc;
  boost::optional<std::string> _pointOfTicketingLoc;
};
}
