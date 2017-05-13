#ifndef TEST_FACTORY_MANAGER_H
#define TEST_FACTORY_MANAGER_H

/**
 * This class manages the factories themselves; all factories register themselves
 * with the factory manager. Global data cleanup can occur from here.
 *
 * Note that there is a circular reference between TestFactoryManager and the
 * TestFactoryInstanceMgrBase. We break this physically with a forward declaration
 * and actual use of the TestFactoryInstanceMgrBase in our own cpp.
 **/

#include <vector>

class TestFactoryInstanceMgrBase;

class TestFactoryManager
{
public:
  static TestFactoryManager* instance();

  void enroll(TestFactoryInstanceMgrBase* factoryInstancePtr);
  void destroyAll();

private:
  TestFactoryManager() {}

  TestFactoryManager(const TestFactoryManager& rhs);
  bool operator=(const TestFactoryManager& rhs);
  bool operator==(const TestFactoryManager& rhs);

  std::vector<TestFactoryInstanceMgrBase*> factoryInstanceMgrs_;
};

#endif
