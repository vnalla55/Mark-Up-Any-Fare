// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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

#include "Taxes/LegacyFacades/AKHIFactorServiceV2.h"
#include "DBAccess/DataHandle.h"
#include "Common/DateTime.h"
#include "DBAccess/TaxAkHiFactor.h"

namespace tse
{

class AKHIFactorServiceV2Mock : public AKHIFactorServiceV2
{
public:
  AKHIFactorServiceV2Mock(const DateTime& ticketingDT)
    : AKHIFactorServiceV2(ticketingDT)
  {
  }

  virtual ~AKHIFactorServiceV2Mock() {}

  TaxAkHiFactor& taxAkHiFactor() { return _taxAkHiFactor; }

private:
  virtual const TaxAkHiFactor* getTaxAkHiFactors(const tax::type::AirportCode& locCode) const
  {
    return &_taxAkHiFactor;
  }

  TaxAkHiFactor _taxAkHiFactor;
};

} // namespace tse
