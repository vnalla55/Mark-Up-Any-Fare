//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include <vector>
#include <fstream>
#include <list>
#include <string>
#include <iostream>
#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>

#include "DBAccess/Flattenizable.h"
#include "Common/Code.h"
#include "Common/DateTime.h"

namespace tse
{
class FlattenizerTestObject : public Flattenizable
{
private:
  std::string _str;
  int _i;
  double _d;
  char _c;
  bool _b;
  Code<7> _code;
  DateTime _dte;
  DateTime* _dp;
  DateTime* _dpn;

  FlattenizerTestObject(const FlattenizerTestObject& rhs);
  FlattenizerTestObject& operator=(const FlattenizerTestObject& rhs);

public:
  FlattenizerTestObject()
    : _str(""),
      _i(0),
      _d(0.00),
      _c(' '),
      _b(false),
      _code(""),
      _dte(0),
      _dp(NULL),
      _dpn(new DateTime(time(NULL)))
  {
  }

  virtual ~FlattenizerTestObject() { killKids(); }

  void killKids()
  {
    if (_dp != NULL)
    {
      delete _dp;
      _dp = NULL;
    }

    if (_dpn != NULL)
    {
      delete _dpn;
      _dpn = NULL;
    }
  }

  bool operator==(const FlattenizerTestObject& rhs)
  {
    bool eq((_str == rhs._str) && (_i == rhs._i) && (_d == rhs._d) && (_c == rhs._c) &&
            (_b == rhs._b) && (_code == rhs._code) && (_dte == rhs._dte));

    if (eq)
    {
      if (_dp == NULL)
      {
        eq = (rhs._dp == NULL);
      }
      else
      {
        eq = (rhs._dp != NULL);
      }
    }

    if (eq)
    {
      if (_dpn == NULL)
      {
        eq = (rhs._dpn == NULL);
      }
      else
      {
        eq = (rhs._dpn != NULL);
      }
    }

    if (eq)
    {
      if (eq && _dp && rhs._dp)
      {
        eq = (*_dp == *(rhs._dp));
      }

      if (eq && _dpn && rhs._dpn)
      {
        eq = (*_dpn == *(rhs._dpn));
      }
    }

    return eq;
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _str);
    FLATTENIZE(archive, _i);
    FLATTENIZE(archive, _d);
    FLATTENIZE(archive, _c);
    FLATTENIZE(archive, _b);
    FLATTENIZE(archive, _code);
    FLATTENIZE(archive, _dte);
    FLATTENIZE(archive, _dp);
    FLATTENIZE(archive, _dpn);
  }

  void dummyData()
  {
    killKids();

    _str = "HELLO";
    _i = 99;
    _d = 123.45;
    _c = 'X';
    _b = true;
    _code = "ABCDEFG";
    _dte = time(NULL);
    _dp = new DateTime(time(NULL));
    _dpn = NULL;
  }

  void dump()
  {
    std::cout << "  _str  = " << _str << std::endl;
    std::cout << "  _i    = " << _i << std::endl;
    std::cout << "  _d    = " << _d << std::endl;
    std::cout << "  _c    = " << _c << std::endl;
    std::cout << "  _b    = " << (_b ? "true" : "false") << std::endl;
    std::cout << "  _code = " << _code << std::endl;
    std::cout << "  _dte  = " << _dte << std::endl;

    if (_dp)
    {
      std::cout << "  _dp   = " << *_dp << std::endl;
    }
    else
    {
      std::cout << "  _dp   = null" << std::endl;
    }

    if (_dpn)
    {
      std::cout << "  _dpn  = " << *_dpn << std::endl;
    }
    else
    {
      std::cout << "  _dpn  = null" << std::endl;
    }

    std::cout << std::endl;
  }
};

class SerializationTestFlattenizer : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SerializationTestFlattenizer);
  CPPUNIT_TEST(testFlattenizer);
  CPPUNIT_TEST_SUITE_END();

public:
  void testFlattenizer()
  {
    FlattenizerTestObject original;
    original.dummyData();

    Flattenizable::Archive archive;

    std::string ignore;
    FLATTENIZE_SAVE(archive, original, 0, ignore, ignore);

    sleep(1);
    FlattenizerTestObject phoenix;

    FLATTENIZE_RESTORE(archive, phoenix, NULL, 0);
    CPPUNIT_ASSERT(original == phoenix);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(SerializationTestFlattenizer);
} // namespace tse
