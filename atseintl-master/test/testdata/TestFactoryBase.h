#ifndef TEST_FACTORY_BASE_H
#define TEST_FACTORY_BASE_H

/**
 * This Factory is for the creation of data in order to facilitate unit testing.
 **/

#include <cstdlib> // for getenv()
#include <libgen.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <exception>
#include <string>

#include <boost/utility/enable_if.hpp>

#include "Common/DateTime.h"
#include "test/testdata/TestFactoryInstanceMgr.h"
#include "test/testdata/TestXMLHelper.h"
#include "test/testdata/tinyxml/tinyxml.h"

template <class FactoryT>
class FactoryClassSufix
{
  template <class T, class funProto>
  struct hasNameFun
  {
    template <typename U, U>
    struct typeMatch
    {
    };
    template <typename _1>
    static char(&funExists(typeMatch<funProto, &_1::getName>*));
    template <typename>
    static int(&funExists(...));
    static bool const value = sizeof(funExists<T>(0)) == sizeof(char);
  };

public:
  // non static function
  template <class DataClassT>
  static typename boost::enable_if<hasNameFun<
      FactoryT,
      void (FactoryT::*)(std::stringstream&, const std::string&, const DataClassT&)> >::type
  getName(FactoryT* t, std::stringstream& os, const std::string& className, const DataClassT& d)
  {
    t->getName(os, className, d);
  }
  template <class DataClassT>
  static typename boost::disable_if<hasNameFun<
      FactoryT,
      void (FactoryT::*)(std::stringstream&, const std::string&, const DataClassT&)> >::type
  getName(FactoryT* t, std::stringstream& os, const std::string& className, const DataClassT& d)
  {
    os << className << TestFactoryInstanceMgr<FactoryT, DataClassT>::instance().instanceNumber();
  }

  // static function
  template <class DataClassT>
  static typename boost::enable_if<
      hasNameFun<FactoryT,
                 void (*)(std::stringstream&, const std::string&, const DataClassT&)> >::type
  getName(std::stringstream& os, const std::string& className, const DataClassT& d)
  {
    FactoryT::getName(os, className, d);
  }
  template <class DataClassT>
  static typename boost::disable_if<
      hasNameFun<FactoryT,
                 void (*)(std::stringstream&, const std::string&, const DataClassT&)> >::type
  getName(std::stringstream& os, const std::string& className, const DataClassT& d)
  {
    os << className << TestFactoryInstanceMgr<FactoryT, DataClassT>::instance().instanceNumber();
  }
};

namespace TestFactoryFlags
{
enum Flags
{
  DEFAULT = 0,
  UNIQUE = 1,
  OWNS_PTR_TEST = 2,
  OWNS_PTR = 3, // is also unique
  DONT_POST_INIT = 4
};
}

template <class FactoryT>
class TestFactoryBase
{
public:
  typedef TestFactoryFlags::Flags Flags;

  /******************************************************************************************
  // This is the key create method used to read factory-based elements from a file. Note that
  // the element name expected must be past in.
  *******************************************************************************************/

  template <class DataClassT>
  static DataClassT*
  create(const std::string& filename, const std::string& elementName, bool unique = false)
  {
    using namespace TestFactoryFlags;
    return create<DataClassT>(filename, elementName, unique ? UNIQUE : DEFAULT);
  }

  template <class DataClassT>
  static DataClassT* create(std::string filename, const std::string& elementName, int flags)
  {
    using namespace TestFactoryFlags;
    handleVobDir(filename);
    DataClassT* item = 0;

    if (!(flags& UNIQUE) &&
        (item = TestFactoryInstanceMgr<FactoryT, DataClassT>::instance().get(filename)))
    {
      return item;
    }
    TiXmlDocument doc(filename.c_str());

    // Catch the error of not being able to open the file.
    if (!doc.LoadFile())
    {
      std::cout << "Could not read file " << filename << "." << std::endl;
      throw std::exception();
    }

    TiXmlElement* rootElement = doc.FirstChildElement(elementName);
    if (!rootElement)
    {
      return 0;
    }

    return create<DataClassT>(filename, rootElement, flags);
  }

  template <class DataClassT>
  static DataClassT* create(const std::string& filename, TiXmlElement* rootElement, int flags)
  {
    using namespace TestFactoryFlags;
    std::string fileAttribute;
    TestXMLHelper::Attribute(rootElement, "file", fileAttribute);

    if (!fileAttribute.empty() && rootElement->NoChildren())
    {
      // Just forward to next file.
      return FactoryT::create(fileAttribute, flags);
    }

    std::string cachename = filename;
    if (rootElement->Row() > 1)
    {
      std::ostringstream oss;
      oss << filename << ':' << rootElement->Row();
      cachename = oss.str();
    }
    if (flags & UNIQUE)
    {
      // This is unreliable, because it has only 1s resolution.
      // cachename += tse::DateTime::localTime().toIsoExtendedString();
      decorateWithCreateCounter(&cachename);
    }

    DataClassT* item = 0;
    if (!fileAttribute.empty())
    {
      // Create a new unique object from file just for us,
      // because we will change it.
      item = FactoryT::create(fileAttribute, OWNS_PTR | DONT_POST_INIT);
    }
    else
    {
      item = FactoryT::construct(filename, rootElement, flags);

      FactoryT::preInit(item);
    }
    TestFactoryInstanceMgr<FactoryT, DataClassT>::instance().put(
        cachename, item, flags & OWNS_PTR_TEST);

    FactoryT::init(item, filename, rootElement, flags);

    if (!(flags & DONT_POST_INIT))
    {
      FactoryT::postInit(item);
    }

    return item;
  }

