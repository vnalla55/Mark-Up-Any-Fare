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

#include "Processor/OCProgressMonitor.h"

namespace tax
{
OCProgressMonitor::OCProgressMonitor(const boost::ptr_vector<OptionalService>& optionalServices)
    : _matchedOc(optionalServices.size(), false)
{
}

void
OCProgressMonitor::update(const boost::ptr_vector<OptionalService>& optionalServices)
{

  for (type::Index index = 0; index < optionalServices.size(); ++index)
  {
    if (!_matchedOc[index] && !optionalServices[index].isFailed())
    {
      _matchedOc.at(index) = true;
      ++_ocValidatedCount;
    }
  }
}

bool
OCProgressMonitor::isFinished() const
{
  return _ocValidatedCount == _matchedOc.size();
}
} // end of tax namespace
