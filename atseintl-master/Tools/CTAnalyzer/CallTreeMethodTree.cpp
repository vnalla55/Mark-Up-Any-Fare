#include <string>
#include <vector>
#include <iostream>
#include <set>
#include <map>
#include <fstream>
#include <boost/tokenizer.hpp>

#include "CTConstants.h"
#include "CTTokenVector.h"
#include "CallTreeHeader.h"
#include "CallTreeMethod.h"
#include "CallTreeMethodTree.h"

using namespace std;
using namespace tse;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool 
CallTreeMethodTree::addChild(CallTreeMethodTree*& child)
{
  //Make sure we have not added this child before
  if (!_children.empty())
  {
    if (std::count(_children.begin(), _children.end(), child) > 0)
    {
      return(false);
    }
  }

  _children.push_back(child);
      
  return(true);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool 
CallTreeMethodTree::addParent(CallTreeMethodTree*& parent)
{
  //Make sure we have not added this parent before
  if (!_parents.empty())
  {
    if (std::count(_parents.begin(), _parents.end(), parent) > 0)
    {
      return(false);
    }
  }

  _parents.push_back(parent);

  return(true);
}
