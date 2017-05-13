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


#include "Processor/YqYrProgressMonitor.h"
#include "Rules/ItineraryPaymentDetail.h"

namespace tax
{

YqYrProgressMonitor::YqYrProgressMonitor(type::Index yqyrSize)
    : _yqYrsProgress(yqyrSize, false),
      _yqYrsValidatedCount(0)
{
}

void
YqYrProgressMonitor::update(const TaxableYqYrs& taxableYqYrs)
{
  if (taxableYqYrs._subject.empty())
    return;

  for (type::Index i = 0; i < _yqYrsProgress.size(); ++i)
  {
    if (!_yqYrsProgress[i] && !taxableYqYrs.isFailedRule(i))
    {
      _yqYrsProgress[i] = true;
      ++_yqYrsValidatedCount;
    }
  }
}

bool
YqYrProgressMonitor::isFinished() const
{
  return _yqYrsValidatedCount == _yqYrsProgress.size();
}

} // end of tax namespace
