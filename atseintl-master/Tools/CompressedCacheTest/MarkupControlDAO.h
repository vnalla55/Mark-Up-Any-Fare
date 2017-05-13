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

#pragma once

#include "HashKey.h"
#include "DataAccessObject.h"
#include "TseTypes.h"

namespace tse
{
typedef HashKey<int> MarkupControlKey;
class MarkupControl;

class MarkupControlDAO : public DataAccessObject<MarkupControlKey, std::vector<MarkupControl*>>
{
public:
  virtual sfc::CompressedData *
    compress (const std::vector<MarkupControl*> *vect) const;

  virtual std::vector<MarkupControl*> *
    uncompress (const sfc::CompressedData &compressed) const;

  MarkupControlDAO()
  {
  }
  std::vector<MarkupControl*>* create(MarkupControlKey key);
  CreateResult<std::vector<MarkupControl*>> create(MarkupControlKey key,
                                                    int);
  void destroy(MarkupControlKey key,
               std::vector<MarkupControl*>* t);
};
} // namespace tse
