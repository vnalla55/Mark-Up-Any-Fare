#include "Common/Config/ConfigMan.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/thread.hpp>
#include "Util/BranchPrediction.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include "Common/FallbackUtil.h"

namespace tse
{
FIXEDFALLBACK_DECL(fallbackCustomToupperInConfigMan);
namespace detail
{
namespace
{

struct ScopedCounter
{
  ~ScopedCounter() { --_counter; }

  explicit ScopedCounter(unsigned& counter) : _counter(counter) { ++_counter; }

  unsigned& _counter;
};

bool
ExpandRef(ConfigManBase::ExpandContext& context, const std::string& value, std::string& result)
{
  bool ok;

  std::string::size_type sep = value.find(":");
  if (sep != std::string::npos && sep < value.size() - 1)
  {
    ok = context.getValue(std::string(value, sep + 1), result, std::string(value, 0, sep));
  }
  else
  {
    ok = context.getValue(value, result, ConfigManBase::DefaultGroup());
  }

  return ok;
}

bool
ExpandEnv(ConfigManBase::ExpandContext& /*config*/, const std::string& value, std::string& result)
{
  bool ok = false;
  if (const char* envValue = std::getenv(value.c_str()))
  {
    result = envValue;
    ok = true;
  }

  return ok;
}

bool
ExpandSys(ConfigManBase::ExpandContext& /*config*/, const std::string& value, std::string& result)
{
  bool ok = false;
  if (FILE* in = ::popen(value.c_str(), "r"))
  {
    char buffer[512];
    bool isEof = false;
    bool readError = false;
    while (!isEof && !readError)
    {
      const std::size_t size = std::fread(buffer, 1, sizeof(buffer), in);
      result.append(buffer, size);
      if (size != sizeof(buffer))
      {
        if (std::feof(in))
        {
          isEof = true;
        }
        else
        {
          readError = true;
        }
      }
    }

    if (!readError)
    {
      ok = true;
    }

    ::pclose(in);
  }

  return ok;
}

bool
ExpandFunc(ConfigManBase::ExpandContext& /*config*/, const std::string& value, std::string& result)
{
  bool ok = false;
  try
  {
    if (value == "PID")
    {
      result = boost::lexical_cast<std::string>(::getpid());
      ok = true;
    }
    else if (value == "THREAD")
    {
      result = boost::lexical_cast<std::string>(::pthread_self());
      ok = true;
    }
    else if (value == "CORES")
    {
      const unsigned cores = boost::thread::hardware_concurrency();
      if (cores == 0)
      {
        ok = false;
      }
      else
      {
        result = boost::lexical_cast<std::string>(cores);
        ok = true;
      }
    }
  }
  catch (boost::bad_lexical_cast&) {}

  return ok;
}

struct MetaTag
{
  typedef bool (*ExpandCallback)(ConfigManBase::ExpandContext& config,
                                 const std::string& value,
                                 std::string& result);

  MetaTag(const std::string& name, ExpandCallback cb)
    : openingName("<" + name + ">"), closingName("</" + name + ">"), callback(cb)
  {
  }

