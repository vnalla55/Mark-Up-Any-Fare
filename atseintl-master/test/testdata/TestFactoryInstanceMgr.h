#ifndef TEST_FACTORY_INSTANCE_MGR_H
#define TEST_FACTORY_INSTANCE_MGR_H

/**
 * This is a singleton class, templatized, to maintain instance status through the
 * factories. When writing out data to the XML format, instances that are the same
 * should always point to the same file. Likewise, when reading, if the same file
 * is being read, this should indicate that the same instance was desired. This
 * singleton registers instance by filename, and when asked for an instance by
 * filename will return that instance.
 *
 * Note that this implementation is NOT threadsafe!!
 **/

#include <malloc.h>

#include <string>
#include <map>

#include "test/testdata/TestFactoryInstanceMgrBase.h"

template <class FactoryT, class DataType>
class TestFactoryInstanceMgr : public TestFactoryInstanceMgrBase
{
public:
  static TestFactoryInstanceMgr<FactoryT, DataType>& instance()
  {
    static TestFactoryInstanceMgr<FactoryT, DataType> instance;
    return instance;
  }

  // For reading, we map a filename (key) to an object.
  void put(const std::string& filename, DataType* objectPtr, bool ownsPtr)
  {
    _fileToObjectRegistry[filename] = ObjectEntry(objectPtr, ownsPtr);
  }
  DataType* get(const std::string& filename) { return _fileToObjectRegistry[filename].object; }

  // For writing, we map an object (key, by pointer) to a filename.
  void put(const DataType* objectPtr, const std::string& filename)
  {
    _objectToFileRegistry[objectPtr] = filename;
  }
  bool wasWritten(const DataType* objectPtr)
  {
    if (!objectPtr)
      return false;

    typename std::map<const DataType*, std::string>::const_iterator iB = _objectToFileRegistry
                                                                             .begin(),
                                                                    iE =
                                                                        _objectToFileRegistry.end();
    for (; iB != iE; ++iB)
    {
      if (!iB->first)
        continue;
      if (FactoryT::compare(iB->first, objectPtr))
        return true;
    }
    return false;
  }
  std::string get(const DataType* objectPtr)
  {
    assert(objectPtr);

    typename std::map<const DataType*, std::string>::const_iterator iB = _objectToFileRegistry
                                                                             .begin(),
                                                                    iE =
                                                                        _objectToFileRegistry.end();
    for (; iB != iE; ++iB)
    {
      if (!iB->first)
        continue;
      if (FactoryT::compare(iB->first, objectPtr))
        return iB->second;
    }
    return "";
  }

  virtual void destroyAll()
  {
    typename std::map<std::string, ObjectEntry>::const_iterator iter =
                                                                    _fileToObjectRegistry.begin(),
                                                                end = _fileToObjectRegistry.end();
    for (; iter != end; ++iter)
    {
      //      if (!iter->second.ownsPtr)
      //        delete iter->second.object;
    }
    _fileToObjectRegistry.clear();
    _objectToFileRegistry.clear(); // All objects already deleted, so just clear

    resetInstanceNum();
  }

  virtual ~TestFactoryInstanceMgr<FactoryT, DataType>() {}

private:
  struct ObjectEntry
  {
    ObjectEntry() : object(0), ownsPtr(false) {}
    ObjectEntry(DataType* object, bool ownsPtr) : object(object), ownsPtr(ownsPtr) {}

    DataType* object;
    bool ownsPtr;
  };

  // We're a singleton, so the default constructor is private.
  TestFactoryInstanceMgr<FactoryT, DataType>() {}

  // Disable the copy constructor, operator=, and operator==.
  TestFactoryInstanceMgr<FactoryT, DataType>(const TestFactoryInstanceMgr<FactoryT, DataType>& rhs);
  bool operator=(const TestFactoryInstanceMgr<FactoryT, DataType>& rhs);
  bool operator==(const TestFactoryInstanceMgr<FactoryT, DataType>& rhs);

  std::map<std::string, ObjectEntry> _fileToObjectRegistry;
  std::map<const DataType*, std::string> _objectToFileRegistry;
};

#endif
