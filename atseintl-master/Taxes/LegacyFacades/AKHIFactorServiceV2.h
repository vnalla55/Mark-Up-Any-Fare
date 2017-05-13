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
#pragma once

#include "Taxes/AtpcoTaxes/DataModel/Common/Types.h"
#include "Taxes/AtpcoTaxes/ServiceInterfaces/AKHIFactorService.h"

#include <memory>

namespace tse
{
class DateTime;
class DataHandle;
class TaxAkHiFactor;

class AKHIFactorServiceV2 : public tax::AKHIFactorService
{
public:
  AKHIFactorServiceV2(const DateTime& ticketingDT);
  virtual ~AKHIFactorServiceV2();

  virtual tax::type::Percent getHawaiiFactor(const tax::type::AirportCode& locCode) const override;
  virtual tax::type::Percent getAlaskaAFactor(const tax::type::AirportCode& locCode) const override;
  virtual tax::type::Percent getAlaskaBFactor(const tax::type::AirportCode& locCode) const override;
  virtual tax::type::Percent getAlaskaCFactor(const tax::type::AirportCode& locCode) const override;
  virtual tax::type::Percent getAlaskaDFactor(const tax::type::AirportCode& locCode) const override;

private:
  AKHIFactorServiceV2(const AKHIFactorServiceV2&);
  AKHIFactorServiceV2& operator=(const AKHIFactorServiceV2&);

  virtual const TaxAkHiFactor* getTaxAkHiFactors(const tax::type::AirportCode& locCode) const;

  std::unique_ptr<DataHandle> _dataHandle;
  const DateTime& _ticketingDT;
};

} // namespace tse

