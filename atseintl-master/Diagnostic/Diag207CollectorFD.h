//-----------------------------------------------------------------------------
//
//  File:     Diag207CollectorFD.h
//
//  Author :  Adam Szalajko
//
//  Copyright Sabre 2008
//
//          The copyright of the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the agreement/contract
//          under which the program(s) have been supplied.
//
//-----------------------------------------------------------------------------

#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{
class FareDisplayTrx;
class FareDisplayInclCd;
class FareDisplayWeb;

// Diag212Collector is the diagnostic for currency
class Diag207CollectorFD : public DiagCollector
{
public:
  explicit Diag207CollectorFD(Diagnostic& root)
    : DiagCollector(root),
      _dtAft(' '),
      _ftApt(' '),
      _dtApt(' '),
      _exceptFareType(' '),
      _exceptPsgType(' '),
      _fareDisplayInclCd(nullptr)
  {
  }

  Diag207CollectorFD()
    : _dtAft(' '),
      _ftApt(' '),
      _dtApt(' '),
      _exceptFareType(' '),
      _exceptPsgType(' '),
      _fareDisplayInclCd(nullptr)
  {
  }

  void displayInclusionCode(FareDisplayTrx& trx, FareDisplayInclCd* inlcusionCode);
  void displayWebInclusionCode(FareDisplayTrx& trx);

private:
  void displayInclCode(const FareDisplayTrx& trx);
  void displayRuleTrf(const FareDisplayTrx& trx);
  void displayRelation(const FareDisplayTrx& trx);
  void displayFareTypes(const FareDisplayTrx& trx);
  void displayDispTypes(const FareDisplayTrx& trx);
  void displayRec1PsgTypes(const FareDisplayTrx& trx);
  void displayRec8PsgTypes(const FareDisplayTrx& trx);
  void displayChdPsgTypes(const FareDisplayTrx& trx);
  void displayInfPsgTypes(const FareDisplayTrx& trx);

  void displayWebFares(const FareDisplayTrx& trx);
  void displayWebFare(const FareDisplayTrx& trx,
                      FareDisplayWeb& web,
                      std::set<PaxTypeCode>& paxTypeCodes,
                      bool& fIsFirstLine);
  void addText(std::ostringstream& str, int size, bool fNewLine = false);
  void addFooter(int lineLength);
  void addStarLine(int LineLength);

  bool getRuleTariffDescription(const FareDisplayTrx& trx,
                                const TariffNumber& ruleTariff,
                                const VendorCode& vendor,
                                const CarrierCode& carrier,
                                TariffCode& ruleTariffCode);

  InclusionCode _inclCode;
  Description _description;
  Indicator _dtAft;
  Indicator _ftApt;
  Indicator _dtApt;
  Indicator _exceptFareType;
  Indicator _exceptPsgType;
  FareDisplayInclCd* _fareDisplayInclCd;
};

} // namespace tse

