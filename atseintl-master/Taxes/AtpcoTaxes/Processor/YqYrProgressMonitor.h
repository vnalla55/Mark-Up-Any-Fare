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

#include "DataModel/Common/Types.h"
#include "Rules/ItineraryPaymentDetail.h"

#include <boost/core/noncopyable.hpp>
#include <vector>

namespace tax
{

class YqYrProgressMonitor : private boost::noncopyable
{
  std::vector<bool> _yqYrsProgress;
  type::Index _yqYrsValidatedCount;

public:
  YqYrProgressMonitor(type::Index yqyrSize);

  void
  update(const TaxableYqYrs& taxableYqYrs);

  bool
  isFinished() const;
};

} // end of tax namespace
