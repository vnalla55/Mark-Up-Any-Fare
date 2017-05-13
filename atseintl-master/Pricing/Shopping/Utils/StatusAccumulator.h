//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

namespace tse
{

namespace utils
{

struct GreaterIntegerWinsPolicy
{
  template <typename T>
  static T filter(const T& oldStatus, const T& newStatus)
  {
    if (static_cast<int>(newStatus) > static_cast<int>(oldStatus))
    {
      return newStatus;
    }
    return oldStatus;
  }
};

struct ZeroIntegerInitPolicy
{
  template <typename T>
  static void init(T& status)
  {
    status = static_cast<T>(0);
  }
};

// Stores and allows to update a status variable
// deciding at each update if the current status
// should be overwritten by the supplied one.
// This way, it provides abstraction for
// status prioritization.
template <typename T,
          typename InitPolicy = ZeroIntegerInitPolicy,
          typename UpdatePolicy = GreaterIntegerWinsPolicy>
class StatusAccumulator
{
public:
  StatusAccumulator() { InitPolicy::init(_status); }

  void updateStatus(const T& status) { _status = UpdatePolicy::filter(_status, status); }

  void forceStatus(const T& status) { _status = status; }

  const T& getStatus() const { return _status; }

private:
  T _status;
};

} // namespace utils

} // namespace tse

