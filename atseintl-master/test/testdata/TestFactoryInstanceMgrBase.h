#ifndef TEST_FACTORY_INSTANCE_MGR_BASE_H
#define TEST_FACTORY_INSTANCE_MGR_BASE_H

/**
 * This is the base class of all instance managers. It's primary purpose is to define
 * a set of methods that all factory instance managers can adhere to (independent of
 * the templatized type). This will aid in simplifying the data cleanup.
 **/

#include <string>
#include <map>

#include "test/testdata/TestFactoryManager.h"

class TestFactoryInstanceMgrBase
{
public:
  int instanceNumber() { return instanceNum_; }

  void incrementInstanceNumber() { instanceNum_++; }

  virtual void destroyAll() = 0;

protected:
  TestFactoryInstanceMgrBase()
  {
    resetInstanceNum();
    TestFactoryManager::instance()->enroll(this);
  }

  virtual ~TestFactoryInstanceMgrBase() {}

  void resetInstanceNum() { instanceNum_ = -1; }

private:
  TestFactoryInstanceMgrBase(const TestFactoryInstanceMgrBase& rhs);
  bool operator=(const TestFactoryInstanceMgrBase& rhs);
  bool operator==(const TestFactoryInstanceMgrBase& rhs);

  int instanceNum_;
};

#endif
