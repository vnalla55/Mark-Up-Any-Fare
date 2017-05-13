//----------------------------------------------------------------------------
//  File:        Diag405Collector.cpp
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
#include "Diagnostic/Diag405Collector.h"

#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/BookingCodeExceptionSequence.h"

namespace tse
{
// ----------------------------------------------------------------------------
// <PRE>
//
// @function  Diag405Collector::operator <<
//
// Description:  This method will be the override base operator << to handle the
//          BookingCode Exception Data Record Diagnostic Display.
//
// @param  BookingCodeException - Specific  BookingCode Exception Data Information
//
//
// </PRE>
// ----------------------------------------------------------------------------
Diag405Collector&
Diag405Collector::operator << (const BookingCodeExceptionSequence& bceSequence)
{
  // !!!!!!!!!!!!!!!!!!! WARNING !!!!!!!!!!!!!!!!!!!!!!!!!!
  //
  // PLEASE DO NOT REMOVE ANY COMMENTED OUT CODE FROM HERE,
  // IT MAY BE NEEDED IN FUTURE
  //
  // !!!!!!!!!!!!!!!!!!! WARNING !!!!!!!!!!!!!!!!!!!!!!!!!!

  if (!_active)
  {
    std::cerr << "DIAG405COLLECTOR::IN OPERATOR IS NOT ACTIVE \n";

    return *this;
  }

  DiagCollector& dc = (DiagCollector&)*this;

  // BookingCodeExceptionSegmentVectorCI  segmentIter;
  // char posTypeTmp, loc1TypeTmp, loc2TypeTmp;

  dc << "\n \nSEQUENCE NUMBER: " << bceSequence.seqNo() // << "\n"
     // << "   PRIME INDICATOR : " << bceSequence.primeInd()
     // << "   TABLE TYPE      : " << bceSequence.tableType() << "\n"
     << "  IF TAG: " << bceSequence.ifTag() << "  SEGMENT COUNT: " << bceSequence.segCnt() << "\n";

  /*
  const BookingCodeExceptionSegmentVector&  segmentVector = (bceSequence.segmentVector());

  for(segmentIter = segmentVector.begin(); segmentIter != segmentVector.end(); segmentIter++)
    {
      posTypeTmp = (*segmentIter)->posLocType();
      if(posTypeTmp == '\0')
      {
          posTypeTmp = '*';
      }

      loc1TypeTmp = (*segmentIter)->loc1Type();
      if(loc1TypeTmp == '\0')
      {
          loc1TypeTmp = '*';
      }

      loc2TypeTmp = (*segmentIter)->loc2Type();
      if(loc2TypeTmp == '\0')
      {
          loc2TypeTmp = '*';
      }

      dc << "   SEGMENT NUMBER " << (*segmentIter)->segNo()             << "\n"
        << "       VIA CARRIER       : " << (*segmentIter)->viaCarrier()
        << "  PRIME/SECONDARY: " << (*segmentIter)->primarySecondary()<< "\n"
        << "       FLT RANGE APPL    : " << (*segmentIter)->fltRangeAppl()
        << "   FLIGHT1/FLIGHT2: " << (*segmentIter)->flight1() << "/" << (*segmentIter)->flight2()
  << "\n"
        << "       EQUIPMENT TYPE    : " << (*segmentIter)->equipType()
        << "    TRAVEL PORTION : " << (*segmentIter)->tvlPortion()      << "\n"
        << "       TSI               : " << (*segmentIter)->tsi()
        << "   DIRECTIONAL IND: " << (*segmentIter)->directionInd()    << "\n"
        << "       LOCATION 1 TYPE   : " << loc1TypeTmp
        << "   LOCATION 1     : " << (*segmentIter)->loc1()            << "\n"
        << "       LOCATION 2 TYPE   : " << loc2TypeTmp
        << "   LOCATION 2     : " << (*segmentIter)->loc2()            << "\n"
        << "       P.O.S. TSI        : " << (*segmentIter)->posTsi()
        << "   P.O.S. LOCTYPE : " << posTypeTmp                        << "\n"
        << "       P.O.S. LOC        : " << (*segmentIter)->posLoc()
        << "    SOLD IN OUT IND: " << (*segmentIter)->soldInOutInd()    << "\n"
        << "       START DTE MM/DD/YY: " << (*segmentIter)->tvlEffMonth()
                                        << "/"
                        << (*segmentIter)->tvlEffDay()
                        << "/"
                        << (*segmentIter)->tvlEffYear()      << "\n"
        << "       STOP DTE  MM/DD/YY: " << (*segmentIter)->tvlDiscMonth()
                                        << "/"
                        << (*segmentIter)->tvlDiscDay()
                        << "/"
                        << (*segmentIter)->tvlDiscYear()     << "\n"
        << "       TRAVEL START TIME : " << (*segmentIter)->tvlStartTime()    << "\n"
        << "       TRAVEL STOP  TIME : " << (*segmentIter)->tvlEndTime()      << "\n"
        << "       DAYS OF WEEK      : " << (*segmentIter)->daysOfWeek()      << "\n"
        << "       FARECLASS TYPE    : " << (*segmentIter)->fareclassType()
        << "   FARECLASS      : " << (*segmentIter)->fareclass()       << "\n"
        << "       RESTRICTION TAG   : " << (*segmentIter)->restrictionTag()  << "\n"
        << "       BOOKING CODE 1    : " << (*segmentIter)->bookingCode1()
        << "    BOOKING CODE 2 : " << (*segmentIter)->bookingCode2()    << "\n";

    }         // end of Segment loop
  */
  return *this;
}
}
