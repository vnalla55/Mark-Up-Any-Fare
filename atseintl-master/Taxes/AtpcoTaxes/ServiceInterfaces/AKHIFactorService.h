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

#include "DataModel/Common/Types.h"

namespace tax
{

class AKHIFactorService
{
public:
  AKHIFactorService() {}
  virtual ~AKHIFactorService() {}

  virtual type::Percent getHawaiiFactor(const type::AirportCode& locCode) const = 0;
  virtual type::Percent getAlaskaAFactor(const type::AirportCode& locCode) const = 0;
  virtual type::Percent getAlaskaBFactor(const type::AirportCode& locCode) const = 0;
  virtual type::Percent getAlaskaCFactor(const type::AirportCode& locCode) const = 0;
  virtual type::Percent getAlaskaDFactor(const type::AirportCode& locCode) const = 0;
};
}

