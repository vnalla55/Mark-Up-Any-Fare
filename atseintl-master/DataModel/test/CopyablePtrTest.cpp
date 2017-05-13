// ----------------------------------------------------------------
//
//   Copyright Sabre 2016
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#include "DataModel/CopyablePtr.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace tse
{
using namespace ::testing;

class ConstructionDestructionMock
{
public:
  MOCK_METHOD0(constructor, void());
  MOCK_METHOD0(copyConstructor, void());
  MOCK_METHOD0(moveConstructor, void());
  MOCK_METHOD0(moveAssignmentOperator, void());
  MOCK_METHOD0(copyAssignmentOperator, void());
  MOCK_METHOD0(destructor, void());
};

ConstructionDestructionMock* globalMock = nullptr;

class MockDelegator
{
public:
  MockDelegator() { globalMock->constructor(); }
  MockDelegator(int value) : _value(value) { globalMock->constructor(); }
  MockDelegator(const MockDelegator& other)
  {
    globalMock->copyConstructor();
    _value = other._value;
  }
  MockDelegator(MockDelegator&& other) { globalMock->moveConstructor(); };
  MockDelegator& operator=(MockDelegator&& other)
  {
    globalMock->moveAssignmentOperator();
    _value = std::move(other._value);
    return *this;
  }
  MockDelegator& operator=(const MockDelegator& other)
  {
    globalMock->copyAssignmentOperator();
    _value = other._value;
    return *this;
  }
  ~MockDelegator() { globalMock->destructor(); }
  int _value = 1;
};

// Class which use all default copy/move constructors/operators=
class CopyablePtrContainer
{
public:
  void setCopyPtr(MockDelegator* ptr) { copyablePtr.reset(ptr); }
  MockDelegator* getPtr() { return copyablePtr.get(); }

private:
  CopyablePtr<MockDelegator> copyablePtr;
};

class CopyablePtrTest : public Test
{
public:
  void SetUp() override { globalMock = new ConstructionDestructionMock(); }
  void TearDown() override { delete globalMock; }
};

TEST_F(CopyablePtrTest, emptyCopyablePtrConstruction)
{
  EXPECT_CALL(*globalMock, constructor()).Times(0);
  EXPECT_CALL(*globalMock, destructor()).Times(0);
  CopyablePtr<ConstructionDestructionMock> copyablePtr;
}

TEST_F(CopyablePtrTest, ConstructionDestruction)
{
  EXPECT_CALL(*globalMock, constructor()).Times(1);
  EXPECT_CALL(*globalMock, destructor()).Times(1);
  CopyablePtr<MockDelegator> copyablePtr(new MockDelegator());
}

TEST_F(CopyablePtrTest, CopyConstructor)
{
  EXPECT_CALL(*globalMock, constructor()).Times(1);
  EXPECT_CALL(*globalMock, destructor()).Times(2);
  EXPECT_CALL(*globalMock, copyConstructor()).Times(1);

  MockDelegator* mockDelegator = new MockDelegator();
  mockDelegator->_value = 70;

  CopyablePtr<MockDelegator> copyablePtr1(mockDelegator);
  CopyablePtr<MockDelegator> copyablePtr2(copyablePtr1);
  ASSERT_EQ(70, copyablePtr2->_value);
}

TEST_F(CopyablePtrTest, MoveConstructor)
{
  EXPECT_CALL(*globalMock, constructor()).Times(1);
  EXPECT_CALL(*globalMock, destructor()).Times(1);

  // is not called copy constructor
  EXPECT_CALL(*globalMock, copyConstructor()).Times(0);

  CopyablePtr<MockDelegator> copyablePtr(CopyablePtr<MockDelegator>(new MockDelegator(90)));

  ASSERT_EQ(90, copyablePtr->_value);
}

TEST_F(CopyablePtrTest, CopyAssignOperator)
{
  EXPECT_CALL(*globalMock, constructor()).Times(1);
  EXPECT_CALL(*globalMock, destructor()).Times(2);
  EXPECT_CALL(*globalMock, copyConstructor()).Times(1);

  MockDelegator* mockDelegator = new MockDelegator();
  mockDelegator->_value = 70;
  CopyablePtr<MockDelegator> copyablePtr1(mockDelegator);
  CopyablePtr<MockDelegator> copyablePtr2;

  copyablePtr2 = copyablePtr1;
  ASSERT_EQ(70, (*copyablePtr2)._value);
}

TEST_F(CopyablePtrTest, CopyAssignOperator_AssignNull)
{
  EXPECT_CALL(*globalMock, constructor()).Times(1);
  EXPECT_CALL(*globalMock, destructor()).Times(1);
  EXPECT_CALL(*globalMock, copyConstructor()).Times(0);

  MockDelegator* mockDelegator = new MockDelegator();
  mockDelegator->_value = 70;
  CopyablePtr<MockDelegator> copyablePtr(mockDelegator);
  CopyablePtr<MockDelegator> copyablePtr2(nullptr);

  copyablePtr = copyablePtr2;
  Mock::VerifyAndClearExpectations(globalMock);

  ASSERT_TRUE(copyablePtr == nullptr);
  EXPECT_CALL(*globalMock, destructor()).Times(0);
}

TEST_F(CopyablePtrTest, MoveAssignOperator)
{
  EXPECT_CALL(*globalMock, constructor()).Times(1);
  EXPECT_CALL(*globalMock, destructor()).Times(1);
  EXPECT_CALL(*globalMock, copyConstructor()).Times(0);
  EXPECT_CALL(*globalMock, copyAssignmentOperator()).Times(0);

  MockDelegator* mockDelegator = new MockDelegator();
  mockDelegator->_value = 70;

  CopyablePtr<MockDelegator> copyablePtr;
  copyablePtr = CopyablePtr<MockDelegator>(mockDelegator);

  ASSERT_EQ(70, copyablePtr->_value);
}

TEST_F(CopyablePtrTest, ContainerCanBeCopied)
{
  EXPECT_CALL(*globalMock, constructor()).Times(1);
  EXPECT_CALL(*globalMock, destructor()).Times(2);
  EXPECT_CALL(*globalMock, copyConstructor()).Times(1);

  MockDelegator* mockDelegator = new MockDelegator();
  mockDelegator->_value = 70;

  CopyablePtrContainer containerFirst;
  containerFirst.setCopyPtr(mockDelegator);
  CopyablePtrContainer containerSecond(containerFirst);

  ASSERT_EQ(70, containerSecond.getPtr()->_value);
}

TEST_F(CopyablePtrTest, ContainerCanBeMoved)
{
  EXPECT_CALL(*globalMock, constructor()).Times(1);
  EXPECT_CALL(*globalMock, destructor()).Times(1);
  EXPECT_CALL(*globalMock, copyConstructor()).Times(0);

  CopyablePtrContainer rvalueContainer;
  rvalueContainer.setCopyPtr(new MockDelegator(70));
  CopyablePtrContainer container(std::move(rvalueContainer));
  ASSERT_EQ(70, container.getPtr()->_value);
}

TEST_F(CopyablePtrTest, ContainerCanBeAssignedCopied)
{
  EXPECT_CALL(*globalMock, constructor()).Times(1);
  EXPECT_CALL(*globalMock, destructor()).Times(2);
  EXPECT_CALL(*globalMock, copyConstructor()).Times(1);

  MockDelegator* mockDelegator = new MockDelegator();
  mockDelegator->_value = 70;

  CopyablePtrContainer containerFirst;
  containerFirst.setCopyPtr(mockDelegator);
  CopyablePtrContainer containerSecond;
  containerSecond = containerFirst;

  ASSERT_EQ(70, containerSecond.getPtr()->_value);
}

TEST_F(CopyablePtrTest, ContainerCanBeAssignedMoved)
{
  EXPECT_CALL(*globalMock, constructor()).Times(1);
  EXPECT_CALL(*globalMock, destructor()).Times(1);
  EXPECT_CALL(*globalMock, copyConstructor()).Times(0);

  CopyablePtrContainer rvalueContainer;
  rvalueContainer.setCopyPtr(new MockDelegator(70));
  CopyablePtrContainer container;

  container = std::move(rvalueContainer);
  ASSERT_EQ(70, container.getPtr()->_value);
}
}
