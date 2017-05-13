#! /usr/bin/env python2.5
from collections import deque
import re
from copy import copy

"""
    Handler used in release note validator. 
    Creates HTML version of validated release notes with big red errors.
"""
class HTMLDecoratingValidationHandler:
    def __init__(self):
        self._content = {}
        self._currentGroup = None
        self._errorList = []

    def startGroup(self, groupName):
        if groupName not in self._content:
            self._content[groupName] = []
        
        self._currentGroup = self._content[groupName]

    def startItem(self):
        self._currentGroup.append("")

    def error(self, line, msg):
        self._currentGroup[-1] += (  line + '<br /><b><color=red>' + msg + '</color></b>' + '\n') 
        self._errorList.append(msg)

    def passed(self, line):
        self._currentGroup[-1] +=  line + '\n'

    def getResult(self):
        return self._content

    def getErrors(self):
        return self._errorList

"""
    Validator of release note file.
    Needs schema of proper release note from configuration.
    All errors are passed to validation handler.
"""        
class ReleaseNoteValidator:
    def __init__(self, errorHandler):
        self._handler = errorHandler

    def _validateGroup(self, group, groupName):
        for item in group:
            self._handler.startItem()


            if groupName in self._config.groups:
                groupConfig = self._config.groups[groupName]
            else:
                # get default config
                groupConfig = self._config.groups["*"]

            validator = ItemValidator(groupConfig.list)
        
            for line in item.splitlines():
                # for errors not having own line like 'missing item'
                additionalErrors = []

                errmsg = validator.validateLine(line, additionalErrors)

                # append additional errors first to second column of table
                for msg in additionalErrors:
                    self._handler.error('  ', msg)

                if errmsg:
                    if not ':' in line:
                        line += ':'# to make errors more readable

                    self._handler.error(line, errmsg)
                else:
                    self._handler.passed(line)
            
            # validator needs information about item end
            for errmsg in validator.finishValidation():
                self._handler.error('  ', errmsg)

    def validate(self, content, config):
        self._config = config

        # validate group by group
        for group in content:
            self._handler.startGroup(group)
            self._validateGroup(content[group], group)


'''
    Silent type of work: no result means passed validation.
    Results contain validation errors.
    Validates item line by line.

    Uses expected elements queue to determine expected order.
    If element is not found on the top of queue it looks for matching element 
      in all not validated so far elements list and informs about wrong order.


'''
class ItemValidator:
    def __init__(self, groupConf):
        self._groupConfig = groupConf

        # stores last used element validator for multiline elements validation
        self._currentElemValidator = None

        # queue of elements in order from configuration
        self._expectedElems = deque(map(lambda confElement: confElement.value, groupConf))

        # elements that can be omitted
        self._optionalElems = map(lambda confElement: confElement.value, filter(lambda confElement: confElement.optional, groupConf))

        # all elements not validated so far
        self._allElems = copy(self._expectedElems)

        # create validators for all elements
        factory = ElemValidatorsFactory()
        self._allValidators = dict(map(lambda confElement: (confElement.value, factory.getValidatorForElement(confElement)), groupConf))

    def validateLine(self, line, additionalErrors):
        # we are not expecting any element more
        if not self._expectedElems:
            return 'Extra position encountered! Expected end of item but got "{0}"'.format(line.strip())

        # if encountered subelement (determined by indendation):
        tagMatch = re.match("^\s+.*$", line)
        if tagMatch:
            if not self._currentElemValidator: # when subelement is first element
                return 'Subitem "{0}" encountered without item before!'.format(line.strip())
            else:
                return self._currentElemValidator.validateSubitem(line.lstrip(), additionalErrors)

        # we need to finish previous validation (important for multiline elements)
        elif self._currentElemValidator:
            msg = self._currentElemValidator.finishValidation()
            if msg:
                additionalErrors.extend(msg)

        # find matching element in allElems list
        for el in self._allElems:
            if self._allValidators[el].match(line):
                elemName = el
                self._currentElemValidator = self._allValidators[elemName]
                break
        else:
            self._currentElemValidator = None   # prevents finishing validation twice
            return 'Unexpected item "{0}"'.format(line.strip())

        # now we can remove this element (we dont want to have possibility to validate twice the same)
        self._allElems.remove(elemName)     
        
        wrongElemOrder = False
        if elemName in self._expectedElems:
            # remove all expectedElems from queue before our current element
            while self._expectedElems and elemName != self._expectedElems[0]:
                if not self._expectedElems[0] in self._optionalElems: # if not has optional flag
                    additionalErrors.append('Missing expected item "{0}"'.format(self._expectedElems[0]))
       
                self._expectedElems.popleft()

            self._expectedElems.popleft()
        else:
            # we removed this element from expected queue before
            wrongElemOrder = True

        errmsg = self._currentElemValidator.validate(line)
        if errmsg:
            return errmsg
        elif wrongElemOrder:
            # element validates but order is wrong
            return 'Wrong position of element "{0}"'.format(elemName)

    def finishValidation(self):
        # filter expected elems from optionals
        self._expectedElems = filter(lambda elem: elem not in self._optionalElems, self._expectedElems)

        # print all the rest of expected elems 
        additionalErrors = []
        if self._currentElemValidator:
            msg = self._currentElemValidator.finishValidation()
            if msg:
                additionalErrors.extend(msg)

        additionalErrors.extend(map(lambda item: 'Missing expected item "{0}"'.format(item), self._expectedElems))
        return additionalErrors
        

