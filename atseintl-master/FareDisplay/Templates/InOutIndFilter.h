//-------------------------------------------------------------------
//
//  File:        InOutIndFilter.h
//  Created:     September 28, 2006
//  Authors:     Svetlana Tsarkova
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
#include "Common/TsePrimitiveTypes.h"

#include <vector>

namespace tse
{
class Field;
class FareDisplayInfo;
class ElementField;

namespace InOutIndFilter
{
void
formatData(FareDisplayInfo& fareDisplayInfo,
           Field& field1,
           std::vector<ElementField>& fields,
           const Indicator dateFormat = '1');
}
} // namespace tse
