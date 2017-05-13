// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include <boost/optional.hpp>

#include <map>
#include <queue>
#include <set>
#include <vector>

#include "Util/BranchPrediction.h"

namespace tax
{
/*
 *  Collects multiple values for different keys
 *  and directed edges between keys. Then returnes
 *  values in 'reversed topologicl order' of keys,
 *  i.e. values for keys wiht least successors count go first.
 */
template <typename K, typename V>
class TopologicalOrderer
{
public:
  typedef std::vector<V> Values;
  struct Node
  {
    Node() : successorsCount(0) {}

    Values values;
    std::set<K> predecessors;
    std::set<K> successors;
    size_t successorsCount;
  };
  typedef std::map<K, Node> ValuesMap;

  TopologicalOrderer() : _isCommited(false) {}

  // Return true if value was added. One cannot add value after calling commit()
  bool addValue(const K& key, const V& value)
  {
    if (_isCommited)
      return false;

    _nodes[key].values.push_back(value);
    return true;
  }

  // Return true if edge was added. One cannot add edge after calling commit()
  bool addEdge(const K& from, const K& to)
  {
    if (_isCommited)
      return false;

    _nodes[from].successors.insert(to);

    return true;
  }

  template <typename Matcher>
  void commit(const Matcher&);

  boost::optional<const Values&> getNext()
  {
    if (!_isCommited || _noInbound.empty()) // if not commited do not return any values
      return boost::none;

    const Node& resultNode = _nodes[_noInbound.front()];
    _noInbound.pop();
    for (const K& predecessorKey : resultNode.predecessors)
    {
      if (--_nodes[predecessorKey].successorsCount == 0)
        _noInbound.push(predecessorKey);
    }

    return resultNode.values;
  }

private:
  bool _isCommited;
  ValuesMap _nodes;
  std::queue<K> _noInbound;

  void createEdge(const K& from, const K& to)
  {
    if (_nodes[to].predecessors.insert(from).second)
      _nodes[from].successorsCount++;
  }
};

template <typename K, typename V>
template <typename Matcher>
void
TopologicalOrderer<K, V>::commit(const Matcher& matcher)
{
  _isCommited = true;
  for (const typename ValuesMap::value_type& node : _nodes)
  {
    if (node.second.values.empty())
      continue;
    for (const K& successorKey : node.second.successors)
    {
      if (matcher.isSimple(successorKey))
      {
        const typename ValuesMap::const_iterator it = _nodes.find(successorKey);
        if (it != _nodes.end() && !it->second.values.empty())
          createEdge(node.first, successorKey);
        continue;
      }

      for (const typename ValuesMap::value_type& successor : _nodes)
      {
        if (matcher(successorKey, successor.first) && !successor.second.values.empty())
          createEdge(node.first, successor.first);
      }
    }
    if (LIKELY(node.second.successorsCount == 0))
      _noInbound.push(node.first);
  }
}
}
