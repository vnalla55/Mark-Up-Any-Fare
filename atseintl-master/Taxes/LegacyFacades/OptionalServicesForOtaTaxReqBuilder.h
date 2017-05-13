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

#include <vector>
#include "DataModel/Common/Types.h"

namespace tse
{
class Itin;
}

namespace tax
{
class InputRequest;
class InputItin;

class OptionalServicesForOtaTaxReqBuilder
{
  friend class OptionalServicesForOtaTaxReqBuilderTest;
public:
  OptionalServicesForOtaTaxReqBuilder(tse::PricingTrx& trx,
                                      InputRequest& inputRequest,
                                      tax::InputItin& itin,
                                      const tse::FarePath& tseFarePath);
  virtual ~OptionalServicesForOtaTaxReqBuilder();

  void addOptionalServices();

private:
  OptionalServicesForOtaTaxReqBuilder(const OptionalServicesForOtaTaxReqBuilder& builder);
  OptionalServicesForOtaTaxReqBuilder& operator=(const OptionalServicesForOtaTaxReqBuilder& builder);

  void buildMappings();

  void addGeoPathMappings();
  void addOCFees(const tse::FareUsage* fareUsage);

  tse::PricingTrx& _trx;
  InputRequest& _inputRequest;
  InputItin& _itin;
  const tse::FarePath& _tseFarePath;

};

} // end of tax
