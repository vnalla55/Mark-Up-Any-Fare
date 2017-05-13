#ifndef CALL_TREE_METHOD_TREE_H
#define CALL_TREE_METHOD_TREE_H

#include <vector>
#include "CallTreeMethod.h"

namespace tse
{

class CallTreeMethodTree
{
 private:
  CallTreeMethod* _method;
  std::vector<CallTreeMethodTree*> _parents;
  std::vector<CallTreeMethodTree*> _children;
  bool _isRoot;

 public:
  explicit CallTreeMethodTree() :
    _method(nullptr),


    _isRoot(false)
  {
  }

  ~CallTreeMethodTree()
  {
    if (_method != nullptr)
    {
      delete _method;
      _method = nullptr;
    }

    if (!_children.empty())
    {
      std::vector<CallTreeMethodTree*>::iterator cIter = _children.begin();
      std::vector<CallTreeMethodTree*>::iterator cEIter = _children.end();
      for (; cIter != cEIter; ++cIter)
      {
        if (*cIter != nullptr)
        {
          delete *cIter;
          *cIter = nullptr;
        }
      }
    }
    _method = nullptr;
    _children.clear();
  }

  bool addChild(CallTreeMethodTree*& child);
  bool addParent(CallTreeMethodTree*& parent);

  CallTreeMethod*& method() { return(_method);};
  const CallTreeMethod* method() const { return(_method);};

  std::vector<CallTreeMethodTree*>& parents() { return(_parents);};
  const std::vector<CallTreeMethodTree*>& parents() const { return(_parents);};

  std::vector<CallTreeMethodTree*>& children() { return(_children);};
  const std::vector<CallTreeMethodTree*>& children() const {return(_children);};

  bool& isRoot() { return(_isRoot);};
  const bool& isRoot() const { return(_isRoot);};
};

} //End namespace tse

#endif //CALL_TREE_METHOD_TREE_H
