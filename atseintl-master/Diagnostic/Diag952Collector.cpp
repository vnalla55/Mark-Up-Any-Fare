//----------------------------------------------------------------------------
//  File:        Diag952Collector.cpp
//  Created:     2010-05-212
//
//  Description: Diagnostic 952 formatter
//
//  Updates:
//
//  Copyright Sabre 2010
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "Diagnostic/Diag952Collector.h"

#include "DBAccess/FareInfo.h"
#include "DBAccess/MarketRoutingInfo.h"

#include <iomanip>
#include <vector>

namespace tse
{
Diag952Collector&
Diag952Collector::operator<<(const ShoppingTrx& shoppingTrx)
{
  DiagCollector& dc(*this);

  dc.setf(std::ios::left, std::ios::adjustfield);

  dc << std::endl;
  dc << "**********************************************************\n"
     << "952 : VIS - FARES AND ROUTINGS CACHE CONTENT\n"
     << "**********************************************************\n";
  dc << std::endl;

  std::string type = "";
  type = dc.rootDiag()->diagParamMapItem("TYPE");

  if (type.empty())
  {
    dc << "\"TYPE\" parameter not specified in request" << std::endl;
    return (*this);
  }

  if (type == "F")
  {
    printFares(shoppingTrx);
  }
  else if (type == "R")
  {
    printRoutings(shoppingTrx);
  }
  else
  {
    dc << "Unknown TYPE parameter" << std::endl;
  }

  return (*this);
}

void
Diag952Collector::printFares(const ShoppingTrx& shoppingTrx)
{
  DiagCollector& dc(*this);

  LocCode origin = "";
  LocCode destination = "";
  CarrierCode carrier = "";
  std::string date = "";
  DateTime dateTime;

  origin = dc.rootDiag()->diagParamMapItem("ORIG");

  if (origin.empty())
  {
    dc << "\"ORIG\" parameter not specified in request" << std::endl;
    return;
  }

  destination = dc.rootDiag()->diagParamMapItem("DEST");

  if (destination.empty())
  {
    dc << "\"DEST\" parameter not specified in request" << std::endl;
    return;
  }

  carrier = dc.rootDiag()->diagParamMapItem("CARRIER");

  if (carrier.empty())
  {
    dc << "\"CARRIER\" parameter not specified in request" << std::endl;
    return;
  }

  date = dc.rootDiag()->diagParamMapItem("DATE");

  if (date.empty())
  {
    dc << "\"DATE\" parameter not specified in request" << std::endl;
    return;
  }
  else
  {
    try
    {
      std::string enhDate = date;
      enhDate += " 00:00:00.000";
      dateTime = DateTime(enhDate);
    }
    catch (...)
    {
      dc << "Date should be in format YYYY-MM-DD" << std::endl;
      return;
    }
  }

  dc << "DIAGNOSTIC ARGUMENTS:" << std::endl << std::endl << "ORIG (Origin market): " << origin
     << std::endl << "DEST (Destination market): " << destination << std::endl
     << "CARRIER (Carrier code): " << carrier << std::endl << "DATE (Requested date): " << date
     << std::endl << std::endl;

  const std::vector<const FareInfo*>* faresVec = nullptr;

  faresVec =
      &(shoppingTrx.dataHandle().getBoundFaresByMarketCxr(origin, destination, carrier, dateTime));

  if (faresVec == nullptr)
  {
    dc << "NO FARES FOUND" << std::endl;
    return;
  }

  dc << "FOUND FARES COUNT: " << faresVec->size() << std::endl << std::endl;

  std::vector<const FareInfo*>::const_iterator fareInfoIter = faresVec->begin();
  std::vector<const FareInfo*>::const_iterator fareInfoIterEnd = faresVec->end();

  dc << "MKT1 MKT2 VENDOR CXR FARE   CURRENCY FARE CLASS       SEQ  LINK CREATE DATE          "
        "TARIFF EXPIRE DATE          EFFECTIVE DATE       DISC DATE             FARE    ADJUSTED "
        "NO DEC FOOTNOTE1 FOOTNOTE2 ROUTING RULE    DIR GLOB OWRT RULE    ROUTING FARE RTG3 RESTR "
        "RTG3 RESTR SAME CXR    TVLY     EXPEDIA  TARIFF DOMINT BOOKING CODES            RECORD1 "
        "RECORD2\n"
     << "                     TARIFF                           NUM  NUM                        "
        "TYPE                                                                 AMOUNT   FAREAMT  "
        "POINTS                       NUM   NUM         DIR       TARIFF  TARIFF  TYPE NNEGVIAPPL "
        "NONSTOPDIR 102 103 104 WEB FARE WEB FARE TYPE   IND                             INFO      "
        "     \n"
     << "---- ---- ------ --- ------ -------- ---------------- ---- ---- -------------------- "
        "------ -------------------- -------------------- -------------------- -------- -------- "
        "------ --------- --------- ------- ------- --- ---- ---- ------- ------- ---- ---------- "
        "---------- --- --- --- -------- -------- ------ ------ ------------------------ ------- "
        "-------\n";

  for (; fareInfoIter != fareInfoIterEnd; ++fareInfoIter)
  {
    const FareInfo* fareInfo = (*fareInfoIter);

    char fareTariffType = ' ';

    if (fareInfo->getPaxType() == "JCB")
    {
      fareTariffType = 'J';
    }
    else
    {
      fareTariffType = fareInfo->getTariffType();
    }

    // Key
    dc << std::setw(4) << fareInfo->market1() << std::setw(1) << " ";
    dc << std::setw(4) << fareInfo->market2() << std::setw(1) << " ";
    dc << std::setw(6) << fareInfo->vendor() << std::setw(1) << " ";
    dc << std::setw(3) << fareInfo->carrier() << std::setw(1) << " ";
    dc << std::setw(6) << fareInfo->fareTariff() << std::setw(1) << " ";
    dc << std::setw(8) << fareInfo->currency() << std::setw(1) << " ";
    dc << std::setw(16) << fareInfo->fareClass() << std::setw(1) << " ";
    dc << std::setw(4) << fareInfo->sequenceNumber() << std::setw(1) << " ";
    dc << std::setw(4) << fareInfo->linkNumber() << std::setw(1) << " ";
    dc << std::setw(20) << fareInfo->createDate() << std::setw(1) << " ";
    dc << std::setw(6) << fareTariffType << std::setw(1) << " ";

    // Value
    dc << std::setw(20) << fareInfo->expireDate() << std::setw(1) << " ";
    dc << std::setw(20) << fareInfo->effDate() << std::setw(1) << " ";
    dc << std::setw(20) << fareInfo->discDate() << std::setw(1) << " ";
    dc << std::setw(8) << fareInfo->originalFareAmount() << std::setw(1) << " ";
    dc << std::setw(8) << fareInfo->fareAmount() << std::setw(1) << " ";
    dc << std::setw(6) << fareInfo->noDec() << std::setw(1) << " ";
    dc << std::setw(9) << fareInfo->footNote1() << std::setw(1) << " ";
    dc << std::setw(9) << fareInfo->footNote2() << std::setw(1) << " ";
    dc << std::setw(7) << fareInfo->routingNumber() << std::setw(1) << " ";
    dc << std::setw(7) << fareInfo->ruleNumber() << std::setw(1) << " ";
    dc << std::setw(3) << fareInfo->directionality() << std::setw(1) << " ";
    dc << std::setw(4) << fareInfo->globalDirection() << std::setw(1) << " ";
    dc << std::setw(4) << fareInfo->owrt() << std::setw(1) << " ";
    dc << std::setw(7) << fareInfo->getRuleTariff() << std::setw(1) << " ";
    dc << std::setw(7) << fareInfo->getRoutingTariff() << std::setw(1) << " ";
    dc << std::setw(4) << fareInfo->getFareType() << std::setw(1) << " ";
    dc << std::setw(10) << fareInfo->negViaAppl() << std::setw(1) << " ";
    dc << std::setw(10) << fareInfo->nonstopDirectInd() << std::setw(1) << " ";
    dc << std::setw(3) << fareInfo->sameCarrier102() << std::setw(1) << " ";
    dc << std::setw(3) << fareInfo->sameCarrier103() << std::setw(1) << " ";
    dc << std::setw(3) << fareInfo->sameCarrier104() << std::setw(1) << " ";
    dc << std::setw(8) << fareInfo->isWebFare(true) << std::setw(1) << " ";
    dc << std::setw(8) << fareInfo->isExpediaWebFare() << std::setw(1) << " ";
    dc << std::setw(6) << fareTariffType << std::setw(1) << " ";
    dc << std::setw(6) << fareInfo->getDomInternInd() << std::setw(1) << " ";

    std::string bkkString = "";

    const std::vector<BookingCode>* bookingCodesBF = fareInfo->getBookingCodes(shoppingTrx);

    if ((bookingCodesBF != nullptr) && (!bookingCodesBF->empty()))
    {
      std::vector<BookingCode>::const_iterator bkkIter =
          fareInfo->getBookingCodes(shoppingTrx)->begin();
      std::vector<BookingCode>::const_iterator bkkIterEnd =
          fareInfo->getBookingCodes(shoppingTrx)->end();

      for (; bkkIter != bkkIterEnd; ++bkkIter)
      {
        const BookingCode bkk = (*bkkIter);

        if (bkkString.empty())
        {
          bkkString += bkk + ",";
        }
        else
        {
          bkkString += " " + bkk + ",";
        }
      }

      if (!bkkString.empty())
      {
        bkkString.erase(bkkString.size() - 1);
      }
    }
    else
    {
      const FareClassCode& fareClass = fareInfo->fareClass();

      if (!fareClass.empty())
      {
        bkkString = fareClass[0];
      }
    }

    dc << std::setw(24) << bkkString << std::setw(1) << " ";

    dc << std::setw(7) << "?" << std::setw(1) << " ";
    dc << std::setw(7) << "?" << std::endl;
  }

  dc << std::endl;
}

void
Diag952Collector::printRoutings(const ShoppingTrx& shoppingTrx)
{
  DiagCollector& dc(*this);

  VendorCode vendor = "";
  CarrierCode carrier = "";
  RoutingNumber routingNumber = "";
  std::string routingTariff = "";
  int tariff = 0;
  LocCode origin = "";
  LocCode destination = "";

  vendor = dc.rootDiag()->diagParamMapItem("VENDOR");

  if (vendor.empty())
  {
    dc << "\"VENDOR\" parameter not specified in request" << std::endl;
    return;
  }

  carrier = dc.rootDiag()->diagParamMapItem("CARRIER");

  if (carrier.empty())
  {
    dc << "\"CARRIER\" parameter not specified in request" << std::endl;
    return;
  }

  routingNumber = dc.rootDiag()->diagParamMapItem("RTGNUM");

  if (routingNumber.empty())
  {
    dc << "\"RTGNUM\" parameter not specified in request" << std::endl;
    return;
  }

  routingTariff = dc.rootDiag()->diagParamMapItem("RTGTARIFF");

  if (routingTariff.empty())
  {
    dc << "\"RTGTARIFF\" parameter not specified in request" << std::endl;
    return;
  }
  else
  {
    try { tariff = atoi(routingTariff.c_str()); }
    catch (...)
    {
      dc << "Incorrect RTGTARIFF value" << std::endl;
      return;
    }
  }

  origin = dc.rootDiag()->diagParamMapItem("ORIG");

  if (origin.empty())
  {
    dc << "\"ORIG\" parameter not specified in request" << std::endl;
    return;
  }

  destination = dc.rootDiag()->diagParamMapItem("DEST");

  if (destination.empty())
  {
    dc << "\"DEST\" parameter not specified in request" << std::endl;
    return;
  }

  dc << "DIAGNOSTIC ARGUMENTS:" << std::endl << std::endl << "VENDOR (Vendor code): " << vendor
     << std::endl << "CARRIER (Carrier code): " << carrier << std::endl
     << "RTGNUM (Routing number): " << routingNumber << std::endl
     << "RTGTARIFF (Routing tariff): " << routingTariff << std::endl
     << "ORIG (Origin market): " << origin << std::endl
     << "DEST (Destination market): " << destination << std::endl << std::endl;

  const MarketRoutingInfo& boundRoutingInfo = shoppingTrx.dataHandle().getMarketRoutingInfo(
      vendor, carrier, routingNumber, tariff, origin, destination, true, true);

  printSingles(boundRoutingInfo);
  printDoubles(boundRoutingInfo);
}

void
Diag952Collector::printSingles(const MarketRoutingInfo& boundRoutingInfo)
{
  DiagCollector& dc(*this);

  if ((boundRoutingInfo.singles() != nullptr) && (!boundRoutingInfo.singles()->empty()))
  {
    dc << "SINGLES:" << std::endl;

    std::string singles = "";

    MarketRoutingSinglesVec::const_iterator singlesIter = boundRoutingInfo.singles()->begin();
    MarketRoutingSinglesVec::const_iterator singlesIterEnd = boundRoutingInfo.singles()->end();

    for (; singlesIter != singlesIterEnd; ++singlesIter)
    {
      const LocCode singleObj = (*singlesIter);

      singles += (singleObj + "|");
    }

    if (!singles.empty())
    {
      singles.erase(singles.size() - 1);
    }

    dc << singles << std::endl << std::endl;
  }
  else
  {
    dc << "NO SINGLES FOUND" << std::endl << std::endl;
  }
}

void
Diag952Collector::printDoubles(const MarketRoutingInfo& boundRoutingInfo)
{
  DiagCollector& dc(*this);

  if ((boundRoutingInfo.doubles() != nullptr) && (!boundRoutingInfo.doubles()->empty()))
  {
    dc << "DOUBLES:" << std::endl;

    std::string doubles = "";

    MarketRoutingDoublesVec::const_iterator doublesIter = boundRoutingInfo.doubles()->begin();
    MarketRoutingDoublesVec::const_iterator doublesIterEnd = boundRoutingInfo.doubles()->end();

    for (; doublesIter != doublesIterEnd; ++doublesIter)
    {
      doubles += "(" + doublesIter->market1() + "," + doublesIter->market2() + ")|";
    }

    if (!doubles.empty())
    {
      doubles.erase(doubles.size() - 1);
    }

    dc << doubles << std::endl << std::endl;
  }
  else
  {
    dc << "NO DOUBLES FOUND" << std::endl << std::endl;
  }
}
}
