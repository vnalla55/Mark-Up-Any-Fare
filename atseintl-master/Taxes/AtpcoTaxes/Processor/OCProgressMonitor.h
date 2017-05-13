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
#include "DomainDataObjects/OptionalService.h"

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/core/noncopyable.hpp>
#include <vector>

namespace tax
{

class OCProgressMonitor : private boost::noncopyable
{
  std::vector<bool> _matchedOc;
  type::Index _ocValidatedCount;

public:
  OCProgressMonitor(const boost::ptr_vector<OptionalService>& optionalServices);

  bool
  isFinished() const;

  const std::vector<bool>& matchedOc() const { return _matchedOc; }

  void
  update(const boost::ptr_vector<OptionalService>& optionalServices);
};

} // end of tax namespace
