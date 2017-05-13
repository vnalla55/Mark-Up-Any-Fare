#ifndef DATABLOBHELPER_H
#define DATABLOBHELPER_H
//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "Common/TimeUtil.h"
#include "DBAccess/DiskCache.h"
#include "DBAccess/Flattenizable.h"

#include <ios>

namespace tse
{
template <typename Key, typename Type>
struct DataBlobHelper
{

  static std::string to_hex(const std::string& buf)
  {
    std::ostringstream os;
    for (const char elem : buf)
    {
      os << std::hex << std::setw(2) << static_cast<unsigned char>(elem);
    }
    return os.str();
  }

  static log4cxx::LoggerPtr& getLogger()
  {
    static log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("atseintl.DBAccess.DistCache"));
    return logger;
  }

  static log4cxx::LoggerPtr& helperLogger()
  {
    static log4cxx::LoggerPtr logger(
        log4cxx::Logger::getLogger("atseintl.DBAccess.DataBlobHelper"));
    return logger;
  }

  // serialize, deserialize a type [ go to or from a DiskCache::DataBlob ]

  inline static void flattenBn2(DiskCache::DataBlob*& blob,
                                const std::string& flatKey, const Type& data,
                                const DiskCache::CacheTypeOptions& cto,
                                const std::string& name, uint32_t& serialSize,
                                bool debug_logging,
                                DiskCache::Timer& serializeTimer,
                                DiskCache::Timer& cmpTimer)
  {
    Flattenizable::Archive archive;
    FLATTENIZE_SAVE(archive, data, cto.ldcMaxBlobSize, name, flatKey);
    serialSize = static_cast<uint32_t>(archive.size());
    if (UNLIKELY(!serialSize))
    {
      delete blob;
      blob = nullptr;
    }
    else
    {
      if (UNLIKELY(debug_logging))
      {
        serializeTimer.checkpoint();
        cmpTimer.reset();
      }
      blob->setData(archive.buf(),
                    static_cast<uint32_t>(archive.size()),
                    static_cast<uint32_t>(cto.compressionLimit),
                    cto.dataFormat);
    }
  }

  static DiskCache::DataBlob* flatten(const std::string& flatKey,
                                      const Type& data,
                                      const std::string& name,
                                      const DiskCache::CacheTypeOptions& cto)
  {
    // Caller is responsible to clear the blob when finished!
    DiskCache::DataBlob* blob(new DiskCache::DataBlob);

    const bool debug_logging(IS_DEBUG_ENABLED(helperLogger()));

    uint32_t serialSize(0);

    DiskCache::Timer serializeTimer;
    DiskCache::Timer cmpTimer;

    if (UNLIKELY(debug_logging))
    {
      serializeTimer.reset();
    }

    try
    {
      switch (cto.dataFormat)
      {
      case DiskCache::DATAFMT_BN2:
      {
        flattenBn2(blob, flatKey, data, cto, name, serialSize, debug_logging,
                   serializeTimer, cmpTimer);
      }
      break;

      default:
        {
          flattenBn2(blob, flatKey, data, cto, name, serialSize, debug_logging,
                     serializeTimer, cmpTimer);
          break;
        }
      }

      if (UNLIKELY(debug_logging))
      {
        cmpTimer.checkpoint();

        LOG4CXX_DEBUG(helperLogger(),
                      "Cache [" << name << "] key [" << flatKey << "] serialized"
                                << " in " << serializeTimer.elapsed() << "ms (elapsed)"
                                << " and " << serializeTimer.cpu() << "ms (cpu).");

        if (UNLIKELY(serialSize > cto.compressionLimit))
        {
          LOG4CXX_DEBUG(helperLogger(),
                        "Cache [" << name << "] key [" << flatKey << "] compressed"
                                  << " in " << cmpTimer.elapsed() << "ms (elapsed)"
                                  << " and " << cmpTimer.cpu() << "ms (cpu).");
        }
      }
    }
    catch (std::exception& e)
    {
      delete blob;
      blob = nullptr;
      LOG4CXX_ERROR(helperLogger(),
                    "LDC data blob creation failure for cache [" << name << "] !!! - " << e.what());
    }
    catch (...)
    {
      delete blob;
      blob = nullptr;
      LOG4CXX_ERROR(helperLogger(),
                    "LDC data blob creation failure for cache [" << name
                                                                 << "] !!! - UNKNOWN EXCEPTION");
    }

    return blob;
  }

