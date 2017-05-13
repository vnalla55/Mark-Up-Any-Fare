//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#ifndef SERIALIZATION_TEST_BASE_H
#define SERIALIZATION_TEST_BASE_H

#include <unistd.h>
#include <sys/timeb.h>
#include <sys/times.h>

#include <list>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "Common/DateTime.h"
#include "Common/ObjectComparison.h"
#include "Common/TimeUtil.h"
#include "Common/Thread/TseScopedExecutor.h"
#include "Common/Utils/DBStash.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/DataBlobHelper.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/DiskCache.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LDCActionQueue.h"
#include "DBAccess/LDCHelper.h"
#include "test/include/CppUnitHelperMacros.h"

#define FLATTENIZE_TRACE false

namespace tse
{
template <typename T>
void
assertIdentical(const T& original, const T& reassembled, bool initialized = true)
{
  bool identical = objectsIdentical(original, reassembled);

  CPPUNIT_ASSERT(identical);
}

class SerializationTestBase : public CppUnit::TestFixture
{
public:
  template <typename I>
  void testInfoTypeWithFlattenize()
  {
    realTestInfoType<I>(true);
  }

  template <typename I>
  void testInfoVectorTypeWithFlattenize()
  {
    realTestInfoVectorType<I>(true);
  }

  template <typename I>
  void testInfoType()
  {
    realTestInfoType<I>(true); // EVERYTHING's flattenizable, now!
  }

  template <typename I>
  void testInfoVectorType()
  {
    realTestInfoVectorType<I>(true); // EVERYTHING's flattenizable, now!
  }

  template <typename I>
  void realTestInfoType(bool flattenizable = false)
  {
    I original;
    I::dummyData(original);
    I uninitialized;

    if (flattenizable)
    {
      flattenizeTest(original);
      flattenizeTest(uninitialized, false);
    }
  }

  template <typename I>
  void realTestInfoVectorType(bool flattenizable = false)
  {
    typedef std::vector<I*> IVEC;

    IVEC original;

    I info1;
    I info2;
    I info3;

    I::dummyData(info1);
    I::dummyData(info2);
    I::dummyData(info3);

    original.push_back(&info1);
    original.push_back(&info2);
    original.push_back(&info3);

    if (flattenizable)
    {
      flattenizeTest(original);
    }

    if (flattenizable)
    {
      testDataBlobWithOptions(original, DiskCache::DATAFMT_BN2);
    }
  }

  std::string human_readable(uint64_t milliseconds)
  {
    uint64_t secs = milliseconds / 1000;
    uint64_t millis = milliseconds % 1000;

    uint64_t mins = secs / 60;
    secs = secs % 60;

    uint64_t hrs = mins / 60;
    mins = mins % 60;

    std::ostringstream os;

    if (hrs)
    {
      os << hrs << " hours, ";
      os << mins << " minutes, ";
      os << secs << " seconds, ";
      os << millis << " milliseconds.";
    }
    else if (mins)
    {
      os << mins << " minutes, ";
      os << secs << " seconds, ";
      os << millis << " milliseconds.";
    }
    else if (secs)
    {
      os << secs << " seconds, ";
      os << millis << " milliseconds.";
    }
    else
    {
      os << millis << " milliseconds.";
    }

    return os.str();
  }

  enum Flavor
  {
    FLAVOR_FLATTENIZE = 0
  };

  template <typename C, typename S>
  class SerializeTask : public TseCallableTask
  {
  public:
    SerializeTask(Flavor flavor, size_t items_per)
      : _flavor(flavor), _items_per(items_per), _blobSize(0), _verified(false)
    {
    }

    virtual ~SerializeTask() {}

    void flattenizerSerialize(C& original, Flattenizable::Archive& archive)
    {
      TIMERCLOCK(true, "flattenizer", "serialize");
      std::string ignore;
      FLATTENIZE_SAVE(archive, original, 0, ignore, ignore);
    }

    void flattenizerDeserialize(C& duplicate, Flattenizable::Archive& archive)
    {
      TIMERCLOCK(true, "flattenizer", "deserialize");
      FLATTENIZE_RESTORE(archive, duplicate, NULL, 0);
    }

    virtual void run()
    {
      C original;
      S::insertNewItems(original, _items_per);
      C duplicate;

      if (_flavor == FLAVOR_FLATTENIZE)
      {
        Flattenizable::Archive archive;
        archive.setTrace(FLATTENIZE_TRACE);
        flattenizerSerialize(original, archive);
        flattenizerDeserialize(duplicate, archive);
        _blobSize = archive.size();
      }

      _verified = objectsIdentical(original, duplicate);

      S::clearContainer(original);
      S::clearContainer(duplicate);

      CPPUNIT_ASSERT(_verified);
    }

    size_t blobSize() const { return _blobSize; }
    bool verified() const { return _verified; }

  private:
    SerializeTask();

    Flavor _flavor;
    size_t _items_per;

    size_t _blobSize;
    bool _verified;
    DiskCache::Timer _timer;
  };

  std::string pctImprove(uint64_t best, uint64_t worst)
  {
    return pctImprove(static_cast<double>(best), static_cast<double>(worst));
  }

