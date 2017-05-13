#include "Xform/CustomXMLParser/IXMLSchema.h"

#include "Util/BranchPrediction.h"
#include "Xform/CustomXMLParser/IAttributes.h"
#include "Xform/CustomXMLParser/IBaseHandler.h"
#include "Xform/CustomXMLParser/IXMLErrorMsgs.h"
#include "Xform/CustomXMLParser/IXMLUtils.h"

#include <stdexcept>

IXMLSchema::IXMLSchema (const ILookupMap &elemLookupMap,
                        const ILookupMap &attrLookupMap,
                        int numberOfAttributeNames,
                        IValueString attrValueArray[],
                        int attrRefArray[],
                        bool checkWellFormedness)
  : ISchemaBase(checkWellFormedness)
  , _elemLookupMap(elemLookupMap)
  , _attrLookupMap(attrLookupMap)
  , _maxNumberAttrs(numberOfAttributeNames)
  , _attrValueArray(attrValueArray)
  , _attrRefArray(attrRefArray)
  , _idx(-1)
{
  for (int i = 0; i < _maxNumberAttrs; ++i)
  {
    _attrRefArray[i] = -1;
  }
}

IXMLSchema::~IXMLSchema ()
{
}

void IXMLSchema::insertAttr (const char *pName,
                             size_t nameLength,
                             const char *pValue,
                             size_t valueLength) const
{
  IKeyString name(pName, nameLength);
  if (LIKELY(_checkWellFormedness))
  {
    checkName(name);
  }
  if (LIKELY(_idx != -1))
  {
    ILookupMap::const_iterator iter(_attrLookupMap.find(name));
    if (_attrLookupMap.end() != iter)
    {
      int idx(iter->second);
      IValueString &val = _attrValueArray[idx];
      if (UNLIKELY(!val.empty()))
      {
        throw std::runtime_error(_xmlErrorMsgs[DUPLICATEATTRIBUTENAME]
                                 + std::string(pName, nameLength) + ",val:"
                                 + std::string(val.c_str(), val.length()));
      }
      decode(pValue, valueLength);
      val.assign(pValue, valueLength);
      _attrRefArray[_attrsSize++] = idx;
    }
  }
}

void IXMLSchema::startElement (IBaseHandler &handler,
                               const IKeyString &) const
{
  if (_idx != -1)
  {
    IAttributes attrs(_maxNumberAttrs,
                      0 == _attrsSize ? nullptr : _attrValueArray);
    handler.startElement(_idx, attrs);
  }
  _idxStack.push(_idx);
}

void IXMLSchema::endElement (IBaseHandler &handler,
                             const IKeyString &name) const
{
  if (LIKELY(_checkWellFormedness))
  {
    onEndElement(name);
  }
  if (_idx != -1)
  {
    handler.endElement(_idx);
  }
  _idx = -1;
  size_t sz(_idxStack.size());
  if (LIKELY(sz > 0))
  {
    _idxStack.pop();
    if (sz > 1)
    {
      _idx = _idxStack.top();
    }
  }
}

void IXMLSchema::characters (IBaseHandler &handler,
                             const char *pValue,
                             size_t length,
                             bool decode) const
{
  if (_idx != -1)
  {
    ISchemaBase::characters(handler, pValue, length, decode);
  }
}

void IXMLSchema::newElement (const IKeyString &name) const
{
  int ref(-1);
  for (int i = 0; i < _attrsSize && (ref = _attrRefArray[i]) != -1; ++i)
  {
    _attrValueArray[ref].clear();
    _attrRefArray[i] = -1;
  }
  _attrsSize = 0;
  if (LIKELY(_checkWellFormedness))
  {
    onStartElement(name);
  }
  _idx = -1;
  ILookupMap::const_iterator iter(_elemLookupMap.find(name));
  if (iter != _elemLookupMap.end())
  {
    _idx = iter->second;
  }
}

void IXMLSchema::clear () const
{
  ISchemaBase::clear();
  _idx = -1;
  while (!_idxStack.empty())
  {
    _idxStack.pop();
  }
}
