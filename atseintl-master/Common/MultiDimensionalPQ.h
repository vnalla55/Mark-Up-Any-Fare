//----------------------------------------------------------------------------
//  File:        MultiPriorityQueue.h
//  Created:     2011-07-29
//
//  Description: Template class for multi-dimensional priority queue
//
//  Updates:
//
//  Copyright Sabre 2011
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include <algorithm>
#include <queue>
#include <set>
#include <vector>

namespace tse
{
template <typename T1, typename T2>
class MultiDimensionalPQ
{
  typedef T2 compare_type;
  typedef std::vector<T1> values_type;
  typedef std::vector<size_t> coords_type;

  class Item
  {
  public:
    Item(const values_type& values, const coords_type& coords)
      : _values(values), _coords(coords), _total(0)
    {
      for (auto& elem : _values)
        _total += elem.queueRank();
    }

    // These are for use in vector
    Item() : _total(0.0) {}
    Item(const Item& other) : _values(other._values), _coords(other._coords), _total(other._total)
    {
    }

    const values_type& values() const { return _values; }
    const coords_type& coords() const { return _coords; }
    compare_type total() const { return _total; }
    bool operator<(const Item& other) const { return _total < other._total; }

  private:
    values_type _values;
    coords_type _coords;
    compare_type _total;
  };

public:
  typedef std::vector<T1> dimension_type;
  typedef std::vector<dimension_type> input_type;

  MultiDimensionalPQ(const input_type& elems) : _elems(elems), _dim(elems.size()), _currentMin(-1.0)
  {
    for (size_t axis = 0; axis < _dim; ++axis)
    {
      if (_elems[axis].empty())
        return;
    }

    values_type v;
    coords_type c;

    for (size_t axis = 0; axis < _dim; ++axis)
    {
      v.push_back(_elems[axis][0]);
      c.push_back(0);
    }

    _items.push_back(Item(v, c));
    _usedItems.insert(c);
    _currentMin = _items.back().total();
  }

  std::vector<T1> next()
  {
    if (_itemsRet.empty())
    {
      generateMore();

      if (_itemsRet.empty())
        return std::vector<T1>();
    }

    Item item = _itemsRet.front();
    _itemsRet.pop();
    return item.values();
  }

private:
  void generateMore()
  {
    std::vector<Item> newItems;
    typename std::vector<Item>::iterator itemsIt = _items.begin();
    typename std::vector<Item>::iterator itemsEnd = _items.end();

    for (; itemsIt != itemsEnd; ++itemsIt)
    {
      Item& item = *itemsIt;

      if (item.total() == _currentMin)
      {
        _itemsRet.push(item);
        coords_type coords = item.coords();
        coords_type plusCoords;

        for (size_t axis = 0; axis < _dim; ++axis)
          plusCoords.push_back(coords[axis] + 1);

        for (size_t axis = 0; axis < _dim; ++axis)
        {
          if (plusCoords[axis] < _elems[axis].size())
          {
            values_type v;
            coords_type c;

            for (size_t ax = 0; ax < _dim; ++ax)
            {
              size_t coord = (ax == axis) ? plusCoords[ax] : coords[ax];
              v.push_back(_elems[ax][coord]);
              c.push_back(coord);
            }

            if (_usedItems.find(c) == _usedItems.end())
            {
              newItems.push_back(Item(v, c));
              _usedItems.insert(c);
            }
          }
        }
      }
      else
      {
        newItems.push_back(item);
      }
    }

    std::sort(newItems.begin(), newItems.end());
    _items.swap(newItems);

    if (!_items.empty())
      _currentMin = _items.at(0).total();
    else
      _currentMin = -1;
  }

private:
  const input_type& _elems;
  const size_t _dim;
  double _currentMin;
  std::vector<Item> _items;
  std::queue<Item> _itemsRet;
  std::set<coords_type> _usedItems;
};

template <typename T1, typename T2>
class MultiDimensionalPQ<T1*, T2>
{
  typedef T2 compare_type;
  typedef std::vector<T1*> values_type;
  typedef std::vector<size_t> coords_type;

  class Item
  {
  public:
    Item(const values_type& values, const coords_type& coords)
      : _values(values), _coords(coords), _total(0.0)
    {
      for (const auto value : _values)
        _total += value->queueRank();
    }

    // These are for use in vector
    Item() : _total(0.0) {}
    Item(const Item& other) : _values(other._values), _coords(other._coords), _total(other._total)
    {
    }

    const values_type& values() const { return _values; }
    const coords_type& coords() const { return _coords; }
    compare_type total() const { return _total; }
    bool operator<(const Item& other) const { return _total < other._total; }

  private:
    values_type _values;
    coords_type _coords;
    compare_type _total;
  };

public:
  typedef std::vector<T1*> dimension_type;
  typedef std::vector<dimension_type*> input_type;

  MultiDimensionalPQ(input_type& elems) : _elems(elems), _dim(elems.size()), _currentMin(-1.0)
  {
    for (size_t axis = 0; axis < _dim; ++axis)
    {
      if (_elems[axis]->empty())
        return;
    }

    values_type v;
    coords_type c;

    for (size_t axis = 0; axis < _dim; ++axis)
    {
      v.push_back((*_elems[axis])[0]);
      c.push_back(0);
    }

    _items.push_back(Item(v, c));
    _usedItems.insert(c);
    _currentMin = _items.back().total();
  }

  std::vector<T1*> next()
  {
    if (_itemsRet.empty())
    {
      generateMore();

      if (_itemsRet.empty())
        return std::vector<T1*>();
    }

    Item item = _itemsRet.front();
    _itemsRet.pop();
    return item.values();
  }

private:
  void generateMore()
  {
    std::vector<Item> newItems;
    typename std::vector<Item>::iterator itemsIt = _items.begin();
    typename std::vector<Item>::iterator itemsEnd = _items.end();

    for (; itemsIt != itemsEnd; ++itemsIt)
    {
      Item& item = *itemsIt;

      if (item.total() == _currentMin)
      {
        _itemsRet.push(item);
        coords_type coords = item.coords();
        coords_type plusCoords;

        for (size_t axis = 0; axis < _dim; ++axis)
          plusCoords.push_back(coords[axis] + 1);

        for (size_t axis = 0; axis < _dim; ++axis)
        {
          if (plusCoords[axis] < _elems[axis]->size())
          {
            values_type v;
            coords_type c;

            for (size_t ax = 0; ax < _dim; ++ax)
            {
              size_t coord = (ax == axis) ? plusCoords[ax] : coords[ax];
              v.push_back((*_elems[ax])[coord]);
              c.push_back(coord);
            }

            if (_usedItems.find(c) == _usedItems.end())
            {
              newItems.push_back(Item(v, c));
              _usedItems.insert(c);
            }
          }
        }
      }
      else
      {
        newItems.push_back(item);
      }
    }

    std::sort(newItems.begin(), newItems.end());
    _items.swap(newItems);

    if (!_items.empty())
      _currentMin = _items.at(0).total();
    else
      _currentMin = -1;
  }

private:
  input_type& _elems;
  const size_t _dim;
  double _currentMin;
  std::vector<Item> _items;
  std::queue<Item> _itemsRet;
  std::set<coords_type> _usedItems;
};
}