  /******************************************************************************************
  // Dummy methods; it is required that the specific template subclasses provide their
  // own implementation of these methods.
  *******************************************************************************************/

  template <class DataClassT>
  static DataClassT* construct(const std::string& filename, TiXmlElement* rootElement, int flags)
  {
    return 0;
  }

  template <class DataClassT>
  static void preInit(DataClassT* item)
  {
  }

  template <class DataClassT>
  static void
  init(DataClassT* item, const std::string& filename, TiXmlElement* rootElement, int flags)
  {
  }

  template <class DataClassT>
  static void postInit(DataClassT* item)
  {
  }

  /******************************************************************************************
  // This method allows a user to remove all items of this type from the local
  // cache so it can be reloaded from the files system without carrying over
  // modifications from a previous test.
  *******************************************************************************************/
  template <class DataClassT>
  static void destroyAll()
  {
    TestFactoryInstanceMgr<FactoryT, DataClassT>::instance().destroyAll();
  }

  /******************************************************************************************
  // The primary client entry point for writing data to files. This method takes only a
  // filename, without the "xml" extension. This will be used for the file that is created,
  // and for the prefix for other secondary files (the prefix is primarily so the developer
  // can easily recognize the primary file that is written). If a full path is passed in
  // (which is preferred), the method will strip off the directory portion of the name
  // and use just the filename portion as the prefix.
  //
  // Note that the destination directory must exist before the call is made.
  *******************************************************************************************/
  template <class DataClassT>
  static void write(std::string fullpathname, const DataClassT& item)
  {
    handleVobDir(fullpathname);
    const std::string preface = fullpathname;
    fullpathname = fullpathname + ".xml";

    // Register that we've written this now, so we don't run into any
    // issues with circular references.
    TestFactoryInstanceMgr<FactoryT, DataClassT>::instance().put(&item, fullpathname);

    TiXmlDocument doc(fullpathname.c_str());
    FactoryT::write(item, &doc, preface);

    if (!doc.SaveFile())
    {
      std::cout << "Could not write to file " << fullpathname << "." << std::endl;
      throw std::exception();
    }
  }

  /******************************************************************************************
  // Write (maybe), and return filename that represents item. Note that this
  // method takes care of all of our file naming, incrementing the instance number,
  // and all that miscellaneous junk.
  *******************************************************************************************/

  template <class DataClassT>
  static void writeSubItem(const std::string& sectionName,
                           const DataClassT& item,
                           TiXmlNode* node,
                           const std::string& filePrefix,
                           const std::string& className)
  {
    if (&item == 0)
      return;

    // If this instance wasn't written earlier, write it out to the provided filePrefix
    if (!TestFactoryInstanceMgr<FactoryT, DataClassT>::instance().wasWritten(&item))
    {
      std::stringstream newname;
      TestFactoryInstanceMgr<FactoryT, DataClassT>::instance().incrementInstanceNumber();
      newname << filePrefix << "_";
      FactoryClassSufix<FactoryT>::getName(newname, className, item);
      newname << ".xml";

      // Register that we've written this now, so we don't run into any
      // issues with circular references.
      TestFactoryInstanceMgr<FactoryT, DataClassT>::instance().put(&item, newname.str());

      TiXmlDocument doc(newname.str().c_str());
      FactoryT::write(item, &doc, filePrefix);

      if (!doc.SaveFile())
      {
        std::cout << "Could not write to file " << newname.str() << "." << std::endl;
        throw std::exception();
      }
    }

    // Write out the file section in the original file.
    TiXmlElement* child = new TiXmlElement(sectionName.c_str());
    const std::string filename =
        TestFactoryInstanceMgr<FactoryT, DataClassT>::instance().get(&item);
    child->SetAttribute("file", filename.c_str());

    node->LinkEndChild(child);
  }

  template <class DataClassT>
  static void write(const std::string& sectionName,
                    const DataClassT& item,
                    TiXmlNode* node,
                    const std::string& filePrefix)
  {
    if (!FactoryT::checkItem(item))
      return;

    TiXmlElement* rootElement = new TiXmlElement(sectionName.c_str());
    node->LinkEndChild(rootElement);
    FactoryT::write(item, rootElement, filePrefix);
  }

  /******************************************************************************************
  // Another static method that subclasses should implement.
  *******************************************************************************************/

  template <class DataClassT>
  static void write(const DataClassT& item, TiXmlNode* node, const std::string& filePrefix)
  {
  }

  /******************************************************************************************
  // Another static method that subclasses should implement. This method takes the class (item)
  // and writes in with the provided section name, into the provided stream (or, writes an
  // entry in that stream, and opens another stream for the actual data).
  *******************************************************************************************/

  template <class DataClassT>
  static void
  write(const DataClassT& item, TiXmlElement* rootElement, const std::string& filePrefix)
  {
  }

  static void handleVobDir(std::string& filename)
  {
    static const char defaultVobDir[] = "/vobs/atseintl";
    static const std::size_t size = sizeof(defaultVobDir) - 1;
    static const std::string vobDir = TSE_VOB_DIR;
    if (!vobDir.empty() && !filename.compare(0, size, defaultVobDir, size))
      filename.replace(0, size, vobDir);
  }

protected:
  TestFactoryBase();

  static int createCounter;

  static void decorateWithCreateCounter(std::string* name)
  {
    std::ostringstream oss;
    oss << *name << '-' << (createCounter++);
    *name = oss.str();
  }
};

template <class FactoryT>
int TestFactoryBase<FactoryT>::createCounter = 0;

#endif
