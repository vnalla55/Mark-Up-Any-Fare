#pragma once

#include "Common/TseStlTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Xform/XMLWriter.h"

namespace tse
{

class XMLShoppingResponse;
class Itin;
class ShoppingTrx;

class GenerateSIDForShortCutPricing
{
public:
  typedef XMLWriter::Node Node;

  GenerateSIDForShortCutPricing(XMLShoppingResponse& shoppingResponse,
                                SopIdVec& sops,
                                XMLWriter& writer,
                                const ShoppingTrx* const trx)
    : _shoppingResponse(shoppingResponse), _sops(sops), _writer(writer), _trx(trx)
  {
  }

  void process(const Itin*);

private:
  XMLShoppingResponse& _shoppingResponse;
  SopIdVec& _sops;
  XMLWriter& _writer;
  const ShoppingTrx* const _trx;
};
}
