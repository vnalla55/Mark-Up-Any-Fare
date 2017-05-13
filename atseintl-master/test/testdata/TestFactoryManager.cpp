/**
 * This class manages the factories themselves; all factories register themselves
 * with the factory manager. Global data cleanup can occur from here.
 *
 * Note that there is a circular reference between TestFactoryManager and the
 * TestFactoryInstanceMgrBase. We break this physically with a forward declaration
 * and actual use of the TestFactoryInstanceMgrBase in our own cpp.
 **/

#include "test/testdata/TestFactoryManager.h"
#include "test/testdata/TestFactoryInstanceMgrBase.h"

/* public static */
TestFactoryManager*
TestFactoryManager::instance()
{
  static TestFactoryManager instance_;
  return &instance_;
}

/* public */
void
TestFactoryManager::enroll(TestFactoryInstanceMgrBase* factoryInstancePtr)
{
  factoryInstanceMgrs_.push_back(factoryInstancePtr);
}

/* public */
void
TestFactoryManager::destroyAll()
{
  std::vector<TestFactoryInstanceMgrBase*>::const_iterator iter = factoryInstanceMgrs_.begin(),
                                                           end = factoryInstanceMgrs_.end();
  for (; iter != end; ++iter)
  {
    (*iter)->destroyAll();
  }
}
