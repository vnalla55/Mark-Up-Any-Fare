//-------------------------------------------------------------------
//
//  File:        MinMaxFilter.h
//  Authors:     LeAnn Perez
//  Created:     February 14, 2005
//  Description: A class to handle the formatting
//               associated with the MIN-MAX column
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
#pragma once
namespace tse
{
class FareDisplayInfo;
class Field;

namespace MinMaxFilter
{
void
formatData(FareDisplayInfo& fareDisplayInfo, Field& field);
}
} // namespace tse
