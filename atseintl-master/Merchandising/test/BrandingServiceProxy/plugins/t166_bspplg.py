from BrandServiceProxy import anywhere
from BrandServiceProxy import add_xmlns
from BrandServiceProxy import XMLTransformerPlugin
from BrandServiceProxy import ET

import ast

def create_table_166_node(parent, ancillaries, container, item):
    container_node = ET.SubElement(parent, container)
    for ancillary in ancillaries:
        attr = ancillary.get('attr', {})
        attr = {str(k): str(v) for k, v in attr.items()}
        val = ancillary.get('val')
        node = ET.SubElement(container_node, item, attr)
        if val:
            node.text = str(val)

def add_fake_t166_item(root, con_log, program, brand, ancillaries, container, item):
    """Add Table 166 info to brand in given program"""
    program_node = root.find(anywhere('BrandProgram[@programID="{}"]'.format(program)))
    if program_node is None:
        con_log(log_category='plg_out', text='Program {} not found in BS response'.format(program))
        return root

    brand_node = None
    for bnode in program_node.findall(anywhere('Brand')):
        code = bnode.find(anywhere('Code'))
        if code.text == brand:
            brand_node = bnode
            break

    if brand_node is None:
        con_log(log_category='plg_out', text='Brand {} not found in program {}'.format(brand, program))
        return root

    create_table_166_node(brand_node, ancillaries, container, item)
    output = 'Found program {} and brand {}, inserting ancillaries {}'.format(program, brand, ancillaries)
    con_log(log_category='plg_out', text=output)
    return root


class t166(XMLTransformerPlugin):
    def plugin_init(self, config):
        config_dict = dict(config)
        container = config_dict.pop('container', 'AncillaryList')
        item = config_dict.pop('item', 'Ancillary')
        for key, value in config_dict.iteritems():
            try:
                program, brand = key.split('_')
                ancillaries = ast.literal_eval(value)
                self.add_transformation(add_fake_t166_item, program=program, brand=brand, ancillaries=ancillaries, container=container, item=item)
            except:
                raise ValueError('Unknown configuration option {:s}={:s}'.format(key, value))

    def default_config(self):
        return """[t166]
;program_brand=[list of ancillaries]
container=AncillaryList
item=Ancillary
123456_SV=[{'attr': {'Sequence': 1, 'SubCode': '03Z', 'Service': 'Z', 'Application': 'F'}, 'val': 'MILEAGE ACCRUAL'},
           {'attr': {'Sequence': 2, 'SubCode': 'P01', 'Service': 'C', 'Application': 'F'}, 'val': 'FREE CHECKED BAG'}]

123456_BZ=[{'attr': {'Sequence': 1, 'SubCode': '03Z', 'Service': 'Z', 'Application': 'F'}, 'val': 'MILEAGE ACCRUAL'},
           {'attr': {'Sequence': 2, 'SubCode': 'P01', 'Service': 'C', 'Application': 'F'}, 'val': 'FREE CHECKED BAG'},
           {'attr': {'Sequence': 3, 'SubCode': '04M', 'Service': 'Z', 'Application': 'F'}, 'val': '25 PERCENT MILES EARNED'}]"""
