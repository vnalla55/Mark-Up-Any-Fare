#ifndef IAttributes_H
#define IAttributes_H

#include "Xform/CustomXMLParser/ICString.h"

#include <tr1/unordered_map>

typedef std::tr1::unordered_map<IKeyString, IValueString, IKeyStringHasher> IMap;

extern const std::string _emptyString;

class IAttributes
{
  friend class IDAttributes;
 public:
  IAttributes (int vectorSz,
               const IValueString valueArray[]);
  explicit IAttributes (const IMap *map);
  ~IAttributes ();

  const IValueString &get (const IKeyString &name) const throw ();
  const IValueString &get (int idx) const throw ();
  bool has (const IKeyString &name) const throw ();
  bool has (int idx) const throw ();

  void get (int idx,
            std::string &value,
            const std::string &defaultValue = _emptyString) const throw ()
  {
    const IValueString &attr(get(idx));
    if (attr.empty())
    {
      value.assign(defaultValue);
    }
    else
    {
      value.assign(attr.c_str(), attr.length());
    }
  }

  template <typename T> 
	void get (int idx, T &value, const T &defaultValue = T()) const
  {
    const IValueString &attr(get(idx));
    if (attr.empty())
    {
      value = defaultValue;
    }
    else
    {
      attr.parse(value);
    }
  }
  
  template <typename T> 
  T get (int idx, const T &defaultValue = T()) const 
  {
    const IValueString &attr(get(idx));
    if (attr.empty())
    {
      return defaultValue;
    }
    else
    {
      T value = defaultValue;
      attr.parse(value);
      return value;
    }
  }
  // for output
  void write (std::ostream &output,
              const char * const *names = nullptr) const;
  // for debugging
  size_t getAll (IMap &attrMap,
                 const char * const *names = nullptr) const throw ();
#if 0
  const IMap &getMap () const throw ();
#endif// 0
 protected:
  const int _vectorSz;
  const IValueString * const _valueArray;
  const IMap * const _map;
 private:
  // not implemented
  IAttributes (const IAttributes &);
  IAttributes &operator = (const IAttributes &);
};
#endif// IAttributes_H
