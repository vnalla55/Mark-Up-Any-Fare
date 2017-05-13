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

#include "Taxes/LegacyFacades/V2TrxMappingDetails.h"
#include "Taxes/AtpcoTaxes/DataModel/RequestResponse/OutputTaxDetails.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/ServiceFeesGroup.h"
#include "DataModel/BaggageCharge.h"
#include "DataModel/BaggageTravel.h"

namespace tax
{
class TaxOnOcConverter
{
  const V2TrxMappingDetails::OptionalServicesRefs& _optServicesMap;
  std::shared_ptr<OutputTaxDetails>& _outputTaxDetails;
  tse::FarePath* _farePath;
  V2TrxMappingDetails::OptionalServicesRefs::const_iterator _ocFeesIterator;

  int
  getSegmentIndex() const;

  tse::OCFees*
  getOCFees() const;

public:
  TaxOnOcConverter(const V2TrxMappingDetails::OptionalServicesRefs& optServicesMap,
                   std::shared_ptr<OutputTaxDetails>& outputTaxDetails,
                   tse::FarePath* farePath);

  void
  convert(tse::OCFees::TaxItem* taxItem) const;
};
}
