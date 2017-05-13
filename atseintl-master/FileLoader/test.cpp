#include "Common/Global.h"
#include "Common/Logger.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/HashKey.h"
#include "DBAccess/SimpleCache.h"
#include "FileLoader/BFCacheUpdate.h"
#include "FileLoader/FileLoader.h"
#include "FileLoader/FileLoaderExtract.h"

#include <iostream>
#include <stdexcept>

#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/xml/domconfigurator.h>

const std::string
_dataDir("/project/esv/data/ilya/");

typedef sfc::KeyedFactory<tse::FareKey, tse::FareInfoVec> KeyedFactoryType;
typedef sfc::SimpleCache<tse::FareKey, tse::FareInfoVec> SimpleBoundFareCache;

int
main(int argc, char* argv[])
{
  std::string logCfgFile("/vobs/atseintl/Server/log4cxx.xml");
  log4cxx::xml::DOMConfigurator::configure(logCfgFile);
  std::string loadUrl(_dataDir + "all-domestic-fares.sorted.load.gz");
  // std::string updateUrl(_dataDir + "all-domestic-fares8.update.sorted.gz");
  // std::string loadUrl(_dataDir + "ProdFares2008-07-24-21-29-18.txt.gz");
  // std::string loadUrl(_dataDir + "test.load.gz");
  // std::string updateUrl(_dataDir + "test.update.gz");
  // std::string updateUrl("test.update.gz");
  // std::string loadUrl("/vobs/atseintl/FileLoader/all-domfares2.load.gz");
  // std::string updateUrl("/vobs/atseintl/FileLoader/test.update.gz");

  size_t capacity(0);
  class TestKeyedFactory : public KeyedFactoryType
  {
    void destroy(tse::FareKey key, tse::FareInfoVec* vect)
    {
      // std::cerr << "TestKeyedFactory::destroy called" << std::endl;
      size_t sz(vect->size());
      for (size_t i = 0; i < sz; ++i)
      {
        delete vect->at(i);
      }
      delete vect;
    }
  } factory;
  SimpleBoundFareCache cache(factory, capacity);

  tse::FileLoader loader(loadUrl, &cache);
  // tse::FileLoaderExtract loader(loadUrl, &cache);
  try { loader.parse(); }
  catch (const std::exception& e) { std::cerr << "exception:" << e.what() << std::endl; }
  log4cxx::BasicConfigurator::resetConfiguration();
  return 0;
}
