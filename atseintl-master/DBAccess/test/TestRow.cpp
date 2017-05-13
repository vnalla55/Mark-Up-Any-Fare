#include "DBAccess/test/TestRow.h"

using namespace tse;
using namespace std;

void
TestRow::set(int index, long long value)
{
  _values[index] = value;
}

void
TestRow::set(int index, string value)
{
  _stringValues[index] = value;
}

void
TestRow::set(int index, DateTime& value)
{
  _dateValues[index] = value;
}

void
TestRow::setNull(int index)
{
  _nulls[index] = true;
}

int
TestRow::getInt(int index) const
{
  return (int)find(_values, index);
}

long int
TestRow::getLong(int index) const
{
  return (long int)find(_values, index);
}

long long int
TestRow::getLongLong(int index) const
{
  return (long long)find(_values, index);
}

const char*
TestRow::getString(int index) const
{
  return ((string)find(_stringValues, index)).c_str();
}

char
TestRow::getChar(int index) const
{
  return (char)find(_values, index);
}

DateTime
TestRow::getDate(int index) const
{
  return (DateTime)find(_dateValues, index);
}

bool
TestRow::isNull(int index) const
{
  return _nulls.find(index) != _nulls.end();
}
