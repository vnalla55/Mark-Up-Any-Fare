//-------------------------------------------------------------------
//
//  File:        TravelTicketFilter.cpp
//  Authors:     LeAnn Perez
//  Created:     January 31, 2005
//  Description: A class to handle the intricate formatting
//               associated with the TRAVEL_TICKET column
//               on the Fare Quote display.
//
//  Updates:     Doug Batchelor
//
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "FareDisplay/Templates/TravelTicketFilter.h"

#include "Common/DateTime.h"
#include "DataModel/FareDisplayInfo.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/TicketInfo.h"
#include "DataModel/TravelInfo.h"
#include "FareDisplay/Templates/Field.h"

#include <string>
#include <vector>

using namespace std;

namespace tse
{
namespace
{
// this value defines the Effective Date Indicator
const string EFF_DATE_INDICATOR = "E";
// this value defines the Discontinue Date Indicator
const string DIS_DATE_INDICATOR = "D";
// this value defines the Return Date Indicator
const string RET_DATE_INDICATOR = "R";
// this value defines the Travel Completion Date Indicator
const string CMP_DATE_INDICATOR = "C";
// this value defines the First Sale Date Indicator
const string FIRST_DATE_INDICATOR = "S";
// Show * when Historical processing gets 1st Sale from Fare's Effective (rare)
const string FIRST_SALE_FROM_EFF = "*";
// this value defines the Last Sale Date Indicator
const string LAST_DATE_INDICATOR = "T";
// this value defines a single blank
const string ONE_BLANK = " ";
// this value defines two blanks
const string TWO_BLANKS = "  ";
// this value defines seven blanks
const string SEVEN_BLANKS = "       ";
// this value defines output when no dates are shown
const string NO_DATES = "    ----     ";
// this value defines output when fare not auto-priceable
const string NOT_AUTO_PRICEABLE = "NP";
// normal date width
const int DATE_WIDTH = 4;
}

namespace TravelTicketFilter
{
namespace
{
enum Sameness
{ Same2 = 1,
  Same3,
  Different };

bool
checkFareDates(FareDisplayInfo& fdInfo, DateTime tktDt)
{
  bool firstSaleFromEff = false;

  const Fare* fr = (fdInfo.paxTypeFare()).fare();
  DateTime frCrt = fr->createDate().date();
  DateTime frExp = fr->expirationDate().date();
  DateTime frEff = fr->effectiveDate().date();
  DateTime frDisc = fr->discDate().date();

  // Check Ticket Dates
  if (fdInfo.ticketInfo().size() > 0)
  { // Got at least one Tkt Date from Cat 15 (Sales Restriction)
    vector<TicketInfo*>::iterator ticketIter = fdInfo.ticketInfo().begin();
    TicketInfo& si = **ticketIter;

    DateTime srFirstTkt = ((DateTime)si.earliestTktDate()).date();
    DateTime srLastTkt = ((DateTime)si.latestTktDate()).date();

    if (!srFirstTkt.isValid() || (srFirstTkt < frCrt))
    { // Check to see if Fare Create date qualifies
      if ((frCrt <= frEff) && (frCrt == tktDt))
      {
        si.earliestTktDate() = frCrt;
      }
    }

    if (!srFirstTkt.isValid() || (srFirstTkt < frEff))
    { // Check to see if Fare Effective date qualifies
      if ((tktDt >= frEff) && (tktDt <= frCrt))
      {
        si.earliestTktDate() = frEff;
        firstSaleFromEff = true;
      }
    }

    if (!srLastTkt.isValid() || (srLastTkt > frExp))
    { // Check to see if Fare Expire date qualifies
      if (frExp >= tktDt)
      {
        si.latestTktDate() = frExp;
      }
    }
  }
  else
  { // No dates from Cat 15, so just stick fare dates in there if they qualify
    DateTime firstDt = DateTime::emptyDate();
    if (frCrt <= frEff)
    {
      if (frCrt == tktDt)
      {
        firstDt = frCrt;
      }
    }
    else // frEff < frCrt
    {
      if ((tktDt >= frEff) && (tktDt <= frCrt))
      {
        firstDt = frEff;
        firstSaleFromEff = true;
      }
    }

    DateTime lastDt = (frExp >= tktDt) ? frExp : DateTime::emptyDate();

    if (firstDt.isValid() || lastDt.isValid())
    {
      fdInfo.addTicketInfo(firstDt, lastDt);
    }
  }

  // Check Travel Dates
  DateTime tvlDt = fdInfo.fareDisplayTrx().travelDate().date();
  if (fdInfo.travelInfo().size() > 0)
  { // Got at least one Tvl Date from Cat 14 (Travel Restriction)
    vector<TravelInfo*>::iterator tvlIter = fdInfo.travelInfo().begin();
    TravelInfo& ti = **tvlIter;

    DateTime trFirstTvl = ((DateTime)ti.earliestTvlStartDate()).date();
    DateTime trLastTvl = ((DateTime)ti.latestTvlStartDate()).date();

    if (!trFirstTvl.isValid() || (trFirstTvl < frEff))
    { // Check to see if Fare Create date qualifies
      if (frEff >= tktDt)
      {
        ti.earliestTvlStartDate() = frEff;
      }
    }

    if (!trLastTvl.isValid() || (trLastTvl > frDisc))
    {
      if ((frDisc >= tktDt) && (frDisc >= tvlDt))
      {
        //                ti.latestTvlStartDate() = frExp;
        ti.latestTvlStartDate() = frDisc;
      }
    }
  }
  else
  { // No dates from Cat 14, so just stick fare dates in there if they qualify

    DateTime firstDt = (frEff >= tktDt) ? frEff : DateTime::emptyDate();

    DateTime lastDt = ((frDisc >= tktDt) && (frDisc >= tvlDt)) ? frDisc : DateTime::emptyDate();

    if (firstDt.isValid() || lastDt.isValid())
    {
      fdInfo.addTravelInfo(firstDt, lastDt, DateTime::emptyDate(), ' ');
    }
  }

  return firstSaleFromEff;
} // checkFareDates()

void
getTicketDates(FareDisplayInfo& fdInfo, DateTime& earliestTktDate, DateTime& latestTktDate)
{
  std::vector<TicketInfo*>::const_iterator ticketInfoI = fdInfo.ticketInfo().begin();
  std::vector<TicketInfo*>::const_iterator ticketInfoEnd = fdInfo.ticketInfo().end();

  for (; ticketInfoI != ticketInfoEnd; ticketInfoI++)
  {
    TicketInfo& ti = **ticketInfoI;
    if (ti.earliestTktDate().isValid())
    {
      if (earliestTktDate == DateTime::emptyDate())
      {
        earliestTktDate = ti.earliestTktDate();
      }
      else
      {
        earliestTktDate.setWithLater(ti.earliestTktDate());
      }
    }
    if (ti.latestTktDate().isValid())
    {
      if (latestTktDate == DateTime::emptyDate())
      {
        latestTktDate = ti.latestTktDate();
      }
      else
      {
        latestTktDate.setWithEarlier(ti.latestTktDate());
      }
    }
  }
}

// This method converts the various dates into displayable fields
// of the form DDMM.  It also counts the total Travel and Sales dates.
// It also compares the dates and sends back a "Sameness" attribute
// for the Travel and Sales dates.
void
convertDates(FareDisplayInfo& fareDisplayInfo,
             int& nTravel,
             int& nSales,
             Sameness& tSame,
             Sameness& sSame,
             string& eDate,
             string& dDate,
             string& rDate,
             string& cDate,
             string& fSale,
             string& lSale)
{
  int travelSize = 0;
  int salesSize = 0;

  nTravel = 0;
  nSales = 0;

  tSame = Different;
  sSame = Different;

  if ((travelSize = fareDisplayInfo.travelInfo().size()) > 0)
  {
    //   =========  Travel info  ============
    vector<TravelInfo*>::iterator travelIter = fareDisplayInfo.travelInfo().begin();
    TravelInfo& ti = **travelIter;

    if (ti.earliestTvlStartDate().isValid())
    {
      nTravel++; // increment number of travel dates
      eDate = ti.earliestTvlStartDate().dateToString(DDMM_FD, "");
    }
    if (ti.latestTvlStartDate().isValid())
    {
      nTravel++; // increment number of travel dates
      dDate = ti.latestTvlStartDate().dateToString(DDMM_FD, "");
      if (nTravel == 2)
      {
        if (eDate == dDate)
          tSame = Same2;
      }
    }
    if (ti.stopDate().isValid())
    {
      nTravel++; // increment number of travel dates
      if (ti.returnTvlInd() == 'C')
      {
        rDate = ti.stopDate().dateToString(DDMM_FD, "");
        if (nTravel == 3)
        {
          if (rDate == eDate)
          {
            if (tSame == Same2)
              tSame = Same3;
            else
              tSame = Same2;
          }
          else if (rDate == dDate)
          {
            tSame = Same2;
          }
        }
        else if (nTravel == 2)
        {
          if ((rDate == dDate) || (rDate == eDate))
            tSame = Same2;
        }
      }
      else
      {
        cDate = ti.stopDate().dateToString(DDMM_FD, "");
        if (nTravel == 3)
        {
          if (cDate == eDate)
          {
            if (tSame == Same2)
              tSame = Same3;
            else
              tSame = Same2;
          }
          else if (cDate == dDate)
          {
            tSame = Same2;
          }
        }
        else if (nTravel == 2)
        {
          if ((cDate == dDate) || (cDate == eDate))
            tSame = Same2;
        }
      }
    }
  }

  if ((salesSize = fareDisplayInfo.ticketInfo().size()) > 0)
  {
    DateTime earliestTktDate = DateTime::emptyDate();
    DateTime latestTktDate = DateTime::emptyDate();

    getTicketDates(fareDisplayInfo, earliestTktDate, latestTktDate);
    if (earliestTktDate.isValid())
    {
      nSales++; // increment number of sales dates
      fSale = earliestTktDate.dateToString(DDMM_FD, "");
    }
    if (latestTktDate.isValid())
    {
      nSales++; // increment number of sales dates
      lSale = latestTktDate.dateToString(DDMM_FD, "");
      if (nSales == 2)
      {
        if (fSale == lSale)
          sSame = Same2;
      }
    }
  }
}

// This method finds the one valid travel date in the
// input group, returns it and the corresponding type.
// It also returns true if it finds a date and false if not.
bool
pick1TravelDateAndType(string& EffDate,
                       string& DisDate,
                       string& RetDate,
                       string& CmpDate,
                       string& pickedDate,
                       string& dateType)
{
  if (EffDate.length() == 0)
  {
    if (DisDate.length() == 0)
    {
      if (RetDate.length() == 0)
      {
        if (CmpDate.length() != 0)
        {
          pickedDate = CmpDate;
          dateType = CMP_DATE_INDICATOR;
          return true;
        }
      }
      else
      {
        pickedDate = RetDate;
        dateType = RET_DATE_INDICATOR;
        return true;
      }
    }
    else
    {
      pickedDate = DisDate;
      dateType = DIS_DATE_INDICATOR;
      return true;
    }
  }
  else
  {
    pickedDate = EffDate;
    dateType = EFF_DATE_INDICATOR;
    return true;
  }
  return false;
}

// This method finds two valid travel dates in the
// input group, returns them and the corresponding types.
// It also returns true if it finds two dates and false if not.
bool
pick2TravelDatesAndTypes(string& EffDate,
                         string& DisDate,
                         string& RetDate,
                         string& CmpDate,
                         string& pickedDate1,
                         string& pickedDate2,
                         string& dateType1,
                         string& dateType2)
{
  int cntr = 0;

  if (EffDate.length() != 0)
  {
    pickedDate1 = EffDate;
    dateType1 = EFF_DATE_INDICATOR;
    cntr++;
  }
  if (DisDate.length() != 0)
  {
    if (cntr == 0)
    {
      pickedDate1 = DisDate;
      dateType1 = DIS_DATE_INDICATOR;
      cntr++;
    }
    else
    {
      pickedDate2 = DisDate;
      dateType2 = DIS_DATE_INDICATOR;
      return true;
    }
  }
  if (RetDate.length() != 0)
  {
    if (cntr == 0)
    {
      // This is an error!! not 2 dates in list
      return false;
    }
    else
    {
      pickedDate2 = RetDate;
      dateType2 = RET_DATE_INDICATOR;
      return true;
    }
  }
  if (CmpDate.length() != 0)
  {
    if (cntr == 0)
    {
      // This is an error!! not 2 dates in list
      return false;
    }
    else
    {
      pickedDate2 = CmpDate;
      dateType2 = CMP_DATE_INDICATOR;
      return true;
    }
  }
  return false;
}

// This method orders the input three dates and dateTypes
// so that the the pair of dates that are equal are first.
// It's an inplace ordering, so inputs parameters are in/out.
void
order2SameOutOf3(string& pickedDate1,
                 string& pickedDate2,
                 string& pickedDate3,
                 string& dateType1,
                 string& dateType2,
                 string& dateType3)
{
  string tempDate;
  string tempType;

  if (pickedDate1 == pickedDate2)
  {
    // They're already ordered.
    return;
  }
  else if (pickedDate1 == pickedDate3)
  {
    tempDate = pickedDate2;
    tempType = dateType2;
    pickedDate2 = pickedDate3;
    dateType2 = dateType3;
    pickedDate3 = tempDate;
    dateType3 = tempType;
  }
  else // must be that pickedDate2 == pickedDated3
  {
    tempDate = pickedDate1;
    tempType = dateType1;
    pickedDate1 = pickedDate2;
    dateType1 = dateType2;
    pickedDate2 = pickedDate3;
    dateType2 = dateType3;
    pickedDate3 = tempDate;
    dateType3 = tempType;
  }
}
}

void
formatData(FareDisplayInfo& fareDisplayInfo, Field& field, Field& field2)
{
  std::ostringstream oss[2];

  // Check for NP condition (Not auto-Priceable)
  if (!fareDisplayInfo.isAutoPriceable())
  {
    field.strValue() = NOT_AUTO_PRICEABLE;
    return;
  }

  // Determine the dates in use and convert them to output format
  int numTravel = 0;
  int numSales = 0;
  Sameness travelSame;
  Sameness salesSame;
  string EffDate;
  string DisDate;
  string RetDate;
  string CmpDate;
  string FirstSale;
  string LastSale;

  DateTime tktDt = fareDisplayInfo.fareDisplayTrx().ticketingDate().date();
  bool firstSaleFromEff = false;

  if (tktDt.date() < DateTime::localTime().date())
  { // Historical, so some date manipulation is needed
    firstSaleFromEff = checkFareDates(fareDisplayInfo, tktDt);
  }

  convertDates(fareDisplayInfo,
               numTravel,
               numSales,
               travelSame,
               salesSame,
               EffDate,
               DisDate,
               RetDate,
               CmpDate,
               FirstSale,
               LastSale);

  // Now determine which of the 22 Travel & Ticket Matrix elements to apply.
  // The matrix element defines the display format.
  switch (numTravel)
  {
  case 0: // No travel dates
  {
    if (numSales == 0)
    {
      // Special case of no dates.
      oss[0] << NO_DATES;
    }
    else if (numSales == 1)
    {
      if (FirstSale.length() == 0)
      {
        oss[0] << SEVEN_BLANKS << LAST_DATE_INDICATOR << setw(DATE_WIDTH) << LastSale;
      }
      else
      {
        string firstDateInd = (firstSaleFromEff) ? FIRST_SALE_FROM_EFF : FIRST_DATE_INDICATOR;
        oss[0] << SEVEN_BLANKS << firstDateInd << setw(DATE_WIDTH) << FirstSale;
      }
    }
    else // numSales == 2
    {
      // Element 3 & 4  - No Travel, 2 sales dates
      string firstDateInd = (firstSaleFromEff) ? FIRST_SALE_FROM_EFF : FIRST_DATE_INDICATOR;
      oss[0] << firstDateInd << setw(DATE_WIDTH) << FirstSale << TWO_BLANKS << LAST_DATE_INDICATOR
             << setw(DATE_WIDTH) << LastSale;
    }
    break;
  }
  case 1: // 1 travel date
  {
    //  First pick out the Travel date and type (E,D,R,C)
    string pickedDate;
    string dateType;

    bool bRet = pick1TravelDateAndType(EffDate, DisDate, RetDate, CmpDate, pickedDate, dateType);
    if (!bRet)
    {
      //  TODO: This is an error, need to handle it here.
    }

    if (numSales == 0) // No sales dates
    {
      oss[0] << dateType << setw(DATE_WIDTH) << pickedDate;
    }
    else if (numSales == 1) //
    {
      // Element 5 & 6  - 1 Travel, 1 sales dates
      if (FirstSale.length() == 0)
      {
        oss[0] << dateType << setw(DATE_WIDTH) << pickedDate << TWO_BLANKS << LAST_DATE_INDICATOR
               << setw(DATE_WIDTH) << LastSale;
      }
      else
      {
        string firstDateInd = (firstSaleFromEff) ? FIRST_SALE_FROM_EFF : FIRST_DATE_INDICATOR;
        oss[0] << dateType << setw(DATE_WIDTH) << pickedDate << TWO_BLANKS << firstDateInd
               << setw(DATE_WIDTH) << FirstSale;
      }
    }
    else // numSales == 2
    {
      if (salesSame == Same2)
      {
        // Element 7 & 8  - 1 Travel, 2 sales dates that are equal
        string firstDateInd = (firstSaleFromEff) ? FIRST_SALE_FROM_EFF : FIRST_DATE_INDICATOR;
        oss[0] << dateType << setw(DATE_WIDTH) << pickedDate << ONE_BLANK << firstDateInd
               << LAST_DATE_INDICATOR << setw(DATE_WIDTH) << FirstSale;
      }
      else
      {
        if (pickedDate == FirstSale)
        {
          // Element 9  - 1 Travel, 2 sales dates that are not
          //              equal AND the travel is equal to one
          //              of the sales
          string firstDateInd = (firstSaleFromEff) ? FIRST_SALE_FROM_EFF : FIRST_DATE_INDICATOR;
          oss[0] << dateType << firstDateInd << setw(DATE_WIDTH) << pickedDate << ONE_BLANK
                 << LAST_DATE_INDICATOR << setw(DATE_WIDTH) << LastSale;
        }
        else if (pickedDate == LastSale)
        {
          // Element 9  - 1 Travel, 2 sales dates that are not
          //              equal AND the travel is equal to one
          //              of the sales
          string firstDateInd = (firstSaleFromEff) ? FIRST_SALE_FROM_EFF : FIRST_DATE_INDICATOR;
          oss[0] << dateType << LAST_DATE_INDICATOR << setw(DATE_WIDTH) << pickedDate << ONE_BLANK
                 << firstDateInd << setw(DATE_WIDTH) << FirstSale;
        }
        else
        {
          // Element 10 - 1 Travel, 2 sales dates that are not
          //              equal AND all three dates are not equal.
          //              This requires two lines
          // oss[0] << dateType << setw(field.valueFieldSize()) <<
          string firstDateInd = (firstSaleFromEff) ? FIRST_SALE_FROM_EFF : FIRST_DATE_INDICATOR;
          oss[0] << dateType << setw(DATE_WIDTH) << pickedDate << TWO_BLANKS << firstDateInd
                 << setw(DATE_WIDTH) << FirstSale;
          // wrapped line
          oss[1] << SEVEN_BLANKS << LAST_DATE_INDICATOR << setw(DATE_WIDTH) << LastSale;
        }
      }
    }
  }
  break;

  case 2: // 2 travel dates
  {
    //  First pick out the two Travel dates and types (E,D,R,C)
    string pickedDate1;
    string pickedDate2;
    string dateType1;
    string dateType2;

    bool bRet = pick2TravelDatesAndTypes(
        EffDate, DisDate, RetDate, CmpDate, pickedDate1, pickedDate2, dateType1, dateType2);
    if (!bRet)
    {
      //  TODO: This is an error, need to handle it here.
    }

    if (numSales == 0) // No sales dates
    {
      if (travelSame == Same2) // Travel dates are the same
      {
        // Element 1  - 2 Travel that are the same, 0 sales dates
        oss[0] << dateType1 << dateType2 << setw(DATE_WIDTH) << pickedDate1;
      }
      else
      {
        // Element 2  - 2 Travel that are the different, 0 sales
        //              dates
        oss[0] << dateType1 << setw(DATE_WIDTH) << pickedDate1 << TWO_BLANKS << dateType2
               << setw(DATE_WIDTH) << pickedDate2;
      }
    }
    else if (numSales == 1) // 1 sale date
    {
      // Pick the sales date and type
      string salesDate;
      string salesType;
      if (FirstSale.length() == 0)
      {
        salesDate = LastSale;
        salesType = LAST_DATE_INDICATOR;
      }
      else
      {
        salesDate = FirstSale;
        salesType = (firstSaleFromEff) ? FIRST_SALE_FROM_EFF : FIRST_DATE_INDICATOR;
      }

      if (travelSame == Same2) // Travel dates are the same
      {
        // Element 11 & 12 - 2 Travel that are the same, 1 sales
        //                   dates
        oss[0] << dateType1 << dateType2 << setw(DATE_WIDTH) << pickedDate1 << ONE_BLANK
               << salesType << setw(DATE_WIDTH) << salesDate;
      }
      else if (pickedDate1 == salesDate)
      {
        // Element 13 - 2 Travel that are different, 1 sales and
        //              sames as 1 travel dates
        oss[0] << dateType2 << setw(DATE_WIDTH) << pickedDate2 << ONE_BLANK << dateType1
               << salesType << setw(DATE_WIDTH) << salesDate;
      }
      else if (pickedDate2 == salesDate)
      {
        // Element 13 - 2 Travel that are different, 1 sales and
        //              sames as 1 travel dates
        oss[0] << dateType1 << setw(DATE_WIDTH) << pickedDate1 << ONE_BLANK << dateType2
               << salesType << setw(DATE_WIDTH) << salesDate;
      }
      else
      {
        // Element 14 - 2 Travel, 1 sales dates that all are not
        //              equal.  This requires two lines
        oss[0] << dateType1 << setw(DATE_WIDTH) << pickedDate1 << TWO_BLANKS << salesType
               << setw(DATE_WIDTH) << salesDate;

        // wrapped line
        oss[1] << dateType2 << setw(DATE_WIDTH) << pickedDate2;
      }
    }
    else if (numSales == 2) // 2 sale dates
    {
      // Element 15 & 16  - 2 Travel, 2 sales dates
      //              This requires two lines
      string firstDateInd = (firstSaleFromEff) ? FIRST_SALE_FROM_EFF : FIRST_DATE_INDICATOR;
      oss[0] << dateType1 << setw(DATE_WIDTH) << pickedDate1 << TWO_BLANKS << firstDateInd
             << setw(DATE_WIDTH) << FirstSale;

      // wrapped line
      oss[1] << dateType2 << setw(DATE_WIDTH) << pickedDate2 << TWO_BLANKS << LAST_DATE_INDICATOR
             << setw(DATE_WIDTH) << LastSale;
    }
  }
  break;

  case 3: // 3 travel dates
  {
    string pickedDate1 = EffDate;
    string pickedDate2 = DisDate;
    string pickedDate3;
    string dateType1 = EFF_DATE_INDICATOR;
    string dateType2 = DIS_DATE_INDICATOR;
    string dateType3;

    // Choose the 3rd date for later use.
    if (CmpDate.length() != 0)
    {
      pickedDate3 = CmpDate;
      dateType3 = CMP_DATE_INDICATOR;
    }
    else
    {
      pickedDate3 = RetDate;
      dateType3 = RET_DATE_INDICATOR;
    }

    if (numSales == 0) // No sales dates
    {
      if (travelSame == Same3) // All 3 Travel dates are the same
      {
        // Element 21  - 2 Travel that are the same, 0 sales dates
        oss[0] << EFF_DATE_INDICATOR << DIS_DATE_INDICATOR << dateType3 << setw(DATE_WIDTH)
               << pickedDate3;
      }
      else if (travelSame == Same2) // 2 of 3 travel dates are the same
      {
        // Element 21  - 3 Travel, 2 are same, 0 sales dates
        //              This requires two lines.
        // First order the string to be used.
        order2SameOutOf3(pickedDate1, pickedDate2, pickedDate3, dateType1, dateType2, dateType3);

        oss[0] << dateType1 << dateType2 << setw(DATE_WIDTH) << pickedDate1;

        // wrapped line
        oss[1] << dateType3 << setw(DATE_WIDTH) << pickedDate3;
      }
      else // all 3 travel dates are different
      {
        // Element 21  - 3 Travel that are all different, 0 sales dates
        oss[0] << EFF_DATE_INDICATOR << setw(DATE_WIDTH) << EffDate << TWO_BLANKS
               << DIS_DATE_INDICATOR << setw(DATE_WIDTH) << DisDate;

        // wrapped line
        oss[1] << dateType3 << setw(DATE_WIDTH) << pickedDate3;
      }
    }
    else if (numSales == 1) // 1 sale date
    {
      // Pick the sales date and type
      string salesDate;
      string salesType;
      if (FirstSale.length() == 0)
      {
        salesDate = LastSale;
        salesType = LAST_DATE_INDICATOR;
      }
      else
      {
        salesDate = FirstSale;
        salesType = (firstSaleFromEff) ? FIRST_SALE_FROM_EFF : FIRST_DATE_INDICATOR;
      }

      if (travelSame == Same3) // All 3 Travel dates are the same
      {
        // Element 22  - 3 Travel that are the same, 1 sales dates
        //              This requires two lines.
        oss[0] << EFF_DATE_INDICATOR << DIS_DATE_INDICATOR << setw(DATE_WIDTH) << EffDate
               << ONE_BLANK << salesType << setw(DATE_WIDTH) << salesDate;

        // wrapped line
        oss[1] << dateType3 << setw(DATE_WIDTH) << pickedDate3;
      }
      else if (travelSame == Same2) // 2 of 3 travel dates are the same
      {
        // Element 17 & 18  - 3 Travel, 2 that are the same, 1
        //                    sales dates
        //                    This requires two lines.
        // First order the string to be used.
        order2SameOutOf3(pickedDate1, pickedDate2, pickedDate3, dateType1, dateType2, dateType3);

        oss[0] << dateType1 << dateType2 << setw(DATE_WIDTH) << pickedDate1 << ONE_BLANK
               << salesType << setw(DATE_WIDTH) << salesDate;

        // wrapped line
        oss[1] << dateType3 << setw(DATE_WIDTH) << pickedDate3;
      }
      else if (EffDate == salesDate)
      {
        // Element ?? - 3 Travel dates that are different, 1 sales date
        //              same as 1 travel date (effective date)
        oss[0] << DIS_DATE_INDICATOR << setw(DATE_WIDTH) << DisDate << ONE_BLANK
               << EFF_DATE_INDICATOR << salesType << setw(DATE_WIDTH) << salesDate;

        // wrapped line
        oss[1] << dateType3 << setw(DATE_WIDTH) << pickedDate3;
      }
      else if (DisDate == salesDate)
      {
        // Element ?? - 3 Travel dates that are different, 1 sales date
        //              same as 1 travel date (discontinue date)
        oss[0] << EFF_DATE_INDICATOR << setw(DATE_WIDTH) << EffDate << ONE_BLANK
               << DIS_DATE_INDICATOR << salesType << setw(DATE_WIDTH) << salesDate;

        // wrapped line
        oss[1] << dateType3 << setw(DATE_WIDTH) << pickedDate3;
      }
      else if (pickedDate3 == salesDate)
      {
        // Element ?? - 3 Travel dates that are different, 1 sales date
        //              same as 1 travel date (completion/return date)
        oss[0] << EFF_DATE_INDICATOR << setw(DATE_WIDTH) << EffDate << ONE_BLANK << dateType3
               << salesType << setw(DATE_WIDTH) << salesDate;

        // wrapped line
        oss[1] << DIS_DATE_INDICATOR << setw(DATE_WIDTH) << DisDate;
      }
      else
      {
        // Element ?? - 3 Travel that are different, 1 sales date
        oss[0] << EFF_DATE_INDICATOR << setw(DATE_WIDTH) << EffDate << TWO_BLANKS << salesType
               << setw(DATE_WIDTH) << salesDate;
        // wrapped line
        oss[1] << DIS_DATE_INDICATOR << setw(DATE_WIDTH) << DisDate << TWO_BLANKS << dateType3
               << setw(DATE_WIDTH) << pickedDate3;
      }
    }
    else if (numSales == 2) // 2 sale dates
    {
      if (travelSame == Same2) // 2 of 3 travel dates are the same
      {
        // Element 19 & 20  - 3 Travel, 2 of them the same, 2 sales
        //                    dates
        //              This requires two lines
        order2SameOutOf3(pickedDate1, pickedDate2, pickedDate3, dateType1, dateType2, dateType3);

        string firstDateInd = (firstSaleFromEff) ? FIRST_SALE_FROM_EFF : FIRST_DATE_INDICATOR;
        oss[0] << dateType1 << dateType2 << setw(DATE_WIDTH) << pickedDate1 << ONE_BLANK
               << firstDateInd << setw(DATE_WIDTH) << FirstSale;

        // wrapped line
        oss[1] << dateType3 << setw(DATE_WIDTH) << pickedDate3 << TWO_BLANKS << LAST_DATE_INDICATOR
               << setw(DATE_WIDTH) << LastSale;
      }
      else
      {
        // Element ?? - 3 Travel that are different, 2 sales date
        oss[0] << EFF_DATE_INDICATOR << setw(DATE_WIDTH) << EffDate << TWO_BLANKS
               << FIRST_DATE_INDICATOR << setw(DATE_WIDTH) << FirstSale;

        // wrapped line
        oss[1] << DIS_DATE_INDICATOR << setw(DATE_WIDTH) << DisDate << TWO_BLANKS
               << LAST_DATE_INDICATOR << setw(DATE_WIDTH) << LastSale;

        // wrapped line
        //                    oss[2] << dateType3 << setw(DATE_WIDTH) << pickedDate3;
      }
    }
  }
  break;

  default:
    // Need error handling here
    break;
  }
  if (!oss[0].str().empty())
    field.strValue() = oss[0].str();
  if (!oss[1].str().empty())
    field2.strValue() = oss[1].str();
}
}
} // tse namespace
