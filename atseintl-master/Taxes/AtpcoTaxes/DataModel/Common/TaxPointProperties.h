// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include "Common/ConnectionTagSet.h"

#include <boost/optional.hpp>
#include <vector>

namespace tax
{
struct TaxPointProperties
{
private:
  bool _isSurface;
  bool _isCASurfaceException;

public:
  bool isFirst;
  bool isLast;
  boost::optional<bool> isTimeStopover;
  bool isFareBreak;
  ConnectionTagSet isExtendedStopover;
  boost::optional<bool> isUSTimeStopover;
  boost::optional<bool> isUSLimitStopover;
  bool isOpen;

  TaxPointProperties()
    : _isSurface(),
      _isCASurfaceException(),
      isFirst(),
      isLast(),
      isTimeStopover(),
      isFareBreak(),
      isExtendedStopover(ConnectionTagSet::none()),
      isUSTimeStopover(),
      isUSLimitStopover(),
      isOpen(false)
  {
  }

  bool isSurface() const { return _isSurface; }
  bool isSurfaceStopover() const { return _isSurface && !_isCASurfaceException; }

  void setIsSurface(bool val) { _isSurface = val; }
  void setIsCASurfaceException(bool val) { _isCASurfaceException = val; }

};

typedef std::vector<TaxPointProperties> TaxPointsProperties;
}

