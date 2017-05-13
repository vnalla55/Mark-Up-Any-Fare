//-------------------------------------------------------------------
//
//  File:        QualifyFltAppRuleData.h
//  Created:     Nov2, 2004
//
//  Description : Save rec 2 if quality cat 4 exist.
//
//  Updates:
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
//-------------------------------------------------------------------

#pragma once

#include "DBAccess/Record2Types.h"

namespace tse
{

class QualifyFltAppRuleData final
{
public:
  const CategoryRuleInfo* categoryRuleInfo() const { return _categoryRuleInfo; }
  const CategoryRuleInfo*& categoryRuleInfo() { return _categoryRuleInfo; }

private:
  const CategoryRuleInfo* _categoryRuleInfo = nullptr; // record 2
};

} // tse namespace

