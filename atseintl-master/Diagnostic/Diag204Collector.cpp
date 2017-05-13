//---------------------------------------------------------------------
//  File:        Diag204Collector.C
//  Authors:
//  Created:     11/23/2005
//
//  Description: Diagnostic 204 formatter
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

#include "Diagnostic/Diag204Collector.h"

#include "DataModel/PricingTrx.h"
#include "Diagnostic/DCFactory.h"

static const char* SEPARATOR = "---------------------------------------------------------------\n";

namespace tse
{
Diag204Collector*
Diag204Collector::getDiag204(PricingTrx& trx)
{
  if (LIKELY(trx.diagnostic().diagnosticType() != Diagnostic204))
    return nullptr;

  Diag204Collector* diag204 = dynamic_cast<Diag204Collector*>(DCFactory::instance()->create(trx));

  if (diag204 != nullptr)
    diag204->enable(Diagnostic204);

  return diag204;
}

bool
Diag204Collector::parseQualifiers(PricingTrx& trx, const Fare& fare, const FareMarket& fareMarket)
{
  if (trx.diagnostic().diagParamMap().empty())
    return true;

  std::map<std::string, std::string>::const_iterator e = trx.diagnostic().diagParamMap().end();

  std::map<std::string, std::string>::const_iterator i;

  i = trx.diagnostic().diagParamMap().find(Diagnostic::DIAG_VENDOR);
  if (i != e)
  {
    if (!i->second.empty())
    {
      if (fare.vendor() == i->second)
        return true;

      return false;
    }
  }

  i = trx.diagnostic().diagParamMap().find(Diagnostic::FARE_CLASS_CODE);
  if (i != e)
  {
    if (!i->second.empty())
    {
      if (fare.fareClass() == i->second)
        return true;

      return false;
    }
  }

  i = trx.diagnostic().diagParamMap().find(Diagnostic::FARE_TYPE);
  if (i != e)
  {
    //    if (fare.fareType() == i->second)
    //       return true;

    return false;
  }

  i = trx.diagnostic().diagParamMap().find(Diagnostic::FARE_BASIS_CODE);
  if (i != e)
  {
    if (fareMarket.fareBasisCode() == i->second)
      return true;

    return false;
  }

  i = trx.diagnostic().diagParamMap().find(Diagnostic::RULE_NUMBER);
  if (i != e)
  {
    if (fare.ruleNumber() == i->second)
      return true;

    return false;
  }

  return (true);
}

void
Diag204Collector::writeFare(const Fare& fare)
{
  std::string globalDirection;
  globalDirectionToStr(globalDirection, fare.globalDirection());

  (*this) << "MKT1: " << fare.market1() << "   MKT2: " << fare.market2()
          << "   GLOBAL: " << globalDirection << "   " << fare.vendor() << "   " << fare.carrier()
          << "   " << fare.fareClass() << '\n' << "RULE TARIFF: " << fare.tcrRuleTariff()
          << "    FARE TARIFF: " << fare.fareTariff() << "    RULE NUMBER: " << fare.ruleNumber()
          << '\n' << "OWRT: " << fare.owrt() << "   ROUTING: " << fare.routingNumber() << '\n';
}

void
Diag204Collector::writeBadGlobalDirection(PricingTrx& trx,
                                          const Fare& fare,
                                          const FareMarket& fareMarket)
{
  if (!parseQualifiers(trx, fare, fareMarket))
    return;

  writeFare(fare);

  std::string fmGlobalDir;
  globalDirectionToStr(fmGlobalDir, fareMarket.getGlobalDirection());

  std::string tcrGlobalDir;
  globalDirectionToStr(tcrGlobalDir, fare.tariffCrossRefInfo()->globalDirection());

  (*this) << "SCOPE: DOM\n"
          << "FARE MARKET GLOBAL: " << fmGlobalDir << '\n' << "FARE TCR GLOBAL: " << tcrGlobalDir
          << '\n' << SEPARATOR << '\n';
}

void
Diag204Collector::writeNoRecord1(PricingTrx& trx, const Fare& fare, const FareMarket& fareMarket)
{
  if (!parseQualifiers(trx, fare, fareMarket))
    return;

  writeFare(fare);

  (*this) << "*** NO RECORD 1 LOCATED FOR FARE *** \n" << SEPARATOR << '\n';
}
void
Diag204Collector::writeNoFCA(PricingTrx& trx, const Fare& fare, const FareMarket& fareMarket)
{
  if (!parseQualifiers(trx, fare, fareMarket))
    return;

  writeFare(fare);

  (*this) << "*** NO FARE CLASS APPLICATION INFO *** \n" << SEPARATOR << '\n';
}
void
Diag204Collector::writeNoMatchFareClass(PricingTrx& trx,
                                        const Fare& fare,
                                        const FareMarket& fareMarket)
{
  if (!parseQualifiers(trx, fare, fareMarket))
    return;

  writeFare(fare);

  (*this) << " \n"
          << "** INDICATOR IS SET TO UNAVAILABLE ** \n"
          << "NO FARE CLASS MATCH TO FARE BASIS CODE\n \n"
          << "   MARKET FARE CLASS   " << fare.fareClass() << " \n"
          << "   FCA FARE BASIS CODE " << fareMarket.fareBasisCode() << " \n" << SEPARATOR << '\n';
}
void
Diag204Collector::writeNoMatchLocation(PricingTrx& trx,
                                       const Fare& fare,
                                       const FareMarket& fareMarket,
                                       const FareClassAppInfo& fcaInfo)
{
  if (!parseQualifiers(trx, fare, fareMarket))
    return;

  writeFare(fare);

  (*this) << " \n"
          << "**** NO LOCATION DIRECTIONALITY MATCH **** \n"
          << "MARKET FARE TO FARE CLASS APPLICATION INFO \n \n"
          << "     MARKET FARE ORIG LOC " << fare.market1() << " \n"
          << "     MARKET FARE DEST LOC " << fare.market2() << " \n"
          << "     FCA ORIG LOC         " << fcaInfo._location1 << " \n"
          << "     FCA DEST LOC         " << fcaInfo._location2 << " \n" << SEPARATOR << '\n';
}
void
Diag204Collector::writeFareCanNotDouble(PricingTrx& trx,
                                        const Fare& fare,
                                        const FareMarket& fareMarket)
{
  if (!parseQualifiers(trx, fare, fareMarket))
    return;

  writeFare(fare);

  (*this) << " \n"
          << "**** FARE HAS DIFFERENT DIRECTIONALITY SETTINGS ***** \n"
          << "FARE MAY BE DOUBLED AND FARE CLASS APPLICATION INFO \n"
          << "CAN NOT DOUBLED \n" << SEPARATOR << '\n';
}
void
Diag204Collector::writeFareCanNotBeHalved(PricingTrx& trx,
                                          const Fare& fare,
                                          const FareMarket& fareMarket)
{
  if (!parseQualifiers(trx, fare, fareMarket))
    return;

  writeFare(fare);

  (*this) << " \n"
          << "**** FARE HAS DIFFERENT DIRECTIONALITY SETTINGS ****** \n"
          << "FARE MAY NOT BE HALVED AND FARE CLASS APPLICATION INFO \n"
          << "MAY BE HALVED \n" << SEPARATOR << '\n';
}
void
Diag204Collector::writeNoMatchRoutingNumber(PricingTrx& trx,
                                            const Fare& fare,
                                            const FareMarket& fareMarket,
                                            const FareClassAppInfo& fcaInfo)
{
  if (!parseQualifiers(trx, fare, fareMarket))
    return;

  writeFare(fare);

  (*this) << " \n"
          << "****** NO FARE ROUTING NUMBER MATCH ****** \n"
          << "MARKET FARE TO FARE CLASS APPLICATION INFO \n \n"
          << "     MARKET FARE ROUTING NUMBER " << fare.routingNumber() << " \n"
          << "     FCA ROUTING NUMBER         " << fcaInfo._routingNumber << " \n" << SEPARATOR
          << '\n';
}
void
Diag204Collector::writeNoFootnoteMatch(PricingTrx& trx,
                                       const Fare& fare,
                                       const FareMarket& fareMarket,
                                       const FareClassAppInfo& fcaInfo)
{
  if (!parseQualifiers(trx, fare, fareMarket))
    return;

  writeFare(fare);

  (*this) << " \n"
          << "***** NO FARE FOOTNOTE NUMBER MATCH ***** \n"
          << "MARKET FARE TO FARE CLASS APPLICATION INFO \n \n"
          << "     MARKET FARE FOOTNOTE 1 " << fare.footNote1() << " \n"
          << "     MARKET FARE FOOTNOTE 2 " << fare.footNote2() << " \n"
          << "     FCA FOOTNOTE 1         " << fcaInfo._footnote1 << " \n"
          << "     FCA FOOTNOTE 2         " << fcaInfo._footnote2 << " \n" << SEPARATOR << '\n';
}
void
Diag204Collector::writeFareTypeNotRetrieved(PricingTrx& trx,
                                            const Fare& fare,
                                            const FareMarket& fareMarket,
                                            const FareClassAppInfo& fcaInfo)
{
  if (!parseQualifiers(trx, fare, fareMarket))
    return;

  writeFare(fare);

  (*this) << " \n"
          << "***** NO FARE TYPE MATRIX TABLE ***** \n"
          << "FARE TYPE " << fcaInfo._fareType << " \n" << SEPARATOR << '\n';
}
void
Diag204Collector::writeNoMatchOnPaxTypes(PricingTrx& trx,
                                         const Fare& fare,
                                         const FareMarket& fareMarket)
{
  if (!parseQualifiers(trx, fare, fareMarket))
    return;

  writeFare(fare);

  (*this) << " \n"
          << "***** NO MATCH ON PAX TYPES ***** \n" << SEPARATOR << '\n';
}
}
