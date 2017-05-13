#ifndef TAXES_TESTSERVER_XFORM_SELECTTAGSLIST_H
#define TAXES_TESTSERVER_XFORM_SELECTTAGSLIST_H
#include <rapidxml_wrapper.hpp>

namespace tax
{

class XmlTagsList;
class XmlTagsFactory;

const XmlTagsList& selectTagsList(const rapidxml::xml_document<>& xml, const XmlTagsFactory& tagsFactory);

} // namespace tax

#endif

