//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "OptionalServicesDAO.h"
#include "OptionalServicesInfo.h"
#include "CompressionTestCommon.h"

namespace {

template<typename T>
struct IsSame
{
  IsSame<T>(T& data): _data(data) {}
  template<typename S> bool operator()(S* obj, T& (S::*mhod)() const) const
{
  return (obj->*mhod)() == _data;
}
private:
  T& _data;
};

}

namespace tse
{
const bool _poolObjects(true);

  std::string OptionalServicesDAO::_name("OptionalServices");
  std::string OptionalServicesDAO::_cacheClass("Rules");
  OptionalServicesDAO* OptionalServicesDAO::_instance = 0;

  OptionalServicesDAO& OptionalServicesDAO::instance()
  {
    return *_instance;
  }

  std::vector<OptionalServicesInfo*>* OptionalServicesDAO::create(OptionalServicesKey key)
  {
    size_t numitems(key._a % MAXNUMBERENTRIES);
    std::vector<OptionalServicesInfo*>* ret = new std::vector<OptionalServicesInfo*>(numitems);
    for (size_t i = 0; i < numitems; ++i)
    {
      OptionalServicesInfo *obj(new OptionalServicesInfo);
      /*
      std::cout << "obj->createDate()=" << obj->createDate() << '\n'
                << "obj->expireDate()=" << obj->expireDate() << '\n'
                << "obj->effDate()=" << obj->effDate() << '\n'
                << "obj->discDate()=" << obj->discDate() << '\n'
                << "obj->ticketEffDate()=" << obj->ticketEffDate() << '\n'
                << "obj->ticketDiscDate()=" << obj->ticketDiscDate()
                << std::endl;
                */
      obj->dummyData();
      /*
      std::cout << "obj->createDate()=" << obj->createDate() << '\n'
                << "obj->expireDate()=" << obj->expireDate() << '\n'
                << "obj->effDate()=" << obj->effDate() << '\n'
                << "obj->discDate()=" << obj->discDate() << '\n'
                << "obj->ticketEffDate()=" << obj->ticketEffDate() << '\n'
                << "obj->ticketDiscDate()=" << obj->ticketDiscDate()
                << std::endl;
                */
      (*ret)[i] = obj;
    }
    return ret;
  }

  void OptionalServicesDAO::destroy(OptionalServicesKey key, std::vector<OptionalServicesInfo*>* recs)
  {
    if (!destroyPooledVector(recs))
    {
      destroyContainer(recs);
    }
  }

  void OptionalServicesDAO::load()
  {
    //not pre_loading
  }

  sfc::CompressedData*
  OptionalServicesDAO::compress(const std::vector<OptionalServicesInfo*>* vect) const
  {
    return compressVector(vect, _poolObjects);
  }

  std::vector<OptionalServicesInfo*>*
  OptionalServicesDAO::uncompress(const sfc::CompressedData& compressed) const
  {
    return uncompressEntry<OptionalServicesInfo>(compressed);
  }

} //tse
