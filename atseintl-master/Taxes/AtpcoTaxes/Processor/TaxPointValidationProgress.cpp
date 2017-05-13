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

#include "Processor/RawSubjectsCollector.h"
#include "Processor/TaxPointValidationProgress.h"

#include "Rules/BusinessRulesContainer.h"
#include "Rules/PaymentDetail.h"

namespace tax
{
TaxPointValidationProgress::TaxPointValidationProgress(const RawSubjects& subjects)
  : _yqyrProgressMonitor(subjects._yqyrs._subject.size()),
    _ocProgressMonitor(subjects._itinerary.optionalServiceItems()),
    _itinProgressMonitor()
{
}

void
TaxPointValidationProgress::update(const PaymentDetail& paymentDetail,
                                   const BusinessRulesContainer& rulesContainer)
{
  if (rulesContainer.taxableUnits().hasTag(type::TaxableUnit::Itinerary) ||
      rulesContainer.taxableUnits().hasTag(type::TaxableUnit::TaxOnTax))
  {
    _itinProgressMonitor.update(paymentDetail.getItineraryDetail());

    if (!paymentDetail.getItineraryDetail().isFailedRule())
      _foundItinApplication = true;
  }

  if (rulesContainer.taxableUnits().hasTag(type::TaxableUnit::YqYr))
  {
    _yqyrProgressMonitor.update(paymentDetail.getYqYrDetails());

    if (!paymentDetail.getYqYrDetails().areAllFailed())
      _foundYqYrApplication = true;
  }

  _ocProgressMonitor.update(paymentDetail.optionalServiceItems());
}

bool
TaxPointValidationProgress::isFinished() const
{
  return _itinProgressMonitor.isFinished() &&
      _yqyrProgressMonitor.isFinished() &&
      _ocProgressMonitor.isFinished();
}

} // end of tax namespace
