//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.

//-------------------------------------------------------------------

#include "MarkupControlDAO.h"
#include "MarkupControl.h"
#include "CompressedData.h"
#include "CompressionTestCommon.h"

const long _delay(0);
const bool _poolObjects(true);

namespace tse
{
std::vector<MarkupControl*>* MarkupControlDAO::create (MarkupControlKey key)
{
  size_t numitems(key._a % MAXNUMBERENTRIES);
  std::vector<MarkupControl*>* ptr(new std::vector<MarkupControl*>(numitems));
  for (size_t i = 0; i < numitems; ++i)
  {
    MarkupControl *obj(new MarkupControl);
    obj->dummyData();
    (*ptr)[i] = obj;
  }
   /*DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
    DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
    try
    {
        QueryGetMarkup mkup(dbAdapter->getAdapter());
        mkup.findMarkupControl(*ret,key._a,key._b,key._c,key._d,key._e);
    }
    catch (...)
    {
        LOG4CXX_WARN(_logger,"DB exception in MarkupControlDAO::create");
        destroyContainer(ret);
        throw;
    }*/
  return ptr;
}

CreateResult<std::vector<MarkupControl*>> MarkupControlDAO::create(MarkupControlKey key,
                                                                    int)
{
  CreateResult<std::vector<MarkupControl*>> result;
  size_t numitems(key._a % MAXNUMBERENTRIES);
  result._ptr = new std::vector<MarkupControl*>(numitems);
  for (size_t i = 0; i < numitems; ++i)
  {
    MarkupControl* obj(new MarkupControl);
    obj->dummyData();
    (*result._ptr)[i] = obj;
  }
   /*DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
    DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
    try
    {
        QueryGetMarkup mkup(dbAdapter->getAdapter());
        mkup.findMarkupControl(*ret,key._a,key._b,key._c,key._d,key._e);
    }
    catch (...)
    {
        LOG4CXX_WARN(_logger,"DB exception in MarkupControlDAO::create");
        destroyContainer(ret);
        throw;
    }*/
  return result;
}

void MarkupControlDAO::destroy (MarkupControlKey key,
                                std::vector<MarkupControl*>* recs)
{
  if (!destroyPooledVector(recs))
  {
    std::vector<MarkupControl*>::const_iterator it(recs->begin()), itend(recs->end());
    for ( ; it != itend; ++it) delete *it;
    delete recs;
  }
}

sfc::CompressedData*
MarkupControlDAO::compress(const std::vector<MarkupControl*>* vect) const
{
  return compressVector(vect, _poolObjects);
}

std::vector<MarkupControl*>*
MarkupControlDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressEntry<MarkupControl>(compressed);
}

}// tse
