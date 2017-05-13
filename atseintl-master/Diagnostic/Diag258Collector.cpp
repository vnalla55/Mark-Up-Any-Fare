//---------------------------------------------------------------------
//  File:        Diag258Collector.C
//  Authors:
//  Created:     4/17/2006
//
//  Description: Diagnostic 258 formatter
//
//  Copyright Sabre 2005
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//---------------------------------------------------------------------

#include "Diagnostic/Diag258Collector.h"

#include "Common/Money.h"
#include "DBAccess/ConstructedFareInfo.h"
#include "Diagnostic/DCFactory.h"

namespace tse
{
namespace
{
const char* PUBLISHED_VS_CONSTRUCTED_TITLE_MSG = "PUBLISHED VS CONSTRUCTED DUPLICATES ELIMINATION";
const char* CXR_PREF_PUBLISHED_OVER_CONSTR_MSG =
    "CARRIER PREFERS PUBLISHED FARES OVER CONSTRUCTED: ";

const char* CONSTRUCTED_FARE_REMOVED = "CONSTRUCTED FARE REMOVED";
const char* PUBLISHED_FARE_REMOVED = "PUBLISHED FARE REMOVED";
const char* CXR_PREFRS_PUBLISHED_OVER_CONSTRUCTED_MSG =
    "CARRIER PREFERS PUBLISHED FARE OVER CONSTRUCTED";
const char* PUBLISHED_LESS_OR_EQ_FARE_AMOUNT_MSG = "PUBLISHED FARE HAS LESS OR EQUAL FARE AMOUNT";
const char* CONSTRUCTED_LESS_FARE_AMOUNT_MSG = "CONSTRUCTED FARE HAS LESS FARE AMOUNT";
}

Diag258Collector*
Diag258Collector::getDiag258(PricingTrx& trx, FareMarket* fm)
{
  if (LIKELY(trx.diagnostic().diagnosticType() != Diagnostic258))
    return nullptr;

  Diag258Collector* diag258 = dynamic_cast<Diag258Collector*>(DCFactory::instance()->create(trx));

  if (diag258 != nullptr)
  {
    diag258->enable(Diagnostic258);
    diag258->fareMarket() = fm;
  }

  diag258->_pricingTrx = &trx;
  diag258->_travelDate = fm->travelDate();

  diag258->initialize(*fm->origin(), *fm->destination());

  return diag258;
}

void
Diag258Collector::reclaim(Diag258Collector* diag258)
{
  if (diag258 != nullptr)
  {
    diag258->flushMsg();
  }
}

void
Diag258Collector::writeDupRemovalHeader(bool pubVsConstructed)
{
  if (!_active)
    return;

  (*this) << " \n" << PUBLISHED_VS_CONSTRUCTED_TITLE_MSG << '\n'
          << CXR_PREF_PUBLISHED_OVER_CONSTR_MSG << (pubVsConstructed ? "YES" : "NO") << '\n'
          << "FARE MARKET: " << _fareMarket->origin()->loc() << '/' << _fareMarket->boardMultiCity()
          << "/ - " << _fareMarket->destination()->loc() << '/' << _fareMarket->offMultiCity()
          << "/\n";

  (*this) << SEPARATOR << "  MK1  MK2    EFF     EXP    O O    AMT/CUR    TAR CLASS\n"
          << "                             R I\n" << SEPARATOR;
}

void
Diag258Collector::writeDupRemovalFooter()
{
  if (_active && _numOfDups != 0)
    (*this) << SEPARATOR;

  _numOfDups = 0;
}

void
Diag258Collector::writePublishedOrConstructed(bool isPublished,
                                              const Indicator inhibit,
                                              const LocCode& city1,
                                              const LocCode& city2,
                                              const DateTime& effectiveDate,
                                              const DateTime& expireDate,
                                              const Indicator owrt,
                                              const Directionality& directionality,
                                              const CurrencyCode& curency,
                                              const MoneyAmount amount,
                                              const FareClassCode& fareClass,
                                              const TariffNumber tariff,
                                              const bool showFareAmount)
{
  Diag258Collector& dc = *this;

  setf(std::ios::left, std::ios::adjustfield);

  dc << (isPublished ? "P " : "C ") << city1 << "  " << city2 << " ";

  setf(std::ios::right, std::ios::adjustfield);

  formatDateTime(effectiveDate);
  formatDateTime(expireDate);

  dc << owrt << ' ';

  if (directionality == BOTH)
    dc << 'B';

  else if (directionality == FROM)
    dc << 'F';

  else
    dc << 'T';

  if (!showFareAmount)
    dc << "       N/A     ";
  else
  {
    setf(std::ios::fixed, std::ios::floatfield);
    dc << std::setw(10) << Money(amount, curency) << " ";
  }

  setf(std::ios::left, std::ios::adjustfield);
  dc << std::setw(4) << tariff << fareClass << '\n';
}

void
Diag258Collector::writeConstructedFare(const ConstructedFareInfo& cfi)
{
  const FareInfo& fareInfo = cfi.fareInfo();

  const Indicator inhibit = (fareInfo.inhibit() == INHIBIT_N ? ' ' : fareInfo.inhibit());

  writePublishedOrConstructed(
      false,
      inhibit,
      cfi.fareInfo().market1(),
      cfi.fareInfo().market2(),
      fareInfo.effDate(),
      fareInfo.expireDate(),
      fareInfo.owrt(),
      fareInfo.directionality(),
      fareInfo.currency(),
      cfi.specifiedFareAmount(),
      fareInfo.fareClass(),
      fareInfo.fareTariff(),
      showFareAmount(fareInfo.fareTariff(), fareInfo.vendor(), fareInfo.carrier()));
}

void
Diag258Collector::writeSpecifiedFare(const FareInfo& fareInfo)
{
  const Indicator inhibit = (fareInfo.inhibit() == INHIBIT_N ? ' ' : fareInfo.inhibit());

  writePublishedOrConstructed(
      true,
      inhibit,
      fareInfo.market1(),
      fareInfo.market2(),
      fareInfo.effDate(),
      fareInfo.expireDate(),
      fareInfo.owrt(),
      fareInfo.directionality(),
      fareInfo.currency(),
      fareInfo.fareAmount(),
      fareInfo.fareClass(),
      fareInfo.fareTariff(),
      showFareAmount(fareInfo.fareTariff(), fareInfo.vendor(), fareInfo.carrier()));
}

void
Diag258Collector::formatDateTime(const DateTime& dt)
{
  if (dt.isInfinity())
    (*this) << "99/99/99 ";
  else
    (*this) << dt.dateToString(MMDDYY, "/") << " ";
}

void
Diag258Collector::writeDupDetail(const Fare& fare1,
                                 const ConstructedFareInfo& cfi2,
                                 const DupRemoveReason reason)
{
  if (!_active)
    return;

  if (_numOfDups == 0)
    writeDupRemovalHeader(reason == DRR_PUBLISHED_OVER_CONSTRUCTED);

  const FareInfo& fi1 = *fare1.fareInfo();

  writeSpecifiedFare(fi1);
  writeConstructedFare(cfi2);

  switch (reason)
  {
  case DRR_PUBLISHED_OVER_CONSTRUCTED:
    (*this) << CXR_PREFRS_PUBLISHED_OVER_CONSTRUCTED_MSG << '\n' << CONSTRUCTED_FARE_REMOVED;
    break;

  case DRR_PUBLISHED_LESS_OR_EQ_FARE_AMOUNT:
    (*this) << PUBLISHED_LESS_OR_EQ_FARE_AMOUNT_MSG << '\n' << CONSTRUCTED_FARE_REMOVED;
    break;

  case DRR_CONSTRUCTED_LESS_FARE_AMOUNT:
    (*this) << CONSTRUCTED_LESS_FARE_AMOUNT_MSG << '\n' << PUBLISHED_FARE_REMOVED;
    break;
  }

  (*this) << "\n \n";

  _numOfDups++;
}
}
