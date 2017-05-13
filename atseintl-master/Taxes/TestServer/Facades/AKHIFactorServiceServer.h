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

#include "AtpcoTaxes/ServiceInterfaces/AKHIFactorService.h"
#include "DataModel/Common/Types.h"
#include "DataModel/Services/AKHIFactor.h"
#include <boost/ptr_container/ptr_vector.hpp>

namespace tax
{

class AKHIFactorServiceServer : public AKHIFactorService
{
public:
  AKHIFactorServiceServer()
  {
  }
  ~AKHIFactorServiceServer()
  {
  }

  type::Percent getHawaiiFactor(const type::AirportCode& locCode) const;
  type::Percent getAlaskaAFactor(const type::AirportCode& locCode) const;
  type::Percent getAlaskaBFactor(const type::AirportCode& locCode) const;
  type::Percent getAlaskaCFactor(const type::AirportCode& locCode) const;
  type::Percent getAlaskaDFactor(const type::AirportCode& locCode) const;

  boost::ptr_vector<AKHIFactor>& aKHIFactor()
  {
    return _AKHIFactor;
  }

  const boost::ptr_vector<AKHIFactor>& aKHIFactor() const
  {
    return _AKHIFactor;
  };

private:
  boost::ptr_vector<AKHIFactor> _AKHIFactor;
};
}
