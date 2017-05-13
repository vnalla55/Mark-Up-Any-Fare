//------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "Common/Memory/LocalManager.h"

#include <vector>

namespace tse
{
class Trx;

namespace Memory
{
class TrxManager : public LocalManager
{
public:
  TrxManager(Trx* trx);
  virtual ~TrxManager();

  size_t getId() const { return _id; }
  void setId(size_t id) { _id = id; }

  virtual void setOutOfMemory() override;

private:
  Trx* _trx = nullptr;
  size_t _id = 0;
};
}
}
