//-------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once

#include "Common/DateTime.h"

namespace tse
{
class Row
{
public:
  virtual ~Row() = default;

  virtual int getInt(int columnIndex) const = 0;
  virtual long int getLong(int columnIndex) const = 0;
  virtual long long int getLongLong(int columnIndex) const = 0;
  virtual const char* getString(int columnIndex) const = 0;
  virtual char getChar(int columnIndex) const = 0;
  virtual DateTime getDate(int columnIndex) const = 0;
  virtual bool isNull(int columnIndex) const = 0;
};
};
