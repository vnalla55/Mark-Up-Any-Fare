#include "Common/TSSCacheCommon.h"

#include "Common/Config/ConfigurableValue.h"
#include "Common/Global.h"
#include "Common/Logger.h"

#include <boost/tokenizer.hpp>

namespace tse
{

namespace tsscache
{
__thread TSSStorage*
storage(nullptr);

namespace
{
Logger
logger("atseintl.Common.TSSCacheCommon");

ConfigurableValue<int64_t>
tssMemoryBufferSize("TSE_SERVER", "TSS_MEMORY_BUFFER_SIZE");

typedef std::map<std::string, Attributes> AttributesMap;

const AttributesMap&
initializeAttributesMap()
{
  static AttributesMap map;
  std::vector<ConfigMan::NameValue> cfgValues;
  if (!(Global::hasConfig() && Global::config().getValues(cfgValues, "TSS_CACHE")))
  {
    return map;
  }
  boost::char_separator<char> sep("|", "", boost::keep_empty_tokens);
  for (const auto& cfg : cfgValues)
  {
    std::string nameUpper(cfg.name);
    boost::to_upper(nameUpper);
    Attributes attrs;
    int count(0);
    boost::tokenizer<boost::char_separator<char>> tokens(cfg.value, sep);
    for (const auto& attr : tokens)
    {
      switch (count)
      {
      case ATTRIBUTES_SIZE:
        attrs._size = atoi(attr.c_str());
        break;
      case ATTRIBUTES_STATS:
        attrs._stats = atoi(attr.c_str());
        break;
      }
      ++count;
    }
    map.insert(std::make_pair(nameUpper, attrs));
  }
  return map;
}

void
destroy(void* ptr)
{
  delete static_cast<TSSStorage*>(ptr);
}

size_t
tssMemoryBufferSizeCfg()
{
  size_t size(0);
  if (Global::hasConfig())
  {
    size = tssMemoryBufferSize.getValue();
  }
  return size;
}

TSSStorage*
createStorage()
{
  const MallocContextDisabler context;
  pthread_key_t storageKey;
  int res(pthread_key_create(&storageKey, destroy));
  if (0 == res)
  {
    storage = new TSSStorage(storageKey);
    pthread_setspecific(storageKey, storage);
  }
  else
  {
    LOG4CXX_ERROR(logger, "pthread_key_create failed " << res);
  }
  return storage;
}

} // namespace

TSSStorage::TSSStorage(pthread_key_t key) : _memoryBuffer(nullptr), _key(key)
{
  for (int i = 0; i < NUMBER_OF_CACHES; ++i)
  {
    _caches[i] = nullptr;
  }
}

TSSStorage::~TSSStorage()
{
  for (int i = 0; i < NUMBER_OF_CACHES; ++i)
  {
    delete _caches[i];
  }
  delete _memoryBuffer;
  int res(pthread_key_delete(_key));
  if (res != 0)
  {
    LOG4CXX_ERROR(logger, "pthread_key_delete failed " << res);
  }
}

TSSStorage*
initStorage()
{
  if (nullptr == storage)
  {
    storage = createStorage();
  }
  return storage;
}

size_t
initializeMemoryBufferSize()
{
  static const size_t memoryBufferSizeCfg(tssMemoryBufferSizeCfg());
  return memoryBufferSizeCfg;
}

Attributes
getCacheAttributes(const std::string& name)
{
  static const AttributesMap& map(initializeAttributesMap());
  Attributes attrs;
  std::string nameUpper(name);
  boost::to_upper(nameUpper);
  auto it(map.find(nameUpper));
  if (it != map.end())
  {
    attrs._size = it->second._size;
    attrs._stats = it->second._stats;
  }
  return attrs;
}

void
printStats(
    const std::string& name, int size, long total, long noTrxId, long reuse, long hits, int stats)
{
  if (0 == total % stats)
  {
    double hashHits(0 == reuse ? 0. : static_cast<double>(hits) / static_cast<double>(reuse));
    double actualReuse(static_cast<double>(hits) / static_cast<double>(total));
    LOG4CXX_FATAL(logger,
                  "!### " << name << ",size:" << size << ",calls:" << total
                          << ",no trxId:" << noTrxId << ",can reuse:" << reuse << ",hits:" << hits
                          << ",hash:" << hashHits << ",reuse:" << actualReuse);
  }
}

TSSCacheEnum
IsInLocEntry::_cacheIndex(IsInLocInd);
TSSCacheEnum
IsAnActualPaxInTrxEntry::_cacheIndex(IsAnActualPaxInTrxInd);
TSSCacheEnum
DateEntry::_cacheIndex(DateInd);
TSSCacheEnum
CurrencyConversionRoundEntry::_cacheIndex(CurrencyConversionRoundInd);
TSSCacheEnum
CurrencyConverterRoundByRuleEntry::_cacheIndex(CurrencyConverterRoundByRuleInd);
TSSCacheEnum
IsInZoneEntry::_cacheIndex(IsInZoneInd);

} // tsscache

} // tse
