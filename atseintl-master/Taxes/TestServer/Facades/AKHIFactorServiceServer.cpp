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

#include "AKHIFactorServiceServer.h"
#include <sstream>

namespace tax
{

type::Percent AKHIFactorServiceServer::getHawaiiFactor(const type::AirportCode& locCode) const
{
  for(const AKHIFactor & akhi : _AKHIFactor)
  {
    if (akhi.locCode == locCode)
    {
      return akhi.hawaiiPercent;
    }
  }
  return type::Percent();
}

type::Percent AKHIFactorServiceServer::getAlaskaAFactor(const type::AirportCode& locCode) const
{
  for(const AKHIFactor & akhi : _AKHIFactor)
  {
    if (akhi.locCode == locCode)
    {
      return akhi.zoneAPercent;
    }
  }
  return type::Percent();
}

type::Percent AKHIFactorServiceServer::getAlaskaBFactor(const type::AirportCode& locCode) const
{
  for(const AKHIFactor & akhi : _AKHIFactor)
  {
    if (akhi.locCode == locCode)
    {
      return akhi.zoneBPercent;
    }
  }
  return type::Percent();
}

type::Percent AKHIFactorServiceServer::getAlaskaCFactor(const type::AirportCode& locCode) const
{
  for(const AKHIFactor & akhi : _AKHIFactor)
  {
    if (akhi.locCode == locCode)
    {
      return akhi.zoneCPercent;
    }
  }
  return type::Percent();
}

type::Percent AKHIFactorServiceServer::getAlaskaDFactor(const type::AirportCode& locCode) const
{
  for(const AKHIFactor & akhi : _AKHIFactor)
  {
    if (akhi.locCode == locCode)
    {
      return akhi.zoneDPercent;
    }
  }
  return type::Percent();
}

} // namespace tax
