#include "TestServer/Xform/SelectTagsList.h"
#include "TestServer/Xform/XmlTagsFactory.h"

namespace tax
{

const XmlTagsList&
selectTagsList(const rapidxml::xml_document<>& xml, const XmlTagsFactory& tagsFactory)
{
  rapidxml::xml_node<>* rootNode = xml.first_node();
  return tagsFactory.getList(rootNode);
}

} // namespace tax

