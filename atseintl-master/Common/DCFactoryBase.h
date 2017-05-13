//----------------------------------------------------------------------------
//  File:         DCFactoryBase.h
//  Description:  Base class for the DCFactory which resides in Diagnostics.
//                This object exists so as to avoid a link time coupling.
//  Authors:      Mike Lillis
//  Created:      May 2005
//
//  Updates:
//
//  Copyright Sabre 2005
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

#pragma once

#include <pthread.h>

namespace tse
{

class Trx;

class DCFactoryBase
{
public:
  static DCFactoryBase* baseInstance();

  //@TODO will be removed, once the transition is done
  static void initialize(DCFactoryBase* baseInstance)
  {
    _baseInstance = baseInstance;
  };

  virtual bool createThreadDiag(Trx& trx, const pthread_t pid, const pthread_t ppid)
  {
    return _baseInstance ? _baseInstance->createThreadDiag(trx, pid, ppid) : false;
  };
  virtual bool endThreadDiag(Trx& trx, const pthread_t pid, const pthread_t ppid)
  {
    return _baseInstance ? _baseInstance->endThreadDiag(trx, pid, ppid) : false;
  };

protected:
  DCFactoryBase() {};
  virtual ~DCFactoryBase()
  {
    _baseInstance = nullptr;
  };

private:
  static DCFactoryBase* _baseInstance;
};

} // namespace tse

