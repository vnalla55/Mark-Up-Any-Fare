//-------------------------------------------------------------------
//
//  File:        AdvancePurchaseFilter.h
//  Created:     February 14, 2005
//  Authors:     Doug Batchelor/LeAnn Perez
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
class FareDisplayInfo;
class Field;

namespace AdvancePurchaseFilter
{
void
formatData(FareDisplayInfo& fareDisplayInfo, Field& field);
}
} // namespace tse
