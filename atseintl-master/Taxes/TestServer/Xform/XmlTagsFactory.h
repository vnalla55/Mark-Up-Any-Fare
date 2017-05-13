// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#pragma once

#include <stdexcept>
#include <boost/ptr_container/ptr_vector.hpp>

#include "AtpcoTaxes/rapidxml/rapidxml_wrapper.hpp"
#include "TestServer/Xform/XmlTagsList.h"

namespace tax
{
class InputRequestWithCache;

class XmlTagsFactory
{
public:
  void registerList(XmlTagsList* xmlTagsList)
  {
    xmlTagsLists.push_back(xmlTagsList);
  }

  const XmlTagsList& getList(const rapidxml::xml_node<>* rootNode) const
  {
    if (!rootNode)
      throw std::domain_error("Invalid XML");

    for (std::size_t index = 0; index < xmlTagsLists.size(); ++index)
    {
      const XmlTagsList& list = xmlTagsLists[index];
      if (rootNode->name() == list.getTagName<InputRequestWithCache>())
        return list;
    }

    throw std::domain_error("Invalid root node");
  }

private:
  boost::ptr_vector<XmlTagsList> xmlTagsLists;
};

} // namespace tax
