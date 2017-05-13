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
#include "Rules/AlternateRefAkHiFactorsApplicator.h"
#include "Rules/AlternateRefAkHiFactorsRule.h"
#include "Rules/PaymentDetail.h"
#include "Rules/GeoUtils.h"
#include "ServiceInterfaces/AKHIFactorService.h"
#include "ServiceInterfaces/LocService.h"

namespace tax
{

AlternateRefAkHiFactorsApplicator::AlternateRefAkHiFactorsApplicator(
    const AlternateRefAkHiFactorsRule& parent,
    const AKHIFactorService& akhiFactorService,
    const LocService& locService)
  : BusinessRuleApplicator(&parent), _AkHiFactorCache(akhiFactorService), _locService(locService)
{
}

AlternateRefAkHiFactorsApplicator::~AlternateRefAkHiFactorsApplicator() {}

bool
AlternateRefAkHiFactorsApplicator::apply(PaymentDetail& paymentDetail) const
{
  const type::AirportCode& locBegin = paymentDetail.getTaxPointBegin().locCode();
  const type::AirportCode& locEnd = paymentDetail.getTaxPointEnd().locCode();

  const bool isHawaiiBegin = GeoUtils::isHawaii(_locService.getState(locBegin));
  const bool isHawaiiEnd = GeoUtils::isHawaii(_locService.getState(locEnd));
  const bool isAlaskaBegin = GeoUtils::isAlaska(_locService.getState(locBegin));
  const bool isAlaskaEnd = GeoUtils::isAlaska(_locService.getState(locEnd));

  if (!(isHawaiiBegin || isHawaiiEnd || isAlaskaBegin || isAlaskaEnd))
  {
    return true;
  }

  if ((isHawaiiBegin && isHawaiiEnd) || (isAlaskaBegin && isAlaskaEnd))
  {
    return true;
  }

  type::Percent percent;

  if (isHawaiiBegin)
  {
    percent = _AkHiFactorCache.getHawaiiFactor(locEnd);
  }
  else if (isHawaiiEnd)
  {
    percent = _AkHiFactorCache.getHawaiiFactor(locBegin);
  }
  else if (isAlaskaBegin)
  {
    percent = getAlaskaFactor(locEnd, locBegin);
  }
  else if (isAlaskaEnd)
  {
    percent = getAlaskaFactor(locBegin, locEnd);
  }
  else
  {
    return true;
  }

  // if no rate found, apply standard percentage
  if (percent == type::Percent(0))
  {
    return true;
  }

  paymentDetail.taxAmt() = percent;
  return true;
}

type::Percent
AlternateRefAkHiFactorsApplicator::getAlaskaFactor(const type::AirportCode& loc,
                                                   const type::AirportCode& zoneLoc) const
{
  const type::AlaskaZone alaskaZone = _locService.getAlaskaZone(zoneLoc);

  if (alaskaZone == type::AlaskaZone::A)
  {
    return _AkHiFactorCache.getAlaskaAFactor(loc);
  }
  else if (alaskaZone == type::AlaskaZone::B)
  {
    return _AkHiFactorCache.getAlaskaBFactor(loc);
  }
  else if (alaskaZone == type::AlaskaZone::C)
  {
    return _AkHiFactorCache.getAlaskaCFactor(loc);
  }
  else if (alaskaZone == type::AlaskaZone::D)
  {
    return _AkHiFactorCache.getAlaskaDFactor(loc);
  }
  else
  {
    return type::Percent();
  }
}

} // namespace tax
