
// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2008
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "Taxes/Pfc/PfcDisplayBuilderHelp.h"

using namespace tse;

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderHelp::PfcDisplayBuilderHelp
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayBuilderHelp::PfcDisplayBuilderHelp(TaxTrx* trx, PfcDisplayData* data)
  : PfcDisplayBuilder(trx, data)
{
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderHelp::~PfcDisplayBuilderHelp
//
// Description:  Destructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayBuilderHelp::~PfcDisplayBuilderHelp() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayBuilderHelp::buildBody
//
// Description:  PXCHELP entry
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayBuilderHelp::buildBody()
{
  return ("PASSENGER FACILITY CHARGE KNOWN AS PFC IS A FEE IMPOSED\n"
          "BY AN AIRPORT ON ENPLANING PASSENGERS TO FINANCE AIRPORT\n"
          "RELATED PROJECTS AND APPLICABLE TO AIRPORT DEPARTURES\n"
          " \n"
          "PXC FORMATS\n"
          " \n"
          "PXC* -\n"
          "    DISPLAY OF ALL AIRPORTS CHARGING A PFC FEE\n"
          " \n"
          "PXC*XXX -\n"
          "    SPECIFIC AIRPORT CODE FOR PFC INFORMATION\n"
          " \n"
          "PXC*XXXYYYZZZ -\n"
          "    MAXIMUM OF 18 AIRPORT CODES ALLOWED\n"
          " \n"
          "PXC*I -\n"
          "    DISPLAY APPLICABLE PFC AIRPORTS FROM ITINERARY\n"
          " \n"
          "PXC*SN/N/N/N, PXC*SN-N -\n"
          "    PFCS APPLICABLE BY SEGMENT SELECTION OR RANGE\n"
          " \n"
          "PXE* -\n"
          "    AIRPORTS WITH ESSENTIAL AIR SERVICE /EAS/ EXEMPTIONS\n"
          " \n"
          "PXE*XXX -\n"
          "    ESSENTIAL AIR SERVICE /EAS/ BY AIRPORT CODE\n"
          " \n"
          "PXM* -\n"
          "    DISPLAY PFC MULTI-AIRPORT CITY CODE TABLE\n"
          " \n"
          "PXM*XXX -\n"
          "    PFC MULTI-AIRPORT CITY CODE TABLE FOR SPECIFIC AIRPORT\n"
          " \n"
          "PXT* -\n"
          "    CARRIER OPTION FOR PFC COLLECTION METHOD\n"
          " \n"
          "PXT*CC -\n"
          "    SPECIFIC CARRIER OPTION FOR PFC COLLECTION METHOD\n"
          " \n"
          "PXQ* -\n"
          "    DISPLAY EQUIPMENT TYPES EXEMPTED FROM PFC\n"
          " \n"
          "PXA* -\n"
          "    DISPLAY CARRIER ABSORBTION FOR AIRPORT AND CARRIER\n"
          " \n"
          "PXA*XXX -\n"
          "    SPECIFC AIRPORT ABSORPTION\n"
          "    REFLECTS BASE FARE - US TRANSPORTATION TAX PERCENTAGE\n"
          " \n"
          "PXA*XXX/CC  OR PXA*XXX/CC/CC -\n"
          "    SPECIFIC AIRPORT ABSORPTION BY CARRIER - MAX 2 CARRIERS\n"
          "    REFLECTS BASE FARE - US TRANSPORTATION TAX PERCENTAGE\n"
          " \n"
          "PXA*I -\n"
          "    DISPLAY CARRIER ABSORBTION FROM ITINERARY\n"
          " \n"
          "PXA*LINE NUMBER -\n"
          "    LINE NUMBER DISPLAY FROM PRIOR PXA* ENTRY LIST TO DISPLAY\n"
          "    SPECIFIC ABSORPTION AIRPORT AND CARRRIER\n"
          " \n"
          "PXA*XXX/CC-.PCT -\n"
          "    AIRPORT/CARRIER ABSORPTION USER PCT PREFERENCE\n"
          "    EXAMPLE: PXA*ABE/US-.067\n"
          " \n"
          "HISTORICAL ENTRIES - /DATE LIMIT 2 YEARS FROM CURRENT DATE/ -\n"
          " \n"
          "PXC*DDMMMYY -\n"
          "    ALL AIRPORTS CHARGING A PFC FEE ON SPECIFIED DATE\n"
          " \n"
          "PXC*XXXDDMMMYY -\n"
          "    PFC FEE BY AIRPORT CODE ON SPECIFIED DATE\n"
          " \n"
          "PXE*DDMMMYY -\n"
          "    AIRPORTS WITH ESSENTIAL AIR SERVICE /EAS/ EXEMPTIONS\n"
          "    FOR SPECIFIED DATE\n"
          " \n"
          "PXE*XXXDDMMMYY -\n"
          "    ESSENTIAL AIR SERVICE /EAS/ BY AIRPORT CODE\n"
          "    FOR SPECIFIED DATE\n"
          " \n"
          "PXM*DDMMMYY -\n"
          "    DISPLAY PFC MULTI-AIRPORT CITY CODE TABLE\n"
          "    FOR SPECIFIED DATE\n"
          " \n"
          "PXM*XXXDDMMMYY -\n"
          "    PFC MULTI-AIRPORT CITY CODE TABLE FOR SPECIFIC AIRPORT\n"
          "    FOR SPECIFIED DATE\n"
          " \n"
          "PXT*DDMMMYY -\n"
          "    CARRIER OPTION FOR PFC COLLECTION METHOD\n"
          "    FOR SPECIFIED DATE\n"
          " \n"
          "PXT*CCDDMMMYY -\n"
          "    SPECIFIC CARRIER OPTION FOR PFC COLLECTION METHOD\n"
          "    FOR SPECIFIED DATE\n"
          " \n"
          "PXQ*DDMMMYY -\n"
          "    DISPLAY EQUIPMENT TYPES EXEMPTED FROM PFC\n"
          "    FOR SPECIFIED DATE\n"
          " \n"
          "PXA*XXX/CCDDMMMYY -\n"
          "    SPECIFIC AIRPORT ABSORPTION FOR SPECIFIED CARRIER\n"
          "    FOR SPECIFIED DATE\n"
          " \n"
          "PXH*XXX -\n"
          "    HISTORICAL RECORD CREATE DATE\n"
          "    FOR SPECIFIC AIRPORT PXC RECORDS \n");
}
