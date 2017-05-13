    #! /usr/bin/env python2.5
from xml.dom import minidom
import re

'''
Configuration tree:
    
    Config:
        .bgcolor
        .groups
        {
            ConfigGroup-1:
                .color
                .visibility
                .list
                [
                    ConfigItem-1:
                        .value
                        .tagtype
                        .required
                        .default
                        .optional
                        .children
                            ConfigItem-subitem1
                                .value
                                (...)
                            ConfigItem-subitem2
                                (...)
                            (...)

                    ConfigItem-2:
                        (...)
                ]

            ConfigGroup-2:
                .color
                .visibility
                .list
                    [...]
            (...)

        }


'''

class Config():
    def __init__(self, groups, bgcolor):
        self._groups = groups              # groups (dict) 
        self._bgcolor = bgcolor            # background color of generated HTML document

    @property
    def groups(self):
        return self._groups

    @property
    def bgcolor(self):
        return self._bgcolor

class ConfigGroup():
    def __init__(self, glist, color, visibility):
        self._list = glist                  # item lists
        self._color = color                 # background color of table with release note in this group
        self._visibility = visibility       # is this group visible in generated HTML document

    @property
    def list(self):
        return self._list

    @property
    def color(self):
        return self._color

    @property
    def visibility(self):
        return self._visibility

class ConfigItem(object):
    def __init__(self, value, tagtype, required, optional, default, children):
        self._value = value         # name of item
        self._tagtype = tagtype     # type of item (one of: text, multiline, fixed-content, checkbox, input-checkbox)
        self._required = required   # is field required to be filled (only for text type)
        self._optional = optional   # if true field may be omitted
        self._children = children   # subitems (for fixed-content type)
        self._default = default     # default value used in generator
                                    #   - for (input-)checkboxes use 'unchecked' or 'checked'
                                    #   - for text fields you can use 'now()' for generating current date
    
    @property
    def value(self):
        return self._value;
    @property
    def tagtype(self):
        return self._tagtype;
    @property
    def required(self):
        return self._required;
    @property
    def optional(self):
        return self._optional;
    @property
    def children(self):
        return self._children;
    @property
    def default(self):
        return self._default;

'''
    Creates Config tree from xml file.
'''
class ConfigBuilder: 
    def __init__(self, filename):
        self._filename = filename
   
    '''
        Creates ConfigItem from single xml <item> node
    '''
    def _getItemFromNode(self, node):
        value = node.attributes["value"].value
        
        tagtype = "text"
        if node.hasAttribute("type"):
            tagtype = node.attributes["type"].value
        
        required = False
        if node.hasAttribute("required"):
            required = ( node.attributes["required"].value == "true" )

        optional = False
        if node.hasAttribute("optional"):
            optional = ( node.attributes["optional"].value == "true" )

        default = ""
        if node.hasAttribute("default"):
            default = node.attributes["default"].value 

        children = []
        for child in node.getElementsByTagName('subitem'):
            children.append(self._getItemFromNode(child))

        return ConfigItem(value, tagtype, required, optional, default, children)

    def build(self):
        try:
            xmldoc = minidom.parse(self._filename)
        except IOError:
            raise Exception("Error opening configuration file: " + self._filename)

        cgroups = {}
        for group in xmldoc.getElementsByTagName('group'):
            
            # get attributes: name, color and visibility
            groupName = group.attributes["name"].value

            groupColor = ""
            if group.hasAttribute("color"):
                groupColor = group.attributes["color"].value
         
            groupVisibility = True
            if group.hasAttribute("visible") and group.attributes["visible"].value == "false":
                groupVisibility = False

            # convert all item nodes to ConfigItems
            items = map(self._getItemFromNode, group.getElementsByTagName('item'))

            # assign group to group list (dict)
            cgroups[groupName] = ConfigGroup(items, groupColor, groupVisibility)   

        # background color of release notes HTML document
        bgcolor = "white"
        if xmldoc.getElementsByTagName('configrelease')[0].hasAttribute("color"):
            bgcolor = xmldoc.getElementsByTagName('configrelease')[0].attributes["color"].value
       
        return Config(cgroups, bgcolor)
      

