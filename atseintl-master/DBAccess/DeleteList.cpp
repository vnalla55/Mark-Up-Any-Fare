//----------------------------------------------------------------------------
//
//  File:           DeleteList.cpp
//
//  Description:    Implementation of DeleteList class.
//
//  Updates:
//
//  Copyright Sabre 2008
//
//  The copyright to the computer program(s) herein
//  is the property of Sabre.
//  The program(s) may be used and/or copied only with
//  the written permission of Sabre or in accordance
//  with the terms and conditions stipulated in the
//  agreement/contract under which the program(s)
//  have been supplied.
//
//----------------------------------------------------------------------------
#include "DBAccess/DeleteList.h"

namespace tse
{
Logger
DeleteList::_logger("atseintl.DBAccess.DeleteList");

unsigned int DeleteList::_debugLoggingFlags = 0;
size_t DeleteList::_reallocCount = 0;
size_t DeleteList::_destroyCount = 0;
size_t DeleteList::_importCount = 0;
size_t DeleteList::_maxPointers = 0;
size_t DeleteList::_maxSmartPointers = 0;
size_t DeleteList::_singleCount = 0;
size_t DeleteList::_smallCount = 0;
size_t DeleteList::_mediumCount = 0;
size_t DeleteList::_largeCount = 0;
size_t DeleteList::_oneContainerCount = 0;
size_t DeleteList::_oneContainerUsedCount = 0;
double DeleteList::_reallocCPU = 0.0;
double DeleteList::_reallocElapsed = 0.0;
size_t _deletelist_counts[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
size_t _deletelist_sizes[16] = { 0,   1,   2,   4,    8,    16,   32,   64,
                                 128, 256, 512, 1024, 2048, 4096, 8192, 16384 };

DeleteList::DeleteList(size_t numContainers, Memory::CompositeManager* parentManager)
  : _numContainers(numContainers), _ptrs(&_ptrsSingle), _smartPtrs(&_smartPtrsSingle)
{
  if (_numContainers > 1)
  {
    _ptrs = new PtrList[_numContainers];
    _smartPtrs = new SmartPtrList[_numContainers];
  }

  if (parentManager)
  {
    for (size_t i = 0; i < _numContainers; ++i)
    {
      _ptrs[i].enableMemoryTracking();
      parentManager->registerManager(&_ptrs[i]);

      _smartPtrs[i].enableMemoryTracking();
      parentManager->registerManager(&_smartPtrs[i]);
    }
  }
}

DeleteList::~DeleteList()
{
  if (UNLIKELY(_debugLoggingFlags & DEBUG_STATISTICS))
  {
    _destroyCount++;
    logContainerStats("statistics");
  }
  // IF the statistics are being collected, log the usage and allocation numbers
  if (UNLIKELY(_debugLoggingFlags & DEBUG_DESTRUCTION))
  {
    tse::StopWatch sw; // used for collecting timing statistics
    logContainerDetails("destruction");
    // START the stopwatch so we can report time for list deletion
    sw.start();
    deallocate();
    if (_ptrs != &_ptrsSingle)
    {
      delete[] _ptrs;
    }
    if (_smartPtrs != &_smartPtrsSingle)
    {
      delete[] _smartPtrs;
    }
    sw.stop();
    LOG4CXX_DEBUG(_logger,
                  "destruction elapsed time: " << sw.elapsedTime()
                                               << " seconds. CPU time: " << sw.cpuTime());
  }
  else
  {
    deallocate();
    if (_ptrs != &_ptrsSingle)
    {
      delete[] _ptrs;
    }
    if (_smartPtrs != &_smartPtrsSingle)
    {
      delete[] _smartPtrs;
    }
  }
}

void
DeleteList::logContainerDetails(const std::string& logPrefix)
{
  // IF the logger will be used
  if (IS_DEBUG_ENABLED(_logger))
  {
    size_t numPointers = 0;
    size_t numSmartPointers = 0;
    size_t pointerCapacity = 0;
    size_t smartPointerCapacity = 0;
    size_t numPointerContainersUsed = 0;
    size_t numSmartPointerContainersUsed = 0;

    for (size_t currContainer = 0; currContainer < _numContainers; currContainer++)
    {
      if (_ptrs[currContainer].size() > 0)
        numPointerContainersUsed++;
      if (_smartPtrs[currContainer].size() > 0)
        numSmartPointerContainersUsed++;
      numPointers += _ptrs[currContainer].size();
      numSmartPointers += _smartPtrs[currContainer].size();
      pointerCapacity += _ptrs[currContainer].capacity();
      smartPointerCapacity += _smartPtrs[currContainer].capacity();
    }
    LOG4CXX_DEBUG(_logger,
                  logPrefix << " NumContainers: " << _numContainers
                            << " PtrContainersUsed: " << numPointerContainersUsed
                            << " SmartPtrContainersUsed: " << numSmartPointerContainersUsed);
    LOG4CXX_DEBUG(_logger,
                  logPrefix << " NumPtrs: " << numPointers << " Capacity: " << pointerCapacity);
    LOG4CXX_DEBUG(_logger,
                  logPrefix << " NumSmartPtrs: " << numSmartPointers
                            << " Capacity: " << smartPointerCapacity);
  }
}

void
DeleteList::logContainerStats(const std::string& logPrefix)
{
  if (IS_DEBUG_ENABLED(_logger))
  {
    size_t numPointers = 0;
    size_t numPointerContainersUsed = 0;
    size_t numSmartPointerContainersUsed = 0;
    for (size_t currContainer = 0; currContainer < _numContainers; currContainer++)
    {
      size_t sz = _ptrs[currContainer].size();
      numPointers += sz;
      if (sz > _maxPointers)
        _maxPointers = sz;
      if (sz > 0)
        numPointerContainersUsed++;
      if (sz <= 1)
        _singleCount++;
      else if (sz <= 4)
        _smallCount++;
      else if (sz <= 16384)
        _mediumCount++;
      else
        _largeCount++;
      sz = _smartPtrs[currContainer].size();
      numPointers += sz;
      if (sz > _maxSmartPointers)
        _maxSmartPointers = sz;
      if (sz > 0)
        numSmartPointerContainersUsed++;
      if (sz <= 1)
        _singleCount++;
      else if (sz <= 4)
        _smallCount++;
      else if (sz <= 16384)
        _mediumCount++;
      else
        _largeCount++;
      if (sz <= 16384)
      {
        for (int i = 0; i < 16; i++)
        {
          if (sz <= _deletelist_sizes[i])
          {
            _deletelist_counts[i]++;
            break;
          }
        }
      }
    }
    if (numPointerContainersUsed <= 1 && numSmartPointerContainersUsed <= 1)
      _oneContainerUsedCount++;
    if (_numContainers <= 1)
      _oneContainerCount++;
    if (_numContainers > 1 && numPointers > 16384)
    {
      LOG4CXX_DEBUG(_logger,
                    logPrefix << " Reallocs: " << _reallocCount << " Destroy: " << _destroyCount
                              << " Imports: " << _importCount);
      LOG4CXX_DEBUG(_logger,
                    logPrefix << " maxPointers: " << _maxPointers
                              << " maxSmartPointers: " << _maxSmartPointers);
      LOG4CXX_DEBUG(_logger,
                    logPrefix << " size (0..1): " << _singleCount << " (2..4): " << _smallCount
                              << " (5..16384): " << _mediumCount << " (>16384): " << _largeCount);
      LOG4CXX_DEBUG(
          _logger,
          logPrefix << " size 0: " << _deletelist_counts[0] << " size 1: " << _deletelist_counts[1]
                    << " size 2: " << _deletelist_counts[2] << " size 4: " << _deletelist_counts[3]
                    << " size 8: " << _deletelist_counts[4] << " size 16: " << _deletelist_counts[5]
                    << " size 32: " << _deletelist_counts[6]
                    << " size 64: " << _deletelist_counts[7]);
      LOG4CXX_DEBUG(_logger,
                    logPrefix << " size 128: " << _deletelist_counts[8] << " size 256: "
                              << _deletelist_counts[9] << " size 512: " << _deletelist_counts[10]
                              << " size 1024: " << _deletelist_counts[11] << " size 2048: "
                              << _deletelist_counts[12] << " size 4096: " << _deletelist_counts[13]
                              << " size 8192: " << _deletelist_counts[14]
                              << " size 16384: " << _deletelist_counts[15]);
      std::string msg = logPrefix;
      msg += " Sizes: ";
      for (int i = 0; i < 16; i++)
      {
        char txt[256];
        sprintf(txt, " %ld=%ld", _deletelist_sizes[i], _deletelist_counts[i]);
        msg += txt;
      }
      LOG4CXX_DEBUG(_logger, msg);
      std::ostringstream os;
      for (int i = 0; i < 16; i++)
      {
        os << " " << _deletelist_sizes[i] << "=" << _deletelist_counts[i];
      }
      LOG4CXX_DEBUG(_logger, " Sizes:" << os.str());
    }
  }
}

void
DeleteList::import(DeleteList& another)
{
  // IF the statistics are being collected, log the usage and allocation numbers
  if (UNLIKELY(_debugLoggingFlags & DEBUG_IMPORT))
  {
    tse::StopWatch sw; // used for collecting timeing statistics
    another.logContainerDetails("import()");
    // START the stopwatch so we can report time for list deletion
    sw.start();
    for (size_t n = 0; n != another._numContainers; ++n)
    {
      const size_t ourn = n % _numContainers;
      // will cause a deadlock if two DeleteLists import from each other at the same time
      // but you'd have to be extremely stupid or malicious to do that
      for (const auto& elem : another._ptrs[n])
      {
        if (!elem.empty())
        {
          _ptrs[ourn].push_back(elem);
        }
      }
      _ptrs[ourn].increaseTotalMemory(another._ptrs[n].accurateTotalMemory());
      another._ptrs[n].clear();
    }
    sw.stop();
    LOG4CXX_DEBUG(_logger,
                  "import() elapsed time: " << sw.elapsedTime()
                                            << " seconds. CPU time: " << sw.cpuTime());
  }
  else
  {
    for (size_t n = 0; n != another._numContainers; ++n)
    {
      const size_t ourn = n % _numContainers;
      // will cause a deadlock if two DeleteLists import from each other at the same time
      // but you'd have to be extremely stupid or malicious to do that
      for (const auto& elem : another._ptrs[n])
      {
        if (!elem.empty())
        {
          _ptrs[ourn].push_back(elem);
        }
      }
      _ptrs[ourn].increaseTotalMemory(another._ptrs[n].accurateTotalMemory());
      another._ptrs[n].clear();
    }
  }
  if (UNLIKELY(_debugLoggingFlags & DEBUG_STATISTICS))
    _importCount++;
}

void
DeleteList::deallocate()
{
  for (size_t n = 0; n != _numContainers; ++n)
  {
    deallocate(n);
  }
}

void
DeleteList::deallocate(size_t n)
{
  // deallocate in reverse order of allocation
  PtrList::iterator i1 = _ptrs[n].end();
  PtrList::iterator i2 = _ptrs[n].begin();
  while (i1 != i2)
  {
    --i1;
    i1->deallocate();
  }
  _ptrs[n].clear();
}

} // tse
