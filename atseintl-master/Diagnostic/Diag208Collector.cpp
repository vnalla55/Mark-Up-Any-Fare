//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------

#include "Diagnostic/Diag208Collector.h"

#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxType.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/Loc.h"
#include "DBAccess/LocKey.h"

#include <iomanip>

namespace tse
{
const char*
Diag208Collector::R8_PASS("R8 PASSES");
const char*
Diag208Collector::TRXREF_NOT_FOUND("TRXREF NOT FOUND");
const char*
Diag208Collector::TARIFF_INHIBIT("TARIFF INHIBIT");
const char*
Diag208Collector::FAIL_TRXREF("TARIFF GEO");
const char*
Diag208Collector::FAIL_PAX_STATUS("PSGR STATUS");
const char*
Diag208Collector::FAIL_GLOBAL("GLOBAL");
const char*
Diag208Collector::FAIL_GEO("GEO");
const char*
Diag208Collector::FAIL_LOC3("LOC3");
const char*
Diag208Collector::FAIL_SECURITY("SEC");
const char*
Diag208Collector::FAIL_SAME_CARRIER("SAME CARRIER");
const char*
Diag208Collector::FAIL_CXRFLT_TBL("TBL986");
const char*
Diag208Collector::FAIL_PL_PV_TARIFF("PUBLIC/PRIVATE TRF");
const char Diag208Collector::BLANK = ' ';

void
Diag208Collector::printHeader()
{
  if (_active)
  {
    *this << "************  FARE BY RULE - RECORD 8 DIAGNOSTICS  ************\n";
  }
}

void
Diag208Collector::writeSeparator(SeparatorType st)
{
  if (_active)
  {
    ((DiagCollector&)*this) << "VDR  CXR TRF  RULE PSG  ACCOUNT              FAIL   \n"
                            << "         NBR  NBR  TYPE CODE                 CODE   \n";
  }
}

Diag208Collector&
Diag208Collector::operator<<(const FareMarket& fareMarket)
{
  if (_active)
  {
    DiagCollector& dc = (DiagCollector&)*this;

    dc << std::endl;
    dc << "***************************************************************";

    dc << std::endl;
    dc << fareMarket.boardMultiCity();
    dc << "-";
    dc << fareMarket.governingCarrier();
    dc << "-";
    dc << fareMarket.offMultiCity();
    dc << std::endl;
    dc << std::endl;

    writeSeparator(FARE_MARKET_HEADER);
  }

  return *this;
}

Diag208Collector&
Diag208Collector::operator<<(const PaxType& paxType)
{
  if (_active)
  {
    DiagCollector& dc = (DiagCollector&)*this;

    dc << "REQUESTED PAXTYPE : ";
    dc << paxType.paxType();
    dc << std::endl;
  }

  return *this;
}

void
Diag208Collector::diag208Collector(FareByRuleApp& fbrApp,
                                   PricingTrx& trx,
                                   const char* failCode,
                                   const FareMarket& fareMarket)
{
  if (_active)
  {
    bool longDisplay = false;
    const std::string& diagRule = trx.diagnostic().diagParamMapItem(Diagnostic::RULE_NUMBER);
    if (!diagRule.empty())
    {
      longDisplay = true;
    }

    DiagCollector& dc(*this);

    dc.setf(std::ios::left, std::ios::adjustfield);

    dc << "---------------------------------------------------------------";
    dc << std::endl;

    dc << std::setw(5) << fbrApp.vendor();
    dc << std::setw(4) << fbrApp.carrier();
    dc << std::setw(5) << fbrApp.ruleTariff();
    dc << std::setw(5) << fbrApp.ruleNo();
    dc << std::setw(5) << fbrApp.primePaxType();
    dc << std::setw(21) << fbrApp.accountCode();

    if (failCode != R8_PASS)
    {
      dc << failCode;
    }

    dc << std::endl;

    if (longDisplay)
    {
      dc << "APPL-";
      if (fbrApp.paxInd() != Diag208Collector::BLANK)
      {
        if (fbrApp.negPaxStatusInd() == Diag208Collector::BLANK)
        {
          dc << "Y ";
        }
        else
        {
          dc << std::setw(2) << fbrApp.negPaxStatusInd();
        }
      }
      else
      {
        dc << "  ";
      }
      dc << "STATUS-";
      dc << std::setw(4) << fbrApp.paxInd();
      dc << "PAX LOCTYPE-";
      dc << std::setw(4) << fbrApp.paxLoc().locType();
      dc << "LOC-";
      dc << std::setw(5) << fbrApp.paxLoc().loc();
      dc << std::endl;

      dc << "TD-";
      dc << std::setw(13) << fbrApp.tktDesignator();
      dc << "DI-";
      dc << std::setw(4) << fbrApp.directionality();
      dc << "GBL-";
      std::string gd;
      globalDirectionToStr(gd, fbrApp.globalDir());
      dc << std::setw(4) << gd;
      dc << "SAME CXR-";
      dc << std::setw(3) << fbrApp.sameCarrier();
      dc << "TBL986-";
      dc << std::setw(10) << fbrApp.carrierFltTblItemNo();
      dc << std::endl;

      dc << "LOC1TYPE-";
      dc << std::setw(4) << fbrApp.fareLoc1().locType();
      dc << "LOC1-";
      dc << std::setw(6) << fbrApp.fareLoc1().loc();
      dc << "LOC1ZONE-";
      dc << std::setw(8) << fbrApp.loc1zoneItemNo();
      dc << std::endl;

      dc << "LOC2TYPE-";
      dc << std::setw(4) << fbrApp.fareLoc2().locType();
      dc << "LOC2-";
      dc << std::setw(6) << fbrApp.fareLoc2().loc();
      dc << "LOC2ZONE-";
      dc << std::setw(8) << fbrApp.loc2zoneItemNo();
      dc << std::endl;

      dc << "LOC3TYPE-";
      dc << std::setw(4) << fbrApp.whollyWithinLoc().locType();
      dc << "LOC3-";
      dc << std::setw(6) << fbrApp.whollyWithinLoc().loc();
      dc << "SEGS-";
      dc << std::setw(5) << fbrApp.segCnt();
      dc << std::endl;
    }

    if ((failCode == R8_PASS || longDisplay) && fbrApp.segCnt() != 0)
    {
      dc << "    SEC PAXTYPE : ";

      std::vector<PaxTypeCode> secPaxTypesVec = fbrApp.secondaryPaxTypes();
      std::vector<PaxTypeCode>::iterator i = secPaxTypesVec.begin();
      std::vector<PaxTypeCode>::iterator j = secPaxTypesVec.end();

      for (; i != j; ++i)
      {
        PaxTypeCode secPaxType = *i;
        dc << std::setw(4) << secPaxType;
      }
      dc << std::endl;
    }
  }
}
}