  std::string openingName;
  std::string closingName;
  ExpandCallback callback;
};

const MetaTag MetaTags[] = {
  MetaTag("ref", ExpandRef), MetaTag("env", ExpandEnv), MetaTag("sys", ExpandSys),
  MetaTag("func", ExpandFunc),
};

const MetaTag*
FindMetaTag(std::string::const_iterator b, std::string::const_iterator e, bool isOpening)
{
  const MetaTag* tag = nullptr;
  for (unsigned i = 0; i < sizeof(MetaTags) / sizeof(MetaTags[0]) && !tag; ++i)
  {
    const std::string& name = isOpening ? MetaTags[i].openingName : MetaTags[i].closingName;
    std::string::const_iterator tagIt = name.begin();
    std::string::const_iterator valueIt = b;
    while (tagIt != name.end() && valueIt != e && *tagIt == *valueIt)
    {
      ++valueIt;
      ++tagIt;
    }

    if (tagIt == name.end())
    {
      // if tagIt == name.end () full name has been compared with success
      tag = &MetaTags[i];
    }
  }

  return tag;
}

std::string::const_iterator
SkipSpaces(std::string::const_iterator b, std::string::const_iterator e)
{
  while (b != e && std::isspace(*b))
  {
    ++b;
  }
  return b;
}

void
WriteValue(std::ostream& out, const std::string& s)
{
  const bool quote = !s.empty() && (std::isspace(*s.begin()) || std::isspace(*s.rbegin()));
  if (quote)
  {
    out << '"';
  }

  for (const char elem : s)
  {
    switch (elem)
    {
    case '\n':
      out << "\\n";
      break;
    case '\r':
      out << "\\r";
      break;
    case '\\':
      out << "\\\\";
      break;
    case ',':
      out << "\\,";
      break;
    default:
    {
      if (std::isspace(elem) && !quote)
      {
        out << "\\";
      }

      out << elem;
      break;
    }
    }
  }

  if (quote)
  {
    out << '"';
  }
}

} // unnamed namespace

////////////////////////////////////////////////////////////////////////////////////////////////////

char ConfigManBase::LessNoCase::toUpperTranslationTable[256];
std::once_flag ConfigManBase::LessNoCase::toUpperTrTblInitFlag;

void ConfigManBase::LessNoCase::initToUpperTranslationTable()
{
  for(int c = 0; c < 256 ; ++c)
    toUpperTranslationTable[c] = (c >= 'a' && c <='z') ?
              c - ('a' - 'A') :
              c;
}


ConfigManBase::ExpandContext::ExpandContext(const ConfigManBase& config)
  : _config(config), _expandCounter(0)
{
}

bool
ConfigManBase::ExpandContext::getValue(const std::string& name,
                                       std::string& value,
                                       const std::string& group)
{
  ScopedCounter scopedCounter(_expandCounter);
  if (_expandCounter >= 10)
  {
    // limit expand resursion to avoid stack overflow
    // if someone creates circular references between keys.
    return false;
  }

  return _config.getValueImpl(name, value, group, this);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ConfigManBase::~ConfigManBase() {}

ConfigManBase::ConfigManBase(const std::string& defaultGroup) : _defaultGroup(defaultGroup)
{
  std::call_once(LessNoCase::toUpperTrTblInitFlag, LessNoCase::initToUpperTranslationTable);
}

bool
ConfigManBase::setValueImpl(const std::string& name,
                            const std::string& value,
                            const std::string& group,
                            const bool overwrite)
{
  if (overwrite)
  {
    eraseValueImpl(name, group);
  }

  Keys& keys = _groups[selectGroup(group)];
  keys[name].push_back(value);

  return true;
}

bool
ConfigManBase::setValuesImpl(const std::string& name,
                             const Values& values,
                             const std::string& group,
                             const bool overwrite)
{
  if (overwrite)
  {
    eraseValueImpl(name, group);
  }

  Keys& keys = _groups[selectGroup(group)];
  Values& currentValues = keys[name];
  currentValues.reserve(currentValues.size() + values.size());
  for (const auto& value : values)
  {
    currentValues.push_back(value);
  }

  return true;
}

bool
ConfigManBase::setValuesImpl(const std::vector<NameValue>& namesValues, bool overwrite)
{
  for (const auto& nameValue : namesValues)
  {
    if (overwrite)
    {
      eraseValueImpl(nameValue.name, nameValue.group);
    }

    Keys& keys = _groups[selectGroup(nameValue.group)];
    Values& currentValues = keys[nameValue.name];
    currentValues.push_back(nameValue.value);
  }

  return true;
}

bool
ConfigManBase::getValueImpl(const std::string& name,
                            std::string& value,
                            const std::string& group,
                            ExpandContext* expandContext) const
{
  bool ok = false;
  Groups::const_iterator groupIt = _groups.find(selectGroup(group));
  if (groupIt != _groups.end())
  {
    const Keys& keys = groupIt->second;
    Keys::const_iterator keyIt = keys.find(name);
    if (keyIt != keys.end())
    {
      const Values& values = keyIt->second;
      if (LIKELY(!values.empty()))
      {
        value = values.front();
        if (UNLIKELY(expandContext))
        {
          ok = expand(value, *expandContext);
        }
        else
        {
          ok = expand(value);
        }
      }
    }
  }

  return ok;
}

bool
ConfigManBase::getValuesImpl(const std::string& name, Values& values, const std::string& group)
    const
{
  bool ok = false;
  values.clear();
  Groups::const_iterator groupIt = _groups.find(selectGroup(group));
  if (groupIt != _groups.end())
  {
    const Keys& keys = groupIt->second;
    Keys::const_iterator keyIt = keys.find(name);
    if (keyIt != keys.end())
    {
      const Values& currentValues = keyIt->second;
      values.reserve(currentValues.size());
      bool valuesOk = true;
      for (const auto& value : currentValues)
      {
        values.push_back(value);
        if (!expand(values.back()))
        {
          valuesOk = false;
        }
      }

      ok = valuesOk;
    }
    else
    {
      // to be consistent with old implementation of ConfigMan
      ok = true;
    }
  }

  return ok;
}

bool
ConfigManBase::getValuesImpl(std::vector<NameValue>& namesValues) const
{
  bool ok = true; // to be consistent with old implementation of ConfigMan
  namesValues.clear();
  for (const auto& elem : _groups)
  {
    const std::string& group = boost::to_upper_copy(elem.first);
    const Keys& keys = elem.second;
    for (const auto& key : keys)
    {
      const std::string& name = boost::to_upper_copy(key.first);
      const Values& currentValues = key.second;
      for (const auto& value : currentValues)
      {
        namesValues.push_back(NameValue());
        namesValues.back().group = group;
        namesValues.back().name = name;
        namesValues.back().value = value;
        if (!expand(namesValues.back().value))
        {
          ok = false;
        }
      }
    }
  }

  return ok;
}

bool
ConfigManBase::getValuesImpl(std::vector<NameValue>& namesValues, const std::string& group) const
{
  bool ok = true;
  namesValues.clear();
  Groups::const_iterator groupIt = _groups.find(selectGroup(group));
  if (groupIt != _groups.end())
  {
    const std::string& groupUpper = boost::to_upper_copy(groupIt->first);
    const Keys& keys = groupIt->second;
    bool valuesOk = true;
    namesValues.reserve(keys.size()); // assume one element for the key
    for (const auto& key : keys)
    {
      const std::string& name = boost::to_upper_copy(key.first);
      const Values& currentValues = key.second;
      for (const auto& value : currentValues)
      {
        namesValues.push_back(NameValue());
        namesValues.back().group = groupUpper;
        namesValues.back().name = name;
        namesValues.back().value = value;
        if (!expand(namesValues.back().value))
        {
          valuesOk = false;
        }
      }
    }

    ok = valuesOk;
  }

  return ok;
}

std::size_t
ConfigManBase::eraseValueImpl(const std::string& name, const std::string& group)
{
  std::size_t erased = 0;
  Groups::iterator groupIt = _groups.find(selectGroup(group));
  if (groupIt != _groups.end())
  {
    Keys& keys = groupIt->second;
    Keys::iterator keyIt = keys.find(name);
    if (keyIt != keys.end())
    {
      const Values& values = keyIt->second;
      erased = values.size();
      keys.erase(keyIt);
    }
  }

  return erased;
}

void
ConfigManBase::clearImpl()
{
  _groups.clear();
}

bool
ConfigManBase::readImpl(const std::string& fileName)
{
  bool ok = false;
  std::ifstream in(fileName.c_str());
  if (in)
  {
    ok = readImpl(in);
  }

  return ok;
}

bool
ConfigManBase::readImpl(std::istream& in)
{
  clearImpl();
  unsigned errors = 0;
  std::string line;
  std::string group(_defaultGroup);
  std::string name;
  std::string value;
  while (std::getline(in, line))
  {
    std::string::const_iterator e = line.end();
    std::string::const_iterator b = SkipSpaces(line.begin(), e);
    if (b != e)
    {
      if (*b == '#')
      {
        // comment
      }
      else if (*b == '[')
      {
        // [group name]
        group.clear();
        std::string::const_iterator n = b + 1;
        b = std::find(n, e, ']');
        if (b == e || SkipSpaces(b + 1, e) != e)
        {
          // no closing ] or unexpected characters after ]
          ++errors;
        }
        else
        {
          group.assign(n, b);
        }
      }
      else
      {
        // key = value
        name.clear();
        std::string::const_iterator n = b;
        b = std::find(b, e, '=');
        if (b == e)
        {
          // no '=' character
          ++errors;
        }
        else
        {
          name.assign(n, b);
          boost::trim_right(name);
          b = SkipSpaces(b + 1, e);

          value.clear();
          bool quote = false;
          for (; b != e; ++b)
          {
            if (*b == '"')
            {
              quote = !quote;
            }
            else if (*b == '\\')
            {
              ++b;
              if (b != e)
              {
                switch (*b)
                {
                case 'n':
                  value.push_back('\n');
                  break;
                case 'r':
                  value.push_back('\r');
                  break;
                case '\n':
                  break;
                case '\r':
                  break;
                case '\\':
                case ',':
                case '\t':
                case ' ':
                  value.push_back(*b);
                  break;

                default:
                  value.push_back('\\');
                  value.push_back(*b);
                  break;
                }
              }
            }
            else
            {
              if (std::isspace(*b) && !quote)
              {
                // skip
              }
              else if (*b == ',' && !quote)
              {
                // values separated by ,
                setValueImpl(name, value, group, false);
                value.clear();
              }
              else
              {
                value.push_back(*b);
              }
            }
          }

          setValueImpl(name, value, group, false);
        }
      }
    }
  }

  return errors == 0;
}

bool
ConfigManBase::writeImpl(const std::string& fileName) const
{
  bool ok = false;
  std::ofstream out(fileName.c_str());
  if (out)
  {
    writeImpl(out);
    ok = true;
  }

  return ok;
}

void
ConfigManBase::writeImpl(std::ostream& out) const
{
  for (const auto& elem : _groups)
  {
    const std::string& group = boost::to_upper_copy(elem.first);
    out << "[" << group << "]\n";

    const Keys& keys = elem.second;
    for (const auto& key : keys)
    {
      const std::string& name = boost::to_upper_copy(key.first);
      const Values& values = key.second;
      for (const auto& value : values)
      {
        out << name << " = ";
        WriteValue(out, value);
        out << "\n";
      }
    }

    out << "\n";
  }
}

const std::string&
ConfigManBase::DefaultGroup()
{
  static const std::string defaultGroup("Main");
  return defaultGroup;
}

const std::string&
ConfigManBase::selectGroup(const std::string& group) const
{
  if (UNLIKELY(boost::iequals(group, DefaultGroup())))
  {
    return _defaultGroup;
  }
  else
  {
    return group;
  }
}

bool
ConfigManBase::expand(std::string& value) const
{
  ExpandContext context(*this);
  return expand(value, context);
}

bool
ConfigManBase::expand(std::string& value, ExpandContext& context) const
{
  if (value.size() < sizeof("<x></x>"))
  {
    // minimum size of tag is 7, so do not perform expanding
    // for values less than or equal to this value.
    return true;
  }

  // note: always process from begin of the value because
  // result of replacing metatag may contain another metatags
  bool process = true;
  bool ok = true;
  while (process && ok)
  {
    process = false;
    const MetaTag* openingTag = nullptr;
    std::string::iterator openingStartIt = value.end();
    for (std::string::iterator it = value.begin(); it != value.end() && !openingTag; ++it)
    {
      if (UNLIKELY(*it == '<'))
      {
        openingTag = FindMetaTag(it, value.end(), true);
        if (openingTag)
        {
          openingStartIt = it;
        }
      }
    }

    const MetaTag* closingTag = nullptr;
    std::string::iterator closingStartIt = value.end();
    if (openingTag)
    {
      for (std::string::iterator it = openingStartIt + openingTag->openingName.size();
           it != value.end() && !closingTag;
           ++it)
      {
        if (*it == '<')
        {
          const MetaTag* currentTag = FindMetaTag(it, value.end(), false);
          if (currentTag && currentTag == openingTag)
          {
            closingTag = currentTag;
            closingStartIt = it;
          }
        }
      }
    }

    if (openingTag && closingTag)
    {
      std::string::iterator openEndIt = openingStartIt + openingTag->openingName.size();
      const std::string tagValue(openEndIt, closingStartIt);
      std::string result;
      if (openingTag->callback(context, tagValue, result))
      {
        value.replace(openingStartIt, closingStartIt + openingTag->closingName.size(), result);
        process = true;
      }
      else
      {
        ok = false;
      }
    }
  }

  return ok;
}

void
AssertErrorHandler::onCastError(const std::string& name,
                                const std::string& value,
                                const std::string& group)
{
  std::cerr << "Cast error " << group << "\\" << name << " value: " << value << std::endl;
  if (!std::getenv("TSE_CONFIG_MAN_NO_ASSERTION"))
  {
    assert(false && "ConfigMan: cast error!"
                    "To disable this assertion set env. variable TSE_CONFIG_MAN_NO_ASSERTION");
  }
}

} // namespace detail
} // namespace tse
