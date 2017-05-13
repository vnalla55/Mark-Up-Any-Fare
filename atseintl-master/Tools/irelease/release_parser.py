#! /usr/bin/env python2.5
import re

'''
    Creates diff for two release note files.
'''
class ReleaseNoteFileDiffer:
    def compare(self, first, second):
        def areDifferent(item1, item2):
            return item1 != item2
    
        def getNewOrChangedItems(firstList, secondList): 
            return filter(lambda l2Item: 
                    all(map(lambda l1Item: 
                                areDifferent(l1Item, l2Item)
                        ,firstList))
            ,secondList)

        sameGroupNames = [group for group in second if group in first]
        sameGroups = map(lambda group: (group, getNewOrChangedItems(first[group], second[group])), sameGroupNames)

        newGroupNames = [group for group in second if group not in first]
        newGroups = map(lambda group: (group, second[group]), newGroupNames)

        nonEmptyGroups = filter(lambda (group, grouplist): grouplist, sameGroups + newGroups)
        return dict(nonEmptyGroups)

'''
    Creates item tree from release note file in format (dict of lists):
    
    {
        group1: [item1, item2, ...]
        group2: [item1, item2, ...]
        ...
    }
'''
class ReleaseNoteFileParser:
    def __init__(self):
        self._groups = {}
        self._currentGroup = "*"
        self._currentItem = ""

    def _parseLine(self, line):
        # matching [GROUP_NAME]
        groupMatch = re.match(r'^\[(\w*)\]\s*$', line)
        if groupMatch: # matched
            self._parseLine("") # finish everything in previous group
            groupName = groupMatch.groups()[0]

            # group encountered for first time so far
            if not groupName in self._groups:
                self._groups[groupName] = []

            self._currentGroup = groupName

        # item end
        elif line.strip() == "": 
            # if we have current item already (not just after group marker or another blank line)
            if self._currentItem: 
                if self._currentGroup == "*" and not "*" in self._groups:   # default group
                    self._groups["*"] = []

                self._groups[self._currentGroup].append(self._currentItem)
                self._currentItem = ""

        # casual item line
        else: 
            self._currentItem += (line.rstrip() + '\n')

    def _getResult(self):
        self._parseLine("") # finish everything
        return self._groups

    def parse(self, fileContent):
        for line in fileContent.splitlines(True):
            self._parseLine(line)

        return self._getResult()

    def parseFile(self, fileToParse):
        for line in fileToParse:
            self._parseLine(line)

        return self._getResult()
