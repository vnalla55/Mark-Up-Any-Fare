#pragma once

#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>

#include <functional>
#include <iosfwd>
#include <map>
#include <string>
#include <vector>
#include <mutex>
#include "Util/BranchPrediction.h"

namespace tse
{
namespace detail
{

class ConfigManBase
{
  struct LessNoCase : public std::binary_function<const std::string&, const std::string&, bool>
  {
    static char toUpperTranslationTable[256];
    static std::once_flag toUpperTrTblInitFlag;
    static void initToUpperTranslationTable();
    inline static bool icaseCmp(char c1, char c2)
    {
       return toUpperTranslationTable[(unsigned char)c1] < toUpperTranslationTable[(unsigned char)c2] ;
    }

    bool operator()(const std::string& l, const std::string& r) const
    {
      return std::lexicographical_compare(l.cbegin(), l.cend(), r.cbegin(), r.cend(), icaseCmp);
    }
  };

public:
  struct ExpandContext
  {
    ExpandContext(const ConfigManBase& config);
    bool getValue(const std::string& name, std::string& value, const std::string& group);

  private:
    const ConfigManBase& _config;
    unsigned _expandCounter;
  };

  // values for key
  typedef std::vector<std::string> Values;
  // map key to value
  typedef std::map<std::string, Values, LessNoCase> Keys;
  // map group to keys
  typedef std::map<std::string, Keys, LessNoCase> Groups;

  struct NameValue
  {
    NameValue() {}
    NameValue(const std::string& n, const std::string& v, const std::string& g)
      : name(n), value(v), group(g)
    {
    }

    std::string name;
    std::string value;
    std::string group;
  };

public:
  ~ConfigManBase();
  explicit ConfigManBase(const std::string& defaultGroup);

  bool setValueImpl(const std::string& name,
                    const std::string& value,
                    const std::string& group,
                    const bool overwrite);
  bool setValuesImpl(const std::string& name,
                     const Values& values,
                     const std::string& group,
                     const bool overwrite);
  bool setValuesImpl(const std::vector<NameValue>& namesValues, bool overwrite);

  bool getValueImpl(const std::string& name,
                    std::string& value,
                    const std::string& group,
                    ExpandContext* expandContext = nullptr) const;
  bool getValuesImpl(const std::string& name, Values& values, const std::string& group) const;
  bool getValuesImpl(std::vector<NameValue>& namesValues) const;
  bool getValuesImpl(std::vector<NameValue>& namesValues, const std::string& group) const;

  std::size_t eraseValueImpl(const std::string& name, const std::string& group);
  void clearImpl();

  bool readImpl(const std::string& fileName);
  bool readImpl(std::istream& in);

  bool writeImpl(const std::string& fileName) const;
  void writeImpl(std::ostream& out) const;

  static const std::string& DefaultGroup();

private:
  const std::string& selectGroup(const std::string& group) const;
  bool expand(std::string& value) const;
  bool expand(std::string& value, ExpandContext& context) const;

private:
  const std::string _defaultGroup;
  Groups _groups;
};

// to keep compability with boost.thread
struct EmptyMutex
{
  struct scoped_lock
  {
    explicit scoped_lock(EmptyMutex&) {}
  };
};

struct EmptyErrorHandler // usunac??
{
  static void onCastError(const std::string&, const std::string&, const std::string&) {}
};

struct AssertErrorHandler
{
  static void
  onCastError(const std::string& name, const std::string& value, const std::string& group);
};

#ifdef NDEBUG
typedef EmptyErrorHandler ErrorHandler;
#else
typedef AssertErrorHandler ErrorHandler;
#endif

} // namespace detail

template <class Mutex, class ErrorHandler>
class ConfigManTemplate : protected detail::ConfigManBase
{
  typedef typename Mutex::scoped_lock ScopedLock;
  typedef detail::ConfigManBase Base;

public:
  using detail::ConfigManBase::DefaultGroup;
  using detail::ConfigManBase::Values;
  using detail::ConfigManBase::NameValue;

public:
  explicit ConfigManTemplate(const std::string& defaultGroup = DefaultGroup());

