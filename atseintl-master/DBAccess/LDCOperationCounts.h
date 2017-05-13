//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#pragma once


namespace tse
{
struct LDCOperationCounts
{
  LDCOperationCounts()
    : goodWrites(0), badWrites(0), goodRemoves(0), badRemoves(0), goodClears(0), badClears(0)
  {
  }

  bool empty() const
  {
    return ((goodWrites == 0) && (badWrites == 0) && (goodRemoves == 0) && (badRemoves == 0) &&
            (goodClears == 0) && (badClears == 0));
  }

  uint32_t goodWrites;
  uint32_t badWrites;
  uint32_t goodRemoves;
  uint32_t badRemoves;
  uint32_t goodClears;
  uint32_t badClears;
};

typedef std::map<std::string, LDCOperationCounts, std::less<std::string> > LDCOPCOUNTS;

} // namespace tse

