//----------------------------------------------------------------------------
//
//     File:           ORACLEDBRow.cpp
//     Description:
//     Created:
//     Authors:
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
//----------------------------------------------------------------------------

#include "DBAccess/ORACLEDBRow.h"

#include "Common/Logger.h"
#include "Common/TSEException.h"
#include "DBAccess/ORACLEAdapter.h"
#include "DBAccess/ORACLEDBResultSet.h"
#include "DBAccess/SQLQuery.h"

#include <iostream>
#include <limits>
#include <string>

namespace tse
{
static Logger
logger("atseintl.DBAccess.ORACLEDBRow");

int
ORACLEDBRow::getInt(int colIdx) const
{
  if (isFieldNull(colIdx))
    return 0;

  return _resultSet->getInt(colIdx);
}

long int
ORACLEDBRow::getLong(int colIdx) const
{
  if (isFieldNull(colIdx))
    return 0;

  return _resultSet->getLong(colIdx);
}

long long int
ORACLEDBRow::getLongLong(int colIdx) const
{
  if (isFieldNull(colIdx))
    return 0;

  return _resultSet->getLongLong(colIdx);
}

const char*
ORACLEDBRow::getString(int colIdx) const
{
  if (isFieldNull(colIdx))
    return "";

  const char* value = _resultSet->getString(colIdx);

  if (value == nullptr)
    return "";

  if (value[0] == ' ' && value[1] == 0)
    return "";

  return value;
}

char
ORACLEDBRow::getChar(int colIdx) const
{
  if (isFieldNull(colIdx))
    return ' ';

  char value = _resultSet->getChar(colIdx);

  if (value == 0)
    return ' ';
  return value;
}

DateTime
ORACLEDBRow::getDate(int colIdx) const
{
  if (isFieldNull(colIdx))
    return boost::date_time::neg_infin;

  return getDateTime(colIdx);
}

bool
ORACLEDBRow::isNull(int colIdx) const
{
  if (colIdx < 0 || colIdx > _numFields)
  {
    if (_queryObject)
    {
      LOG4CXX_ERROR(logger,
                    "ORACLEDBRow - Invalid Column Index (QueryName: "
                        << _queryObject->getQueryName() << ", Column: " << colIdx << "). "
                        << _queryObject->getSQL());
    }
    else
    {
      LOG4CXX_ERROR(logger,
                    "ORACLEDBRow - Invalid Column Index (QueryID: "
                        << _qryID << ", Column: " << colIdx << "). " << *_sqlStatement);
    }
    colIdx = 0;
  }

  return _resultSet->isNull(colIdx);
}

bool
ORACLEDBRow::isFieldNull(int colIdx) const
{
  if (colIdx < 0 || colIdx > _numFields)
  {
    if (_queryObject)
    {
      LOG4CXX_ERROR(logger,
                    "ORACLEDBRow - Invalid Column Index (QueryName: "
                        << _queryObject->getQueryName() << ", Column: " << colIdx << "). "
                        << _queryObject->getSQL());
    }
    else
    {
      LOG4CXX_ERROR(logger,
                    "ORACLEDBRow - Invalid Column Index (QueryID: "
                        << _qryID << ", Column: " << colIdx << "). " << *_sqlStatement);
    }
    colIdx = 0;
  }

  return _resultSet->isNull(colIdx);
}

void
ORACLEDBRow::init(ORACLEAdapter* dbAdapt, ORACLEDBResultSet* resultSet, const SQLQuery* queryObject)
{
  _dbAdapt = dbAdapt;
  _resultSet = resultSet;
  _qryID = -1;
  _sqlStatement = queryObject;
  _queryObject = queryObject;
  _numFields = 0;
}

void
ORACLEDBRow::setFieldCount(int numFields)
{
  _numFields = numFields;
}

DateTime
ORACLEDBRow::getDateTime(int colIdx) const
{
  return _resultSet->getDateTime(colIdx);
}
}
