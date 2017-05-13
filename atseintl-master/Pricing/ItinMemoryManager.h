/*---------------------------------------------------------------------------
 *  Copyright Sabre 2015
 *    The copyright to the computer program(s) herein
 *    is the property of Sabre.
 *    The program(s) may be used and/or copied only with
 *    the written permission of Sabre or in accordance
 *    with the terms and conditions stipulated in the
 *    agreement/contract under which the program(s)
 *    have been supplied.
 *-------------------------------------------------------------------------*/
#pragma once

#include "Common/Memory/LocalManager.h"
#include "DataModel/Itin.h"

namespace tse
{
class ItinMemoryManager : public Memory::LocalManager
{
public:
  ItinMemoryManager(Memory::CompositeManager& parent, Itin& itin) : _itin(itin)
  {
    parent.registerManager(this);
    _itin.setMemoryManager(this);
  }

  ~ItinMemoryManager()
  {
    _itin.setMemoryManager(nullptr);
    parent()->archiveManager(this);
  }

private:
  Itin& _itin;
};
}

