//----------------------------------------------------------------------------
//
//     File:           ORACLEDefineBufferCollection.h
//     Description:
//     Created:        10/12/2009
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
//----------------------------------------------------------------------------

#pragma once

#include "Common/DateTime.h"
#include "Common/Logger.h"
#include "DBAccess/ORACLEDefineBufferTypes.h"

#include <boost/thread/mutex.hpp>
#include <map>
#include <stack>
#include <vector>


namespace tse
{
class ORACLEAdapter;

class ORACLEDefineBufferCollection
{
public:
  ORACLEDefineBufferCollection(const char* queryName) : _queryName(queryName) {}

  ~ORACLEDefineBufferCollection();

  bool createDefineBuffers(ORACLEAdapter& adapter,
                           uint16_t arrayFetchSize,
                           uint16_t maxBufferStackSize,
                           int32_t& oracleStatusCode);

  void clearDefineBuffers(ORACLEAdapter& adapter, int32_t& oracleStatusCode);

  void freeDefineBuffers(ORACLEAdapter& adapter);

  bool setOracleDefines(ORACLEAdapter& adapter, int32_t& oracleStatusCode);

  int32_t getBufferCount() const;
  uint16_t getDataType(int index) const;
  int32_t getSize(int index) const;
  uint16_t getSizeInChars(int index) const;
  uint16_t getArrayFetchSize() const { return _arrayFetchSize; }
  uint16_t getMaxBufferStackSize() const { return _maxBufferStackSize; }
  const std::string& getQueryName() const { return _queryName; }

  int getInt(int columnIndex, uint16_t rowIndex);
  long int getLong(int columnIndex, uint16_t rowIndex);
  long long int getLongLong(int columnIndex, uint16_t rowIndex);
  const char* getString(int columnIndex, uint16_t rowIndex);
  char getChar(int columnIndex, uint16_t rowIndex);
  DateTime getDateTime(int columnIndex, uint16_t rowIndex, ORACLEAdapter& adapter);
  bool isNull(int columnIndex, uint16_t rowIndex);

  static ORACLEDefineBufferCollection* findDefineBufferCollection(const char* queryName);

  static ORACLEDefineBufferCollection* createDefineBufferCollection(const char* queryName,
                                                                    ORACLEAdapter& adapter,
                                                                    uint16_t arrayFetchSize,
                                                                    uint16_t maxBufferStackSize,
                                                                    int32_t& oracleStatusCode);

  static void
  releaseDefineBufferCollection(ORACLEDefineBufferCollection*& target, ORACLEAdapter& adapter);

private:
  void deleteDefineBuffers();
  void addDefineBuffer(ORACLEDefineBuffer* defineBuffer);
  const ORACLEDefineBuffer* getDefineBuffer(uint16_t index) const;

  std::vector<ORACLEDefineBuffer*> _defineBuffers;

  std::string _queryName;
  uint16_t _arrayFetchSize = 1;
  uint16_t _maxBufferStackSize = 10;

  static boost::mutex _mutex;

  using BufferStack = std::stack<ORACLEDefineBufferCollection*>;
  using BufferStackMap = std::map<std::string, BufferStack>;

  static BufferStackMap _bufferCache;

  static log4cxx::LoggerPtr& getLogger();
  static log4cxx::LoggerPtr _logger;

}; // class ORACLEDefineBufferCollection

} // namespace tse

