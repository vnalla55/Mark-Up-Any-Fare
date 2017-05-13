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
#include "Diagnostic/LoggingService.h"
#include "ServiceInterfaces/LocService.h"

#include <boost/core/noncopyable.hpp>
#include <memory>

namespace tax
{

class LoggingLocService : public LocService
                        , public LoggingService
                        , private boost::noncopyable
{
  const LocService& base() const;
  class Impl;
  std::unique_ptr<Impl> _impl;
  Impl& impl();
  const Impl& impl() const;

public:
  typedef LocService BaseService;
  LoggingLocService(std::unique_ptr<BaseService> base);
  ~LoggingLocService();

  type::Nation getNation(const type::AirportCode& loc) const override;
  type::Nation getNationByName(const type::NationName& nationName) const override;
  type::NationName getNationName(const type::Nation& nationCode) const override;
  type::CityCode getCityCode(const type::AirportCode& loc) const override;
  type::AlaskaZone getAlaskaZone(const type::AirportCode& loc) const override;
  type::StateProvinceCode getState(const type::AirportCode& loc) const override;
  type::CurrencyCode getCurrency(const type::AirportCode& loc) const override;

  bool isInLoc(const type::AirportOrCityCode& airportCode,
               const LocZone& jrnyLoc1LocZone,
               const type::Vendor& vendor) const override;

  bool matchPassengerLocation(const type::LocCode& passengerLoc,
                              const LocZone& requiredLoc,
                              const type::Vendor& vendor) const override;
  std::string getLog() override;
};

} // namespace tax

