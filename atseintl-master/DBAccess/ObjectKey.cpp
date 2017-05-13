//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/ObjectKey.h"

#include <boost/tokenizer.hpp>

#include <sstream>

#include <stdlib.h>

namespace tse
{

bool
ObjectKey::getValue(const char* id, std::string& value) const
{
  KeyFields::const_iterator i = _keyFields.find(id);
  if (i == _keyFields.end())
    return false;

  value = i->second;
  return true;
}

bool
ObjectKey::getValue(const char* id, boost::container::string& value) const
{
  KeyFields::const_iterator i = _keyFields.find(id);
  if (UNLIKELY(i == _keyFields.end()))
    return false;

  value = i->second.c_str();
  return true;
}

bool
ObjectKey::getValue(const char* id, int& value) const
{
  KeyFields::const_iterator i = _keyFields.find(id);
  if (UNLIKELY(i == _keyFields.end()))
    return false;

  value = atoi(i->second.c_str());
  return true;
}

bool
ObjectKey::getValue(const char* id, char& value) const
{
  KeyFields::const_iterator i = _keyFields.find(id);
  if (i == _keyFields.end())
    return false;

  value = i->second[0];
  if (value == '\000')
    value = ' ';
  return true;
}

bool
ObjectKey::getValue(const char* id, long long& value) const
{
  KeyFields::const_iterator i = _keyFields.find(id);
  if (i == _keyFields.end())
    return false;

  value = atoll(i->second.c_str());
  return true;
}

bool
ObjectKey::getValue(const char* id, RecordScope& value) const
{
  KeyFields::const_iterator i = _keyFields.find(id);
  if (i == _keyFields.end())
    return false;

  value = (i->second == "D" ? DOMESTIC : INTERNATIONAL);

  return true;
}

bool
ObjectKey::getValue(const char* id, GlobalDirection& value) const
{
  KeyFields::const_iterator i = _keyFields.find(id);
  if (i == _keyFields.end())
    return false;

  strToGlobalDirection(value, i->second.c_str());
  return true;
}

bool
ObjectKey::getValue(const char* id, uint64_t& value) const
{
  KeyFields::const_iterator i = _keyFields.find(id);
  if (i == _keyFields.end())
    return false;

  value = atoll(i->second.c_str());
  return true;
}

bool
ObjectKey::getValue(const char* id, DateTime& value) const
{
  KeyFields::const_iterator i = _keyFields.find(id);
  if (i == _keyFields.end())
    return false;

  std::string dateString = i->second; // DateTime requires non-const
  value = DateTime(dateString);
  if (value.isNotADate() || (!value.isValid()))
    return false;
  return true;
}

std::string
ObjectKey::toString(bool includeLabel) const
{
  std::string retval;

  for (KeyFields::const_iterator i = _keyFields.begin(); i != _keyFields.end(); ++i)
  {
    if (i != _keyFields.begin())
    {
      retval += '|';
    }
    if (LIKELY(includeLabel))
    {
      retval.append((*i).first);
      retval += ':';
    }
    const std::string& val = (*i).second;
    if (LIKELY(!val.empty()))
    {
      if (LIKELY(val[0] != '\000'))
      {
        retval.append(val);
      }
    }
  }

#if 1
  char* p = const_cast<char*>(retval.data());
  for (size_t counter = 0; counter < retval.size(); ++counter, ++p)
  {
    if (UNLIKELY(*p == '\0'))
    {
      std::cout << "WARNING: Embedded NULL detected at byte " << counter << " of ObjectKey ["
                << retval << "]!" << std::endl;
    }
  }
#endif

  return retval;
}

ObjectKey&
ObjectKey::fromString(const std::string& str)
{
  _keyFields.clear();

  boost::char_separator<char> fieldSep("|", "", boost::keep_empty_tokens);
  boost::tokenizer<boost::char_separator<char> > fields(str, fieldSep);
  for (boost::tokenizer<boost::char_separator<char> >::const_iterator i = fields.begin();
       i != fields.end();
       ++i)
  {
    const std::string& field = (*i);
    std::string saveKey;
    std::string saveVal = "\000";
    boost::char_separator<char> pairSep(":", "", boost::keep_empty_tokens);
    boost::tokenizer<boost::char_separator<char> > elements(field, pairSep);
    for (boost::tokenizer<boost::char_separator<char> >::const_iterator j = elements.begin();
         j != elements.end();
         ++j)
    {
      const std::string& elem = (*j);
      if (j == elements.begin())
      {
        saveKey = elem;
      }
      else
      {
        saveVal = elem;
        break;
      }
    }

    _keyFields[saveKey] = saveVal;
  }

  return *this;
}

void
ObjectKey::setValue(const char* id, const char* value)
{
  _keyFields[id] = value;
}

void
ObjectKey::setValue(const char* id, const std::string& value)
{
  _keyFields[id] = value;
}

void
ObjectKey::setValue(const char* id, const boost::container::string& value)
{
  _keyFields[id] = value.c_str();
}

void
ObjectKey::setValue(const char* id, const int& value)
{
  std::stringstream ss;
  ss << value;
  _keyFields[id] = ss.str();
}

void
ObjectKey::setValue(const char* id, const char& value)
{
  std::stringstream ss;
  if (value == ' ')
  {
    ss << '\000';
  }
  else
  {
    ss << value;
  }
  _keyFields[id] = ss.str();
}

void
ObjectKey::setValue(const char* id, const long long& value)
{
  std::stringstream ss;
  ss << value;
  _keyFields[id] = ss.str();
}

void
ObjectKey::setValue(const char* id, const RecordScope& value)
{
  _keyFields[id] = (value == DOMESTIC ? "D" : "I");
}

void
ObjectKey::setValue(const char* id, const GlobalDirection& value)
{
  _keyFields[id] = *(globalDirectionToStr(value));
}

void
ObjectKey::setValue(const char* id, const uint64_t& value)
{
  std::stringstream ss;
  ss << value;
  _keyFields[id] = ss.str();
}

void
ObjectKey::setValue(const char* id, const DateTime& value)
{
  _keyFields[id] = value.toSimpleString();
}

} // namespace tse
