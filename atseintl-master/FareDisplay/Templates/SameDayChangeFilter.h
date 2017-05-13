//-------------------------------------------------------------------
//
//  File:        SameDayChangeFilter.h
//  Created:     April 7, 2005
//  Authors:     Doug Batchelor
//  Description: This class formats the same day change indicator
//
//  Updates:
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

#pragma once

namespace tse
{
class Field;
class FareDisplayInfo;

namespace SameDayChangeFilter
{
void
formatData(FareDisplayInfo& fareDisplayInfo, Field& field);
}
} // namespace tse
