import re

from BrandServiceProxy import add_xmlns
from BrandServiceProxy import anywhere
from BrandServiceProxy import ET
from BrandServiceProxy import find_or_create
from BrandServiceProxy import XMLTransformerPlugin


def find_brand_codes(root):
    return root.findall(anywhere('Brand/{}'.format(add_xmlns('Code'))))


def map_brandcodes(root, con_log, bcmap):
    """ Transform brandcodes in response to brandcodes from config """
    for bc in find_brand_codes(root):
        if bc.text in bcmap:
            output = 'Changed {} to {}'.format(bc.text, bcmap[bc.text])
            con_log(log_category='plg_out', text=output)
            bc.text = bcmap[bc.text]
    return root


def rep(s, m):
    a, b = divmod(m, len(s))
    return s * a + s[:b]


def change_length(root, con_log, new_len):
    """ Changes length of each brandcode to length from config by cloning original brandcode """
    for bc in find_brand_codes(root):
        new_code = rep(bc.text, new_len)
        output = 'Changed {} to {}'.format(bc.text, new_code)
        con_log(log_category='plg_out', text=output)
        bc.text = new_code
    return root


class bcmap(XMLTransformerPlugin):
    def plugin_init(self, config):
        for key, value in config:
            if key == 'map_brandcodes':
                bcmap_values = [tuple(bm.split('=')) for bm in value.split(',')]
                bcmap_values = [(k.strip(), v.strip()) for k,v in bcmap_values]
                self.add_transformation(map_brandcodes, bcmap=dict(bcmap_values))
            elif key == 'change_length':
                value = int(value)
                if value < 0:
                    raise ValueError('Length cannot be < 0')
                self.add_transformation(change_length, new_len=value)
            else:
                raise ValueError('Unknown configuration option {:s}={:s}'.format(key, value))
    def default_config(self):
        return """[bcmap]
#map_brandcodes: SV=BZ, FL=SV
change_length: 10"""
