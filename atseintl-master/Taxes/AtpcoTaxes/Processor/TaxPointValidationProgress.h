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

#include "DataModel/Common/Types.h"
#include "Processor/ItinProgressMonitor.h"
#include "Processor/OCProgressMonitor.h"
#include "Processor/YqYrProgressMonitor.h"

#include <boost/core/noncopyable.hpp>
#include <vector>

namespace tax
{
class BusinessRulesContainer;
class PaymentDetail;
struct RawSubjects;

class TaxPointValidationProgress : private boost::noncopyable
{
  YqYrProgressMonitor _yqyrProgressMonitor;
  OCProgressMonitor _ocProgressMonitor;
  ItinProgressMonitor _itinProgressMonitor;
  bool _foundItinApplication {false};
  bool _foundYqYrApplication {false};

public:
  explicit TaxPointValidationProgress(const RawSubjects& subjects);

  void
  update(const PaymentDetail& paymentDetail, const BusinessRulesContainer& rulesContainer);

  bool
  isFinished() const;

  bool& foundItinApplication() { return _foundItinApplication; }
  bool foundItinApplication() const { return _foundItinApplication; }

  bool& foundYqYrApplication() { return _foundYqYrApplication; }
  bool foundYqYrApplication() const { return _foundYqYrApplication; }

  const OCProgressMonitor& ocProgressMonitor() const { return _ocProgressMonitor; }
};
}