  template <typename T>
  bool setValue(const std::string& name,
                const T& value,
                const std::string& group = DefaultGroup(),
                const bool overwrite = false);
  bool setValue(const std::string& name,
                const std::string& value,
                const std::string& group,
                const bool overwrite);
  bool setValues(const std::string& name,
                 const Values& values,
                 const std::string& group = DefaultGroup(),
                 const bool overwrite = false);
  bool setValues(const std::vector<NameValue>& namesValues, bool overwrite = false);

  template <typename T>
  bool getValue(const std::string& name, T& value, const std::string& group = DefaultGroup()) const;
  bool getValue(const std::string& name,
                std::string& value,
                const std::string& group = DefaultGroup()) const;
  bool getValues(const std::string& name,
                 Values& values,
                 const std::string& group = DefaultGroup()) const;
  bool getValues(std::vector<NameValue>& namesValues) const;
  bool getValues(std::vector<NameValue>& namesValues, const std::string& group) const;

  std::size_t eraseValue(const std::string& name, const std::string& group = DefaultGroup());
  void clear();

  bool read(const std::string& fileName);
  bool read(std::istream& in);

  bool write(const std::string& fileName) const;
  void write(std::ostream& out) const;

private:
  mutable Mutex _mutex;
};

template <class Mutex, class ErrorHandler>
ConfigManTemplate<Mutex, ErrorHandler>::ConfigManTemplate(const std::string& defaultGroup)
  : detail::ConfigManBase(defaultGroup)
{
}

template <class Mutex, class ErrorHandler>
template <typename T>
bool
ConfigManTemplate<Mutex, ErrorHandler>::setValue(const std::string& name,
                                                 const T& value,
                                                 const std::string& group,
                                                 const bool overwrite)
{
  ScopedLock lock(_mutex);
  bool ok = false;
  try
  {
    // NOTE since boost 1.47.0 boost::lexical_cast outperforms
    //      std::stringstream expecialy for integral types.
    const std::string& rawValue = boost::lexical_cast<std::string>(value);
    ok = Base::setValueImpl(name, rawValue, group, overwrite);
  }
  catch (boost::bad_lexical_cast&)
  {
    ErrorHandler::onCastError(name, std::string("<unknown>"), group);
    ok = false;
  }
  return ok;
}

template <class Mutex, class ErrorHandler>
bool
ConfigManTemplate<Mutex, ErrorHandler>::setValue(const std::string& name,
                                                 const std::string& value,
                                                 const std::string& group,
                                                 const bool overwrite)
{
  ScopedLock lock(_mutex);
  return Base::setValueImpl(name, value, group, overwrite);
}

template <class Mutex, class ErrorHandler>
bool
ConfigManTemplate<Mutex, ErrorHandler>::setValues(const std::string& name,
                                                  const Values& values,
                                                  const std::string& group,
                                                  const bool overwrite)
{
  ScopedLock lock(_mutex);
  return Base::setValuesImpl(name, values, group, overwrite);
}

template <class Mutex, class ErrorHandler>
bool
ConfigManTemplate<Mutex, ErrorHandler>::setValues(const std::vector<NameValue>& namesValues,
                                                  bool overwrite)
{
  ScopedLock lock(_mutex);
  return Base::setValuesImpl(namesValues, overwrite);
}

template <class Mutex, class ErrorHandler>
template <typename T>
bool
ConfigManTemplate<Mutex, ErrorHandler>::getValue(const std::string& name,
                                                 T& value,
                                                 const std::string& group) const
{
  ScopedLock lock(_mutex);
  std::string rawValue;
  bool ok = Base::getValueImpl(name, rawValue, group);
  if (ok)
  {
    try
    {
      // NOTE since boost 1.47.0 boost::lexical_cast outperforms
      //      std::stringstream expecialy for integral types.
      if (UNLIKELY(boost::is_unsigned<T>::value && !rawValue.empty() && rawValue[0] == '-'))
      {
        // The old implementation is buggy it does not check that result
        // of string->type casting. It also relay of nonportable assumption
        // that is: currently used libstdc++ fails if required value is unsigned but casted value
        // is signed, but newer implementation (gcc 4.5 or higher) pass with returning huge number.
        // We may continue assumption that lexical casting signed to unsigned fails to be
        // independent of C++ library implementation.
        ErrorHandler::onCastError(name, rawValue, group);
        ok = false;
      }
      else
      {
        value = boost::lexical_cast<T>(rawValue);
      }
    }
    catch (boost::bad_lexical_cast&)
    {
      ErrorHandler::onCastError(name, rawValue, group);
      ok = false;
    }
  }

  return ok;
}

template <class Mutex, class ErrorHandler>
bool
ConfigManTemplate<Mutex, ErrorHandler>::getValue(const std::string& name,
                                                 std::string& value,
                                                 const std::string& group) const
{
  ScopedLock lock(_mutex);
  return Base::getValueImpl(name, value, group);
}

template <class Mutex, class ErrorHandler>
bool
ConfigManTemplate<Mutex, ErrorHandler>::getValues(const std::string& name,
                                                  Values& values,
                                                  const std::string& group) const
{
  ScopedLock lock(_mutex);
  return Base::getValuesImpl(name, values, group);
}

template <class Mutex, class ErrorHandler>
bool
ConfigManTemplate<Mutex, ErrorHandler>::getValues(std::vector<NameValue>& namesValues) const
{
  ScopedLock lock(_mutex);
  return Base::getValuesImpl(namesValues);
}

template <class Mutex, class ErrorHandler>
bool
ConfigManTemplate<Mutex, ErrorHandler>::getValues(std::vector<NameValue>& namesValues,
                                                  const std::string& group) const
{
  ScopedLock lock(_mutex);
  return Base::getValuesImpl(namesValues, group);
}

template <class Mutex, class ErrorHandler>
std::size_t
ConfigManTemplate<Mutex, ErrorHandler>::eraseValue(const std::string& name,
                                                   const std::string& group)
{
  ScopedLock lock(_mutex);
  return Base::eraseValueImpl(name, group);
}

template <class Mutex, class ErrorHandler>
void
ConfigManTemplate<Mutex, ErrorHandler>::clear()
{
  ScopedLock lock(_mutex);
  return Base::clearImpl();
}

template <class Mutex, class ErrorHandler>
bool
ConfigManTemplate<Mutex, ErrorHandler>::read(const std::string& fileName)
{
  ScopedLock lock(_mutex);
  return Base::readImpl(fileName);
}

template <class Mutex, class ErrorHandler>
bool
ConfigManTemplate<Mutex, ErrorHandler>::read(std::istream& in)
{
  ScopedLock lock(_mutex);
  return Base::readImpl(in);
}

template <class Mutex, class ErrorHandler>
bool
ConfigManTemplate<Mutex, ErrorHandler>::write(const std::string& fileName) const
{
  ScopedLock lock(_mutex);
  return Base::writeImpl(fileName);
}

template <class Mutex, class ErrorHandler>
void
ConfigManTemplate<Mutex, ErrorHandler>::write(std::ostream& out) const
{
  ScopedLock lock(_mutex);
  Base::writeImpl(out);
}

template <class Mutex, class ErrorHandler>
std::ostream& operator<<(std::ostream& out, const ConfigManTemplate<Mutex, ErrorHandler>& cfg)
{
  cfg.write(out);
  return out;
}

template <class Mutex, class ErrorHandler>
std::istream& operator>>(std::istream& in, ConfigManTemplate<Mutex, ErrorHandler>& cfg)
{
  cfg.read(in);
  return in;
}

// to keep compability with existing code
class ConfigMan : public ConfigManTemplate<detail::EmptyMutex, detail::ErrorHandler>
{
public:
  explicit ConfigMan(const std::string& defaultGroup = DefaultGroup())
    : ConfigManTemplate<detail::EmptyMutex, detail::ErrorHandler>(defaultGroup)
  {
  }
};
} // namespace tse
