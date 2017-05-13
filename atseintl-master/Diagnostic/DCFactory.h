//----------------------------------------------------------------------------
//  File:         DCFactory.h
//  Description:  Parameterized Factory Method for creating specific DiagCollector
//                object.
//  Authors:      Mohammad Hossan
//  Created:      Dec 2003
//
//  Updates:
//          date - initials - description.
//
//  Copyright Sabre 2003
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

#include "Common/DCFactoryBase.h"
#include "DataModel/Trx.h"
#include "Diagnostic/DiagCollector.h"
#include "Diagnostic/Diagnostic.h"

#include <pthread.h>

#include <boost/unordered_map.hpp>

#include <map>

namespace tse
{
class Diagnostic;
class DiagCollector;

class DiagCollectorCreatorI
{
public:
  virtual DiagCollector* createDiagCollector(Trx& trx, Diagnostic& root, DiagnosticTypes diagType) = 0;
  virtual ~DiagCollectorCreatorI() = default;
};

template <class DiagCollectorT>
class DiagCollectorCreator : public DiagCollectorCreatorI
{
public:
  DiagCollector* createDiagCollector(Trx& trx, Diagnostic& root, DiagnosticTypes diagType) override
  {
    DiagCollectorT* dc;
    trx.dataHandle().get(dc);
    dc->rootDiag() = &root;
    dc->trx() = &trx;
    dc->diagnosticType() = diagType;
    dc->initParam(root);
    return dc;
  }
};

class DCFactory : public DCFactoryBase
{
public:
  friend class DCFactoryTest;

  static DCFactory* instance();

  DiagCollector* create(Trx& trx);

  bool createThreadDiag(Trx& trx, const pthread_t pid);
  bool endThreadDiag(Trx& trx, const pthread_t pid);

  bool createThreadDiag(Trx& trx, const pthread_t pid, const pthread_t ppid) override;
  bool endThreadDiag(Trx& trx, const pthread_t pid, const pthread_t ppid) override;

  Diagnostic& threadDiag(Trx& trx, const pthread_t pid);
  DiagCollector* thrDisabledDC(Trx& trx);

protected:
  DCFactory();
  virtual ~DCFactory() {}

  static boost::mutex _mutex;

  using DiagnosticCreatorMap =
      boost::unordered_map<DiagnosticTypes, std::shared_ptr<DiagCollectorCreatorI>>;

  DiagnosticCreatorMap _diagCreatorMap;

  void initializeDiagCollectorCreators();

  template <class T>
  std::shared_ptr<DiagCollectorCreatorI> createSharedPtr(T* t)
  {
    return std::shared_ptr<DiagCollectorCreatorI>(t);
  }

  virtual DiagCollector* threadCreate(Trx& trx, Diagnostic& diag);

private:
  static DCFactory* _instance;

  DCFactory(const DCFactory&) = delete;
  DCFactory& operator=(const DCFactory&) = delete;

  class ThreadDiagInfo
  {
  public:
    Diagnostic* diag = nullptr;
  };

  using PidDiagMap = std::map<pthread_t, ThreadDiagInfo*>;
  using PidDiagMapCI = PidDiagMap::const_iterator;

  static boost::mutex _mutexThreadDiag;
  static PidDiagMap _pidDiagMap;

  ThreadDiagInfo* threadDiagInfo(const pthread_t pid);
};
} // namespace tse