'''
    Creates validator from given configuration entry for element.
'''
class ElemValidatorsFactory:
    def getValidatorForElement(self, confElement):
        if confElement.tagtype == "text":
            return TextElemValidator(confElement.value, confElement.required, False)
        elif confElement.tagtype == "multiline":
            return TextElemValidator(confElement.value, confElement.required, True)
        elif confElement.tagtype == "checkbox":
            return CheckBoxElemValidator(confElement.value)
        elif confElement.tagtype == "input-checkbox":
            return InputCheckBoxElemValidator(confElement.value)
        elif confElement.tagtype == "fixed-content":
            return FixedElemValidator(confElement.value, confElement.children)

'''
    Validator for <item type="text" /> and <item type="multiline" />

    match() only checks item value, validate() check properness of item
'''
class TextElemValidator:
    def __init__(self, value, required, multiline):
        self._value = value
        self._required = required
        self._multiline = multiline
        self._empty = True

    def match(self, line):
        return bool(re.match("^"+ self._value + ":(.*)$", line.strip()))

    def validate(self, line):
        tagMatch = re.match("^"+ self._value + ":(.*)$", line.strip())
        if not self._multiline and self._required and tagMatch.groups()[0] == "":
            return 'Element "{0}" cannot be empty!'.format(self._value)

        if tagMatch.groups()[0] != "":
            self._empty = False

    def validateSubitem(self, line, additionalErrors):
        if not self._multiline:
            return 'Element "{0}" cannot have multiple lines'.format(self._value)
        else:
            if line.strip() != "":
                self._empty = False

    def finishValidation(self):
        if self._multiline and self._required and self._empty:
           return ['Element "{0}" cannot be empty!'.format(self._value)]
        

'''
    Validator for <item type="checkbox" />
'''
class CheckBoxElemValidator:
    def __init__(self, value):
        self._value = value

    def match(self, line):
        return bool(re.match("^\[(X|x| )\]\s+"+ self._value + "$", line.strip()))
        
    def validate(self, line):
        return

    def finishValidation(self):
        return

'''
    Validator for <item type="input-checkbox" />
'''
class InputCheckBoxElemValidator:
    def __init__(self, value):
        self._value = value

    def match(self, line):
        return bool(re.match("^\[(X|x| )\]\s+"+ self._value + ":(.*)$", line.strip()))
        
    def validate(self, line):
        tagMatch = re.match("^\[(X|x| )\]\s+"+ self._value + ":(.*)$", line.strip())
        if tagMatch.groups()[0] in ('x', 'X') and tagMatch.groups()[1] == "":
            return 'Checked input-checkbox text field cannot be empty!'

    def finishValidation(self):
        return

'''
    Validator for <item type="fixed-content" />
'''
class FixedElemValidator:
    def __init__(self, value, subitemsConf):
        self._value = value
        self._subitemValidator = ItemValidator(subitemsConf)

    def match(self, line):
        return bool(re.match("^"+ self._value + ":(.*)$", line.strip()))

    def validate(self, line):
        tagMatch = re.match("^"+ self._value + ":(.*)$", line.strip())
        if tagMatch.groups()[0] != "":
            return 'Fixed content field cannot have text after semicolon!'

    def validateSubitem(self, line, additionalErrors):
        return self._subitemValidator.validateLine(line, additionalErrors)

    def finishValidation(self):
        return self._subitemValidator.finishValidation()
