//----------------------------------------------------------------------------
//  Copyright Sabre 2012
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
#include <list>
#include <stdexcept>
#include <memory>
#include <utility>
#include <vector>

#include <boost/assign/list_of.hpp>

#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/Exception.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>

#include "Common/Thread/TseCallableTrxTask.h"
#include "Common/Thread/TseRunnableExecutor.h"
#include "DataModel/PricingTrx.h"
#include "Diagnostic/DCFactory.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestLogger.h"

namespace tse
{
class TseCallableTrxTask_SequenceIDTest : public CppUnit::TestFixture
{
private:
  CPPUNIT_TEST_SUITE(TseCallableTrxTask_SequenceIDTest);
  CPPUNIT_TEST(testAdvanceTrxTaskHighLevelSeqNum);
  CPPUNIT_SKIP_TEST(testDeterministicExecutionPlan); // Error logger is affected by usage of
  // ConfigurableValue
  CPPUNIT_SKIP_TEST(testIndeterministicExecutionPlan); // Error logger is affected by usage of
  // ConfigurableValue
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memH.create<TestConfigInitializer>();
    _trx = _memH(new PricingTrx);
    DCFactory::instance();

    _asserts = _memH(new PostponedAssertCollection);
  }

  void tearDown() { _memH.clear(); }

  void testAdvanceTrxTaskHighLevelSeqNum()
  {
    CPPUNIT_ASSERT_EQUAL((uint32_t)0, _trx->advanceTrxTaskHighLevelSeqNum());
    CPPUNIT_ASSERT_EQUAL((uint32_t)1, _trx->advanceTrxTaskHighLevelSeqNum());
    CPPUNIT_ASSERT_EQUAL((uint32_t)2, _trx->advanceTrxTaskHighLevelSeqNum());
  }

  void testDeterministicExecutionPlan()
  {
    using boost::assign::list_of;

    TestLogger errLogger(log4cxx::Logger::getRootLogger(), log4cxx::Level::getWarn());

    IndeterministicTask* task1 = createTaskWithSeqID(list_of(0));
    IndeterministicTask* task2 = createTaskWithSeqID(list_of(1));
    IndeterministicTask* task3 = createTaskWithSeqID(list_of(2));

    task1->run();
    task2->run();
    task3->run();

    _asserts->verify();
    CPPUNIT_ASSERT_EQUAL(std::string(), errLogger.str());
  }

  void testIndeterministicExecutionPlan()
  {
    using boost::assign::list_of;

    TestLogger errLogger(log4cxx::Logger::getRootLogger(), log4cxx::Level::getWarn());
    IndeterministicTask::ExecutionPlan plan;

    // task level 1
    plan._expectSelfSeqID = list_of(0);

    // tasks level 2
    plan._subTasks.resize(4);
    plan._subTasks[0]._expectSelfSeqID = list_of(0)(0);
    plan._subTasks[1]._expectSelfSeqID = list_of(0)(1);
    plan._subTasks[2]._expectSelfSeqID = list_of(0)(2);
    plan._subTasks[3]._expectSelfSeqID = list_of(0)(3);

    // tasks level 3
    plan._subTasks[1]._subTasks.resize(3);
    plan._subTasks[1]._subTasks[0]._expectSelfSeqID = list_of(0)(1)(0);
    plan._subTasks[1]._subTasks[1]._expectSelfSeqID = list_of(0)(1)(1);
    plan._subTasks[1]._subTasks[2]._expectSelfSeqID = list_of(0)(1)(2);

    // tasks level 3
    plan._subTasks[3]._subTasks.resize(3);
    plan._subTasks[3]._subTasks[0]._expectSelfSeqID = list_of(0)(3)(0);
    plan._subTasks[3]._subTasks[1]._expectSelfSeqID = list_of(0)(3)(1);
    plan._subTasks[3]._subTasks[2]._expectSelfSeqID = list_of(0)(3)(2);

    // tasks level 4
    plan._subTasks[3]._subTasks[2]._subTasks.resize(2);
    plan._subTasks[3]._subTasks[2]._subTasks[0]._expectSelfSeqID = list_of(0)(3)(2)(0);
    plan._subTasks[3]._subTasks[2]._subTasks[1]._expectSelfSeqID = list_of(0)(3)(2)(1);

    IndeterministicTask task;
    task.init(_trx, plan, _asserts);
    task.run();

    _asserts->verify();
    CPPUNIT_ASSERT_EQUAL(std::string(), errLogger.str());
  }

private:
  typedef TseCallableTrxTask::SequenceID TaskSequenceID;

