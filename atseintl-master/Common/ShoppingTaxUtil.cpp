/*
 * ShoppingTaxUtil.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: SG0892420
 */

#include "Common/ShoppingTaxUtil.h"

#include "Common/FallbackUtil.h"
#include "Common/LocUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/TaxNation.h"

#include <functional>
#include <iostream>
#include <iterator>

namespace tse
{
FIXEDFALLBACK_DECL(transitTimeByTaxes);

namespace
{
const TaxCode
AY_TAX("AY");
}

bool
ShoppingTaxUtil::doesAYApply(const TravelSeg* ts, const Itin* itin, PricingTrx& trx)
{
  if (itin->paxGroup().front()->paxType() == INFANT)
    return false;

  return AYPrevalidator(itin, ts, trx).doesAYApply();
}

ShoppingTaxUtil::AYPrevalidator::AYPrevalidator(const Itin* itin,
                                                const TravelSeg* ts,
                                                PricingTrx& trx)
  : _itin(itin), _ts(ts), _taxRec(nullptr), _trx(trx)
{
}

bool
ShoppingTaxUtil::AYPrevalidator::doesAYApply()
{
  bool AYapplies(false);
  DataHandle dh;
  const std::vector<TaxCodeReg*>& taxCodeReg =
      dh.getTaxCode(AY_TAX, _trx.getRequest()->ticketingDT());
  for (std::vector<TaxCodeReg*>::const_iterator taxRecIt = taxCodeReg.begin();
       taxRecIt != taxCodeReg.end();
       ++taxRecIt)
  {
    _taxRec = *taxRecIt;

    if (!checkEnplanementLoc())
      continue;

    if (!checkPOS())
      continue;

    if (!checkValCxr())
      continue;

    if (!checkCxrFlt())
      continue;

    if (!checkEqpmt())
      continue;

    AYapplies = true;
    break;
  }

  return AYapplies;
}

bool
ShoppingTaxUtil::AYPrevalidator::checkEnplanementLoc() const
{
  if (_taxRec->loc1().empty())
    return true;

  const bool geoMatch = LocUtil::isInLoc(*(_ts->origin()),
                                         _taxRec->loc1Type(),
                                         _taxRec->loc1(),
                                         Vendor::SABRE,
                                         MANUAL,
                                         LocUtil::TAXES,
                                         GeoTravelType::International,
                                         EMPTY_STRING(),
                                         _trx.getRequest()->ticketingDT());
  return geoMatch != (_taxRec->loc1ExclInd() == YES);
}

bool
ShoppingTaxUtil::AYPrevalidator::checkPOS() const
{

  if (_taxRec->posLoc().empty())
    return true;

  const bool locMatch = LocUtil::isInLoc(*(TrxUtil::saleLoc(_trx)),
                                         _taxRec->posLocType(),
                                         _taxRec->posLoc(),
                                         Vendor::SABRE,
                                         MANUAL,
                                         LocUtil::TAXES,
                                         GeoTravelType::International,
                                         EMPTY_STRING(),
                                         _trx.getRequest()->ticketingDT());

  return locMatch != (_taxRec->posExclInd() == YES);
}

bool
ShoppingTaxUtil::AYPrevalidator::checkValCxr() const
{
  if (_taxRec->restrictionValidationCxr().empty())
    return true;

  if (_itin->validatingCarrier().empty())
    return true;

  const bool found(std::find(_taxRec->restrictionValidationCxr().begin(),
                             _taxRec->restrictionValidationCxr().end(),
                             _itin->validatingCarrier()) !=
                   _taxRec->restrictionValidationCxr().end());
  return found != (_taxRec->valcxrExclInd() == YES);
}

bool
ShoppingTaxUtil::AYPrevalidator::checkEqpmt() const
{
  if (_taxRec->equipmentCode().empty())
    return true;

  const bool found(std::find(_taxRec->equipmentCode().begin(),
                             _taxRec->equipmentCode().end(),
                             _ts->equipmentType()) != _taxRec->equipmentCode().end());

  return found != (_taxRec->exempequipExclInd() == YES);
}

bool
ShoppingTaxUtil::AYPrevalidator::checkCxrFlt() const
{
  if (_taxRec->exemptionCxr().empty())
    return true;

  const AirSeg* airSeg = dynamic_cast<const AirSeg*>(_ts);
  if (!airSeg)
    return false;

  bool matched(false);
  for (std::vector<TaxExemptionCarrier>::const_iterator tecIt = _taxRec->exemptionCxr().begin();
       tecIt != _taxRec->exemptionCxr().end();
       ++tecIt)
  {
    if (tecIt->carrier() != airSeg->marketingCarrierCode())
      continue;

    if (tecIt->flight1() > 0)
    {
      if (tecIt->flight1() > airSeg->marketingFlightNumber())
        continue;
      if (tecIt->flight2() > 0)
      {
        if (tecIt->flight2() < airSeg->marketingFlightNumber())
          continue;
      }
      else
      {
        if (tecIt->flight1() != airSeg->marketingFlightNumber())
          continue;
      }
    }

    if (!tecIt->airport1().empty())
    {
      if (tecIt->direction() == 'F')
      {
        if (tecIt->airport1() != airSeg->origin()->loc())
          continue;
        if (tecIt->airport2() != airSeg->destination()->loc())
          continue;
      }
      else
      {
        if (tecIt->airport1() != airSeg->origin()->loc() &&
            tecIt->airport2() != airSeg->origin()->loc())
          continue;
        if (tecIt->airport1() != airSeg->destination()->loc() &&
            tecIt->airport2() != airSeg->destination()->loc())
          continue;
      }
    }
    matched = true;
    break;
  }
  return matched != (_taxRec->exempcxrExclInd() == YES);
}

void
ShoppingTaxUtil::DateSegmentation::buildDateSegmentKey(const Itin& itin, std::string& key) const
{
  key.append("|");
  key.append(_nation.c_str());
  if (!_journeyCmtDateLimits.empty())
  {
    const DateTime travelDate(itin.travelDate().date());
    int i = 0;
    for (std::vector<DateTime>::const_iterator it = _journeyCmtDateLimits.begin();
         it != _journeyCmtDateLimits.end();
         ++it, ++i)
      if (*it > travelDate)
        break;

    char buf[32];
    ::snprintf(buf, 32, "-JRN%d", i);
    key.append(buf);
  }
  std::vector<DateTime>::const_iterator dtIt = _depDateLimits.begin();
  int i = 0;
  for (std::vector<TravelSeg*>::const_iterator segIt = itin.travelSeg().begin();
       segIt != itin.travelSeg().end();
       ++segIt)
  {
    if ((*segIt)->origin()->nation() != _nation && (*segIt)->destination()->nation() != _nation)
      continue;

    const DateTime& depDate = (*segIt)->departureDT();
    for (; dtIt != _depDateLimits.end(); ++dtIt, ++i)
      if (*dtIt > depDate)
        break;

    char buf[32];
    ::snprintf(buf, 32, "|%d", i);
    key.append(buf);
  }
}

void
ShoppingTaxUtil::DateSegmentation::initDateLimits()
{
  DataHandle dh(_tktDate);
  const TaxNation* taxNation = dh.getTaxNation(_nation, _tktDate);
  if (!taxNation)
    return;
  for (std::vector<TaxCode>::const_iterator taxCodeIt = taxNation->taxCodeOrder().begin();
       taxCodeIt != taxNation->taxCodeOrder().end();
       ++taxCodeIt)
  {
    const std::vector<TaxCodeReg*>& taxCodeReg = dh.getTaxCode(*taxCodeIt, _tktDate);
    for (std::vector<TaxCodeReg*>::const_iterator it = taxCodeReg.begin(); it != taxCodeReg.end();
         ++it)
    {
      if ((*it)->tvlDateasoriginInd() == 'Y')
      {
        _journeyCmtDateLimits.push_back((*it)->firstTvlDate());
        _journeyCmtDateLimits.push_back((*it)->lastTvlDate());
      }
      else
      {
        _depDateLimits.push_back((*it)->firstTvlDate());
        _depDateLimits.push_back((*it)->lastTvlDate());
      }
    }
  }

  std::sort(_journeyCmtDateLimits.begin(), _journeyCmtDateLimits.end());
  _journeyCmtDateLimits.erase(
      std::unique(_journeyCmtDateLimits.begin(), _journeyCmtDateLimits.end()),
      _journeyCmtDateLimits.end());
  _journeyCmtDateLimits.erase(_journeyCmtDateLimits.begin(),
                              std::find_if(_journeyCmtDateLimits.begin(),
                                           _journeyCmtDateLimits.end(),
                                           std::bind2nd(std::greater<DateTime>(), _tktDate)));

  std::sort(_depDateLimits.begin(), _depDateLimits.end());
  _depDateLimits.erase(std::unique(_depDateLimits.begin(), _depDateLimits.end()),
                       _depDateLimits.end());
  _depDateLimits.erase(_depDateLimits.begin(),
                       std::find_if(_depDateLimits.begin(),
                                    _depDateLimits.end(),
                                    std::bind2nd(std::greater<DateTime>(), _tktDate)));

  //  std::ostream_iterator<DateTime> out_it (std::cout,",\n");
  //  std::cout << "DATE LIMITS FOR = " <<  _nation.c_str() << std::endl;
  //  std::cout << "JOURNEY: ";
  //  std::copy(_journeyCmtDateLimits.begin(), _journeyCmtDateLimits.end(), out_it);
  //  std::cout <<"\nDEPARTURE: ";
  //  std::copy(_depDateLimits.begin(), _depDateLimits.end(), out_it);
}

void
ShoppingTaxUtil::FlightRanges::buildFltRangeKey(const Itin& itin, std::string& key) const
{
  key.append("|FLTRNG-");
  key.append(_nation.c_str());

  for (std::vector<TravelSeg*>::const_iterator segIt = itin.travelSeg().begin();
       segIt != itin.travelSeg().end();
       ++segIt)
  {
    if ((*segIt)->origin()->nation() != _nation && (*segIt)->destination()->nation() != _nation)
      continue;
    if (!(*segIt)->isAir())
      continue;
    AirSeg* airSeg = static_cast<AirSeg*>(*segIt);

    const FlightNumber flt = airSeg->flightNumber();
    FlightRangesByCarrier::const_iterator fltByCxr = _flightRanges.find(airSeg->carrier());
    if (fltByCxr == _flightRanges.end())
      continue;

    const std::vector<FlightRange>& cxrFltVec = fltByCxr->second;

    key.append(airSeg->carrier());
    for (uint32_t i = 0; i < cxrFltVec.size(); ++i)
    {
      const FlightRange& fltR = cxrFltVec[i];
      if (flt >= fltR.first and flt <= fltR.second)
      {
        char buf[16];
        ::snprintf(buf, 16, "_%d", i + 1);
        key.append(buf);
      }
    }
  }
}

void
ShoppingTaxUtil::FlightRanges::initFlightRanges()
{

  const TaxNation* taxNation = getTaxNation();
  if (!taxNation)
    return;
  for (std::vector<TaxCode>::const_iterator taxCodeIt = taxNation->taxCodeOrder().begin();
       taxCodeIt != taxNation->taxCodeOrder().end();
       ++taxCodeIt)
  {
    const std::vector<TaxCodeReg*>& taxCodeReg = getTaxCode(*taxCodeIt);
    for (std::vector<TaxCodeReg*>::const_iterator it = taxCodeReg.begin(); it != taxCodeReg.end();
         ++it)
    {
      for (std::vector<TaxExemptionCarrier>::const_iterator fltIt = (*it)->exemptionCxr().begin();
           fltIt != (*it)->exemptionCxr().end();
           ++fltIt)
      {
        if (fltIt->flight1() > 0)
        {
          FlightRange r(fltIt->flight1(), fltIt->flight2());
          if (fltIt->flight2() <= 0)
            r.second = fltIt->flight1();
          _flightRanges[fltIt->carrier()].push_back(r);
        }
      }
    }
  }

  for (FlightRangesByCarrier::iterator cxrIt = _flightRanges.begin(); cxrIt != _flightRanges.end();
       ++cxrIt)
  {
    std::vector<FlightRange>& cxrFltVec = cxrIt->second;
    std::sort(cxrFltVec.begin(), cxrFltVec.end());
    cxrFltVec.erase(std::unique(cxrFltVec.begin(), cxrFltVec.end()), cxrFltVec.end());
  }
}

const TaxNation*
ShoppingTaxUtil::FlightRanges::getTaxNation()
{
  return _dh.getTaxNation(_nation, _dh.ticketDate());
}

const std::vector<TaxCodeReg*>&
ShoppingTaxUtil::FlightRanges::getTaxCode(const TaxCode& tc)
{
  return _dh.getTaxCode(tc, _dh.ticketDate());
}

const TaxNation*
ShoppingTaxUtil::getTaxNation(const NationCode nation, DataHandle& dh) const
{
  return dh.getTaxNation(nation, dh.ticketDate());
}

const std::vector<TaxCodeReg*>&
ShoppingTaxUtil::getTaxCode(const TaxCode& tc, DataHandle& dh) const
{
  return dh.getTaxCode(tc, dh.ticketDate());
}

bool ShoppingTaxUtil::getRestrWithNextStopover(const NationCode nation,
                                               const DateTime& tktDate) const
{
  DataHandle dh(tktDate);
  const TaxNation* taxNation = getTaxNation(nation, dh);
  if (!taxNation)
    return false;

  for (const TaxCode& taxCode : taxNation->taxCodeOrder())
  {
    const std::vector<TaxCodeReg*>& taxCodeRegVec = getTaxCode(taxCode, dh);
    if (std::any_of(taxCodeRegVec.begin(), taxCodeRegVec.end(),
                    [](const TaxCodeReg* code){ return code->nextstopoverrestr() == 'Y'; }))
      return true;
  }

  return false;
}

void
ShoppingTaxUtil::getNationTransitTimes(std::set<Hours>& transitHours,
                                       std::set<Minutes>& transitTotalMinutes,
                                       const NationCode nation,
                                       const TaxCodesVec& transitMinutesRoundTaxes,
                                       const DateTime& tktDate) const
{
  DataHandle dh(tktDate);
  const TaxNation* taxNation = getTaxNation(nation, dh);
  if (!taxNation)
    return;

  auto insertTransitTime = [&transitMinutesRoundTaxes,
                            &transitHours,
                            &transitTotalMinutes]
    (const TaxCode& taxCode, const TaxRestrictionTransit& restriction)
    {
      const bool roundNegativeMin = !std::binary_search(transitMinutesRoundTaxes.begin(),
                                                        transitMinutesRoundTaxes.end(),
                                                        taxCode);

      const int32_t hours = restriction.transitHours();
      const int32_t minutes = (roundNegativeMin ? std::max(0, restriction.transitMinutes())
                                                : restriction.transitMinutes());

      if ( hours > 0 )
        transitHours.insert(Hours(hours));
      if ( hours > 0 || minutes > 0 )
        transitTotalMinutes.insert(Minutes(std::max(0, hours) * 60 + minutes));
    };

  for (std::vector<TaxCode>::const_iterator taxCodeIt = taxNation->taxCodeOrder().begin();
       taxCodeIt != taxNation->taxCodeOrder().end();
       ++taxCodeIt)
  {
    const std::vector<TaxCodeReg*>& taxCodeRegVec = getTaxCode(*taxCodeIt, dh);
    for (std::vector<TaxCodeReg*>::const_iterator codeIt = taxCodeRegVec.begin();
         codeIt != taxCodeRegVec.end();
         ++codeIt)
    {
      const TaxCodeReg* taxCode = *codeIt;
      const std::vector<TaxRestrictionTransit>& restrictionTransit = taxCode->restrictionTransit();
      std::vector<TaxRestrictionTransit>::const_iterator rtIt = restrictionTransit.begin();
      for (; rtIt != restrictionTransit.end(); ++rtIt)
      {
        const TaxRestrictionTransit& rest = *rtIt;
        if (fallback::fixed::transitTimeByTaxes())
        {
          if (rest.transitHours() > 0)
          {
            transitHours.insert(Hours(rest.transitHours()));
            transitTotalMinutes.insert(Minutes(rest.transitHours() * 60 + rest.transitMinutes()));
          }
        }
        else
          insertTransitTime(taxCode->taxCode(), rest);
      }
    }
  }
}

void
ShoppingTaxUtil::getTaxFirstTravelDates(std::set<boost::gregorian::date>& firstTravelDates,
                                        const boost::gregorian::date firstDate,
                                        const boost::gregorian::date lastDate,
                                        const NationCode& nation,
                                        const DateTime& tktDate) const
{
  DataHandle dh(tktDate);
  const TaxNation* taxNation = getTaxNation(nation, dh);
  if (!taxNation)
    return;

  for (const TaxCode& taxCode : taxNation->taxCodeOrder())
  {
    const std::vector<TaxCodeReg*>& taxCodeRegVec = getTaxCode(taxCode, dh);
    for (const TaxCodeReg* tax : taxCodeRegVec)
    {
      if (tax->firstTvlDate().date() > firstDate && tax->firstTvlDate().date() < lastDate)
        firstTravelDates.emplace(tax->firstTvlDate().date());
    }
  }
}
}
