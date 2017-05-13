//----------------------------------------------------------------------------
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

#include "Diagnostic/Diag413Collector.h"

#include "Common/ClassOfService.h"
#include "Common/FareMarketUtil.h"
#include "Common/TseConsts.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/DifferentialData.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"

#include <iomanip>
#include <iostream>
#include <vector>

using namespace std;

namespace tse
{
// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag413Collector::bkg413Header
//
// Description:  This method will display diagnostic information to allow for a
//               quick debug of Differential process.
//               Diagnostic number must be set in the Transaction Orchestrator
//               to apply the following methods:
//
//
// </PRE>
// ----------------------------------------------------------------------------

void
Diag413Collector::bkg413Header(DiffDiagHeader bkgH)
{
  if (_active)
  {
    if (bkgH == HEADER)
    {
      ((DiagCollector&)*this) << "---------------------------------------------------------------\n"
                              << "\n                  DIFFERENTIALS DIAGNOSTIC \n\n";
    }
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag413Collector::operator <<
//
// Description:  This method will override the base operator << to handle the
//               FareMarket data for the Differential validation Diagnostic Display.
//
// @param  mkt - FareMarket
//
//
// </PRE>
// ----------------------------------------------------------------------------

Diag413Collector&
Diag413Collector::operator << ( const FareMarket& mkt )
{
  if (!_active)
    return *this;

  DiagCollector& dc = (DiagCollector&)*this;

  if (!_header)
  {
    bkg413Header(HEADER);
    ++_header;
  }

  // If we dont have travel segments, we count output this line
  if (mkt.travelSeg().size() == 0 || mkt.governingCarrier().empty())
  {
    dc << " *** THERE ARE NO APPLICABLE FARES FOR THIS FARE MARKET ***\n";
    return *this;
  }
  dc << "\nTHROUGH MARKET  " << FareMarketUtil::getBoardMultiCity(mkt, *mkt.travelSeg().front())
     << "-" << mkt.governingCarrier() << "-"
     << FareMarketUtil::getOffMultiCity(mkt, *mkt.travelSeg().back());

  if (mkt.travelBoundary().isSet(FMTravelBoundary::TravelWithinUSCA)) // It is Domestic fare component
    dc << "    US/CA\n";
  else
    dc << "    INTERNATIONAL\n";

  return *this;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag413Collector::operator <<
//
// Description:  This method will override the base operator << to handle the
//               FareUsage data for the Differential validation Diagnostic Display.
//
// @param  fu - FareUsage
//
//
// </PRE>
// ----------------------------------------------------------------------------
Diag413Collector&
Diag413Collector::operator << (const FareUsage  &fu)
{
  if (_active)
  {
    _fareUsage = &fu;
    _paxTfare = const_cast<PaxTypeFare*>(fu.paxTypeFare());
    try
    {
      const vector<DifferentialData*> differV = differential();

      bool consolid = false;
      bool adjacent = false;
      bool premiumFare = false;
      if (_paxTfare->cabin().isPremiumBusinessClass() || _paxTfare->cabin().isPremiumEconomyClass())
        premiumFare = true;

      for (const auto elem : differV)
      {
        if (elem->status() == DifferentialData::SC_CONSOLIDATED_PASS ||
            elem->status() == DifferentialData::SC_CONSOLIDATED_FAIL ||
            elem->status() == DifferentialData::SC_COMBINATION_FAIL)
        {
          consolid = true;
        }
        else if (elem->status() == DifferentialData::SC_ADJACENT_FAIL)
          adjacent = true;
      }

      DiagCollector& dc = (DiagCollector&)*this;

      dc.setf(std::ios::left, std::ios::adjustfield);

      dc << " " << setw(9) << _paxTfare->fareClass()
         << (_paxTfare->directionality() == FROM ? "O" : "I");

      dc << DiagnosticUtil::getOwrtChar(*_paxTfare);

      dc << "    NUC";

      dc.setf(std::ios::left, std::ios::adjustfield);
      dc.setf(std::ios::fixed, std::ios::floatfield);
      dc.precision(2);
      dc << std::setw(9) << _paxTfare->nucFareAmount();
      dc.unsetf(std::ios::left);

      if (_fareUsage->diffCarrier().size() != 0)
      {
        dc << "  " << _fareUsage->diffCarrier();
      }
      else
        dc << "    ";

      if (_fareUsage->differSeqNumber() != 0)
      {
        dc << std::setw(5) << _fareUsage->differSeqNumber();
      }
      else
        dc << "     ";

      if (_fareUsage->calculationInd() == 'A')
        dc << " L";
      else if (_fareUsage->calculationInd() == ' ')
        dc << " S";
      else if (_fareUsage->calculationInd() == 'N')
        dc << " " << _fareUsage->calculationInd();
      else
        dc << "  ";

      //      if (  _fareUsage->calculationInd() != 'N' )
      {
        dc << " ";
        if (_fareUsage->hipExemptInd() == 'N')
        {
          dc << "  ";
        }
        else
          dc << "HE";

        if (_paxTfare->mileageSurchargePctg() != 0 && consolid)
        {
          dc << " " << _paxTfare->mileageSurchargePctg() << "M";
        }
        else
        {
          dc << "    ";
        }

        if (_paxTfare->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
          dc << "  R/T DIF"; // tag 2
        else
          dc << "  O/W DIF";

        if (premiumFare)
          dc << " PS ";
        else
          dc << "    ";
        dc << "CALC\n";
        MoneyAmount commonOW = 0.f;

        size_t len = 0;
        char buff[5];

        for (const auto elem : differV)
        {
          if (elem->isDiffFMFalse())
          {
            dc << "      FAIL - ORIGIN/DEST ARE EQUAL TO FARE COMPONENT";
            break;
          }
          dc.unsetf(std::ios::left);
          dc.setf(std::ios::right, std::ios::adjustfield);

          if ((elem->status() == DifferentialData::SC_CONSOLIDATED_PASS) ||
              (elem->status() == DifferentialData::SC_CONSOLIDATED_FAIL) ||
              (elem->status() == DifferentialData::SC_COMBINATION_FAIL) ||
              elem->thruNumber() == 0) //
          {
            DifferentialDataPtrVecIC itOWCons = elem->inConsolDiffData().begin();
            DifferentialData* endOfCons = elem->inConsolDiffData().back();

            if (elem->isItemConsolidated())
            {
              if (((*itOWCons)->status() == DifferentialData::SC_CONSOLIDATED_PASS) ||
                  ((*itOWCons)->status() == DifferentialData::SC_CONSOLIDATED_FAIL) ||
                  ((*itOWCons)->status() == DifferentialData::SC_COMBINATION_FAIL) ||
                  (*itOWCons)->thruNumber() == 0) //
              {
                findThruNumber(buff, **itOWCons);
                dc << buff;
              }
              else
              {
                sprintf(buff, "%d", elem->inConsolDiffData().front()->thruNumber());
                dc << buff;
              }
              dc << "-";

              if ((endOfCons->status() == DifferentialData::SC_CONSOLIDATED_PASS) ||
                  (endOfCons->status() == DifferentialData::SC_CONSOLIDATED_FAIL) ||
                  (endOfCons->status() == DifferentialData::SC_COMBINATION_FAIL) ||
                  (endOfCons)->thruNumber() == 0)
              {
                sprintf(buff, "%d", endOfCons->inConsolDiffData().back()->thruNumber());
                dc << buff;
              }
              else
              {
                sprintf(buff, "%d", elem->inConsolDiffData().back()->thruNumber());
                dc << buff;
              }
            }
            else
            {
              sprintf(buff, "%d", elem->inConsolDiffData().front()->thruNumber());
              dc << buff;
              dc << "-";
              sprintf(buff, "%d", elem->inConsolDiffData().back()->thruNumber());
              dc << buff;
            }
          }
          else
          {
            sprintf(buff, "%d", elem->thruNumber());
            dc << std::setw(3) << buff;
          }

          dc.unsetf(std::ios::right);
          dc.setf(std::ios::left, std::ios::adjustfield);

          if (elem->status() == DifferentialData::SC_CONSOLIDATED_PASS ||
              elem->status() == DifferentialData::SC_CONSOLIDATED_FAIL ||
              elem->status() == DifferentialData::SC_COMBINATION_FAIL ||
              elem->thruNumber() == 0) //
          {
            dc << " CON " << (Indicator)elem->cabin() << " ";
          }
          else
          {
            dc << " DIF   ";
          }

          vector<FareMarket*>::iterator fmi = elem->fareMarket().begin();
          dc << FareMarketUtil::getBoardMultiCity(**fmi, *(*fmi)->travelSeg().front())
             << FareMarketUtil::getOffMultiCity(**fmi, *(*fmi)->travelSeg().back()) << " ";

          vector<CarrierCode>::iterator carr = elem->carrier().begin();
          if (!elem->carrierDiff().empty() &&
              elem->carrierDiff() != _paxTfare->fareMarket()->governingCarrier())
          {
            dc << elem->carrierDiff() << " ";
          }
          else
          if ((*carr) != _paxTfare->fareMarket()->governingCarrier())
          {
            dc << (*carr) << " ";
          }
          else
          {
            dc << "   ";
          }

          if (elem->status() == DifferentialData::SC_ADJACENT_FAIL)
          {
            dc << "FAILED: ADJACENT                   ";
          }
          else
          {
            if ((elem->amountFareClassHigh() == 0 && elem->amountFareClassLow() == 0) &&
                !elem->fareHigh() && !elem->fareLow())
            {
              if (adjacent)
                dc << "NOT PROCESSED, FAIL ADJACENT       ";
              else if (elem->calculationIndicator() != 'N')
              {
                if (consolid)
                  dc << "FAIL:LOW/HIGH DIFF FARES NOT FOUND ";
                else if (elem->isFailCat23())
                  dc << "FAIL CAT23: HIGH/LOW DIFF FARES    ";
                else
                  dc << "                                   ";
              }
              else if (elem->status() == DifferentialData::SC_NOT_PROCESSED_YET)
              {
                dc << "NOT PROCESSED BECAUSE OF THE ABOVE ";
              }
              else
              {
                dc << "FAILED: DIFF TABLE ITEM  - ";
                if (elem->differSeqNumber() != 0)
                {
                  dc.unsetf(std::ios::right);
                  dc.setf(std::ios::left, std::ios::adjustfield);
                  dc << std::setw(7) << elem->differSeqNumber();
                }
                else
                  dc << "     ";

                dc << elem->calculationIndicator();
              }
            }
            else if (elem->amountFareClassHigh() == 0 && !elem->fareHigh())
            {
              dc << "FAILED: HIGH DIFFL FARES NOT FOUND ";
            }
            else if (elem->amountFareClassLow() == 0 && !elem->fareLow())
            {
              dc << "FAILED: LOW DIFFL FARES NOT FOUND  ";
            }
            else
            {
              FareClassCode fareBasisH = elem->fareClassHigh();
              FareClassCode fareBasisL = elem->fareClassLow();
              if (fareBasisH.size() > 8)
                fareBasisH = fareBasisH.substr(0, 7) + "*";
              if (fareBasisL.size() > 8)
                fareBasisL = fareBasisL.substr(0, 7) + "*";

              dc.setf(std::ios::left, std::ios::adjustfield);
              dc << setw(8) << fareBasisH // Fare High selection
                 << setw(8) << fareBasisL // Fare Low selection
                 << " NUC";

              dc.unsetf(std::ios::left);
              dc.setf(std::ios::right, std::ios::adjustfield);
              dc.setf(std::ios::fixed, std::ios::floatfield);
              dc.precision(2);
              if (elem->hipAmount() > 0)
                dc << std::setw(7) << elem->hipAmount(); // Differential amount
              else
                dc << std::setw(7) << elem->amount();
              dc << " ";

              if (elem->differSeqNumber() != 0)
                dc << std::setw(5) << elem->differSeqNumber();
              else
                dc << "     ";

              if (elem->calculationIndicator() == 'A')
                dc << " L";
              else if (elem->calculationIndicator() == ' ')
                dc << " S";
              else
                dc << " " << elem->calculationIndicator();
            }
          }
          if (premiumFare)
          {
            if (elem->isSlideAllow())
              dc << "  Y ";
            else
              dc << "  N ";
          }
          else
            dc << "    ";

          if (elem->tripType() == 'O')
          {
            dc << "  OW";
          }
          else
            dc << "  RT";

          if ((elem->status() == DifferentialData::SC_CONSOLIDATED_PASS) ||
              (elem->status() == DifferentialData::SC_COMBINATION_FAIL) ||
              ((elem->status() == DifferentialData::SC_CONSOLIDATED_FAIL ||
                elem->thruNumber() == 0) &&
               elem->amountFareClassHigh() != 0 && elem->amountFareClassLow() != 0))
          {
            DifferentialDataPtrVecIC itOWCons = elem->inConsolDiffData().begin();
            DifferentialDataPtrVecIC itOWConsE = elem->inConsolDiffData().end();

            std::string sn;
            std::ostringstream sOW;

            commonOW = 0;
            bool noCombAmount = false;

            for (; itOWCons != itOWConsE; itOWCons++)
            {
              commonOW = commonOW + (((*itOWCons)->hipAmount() != 0.f) ? (*itOWCons)->hipAmount()
                                                                       : (*itOWCons)->amount());
              if ((*itOWCons)->isItemConsolidated())
              {
                sprintf(buff, "%d", (*itOWCons)->inConsolDiffData().front()->thruNumber());
                sn += buff;
                sn += "-";
                sprintf(buff, "%d", (*itOWCons)->inConsolDiffData().back()->thruNumber());
                sn += buff;
              }
              else
              {
                if ((*itOWCons)->thruNumber() == 0) //
                { //
                  sprintf(buff, "%d", (*itOWCons)->inConsolDiffData().front()->thruNumber());
                  sn += buff;
                  sn += "-";
                  sprintf(buff, "%d", (*itOWCons)->inConsolDiffData().back()->thruNumber());
                  sn += buff;
                } //
                else
                { //
                  sprintf(buff, "%d", (*itOWCons)->thruNumber());
                  sn += buff;
                }
              }
              sn += " ";
            }
            if (commonOW == 0.f)
              noCombAmount = true;

            sOW << sn << " ";
            if (!sOW.str().empty())
            {
              len = sOW.str().size() - 2;

              if (elem->tripType() == 'O')
              {
                dc << "     COMBINATION OW:  ";
              }
              else
                dc << "     COMBINATION RT:  ";

              dc << sOW.str().substr(0, len);

              // number 20 is calculated by 60 - lenght of "COMBINATION.." - ...
              int16_t emptyChar = 20 - len;
              for (; emptyChar >= 0; emptyChar--)
                dc << " ";

              if (!noCombAmount)
              {
                dc.unsetf(std::ios::left);
                dc.setf(std::ios::right, std::ios::adjustfield);
                dc.setf(std::ios::fixed, std::ios::floatfield);
                dc.precision(2);
                dc << "NUC" << std::setw(7) << commonOW;
              }
              else
              {
                dc << "NOT APPLICABLE";
              }
            }
          }
          dc << "\n";

          if (elem->hipAmount() > 0)
          {
            dc << "          HIGH HIP: ";
            if (elem->hipAmtFareClassHigh() != 0)
              dc << elem->hipHighOrigin() << elem->hipHighDestination() << "  ";
            else
              dc << "NO HIP  ";

            dc << "LOW HIP: ";
            if ((elem->hipAmtFareClassLow() != 0) &&
                ((elem->hipLowOrigin() != elem->hipHighOrigin()) ||
                 (elem->hipLowDestination() != elem->hipHighDestination())))
              dc << elem->hipLowOrigin() << elem->hipLowDestination() << "\n";
            else
              dc << "NO HIP\n";
          }

          if (elem->amount() == 0 && !elem->fareClassHigh().empty() &&
              !elem->fareClassLow().empty())
          {
            FareClassCode fareBasisH = elem->fareClassHigh();
            FareClassCode fareBasisL = elem->fareClassLow();
            if (fareBasisH.size() > 8)
              fareBasisH = fareBasisH.substr(0, 7) + "*";
            if (fareBasisL.size() > 8)
              fareBasisL = fareBasisL.substr(0, 7) + "*";

            dc << "                            ";
            dc.setf(std::ios::left, std::ios::adjustfield);
            dc << setw(8) << fareBasisH // Fare High selection
               << " NUC";

            dc.unsetf(std::ios::left);
            dc.setf(std::ios::right, std::ios::adjustfield);
            dc.setf(std::ios::fixed, std::ios::floatfield);
            dc.precision(2);
            dc << std::setw(7) << elem->amountFareClassHigh(); // Fare High amount
            dc << "\n                            ";

            dc.setf(std::ios::left, std::ios::adjustfield);
            dc << setw(8) << fareBasisL // Fare Low selection
               << " NUC";

            dc.unsetf(std::ios::left);
            dc.setf(std::ios::right, std::ios::adjustfield);
            dc.setf(std::ios::fixed, std::ios::floatfield);
            dc.precision(2);
            dc << std::setw(7) << elem->amountFareClassLow(); // Fare Low amount
            dc << "\n";
          }
        }
      }
      dc << "\n";
    }
    catch (...) { return *this; }
  }
  return *this;
}

void
Diag413Collector::findThruNumber(char buff[5], DifferentialData& df)
{
   if(df.thruNumber())
   {
     sprintf(buff, "%d", df.thruNumber());
   }
   else
   {
     findThruNumber(buff, *(df.inConsolDiffData()[0]));
   }
   return;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag413Collector::operator <<
//
// Description:  This method will sent messages to diag buffer. All messages are in the
//               enum  Diag411Message
//
// @param  num - number
//
//
// </PRE>
// ----------------------------------------------------------------------------

void
Diag413Collector::lineSkip(int num)
{

  if (_active)
  {
    DiagCollector& dc = (DiagCollector&)*this;

    switch (num)
    {
    case 0:
      dc << "--------------------------------------------------------\n";
      break;
    case 1:
      dc << "\n";
      break;
    case 2:
      dc << "\n\n";
      break;
    case 3:
      dc << "\n\n\n";
      break;
    }
  }
}

const vector<DifferentialData*>&
Diag413Collector::differential(void) const throw(string&)
{
  if (!_fareUsage)
    throw(string("NULL fareUsage address"));

  return _fareUsage->differentialPlusUp();
}
}