  class PostponedAssertCollection
  {
  private:
    std::list<std::shared_ptr<CppUnit::Exception>> _asserts;
    boost::mutex _mutex;

  public:
    /**
     * @thread-safe
     */
    void add(const CppUnit::Exception& assertion)
    {
      boost::lock_guard<boost::mutex> g(_mutex);
      std::shared_ptr<CppUnit::Exception> p(assertion.clone());
      _asserts.push_back(p);
    }
    /**
     * Must be run from CppUnit test thread context
     */
    void verify()
    {
      if (!_asserts.empty())
        throw * *_asserts.begin();
    }
  };

  class IndeterministicTask : public TseCallableTrxTask
  {
  public:
    struct ExecutionPlan
    {
      SequenceID _expectSelfSeqID;
      std::vector<ExecutionPlan> _subTasks;

      ExecutionPlan() {}
      ExecutionPlan(const SequenceID& expect) : _expectSelfSeqID(expect) {}
    };

    IndeterministicTask() : _assertCollection(0) {}
    void init(PricingTrx* trx, const ExecutionPlan& plan, PostponedAssertCollection* asserts)
    {
      this->trx(trx);

      _plan = plan;
      _assertCollection = asserts;
    }

  protected:
    /**
     * CppUnit assertions are wrapped here
     *
     * @override
     */
    virtual void performTask()
    {
      try
      {
        performTaskImpl();
      }
      catch (const CppUnit::Exception& assert)
      {
        _assertCollection->add(assert);
      }
      catch (const std::exception& stdex) // convert unexpected exception into assert
      {
        try
        {
          CPPUNIT_ASSERT_NO_THROW(throw);
        }
        catch (const CppUnit::Exception& assert)
        {
          _assertCollection->add(assert);
        }
      }
    }

  private:
    ExecutionPlan _plan;
    PostponedAssertCollection* _assertCollection;

    void performTaskImpl()
    {
      CPPUNIT_ASSERT(currentTask() != 0);

      TseCallableTrxTask::SequenceID expect(_plan._expectSelfSeqID),
          actual(currentTask()->getSequenceID());
      CPPUNIT_ASSERT_EQUAL(expect, actual);

      if (!_plan._subTasks.empty())
        goSubTasks();
    }

    void goSubTasks()
    {
      TseRunnableExecutor taskExecutor(TseThreadingConst::SYNCHRONOUS_TASK);
      for (const ExecutionPlan& plan : _plan._subTasks)
      {
        IndeterministicTask* subTask;
        trx()->dataHandle().get(subTask);
        subTask->init(trx(), plan, _assertCollection);

        taskExecutor.execute(*subTask);
      }
      CPPUNIT_ASSERT_NO_THROW(taskExecutor.wait(/*rewthrowTaskException*/ true));
    }
  };

  PricingTrx* _trx;
  TestMemHandle _memH;
  PostponedAssertCollection* _asserts;

  IndeterministicTask* createTaskWithSeqID(const TaskSequenceID& id)
  {
    IndeterministicTask* task;
    _memH.get(task);
    task->init(_trx, id, _asserts);
    return task;
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TseCallableTrxTask_SequenceIDTest);

} // tse
