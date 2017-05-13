#include "Xform/CustomXMLParser/IAttributes.h"

#include <ostream>

const IValueString _emptyValueString;
const std::string _emptyString;

IAttributes::IAttributes (int vectorSz,
                          const IValueString valueArray[])
  : _vectorSz(vectorSz)
  , _valueArray(valueArray)
  , _map(nullptr)
{
}

IAttributes::IAttributes (const IMap *map)
  : _vectorSz(-1)
  , _valueArray(nullptr)
  , _map(map)
{
}

IAttributes::~IAttributes ()
{
}

bool IAttributes::has (int idx) const throw ()
{
  return nullptr != _valueArray && !_valueArray[idx].empty();
}

const IValueString &IAttributes::get (int idx) const throw ()
{
  return nullptr != _valueArray ? _valueArray[idx] : _emptyValueString;
}

bool IAttributes::has (const IKeyString &name) const throw ()
{
  return nullptr != _map && _map->count(name) > 0;
}

const IValueString &IAttributes::get (const IKeyString &name) const throw ()
{
  IMap::const_iterator iter;
  return nullptr != _map
         && (iter = _map->find(name)) != _map->end() ?
                                iter->second : _emptyValueString;
}
#if 0
// for debugging
void IAttributes::write (std::ostream &output,
                         const char * const *names) const
{
  if (0 != _map)
  {
    for (IMap::const_iterator it(_map->begin()),
                              itEnd(_map->end());
         it != itEnd;
         ++it)
    {
      const IValueString &value = it->second;
      if (!value.empty())
      {
        output << ' ' << it->first << "=\"" << value << '\"';
      }
    }
  }
  else if (0 != _valueArray && 0 != names)
  {
    for (int idx = 0; idx < _vectorSz; ++idx)
    {
      const IValueString &value = _valueArray[idx];
      if (!value.empty())
      {
        output << ' ' << names[idx] << "=\"" << value << '\"';
      }
    }
  }
}
// for debugging
size_t IAttributes::getAll (IMap &attrMap,
                            const char * const *names) const throw ()
{
  if (0 != _map)
  {
    for (IMap::const_iterator iter(_map->begin()),
                              iterEnd(_map->end());
         iter != iterEnd;
         ++iter)
    {
      attrMap[iter->first] = iter->second;
    }
  }
  else if (0 != names && 0 != _valueArray)
  {
    for (int idx = 0; idx < _vectorSz; ++idx)
    {
      const IValueString &value = _valueArray[idx];
      if (!value.empty())
      {
        const char *pName = names[idx];
        IKeyString nm(pName, strlen(pName));
        attrMap[nm] = value;
      }
    }
  }
  return attrMap.size();
}

const IMap &IAttributes::getMap () const
{
  if (0 != _map)
  {
    return *_map;
  }
  else
  {
    static const IMap dummy;
    return dummy;
  }
}
#endif// 0
