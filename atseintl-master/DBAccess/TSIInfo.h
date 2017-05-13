//----------------------------------------------------------------------------
//
//  File:           TSIInfo.h
//  Description:    TSI data
//  Created:        5/6/2004
//  Authors:        Andrew Ahmad
//
//  Updates:
//
// � 2004, Sabre Inc.  All rights reserved.  This software/documentation is
// the confidential and proprietary product of Sabre Inc. Any unauthorized
// use, reproduction, or transfer of this software/documentation, in any
// medium, or incorporation of this software/documentation into any system
// or publication, is strictly prohibited
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseStringTypes.h"
#include "DBAccess/Flattenizable.h"

#include <vector>

#include <stdint.h>

namespace tse
{

/**
 * @class TSIInfo
 *
 * @brief Defines a wrapper for a TSI record.
 */

class TSIInfo
{
public:
  enum TSIMatchCriteria
  {
    STOP_OVER = 'S',
    INBOUND = 'I',
    OUTBOUND = 'O',
    FURTHEST = 'F',
    DOMESTIC = 'D',
    ONE_COUNTRY = 'C',
    INTERNATIONAL = 'N',
    GATEWAY = 'G',
    ORIG_GATEWAY = 'R',
    DEST_GATEWAY = 'E',
    TRANS_ATLANTIC = 'A',
    TRANS_PACIFIC = 'P',
    TRANS_OCEANIC = 'T',
    INTERCONTINENTAL = 'L',
    OVER_WATER = 'W',
    INTL_DOM_TRANSFER = 'X'
  };

  TSIInfo()
    : _tsi(0),
      _geoRequired(' '),
      _geoNotType(' '),
      _geoOut(' '),
      _geoItinPart(' '),
      _geoCheck(' '),
      _loopDirection(' '),
      _loopOffset(0),
      _loopToSet(0),
      _loopMatch(' '),
      _scope(' '),
      _type(' ')
  {
  }

  ~TSIInfo() {}

  int16_t& tsi() { return _tsi; }
  const int16_t& tsi() const { return _tsi; }

  std::string& description() { return _description; }
  const std::string& description() const { return _description; }

  char& geoRequired() { return _geoRequired; }
  const char& geoRequired() const { return _geoRequired; }

  char& geoNotType() { return _geoNotType; }
  const char& geoNotType() const { return _geoNotType; }

  char& geoOut() { return _geoOut; }
  const char& geoOut() const { return _geoOut; }

  char& geoItinPart() { return _geoItinPart; }
  const char& geoItinPart() const { return _geoItinPart; }

  char& geoCheck() { return _geoCheck; }
  const char& geoCheck() const { return _geoCheck; }

  char& loopDirection() { return _loopDirection; }
  const char& loopDirection() const { return _loopDirection; }

  int16_t& loopOffset() { return _loopOffset; }
  const int16_t& loopOffset() const { return _loopOffset; }

  int16_t& loopToSet() { return _loopToSet; }
  const int16_t& loopToSet() const { return _loopToSet; }

  char& loopMatch() { return _loopMatch; }
  const char& loopMatch() const { return _loopMatch; }

  char& scope() { return _scope; }
  const char& scope() const { return _scope; }

  char& type() { return _type; }
  const char& type() const { return _type; }

  std::vector<TSIMatchCriteria>& matchCriteria() { return _matchCriteria; }
  const std::vector<TSIMatchCriteria>& matchCriteria() const { return _matchCriteria; }

  bool operator==(const TSIInfo& rhs) const
  {
    return ((_tsi == rhs._tsi) && (_description == rhs._description) &&
            (_geoRequired == rhs._geoRequired) && (_geoNotType == rhs._geoNotType) &&
            (_geoOut == rhs._geoOut) && (_geoItinPart == rhs._geoItinPart) &&
            (_geoCheck == rhs._geoCheck) && (_loopDirection == rhs._loopDirection) &&
            (_loopOffset == rhs._loopOffset) && (_loopToSet == rhs._loopToSet) &&
            (_loopMatch == rhs._loopMatch) && (_scope == rhs._scope) && (_type == rhs._type) &&
            (_matchCriteria == rhs._matchCriteria));
  }

  static void dummyData(TSIInfo& obj)
  {
    obj._tsi = 1;
    obj._description = "aaaaaaaa";
    obj._geoRequired = 'A';
    obj._geoNotType = 'B';
    obj._geoOut = 'C';
    obj._geoItinPart = 'D';
    obj._geoCheck = 'E';
    obj._loopDirection = 'F';
    obj._loopOffset = 2;
    obj._loopToSet = 3;
    obj._loopMatch = 'G';
    obj._scope = 'H';
    obj._type = 'I';

    obj._matchCriteria.push_back(STOP_OVER);
    obj._matchCriteria.push_back(INBOUND);
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _tsi);
    FLATTENIZE(archive, _description);
    FLATTENIZE(archive, _geoRequired);
    FLATTENIZE(archive, _geoNotType);
    FLATTENIZE(archive, _geoOut);
    FLATTENIZE(archive, _geoItinPart);
    FLATTENIZE(archive, _geoCheck);
    FLATTENIZE(archive, _loopDirection);
    FLATTENIZE(archive, _loopOffset);
    FLATTENIZE(archive, _loopToSet);
    FLATTENIZE(archive, _loopMatch);
    FLATTENIZE(archive, _scope);
    FLATTENIZE(archive, _type);
    FLATTENIZE_ENUM_VECTOR(archive, _matchCriteria);
  }

private:
  int16_t _tsi;
  std::string _description;
  char _geoRequired;
  char _geoNotType;
  char _geoOut;
  char _geoItinPart;
  char _geoCheck;
  char _loopDirection;
  int16_t _loopOffset;
  int16_t _loopToSet;
  char _loopMatch;
  char _scope;
  char _type;
  std::vector<TSIMatchCriteria> _matchCriteria;

}; // class TSIInfo

} // namespace tse

