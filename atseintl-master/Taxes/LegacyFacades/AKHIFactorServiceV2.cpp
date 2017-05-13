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
#include "Taxes/LegacyFacades/ConvertCode.h"
#include "Common/DateTime.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/TaxAkHiFactor.h"
#include "Taxes/AtpcoTaxes/Common/MoneyUtil.h"

namespace tse
{

AKHIFactorServiceV2::AKHIFactorServiceV2(const DateTime& ticketingDT)
  : _dataHandle(new DataHandle(ticketingDT)), _ticketingDT(ticketingDT)
{
}

AKHIFactorServiceV2::~AKHIFactorServiceV2() {}

tax::type::Percent
AKHIFactorServiceV2::getHawaiiFactor(const tax::type::AirportCode& locCode) const
{
  const TaxAkHiFactor* taxAkHiFactor = getTaxAkHiFactors(locCode);
  if (taxAkHiFactor)
  {
    return tax::doubleToPercent(taxAkHiFactor->hawaiiPercent());
  }

  return tax::type::Percent();
}

tax::type::Percent
AKHIFactorServiceV2::getAlaskaAFactor(const tax::type::AirportCode& locCode) const
{
  const TaxAkHiFactor* taxAkHiFactor = getTaxAkHiFactors(locCode);
  if (taxAkHiFactor)
  {
    return tax::doubleToPercent(taxAkHiFactor->zoneAPercent());
  }

  return tax::type::Percent();
}

tax::type::Percent
AKHIFactorServiceV2::getAlaskaBFactor(const tax::type::AirportCode& locCode) const
{
  const TaxAkHiFactor* taxAkHiFactor = getTaxAkHiFactors(locCode);
  if (taxAkHiFactor)
  {
    return tax::doubleToPercent(taxAkHiFactor->zoneBPercent());
  }

  return tax::type::Percent();
}

tax::type::Percent
AKHIFactorServiceV2::getAlaskaCFactor(const tax::type::AirportCode& locCode) const
{
  const TaxAkHiFactor* taxAkHiFactor = getTaxAkHiFactors(locCode);
  if (taxAkHiFactor)
  {
    return tax::doubleToPercent(taxAkHiFactor->zoneCPercent());
  }

  return tax::type::Percent();
}

tax::type::Percent
AKHIFactorServiceV2::getAlaskaDFactor(const tax::type::AirportCode& locCode) const
{
  const TaxAkHiFactor* taxAkHiFactor = getTaxAkHiFactors(locCode);
  if (taxAkHiFactor)
  {
    return tax::doubleToPercent(taxAkHiFactor->zoneDPercent());
  }

  return tax::type::Percent();
}

const TaxAkHiFactor*
AKHIFactorServiceV2::getTaxAkHiFactors(const tax::type::AirportCode& locCode) const
{
  const std::vector<TaxAkHiFactor*>& taxAkHiVect =
      _dataHandle->getTaxAkHiFactor(toTseAirportCode(locCode), _ticketingDT);

  if (!taxAkHiVect.empty())
  {
    return taxAkHiVect.front();
  }

  return nullptr;
}

} // namespace tse
