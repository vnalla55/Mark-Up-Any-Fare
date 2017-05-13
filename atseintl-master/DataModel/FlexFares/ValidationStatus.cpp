//----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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

#include "DataModel/FlexFares/ValidationStatus.h"

#include "Common/Assert.h"

namespace tse
{
namespace flexFares
{

template <>
void
ValidationStatus::setStatus<CORP_IDS>(ConstVStatus<CORP_IDS>::Ref valid)
{
  TSE_ASSERT(false);
}

template <>
void
ValidationStatus::setStatus<ACC_CODES>(ConstVStatus<ACC_CODES>::Ref valid)
{
  TSE_ASSERT(false);
}

template <>
void
ValidationStatus::updateAttribute<NO_ADVANCE_PURCHASE>(const ValidationStatusPtr& source)
{
  updateValidationResult<NO_ADVANCE_PURCHASE>(source->getStatusForAttribute<NO_ADVANCE_PURCHASE>());
}

template <>
void
ValidationStatus::updateAttribute<NO_PENALTIES>(const ValidationStatusPtr& source)
{
  updateValidationResult<NO_PENALTIES>(source->getStatusForAttribute<NO_PENALTIES>());
}

template <>
void
ValidationStatus::updateAttribute<NO_MIN_MAX_STAY>(const ValidationStatusPtr& source)
{
  updateValidationResult<NO_MIN_MAX_STAY>(source->getStatusForAttribute<NO_MIN_MAX_STAY>());
}

template <>
void
ValidationStatus::updateAttribute<CORP_IDS>(const ValidationStatusPtr& source)
{
  updateCorpId(source->getStatusForAttribute<CORP_IDS>());
}

template <>
void
ValidationStatus::updateAttribute<ACC_CODES>(const ValidationStatusPtr& source)
{
  updateAccCode(source->getStatusForAttribute<ACC_CODES>());
}

} // flexFares
} // tse
