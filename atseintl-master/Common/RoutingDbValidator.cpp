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

#include "Common/RoutingDbValidator.h"
#include "Common/Logger.h"
#include "DBAccess/RoutingMap.h"

#include <cstdint>

namespace tse
{
static Logger
logger("atseintl.Common.RoutingDbValidator");

namespace RoutingDbValidator
{
namespace
{
enum NodeState : uint8_t
{
  NOT_VISITED,
  ENTERED,
  EXITED
};

struct Context
{
  std::vector<const RoutingMap*> map;
  std::vector<NodeState> nodeState;
};

#define LOG_RTG_KEY(node)                                                                       \
  (node).vendor() << "|" << (node).carrier() << "|" << (node).routingTariff() << "|"            \
                  << (node).routing() << "|" << (node).effDate().toSimpleString()

bool
validateFastPath(const std::vector<RoutingMap*>& map)
{
  // If all edges are of the form (X, Y) st.
  // - X < Y
  // - both X and Y are valid node IDs
  // Then we can be sure that the map doesn't contain cycles and can exit immediately
  // without allocating memory for intermediate data.

  const int lastNodeId = int(map.size());
  int currentNodeId = 0;

  for (const RoutingMap* node : map)
  {
    ++currentNodeId;

    if (node->loc1No() != currentNodeId)
      return false;

    const int childId = node->nextLocNo();

    if (childId && (childId > lastNodeId || childId <= currentNodeId))
      return false;

    const int brotherId = node->altLocNo();

    if (brotherId && (brotherId > lastNodeId || brotherId <= currentNodeId))
      return false;
  }

  return true;
}

bool
initialize(const std::vector<RoutingMap*>& map, Context& ctx)
{
  int lastNodeId = 0;
  for (const RoutingMap* node : map)
    lastNodeId = std::max(lastNodeId, node->loc1No());

  ctx.map.assign(lastNodeId + 1, nullptr);
  ctx.nodeState.assign(lastNodeId + 1, NOT_VISITED);

  for (const RoutingMap* node : map)
    ctx.map[node->loc1No()] = node;

  const auto checkNodeId = [&](const int id) { return !id || (id <= lastNodeId && ctx.map[id]); };

  return std::all_of(map.begin(), map.end(), [&](const RoutingMap* node)
  {
    return checkNodeId(node->nextLocNo()) && checkNodeId(node->altLocNo());
  });
}

bool
checkAltAltCycle(const Context& ctx, const RoutingMap& node)
{
  const RoutingMap* slow = ctx.map[node.nextLocNo()];
  const RoutingMap* fast = slow;

  while (slow && fast && ctx.map[fast->altLocNo()])
  {
    slow = ctx.map[slow->altLocNo()];
    fast = ctx.map[ctx.map[fast->altLocNo()]->altLocNo()];

    if (slow == fast)
    {
      LOG4CXX_ERROR(logger,
                    "Alt-alt cycle detected in routing map: " << LOG_RTG_KEY(node) << ", node id: "
                                                              << node.loc1No());
      return false;
    }
  }

  return true;
}

bool
checkNode(Context& ctx, const RoutingMap* node, uint32_t parent = 0)
{
  if (!node)
    return false; // sth is wrong with map's edges

  const uint32_t id = node->loc1No();

  if (ctx.nodeState[id] == ENTERED)
  {
    LOG4CXX_ERROR(logger,
                  "A cycle detected in routing map: " << LOG_RTG_KEY(*node) << ", edge: " << parent
                                                      << "-" << id);
    return false;
  }

  if (ctx.nodeState[id] == EXITED)
    return true; // the node has already been checked

  // Now check if the node is well-formed (if there's no alt-alt cycle)
  if (!checkAltAltCycle(ctx, *node))
    return false;

  // Process children recursively
  const RoutingMap* child = ctx.map[node->nextLocNo()];
  ctx.nodeState[id] = ENTERED;

  while (child)
  {
    if (!checkNode(ctx, child, id))
      return false;
    child = ctx.map[child->altLocNo()];
  }

  ctx.nodeState[id] = EXITED;
  return true;
}
} // ns

bool
validate(const std::vector<RoutingMap*>& map)
{
  if (validateFastPath(map))
    return true;

  Context ctx;

  if (!initialize(map, ctx))
    return false;

  for (const RoutingMap* node : ctx.map)
  {
    if (!node)
      continue;
    if (!checkNode(ctx, node))
      return false;
  }

  return true;
}
} // ns RoutingDbValidator
} // ns tse
