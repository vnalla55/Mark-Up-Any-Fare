//----------------------------------------------------------------------------
//  Copyright Sabre 2009
//
//  The copyright to the computer program(s) herein
//  is the property of Sabre.
//  The program(s) may be used and/or copied only with
//  the written permission of Sabre or in accordance
//  with the terms and conditions stipulated in the
//  agreement/contract under which the program(s)
//  have been supplied.
//
//----------------------------------------------------------------------------

#include "test/include/TestMemHandle.h"

namespace tse
{

void
TestMemHandle::import(TestMemHandle& another)
{
  _deleteList.import(another._deleteList);
}

void
TestMemHandle::clear()
{
  _deleteList.clear();
}

const DeleteList&
TestMemHandle::deleteList() const
{
  return _deleteList;
}
}