  std::string pctImprove(double best, double worst)
  {
    std::ostringstream os;
    double pct = 100.00 - (best / worst * 100.00);
    os << std::setprecision(5) << pct;
    return os.str();
  }

  template <typename C, typename S>
  void
  testContainer(size_t num_objects, size_t num_items, size_t pool_size)
  {
    /*std::cout << std::endl ;
    std::cout << std::endl ;
    std::cout << "  Container description      : " << S::containerDescription() << std::endl ;
    std::cout << "  Number of objects          : " << num_objects << std::endl ;
    std::cout << "  Number of items per object : " << num_items   << std::endl ;
    std::cout << "  Thread pool size           : " << pool_size   << std::endl ;
    std::cout << std::endl ;
    std::cout << std::endl ;*/

    DataHandle dh;
    std::list<SerializeTask<C, S> > flattenizeTasks;

    // std::cout << "  Performing FLATTENIZE serialization/deserialization..." << std::endl ;
    TseScopedExecutor flattenizeTaskPool(TseThreadingConst::SCOPED_EXECUTOR_TASK, pool_size);
    for (size_t i = 0; i < num_objects; ++i)
    {
      flattenizeTasks.push_back(SerializeTask<C, S>(FLAVOR_FLATTENIZE, num_items));
      flattenizeTaskPool.execute(flattenizeTasks.back());
    }
    flattenizeTaskPool.wait();
    bool allVerified = true;
    for (typename std::list<SerializeTask<C, S> >::const_iterator i = flattenizeTasks.begin();
         ((allVerified) && (i != flattenizeTasks.end()));
         ++i)
    {
      allVerified = (*i).verified();
    }
    /*if( allVerified )
    {
      std::cout << "    ==> All reconstituted objects verified against the originals." << std::endl
    ;
    }
    else
    {
      std::cout << "    ==> ERROR! A reconstituted object was NOT identical to the original!" <<
    std::endl ;
    }

    std::cout << std::endl ;
    std::cout << "  FLATTENIZE blob size : " << (*flattenizeTasks.begin()).blobSize() << std::endl
    ;*/

    /*std::cout << std::endl ;
    std::cout << "  Tallying the results..." << std::endl ;*/

    /*std::cout << std::endl ;
    std::cout << "  Name             Operation             Count     Time (ms)  % Improve  CPU (ms)
    % Improve" << std::endl ;
    std::cout << "  ===============  ====================  ========  =========  =========  =========
    =========" << std::endl ;*/

    for (TimeUtil::TimeMap::const_iterator mi = TimeUtil::TimerClock::m_map.begin();
         mi != TimeUtil::TimerClock::m_map.end();
         ++mi)
    {
      /*std::cout << std::setw( 2  ) << std::left  << "  "
                << std::setw( 15 ) << std::left  << mi->m_name
                << std::setw( 2  ) << std::left  << "  "
                << std::setw( 20 ) << std::left  << mi->m_operation
                << std::setw( 2  ) << std::left  << "  "
                << std::setw( 8  ) << std::setprecision( 8 ) << std::right << mi->count
                << std::setw( 2  ) << std::left  << "  "
                << std::setw( 9  ) << std::setprecision( 8 ) << std::right << mi->time; */

      /*std::cout << std::setw( 2  ) << std::left  << "  " ;
      std::cout << std::setw( 9  ) << std::setprecision( 8 ) << std::right ;*/


      /*std::cout << std::setw( 2  ) << std::left  << "  " ;
      std::cout << std::setw( 9  ) << std::setprecision( 8 ) << std::right << mi->cputime ;

      std::cout << std::setw( 2  ) << std::left  << "  " ;
      std::cout << std::setw( 9  ) << std::setprecision( 8 ) << std::right ;*/

      // std::cout << std::endl ;
    }
    // std::cout << std::endl ;
  }

protected:
  template <typename T>
  void flattenizeTest(const T& original, bool initialized = true)
  {
    Flattenizable::Archive archive;
    archive.setTrace(FLATTENIZE_TRACE);
    std::string ignore;
    FLATTENIZE_SAVE(archive, original, 0, ignore, ignore);
    T* reassembled = new T;
    FLATTENIZE_RESTORE(archive, *reassembled, NULL, 0);
    assertIdentical(original, *reassembled, initialized);
    delete reassembled;
  }

  template <typename DATA>
  void testDataBlobWithOptions(DATA& original, DiskCache::DataFormat fmt)
  {
    const char* name("DUMMY_TYPE");
    const char* key("DUMMY_KEY");

    DiskCache::CacheTypeOptions cto(name, true, fmt, 240, NULL, 0, false, 0, 0);
    DATA* reassembled(NULL);

    DiskCache::DataBlob* blob = DataBlobHelper<StringKey, DATA>::flatten(key, original, name, cto);
    if (blob != NULL)
    {
      reassembled = DataBlobHelper<StringKey, DATA>::unflatten(blob, key, cto.name, cto);
    }

    CPPUNIT_ASSERT(blob != NULL);
    CPPUNIT_ASSERT(reassembled != NULL);

    if (blob && reassembled)
    {
      CPPUNIT_ASSERT(objectsIdentical(original, *reassembled));
    }

    delete blob;
    delete reassembled;
  }
};

} // namespace tse

#endif // SERIALIZATION_TEST_BASE_H
