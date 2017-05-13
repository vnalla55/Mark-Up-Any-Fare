from BrandServiceProxy import ET
from BrandServiceProxy import XMLTransformerPlugin


def swap_with_text(root, con_log, text):
    """ Swaps original response with text from config """
    output = 'Set custom response: {}'.format(text)
    con_log(log_category='plg_out', text=output)
    return text


def swap_with_xml(root, con_log, xml_string):
    """ Swaps original xml with custom xml """
    try:
        xml = ET.XML(xml_string)
        output = 'Set custom xml: {}'.format(xml_string)
        con_log(log_category='plg_out', text=output)
        return xml
    except ET.ParseError as e:
        raise ValueError('Failed to parse custom xml: {}'.format(xml_string))


class custom(XMLTransformerPlugin):
    def plugin_init(self, config):
        for key, value in config:
            if key == 'text':
                self.add_transformation(swap_with_text, text=value)
            elif key == 'xml':
                self.add_transformation(swap_with_xml, xml_string=value)
            else:
                raise ValueError('Unknown configuration option {:s}={:s}'.format(key, value))

    def default_config(self):
        return """[custom]
# Custom text
text: UNKNOWN EXCEPTION;

# Custom response xml
#xml: <tag>value</tag>"""
