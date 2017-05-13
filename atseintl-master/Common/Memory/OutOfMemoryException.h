/*---------------------------------------------------------------------------
 *
 *  Copyright Sabre 2015
 *    The copyright to the computer program(s) herein
 *    is the property of Sabre.
 *    The program(s) may be used and/or copied only with
 *    the written permission of Sabre or in accordance
 *    with the terms and conditions stipulated in the
 *    agreement/contract under which the program(s)
 *    have been supplied.
 *
 *-------------------------------------------------------------------------*/
#pragma once

#include "Common/ErrorResponseException.h"

namespace tse
{
namespace Memory
{
class OutOfMemoryException : public ErrorResponseException
{
public:
  OutOfMemoryException() : ErrorResponseException(MAX_NUMBER_COMBOS_EXCEEDED) {}
};
}
}
