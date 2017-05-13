// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#pragma once

#include <map>
#include <boost/ptr_container/ptr_vector.hpp>

namespace tax
{

template <class T>
class CacheItem
{
public:
  typedef T entry_type;
  CacheItem() : vendor(UninitializedCode), itemNo(0), entries() {}
  CacheItem(const type::Vendor& vendor, const type::Index& itemNo, const boost::ptr_vector<T> items)
    : vendor(vendor), itemNo(itemNo), entries(items)
  {
  }
  type::Vendor vendor;
  type::Index itemNo;
  boost::ptr_vector<T> entries;
};

template <class T>
class SubCache
{
public:
  typedef std::pair<type::Vendor, type::Index> key_type;
  typedef std::map<key_type, boost::ptr_vector<T> > map_type;
  const boost::ptr_vector<T>& get(const type::Vendor& vendor, const type::Index& itemNo) const
  {
    key_type key(vendor, itemNo);
    typename map_type::const_iterator it = _cacheMap.find(key);
    if (it == _cacheMap.end())
      return _empty;
    return it->second;
  }

  SubCache() : _cacheMap(), _empty() {}

  virtual ~SubCache() {};

  void add(const CacheItem<T>& item)
  {
    key_type key(item.vendor, item.itemNo);
    for (const T & subItem : item.entries)
      _cacheMap[key].push_back(new T(subItem));
  }

  void add(const key_type& key, const T& item) { _cacheMap[key].push_back(new T(item)); }

  template <typename Iterator>
  void addAll(const key_type& key, Iterator from, Iterator to)
  {
    for (; from != to; from++)
      add(key, *new T(*from));
  }

private:
  map_type _cacheMap;
  const boost::ptr_vector<T> _empty;
};
}
