//----------------------------------------------------------------------------
//
//     File:           BoundParameterTypes.cpp
//     Description:
//     Created:        07/02/2009
//     Authors:        Andrew Ahmad
//
//     Updates:
//
//     Copyright @2009, Sabre Inc.  All rights reserved.
//         This software/documentation is the confidential and proprietary
//         product of Sabre Inc.
//         Any unauthorized use, reproduction, or transfer of this
//         software/documentation, in any medium, or incorporation of this
//         software/documentation into any system or publication,
//         is strictly prohibited.
//
//----------------------------------------------------------------------------

#include "DBAccess/BoundParameterTypes.h"

#include "DBAccess/ParameterBinder.h"

using namespace DBAccess;

BoundInteger::~BoundInteger() {}

void
BoundInteger::bind(const ParameterBinder& binder)
{
  binder.bindParameter(_value, index());
}

BoundLong::~BoundLong() {}

void
BoundLong::bind(const ParameterBinder& binder)
{
  binder.bindParameter(_value, index());
}

BoundFloat::~BoundFloat() {}

void
BoundFloat::bind(const ParameterBinder& binder)
{
  binder.bindParameter(_value, index());
}

BoundString::~BoundString() {}

void
BoundString::bind(const ParameterBinder& binder)
{
  binder.bindParameter(_value, index());
}

BoundDate::~BoundDate() {}

void
BoundDate::bind(const ParameterBinder& binder)
{
  binder.bindParameter(_value, index(), true); // dateOnly = true
}

BoundDateTime::~BoundDateTime() {}

void
BoundDateTime::bind(const ParameterBinder& binder)
{
  binder.bindParameter(_value, index());
}
