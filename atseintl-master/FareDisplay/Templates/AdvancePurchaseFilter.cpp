//-------------------------------------------------------------------
//
//  File:        AdvancePurchaseFilter
//  Created:     January 31, 2005
//  Authors:     LeAnn Perez
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

#include "FareDisplay/Templates/AdvancePurchaseFilter.h"

#include "FareDisplay/Templates/Field.h"
#include "DataModel/FareDisplayInfo.h"

#include <string>

namespace tse
{
namespace AdvancePurchaseFilter
{
void
formatData(FareDisplayInfo& fareDisplayInfo, Field& field)
{
  std::string s;
  // for those that define a small width (Abacus), use different formatting
  if (field.valueFieldSize() == 2)
    fareDisplayInfo.getAPStringShort(s);
  else
    fareDisplayInfo.getAPString(s);
  field.strValue() = s;
}
}
}
