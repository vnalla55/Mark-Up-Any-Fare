#ifndef TESTROW_H
#define TESTROW_H

#include <cppunit/extensions/HelperMacros.h>
#include <string>
#include <map>
#include "Common/DateTime.h"
#include "DBAccess/Row.h"

namespace tse
{
class TestRow : public Row
{
public:
  void set(int index, long long value);
  void set(int index, std::string value);
  void set(int index, DateTime& value);

  // virtual void function implementations for base class Row
  int getInt(int index) const;
  long int getLong(int index) const;
  long long int getLongLong(int index) const;
  const char* getString(int index) const;
  char getChar(int index) const;
  DateTime getDate(int index) const;
  void setNull(int index);
  bool isNull(int index) const;
  // don't use this function anymore. We are trying to eliminate the
  // Date string conversion except for printing.
  const char* getDateAsString(int columnIndex) const { return ""; }

private:
  std::map<int, bool> _nulls;
  std::map<int, long long> _values;
  std::map<int, std::string> _stringValues;
  std::map<int, DateTime> _dateValues;

  template <typename T>
  T find(const std::map<int, T>& values, int index) const
  {
    typename std::map<int, T>::const_iterator it = values.find(index);
    CPPUNIT_ASSERT(it != values.end());
    return it->second;
  }
};
} // namespace tse

#endif