  static Type* deserialize(char* text,
                           size_t length,
                           const char* flatKey,
                           const std::string& name,
                           const DiskCache::CacheTypeOptions& cto)
  {
    const bool debug_logging(IS_DEBUG_ENABLED(helperLogger()));
    TIMERCLOCK(debug_logging, name, __FUNCTION__);
    DiskCache::DataBlob blob;
    blob.setRawData(text, static_cast<uint32_t>(length));
    return unflatten(&blob, flatKey, name, cto);
  }

  inline static void unflattenBn2(Type* obj, char* buffer, size_t dataSize)
  {
    Flattenizable::Archive archive;
    FLATTENIZE_RESTORE(archive, *obj, buffer, static_cast<uint32_t>(dataSize));
  }

  static Type* unflatten(DiskCache::DataBlob* blob,
                         const char* flatKey,
                         const std::string& name,
                         const DiskCache::CacheTypeOptions& cto)
  {
    const bool debug_logging(IS_DEBUG_ENABLED(helperLogger()));
    TIMERCLOCK(debug_logging, name, __FUNCTION__);
    Type* obj(nullptr);
    char* bufferToDelete(nullptr);
    if (LIKELY(blob))
    {
      try
      {
        DiskCache::Timer serializeTimer;
        DiskCache::Timer cmpTimer;
        DiskCache::DataFormat fmt(DiskCache::DATAFMT_BN2);
        char* buffer(nullptr);
        size_t dataSize(0);
        if (UNLIKELY(debug_logging))
        {
          cmpTimer.reset();
        }
        {
          TIMERCLOCK(debug_logging, name, "getData");
          fmt = blob->getData(buffer, dataSize, bufferToDelete);
        }
        if (UNLIKELY(debug_logging))
        {
          cmpTimer.checkpoint();
          if (dataSize > cto.compressionLimit)
          {
            LOG4CXX_DEBUG(helperLogger(),
                          "Cache [" << name << "] key [" << flatKey << "] decompressed"
                                    << " in " << cmpTimer.elapsed() << "ms (elapsed)"
                                    << " and " << cmpTimer.cpu() << "ms (cpu).");
          }
        }
        if (LIKELY(dataSize > 0))
        {
          obj = new Type;
#if 0
            LOG4CXX_DEBUG(helperLogger(), "[" << flatKey << "] Constructing iarchive for blob of size ["
                          << dataSize
                          << "] with contents ["
                          << to_hex(data)
                          << "]");
#endif
          if (UNLIKELY(debug_logging))
          {
            serializeTimer.reset();
          }
          TIMERCLOCK(debug_logging, name, "boost::archive::iarchive[deserialize]");
          switch (fmt)
          {
          case DiskCache::DATAFMT_BN2:
          {
            unflattenBn2(obj, buffer, dataSize);
          }
          break;
          default:
          {
            unflattenBn2(obj, buffer, dataSize);
          }
          break;
          }
          if (UNLIKELY(debug_logging))
          {
            serializeTimer.checkpoint();
            LOG4CXX_DEBUG(helperLogger(),
                          "Cache [" << name << "] key [" << flatKey << "] deserialization performed"
                                    << " in " << serializeTimer.elapsed() << "ms (elapsed)"
                                    << " and " << serializeTimer.cpu() << "ms (cpu).");
          }
        }
      }
      catch (const std::exception& e)
      {
        LOG4CXX_ERROR(helperLogger(),
                      "LDC load failure for cache [" << name << "] key [" << flatKey << "] !!! - "
                                                     << e.what());
        delete obj;
        obj = nullptr;
      }
      catch (...)
      {
        LOG4CXX_ERROR(helperLogger(),
                      "LDC load failure for cache [" << name << "] key [" << flatKey
                                                     << "] !!! - UNKNOWN EXCEPTION");
        delete obj;
        obj = nullptr;
      }
    }
    delete[] bufferToDelete;
    bufferToDelete = nullptr;
    return obj;
  }

private:
  DataBlobHelper(const DataBlobHelper&);
  DataBlobHelper& operator=(const DataBlobHelper&);
};
}
#endif // DATABLOBHELPER_H
