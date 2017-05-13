//-------------------------------------------------------------------
//
//  File:        TravelTicketFilter.h
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

// Travelticket.h: interface for the travelticket class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

namespace tse
{
class FareDisplayInfo;
class Field;

namespace TravelTicketFilter
{
void
formatData(FareDisplayInfo& fareDisplayInfo, Field& field, Field& field2);
}
} // namespace tse
