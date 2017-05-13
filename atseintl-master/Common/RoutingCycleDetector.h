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

#pragma once

#include <cstdint>
#include <vector>

//TODO Remove with ROUTING_CYCLE_DETECTOR_HARDENING
namespace tse
{
class RoutingMap;

class RoutingCycleDetector
{
  enum NodeState : uint8_t
  {
    NOT_VISITED,
    ENTERED,
    EXITED
  };

public:
  RoutingCycleDetector(const std::vector<RoutingMap*>& map);
  bool validate(); // return false if a cycle exists

private:
  bool checkAltAltCycle(const RoutingMap* node);
  bool checkNode(const RoutingMap* node, uint32_t parent = 0);

  std::vector<const RoutingMap*> _map;
  std::vector<NodeState> _nodeState;
};
}
