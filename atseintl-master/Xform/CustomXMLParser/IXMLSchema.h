#pragma once

#include "Xform/CustomXMLParser/ISchemaBase.h"

#include <tr1/unordered_map>

typedef std::tr1::unordered_map<IKeyString, int, IKeyStringHasher> ILookupMap;

class IXMLSchema : public ISchemaBase
{
 public:
  IXMLSchema (const ILookupMap &elemLookupMap,
              const ILookupMap &attrLookupMap,
              int numberOfAttributeNames,
              IValueString attrValueArray[],
              int attrRefArray[],
              bool checkNames);
  virtual ~IXMLSchema ();

  void insertAttr(const char* name,
                  size_t nameLength,
                  const char* value,
                  size_t valueLength) const override;
  void startElement(IBaseHandler& handler, const IKeyString& name) const override;
  void endElement(IBaseHandler& handler, const IKeyString& name) const override;
  void
  characters(IBaseHandler& handler, const char* value, size_t length, bool decode) const override;
  void newElement(const IKeyString& name) const override;
  void clear() const override;

private:
  const ILookupMap &_elemLookupMap,
                   &_attrLookupMap;
  const int _maxNumberAttrs;
  IValueString *_attrValueArray;
  int *_attrRefArray;
  mutable std::stack<int> _idxStack;
  mutable int _idx;
};
