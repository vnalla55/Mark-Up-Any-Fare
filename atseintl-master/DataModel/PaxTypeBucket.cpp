//-------------------------------------------------------------------
//
//  File:        PaxTypeBucket.cpp
//  Created:     July 3, 2006
//  Design:      Doug Steeb, Jeff Hoffman
//  Authors:
//
//  Description: a collection of fares associated with one pax type
//
//  Updates:
//          07/03/06 - JH - Moved from FareMarket::PaxTypeCortege
//
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "DataModel/PaxTypeBucket.h"

#include "Common/PaxTypeUtil.h"
#include "Common/Vendor.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxTypeFare.h"

#include <algorithm>
#include <utility>

namespace tse
{
bool
PaxTypeBucket::isPaxTypeInActualPaxType(const PaxTypeCode paxTypeCode) const
{
  return std::any_of(_actualPaxType.begin(),
                     _actualPaxType.end(),
                     [paxTypeCode](const PaxType* const pt)
                     { return pt->paxType() == paxTypeCode; });
}

bool
PaxTypeBucket::hasAnyFareValid() const
{
  return std::any_of(_paxTypeFare.cbegin(),
                     _paxTypeFare.cend(),
                     [](const auto paxTypeFare)
                     { return paxTypeFare->isValidFare(); });
}

bool
PaxTypeBucket::
operator==(const PaxTypeBucket& rhs) const
{
  if (_requestedPaxType != rhs._requestedPaxType)
    return false;
  if (_actualPaxType != rhs._actualPaxType)
    return false;
  if (_inboundCurrency != rhs._inboundCurrency)
    return false;
  if (_outboundCurrency != rhs._outboundCurrency)
    return false;
  if (_isMarketCurrencyPresent != rhs._isMarketCurrencyPresent)
    return false;
  if (_paxTypeFare != rhs._paxTypeFare)
    return false;

  return true;
}

void
PaxTypeBucket::setMarketCurrencyPresent(PricingTrx& trx, FareMarket& fm)
{
  // Rename as isMarketCurrencyPresent
  _isMarketCurrencyPresent = false;

  if (_paxTypeFare.empty())
    return;

  typedef std::vector<PaxTypeFare*>::const_iterator It;
  for (It ptf = _paxTypeFare.begin(); ptf != _paxTypeFare.end(); ++ptf)
  {
    // IF valid XO option then skip fare
    PaxTypeFare& paxTypeFare = *(*ptf);
    if (isFareSkippedByOptionXO(trx, paxTypeFare))
      continue;

    if (isFareSkippedByOptionXC(trx, paxTypeFare))
      continue;

    if (fm.direction() == FMDirection::OUTBOUND)
    {
      if ((*ptf)->currency() == outboundCurrency())
      {
        _isMarketCurrencyPresent = true;
        return;
      }
    }
    else
    // if ( fm.direction() == FMDirection::INBOUND & OTHER )
    {
      if ((*ptf)->currency() == inboundCurrency())
      {
        _isMarketCurrencyPresent = true;
        return;
      }
    }
  }
}

bool
PaxTypeBucket::isFareSkippedByOptionXC(PricingTrx& _trx, PaxTypeFare& paxTypeFare)
{
  return (_trx.getOptions()->forceCorpFares() && (!paxTypeFare.matchedCorpID()));
}

bool
PaxTypeBucket::isFareSkippedByOptionXO(PricingTrx& _trx, PaxTypeFare& paxTypeFare)
{
  if (!_trx.getOptions()->isXoFares())
    return false;

  // conflict option , if conflict we remove XO option processing
  if (_trx.getOptions()->isFareFamilyType())
    return false;

  // lint !e578
  bool paxTypeMatch = true;

  if (requestedPaxType()->vendorCode() == Vendor::SABRE)
  {
    if (PaxTypeUtil::sabreVendorPaxType(_trx, *(requestedPaxType()), paxTypeFare))
      paxTypeMatch = false;
  }
  else
  {
    PaxTypeCode farePaxType = paxTypeFare.fcasPaxType();
    if (farePaxType.empty())
      farePaxType = ADULT;

    if (farePaxType == requestedPaxType()->paxType())
      paxTypeMatch = false;
  }
  return paxTypeMatch;
}

} // tse
