//----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "Common/RoutingCycleDetector.h"
#include "Common/Logger.h"
#include "DBAccess/RoutingMap.h"

namespace tse
{
static Logger
logger("atseintl.Common.RoutingCycleDetector");

RoutingCycleDetector::RoutingCycleDetector(const std::vector<RoutingMap*>& map)
{
  uint32_t maxSeqNo = 0;
  for (const RoutingMap* node : map)
    maxSeqNo = std::max(maxSeqNo, uint32_t(node->lnkmapsequence()));

  _map.assign(maxSeqNo + 1, nullptr);
  _nodeState.assign(maxSeqNo + 1, NOT_VISITED);

  for (const RoutingMap* node : map)
    _map[node->lnkmapsequence()] = node;
}

bool
RoutingCycleDetector::validate()
{
  for (const RoutingMap* node : _map)
  {
    if (!node)
      continue;
    if (!checkNode(node))
      return false;
  }

  return true;
}

#define LOG_RTG_KEY(node)                                                                          \
  (node)->vendor() << "|" << (node)->carrier() << "|" << (node)->routingTariff() << "|"            \
                   << (node)->routing() << "|" << (node)->effDate().toSimpleString()

bool
RoutingCycleDetector::checkAltAltCycle(const RoutingMap* node)
{
  const RoutingMap* slow = _map[node->nextLocNo()];
  const RoutingMap* fast = slow;

  while (slow && fast && _map[fast->altLocNo()])
  {
    slow = _map[slow->altLocNo()];
    fast = _map[_map[fast->altLocNo()]->altLocNo()];

    if (slow == fast)
    {
      LOG4CXX_ERROR(logger,
                    "Alt-alt cycle detected in routing map: " << LOG_RTG_KEY(node) << ", node id: "
                                                              << node->lnkmapsequence());
      return false;
    }
  }

  return true;
}

bool
RoutingCycleDetector::checkNode(const RoutingMap* node, uint32_t parent)
{
  if (!node)
    return false; // sth is wrong with map's edges

  const uint32_t id = node->lnkmapsequence();

  if (_nodeState[id] == ENTERED)
  {
    LOG4CXX_ERROR(logger,
                  "A cycle detected in routing map: " << LOG_RTG_KEY(node) << ", edge: " << parent
                                                      << "-" << id);
    return false;
  }

  if (_nodeState[id] == EXITED)
    return true; // the node has already been checked

  // Now check if the node is well-formed (if there's no alt-alt cycle)
  if (!checkAltAltCycle(node))
    return false;

  // Process children recursively
  const RoutingMap* child = _map[node->nextLocNo()];
  _nodeState[id] = ENTERED;

  while (child)
  {
    if (!checkNode(child, id))
      return false;
    child = _map[child->altLocNo()];
  }

  _nodeState[id] = EXITED;
  return true;
}
}
