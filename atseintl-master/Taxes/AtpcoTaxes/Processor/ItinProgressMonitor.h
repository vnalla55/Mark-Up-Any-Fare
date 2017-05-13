// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include <boost/noncopyable.hpp>

namespace tax
{

class ItinProgressMonitor : private boost::noncopyable
{
  bool _itineraryValidated = false;

public:

  void
  update(const TaxableItinerary& itineraryDetail)
  {
    _itineraryValidated = !itineraryDetail.isFailedRule() || _itineraryValidated;
  }

  bool
  isFinished() const
  {
    return _itineraryValidated;
  }
};

} // end of tax namespace
