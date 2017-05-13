//----------------------------------------------------------------------------
//
//  File:           NonSerializable
//  Description:    Abstract interface for *NON* serializable objects.
//  Created:        02/02/2009
//  Authors:        John Watilo
//
//  Updates:
//
// © 2009, Sabre Inc.  All rights reserved.  This software/documentation is
// the confidential and proprietary product of Sabre Inc. Any unauthorized
// use, reproduction, or transfer of this software/documentation, in any
// medium, or incorporation of this software/documentation into any system
// or publication, is strictly prohibited
//
//----------------------------------------------------------------------------

#pragma once

namespace tse
{
class NonSerializable
{
public:
  NonSerializable() {}
  NonSerializable(const NonSerializable& rhs) {}
  virtual ~NonSerializable() {}
  virtual void destroyChildren() {}

  virtual bool operator==(const NonSerializable& rhs) const { return false; }
  virtual bool operator!=(const NonSerializable& rhs) const { return !(*this == rhs); }

}; // class NonSerializable

}; // namespace tse

