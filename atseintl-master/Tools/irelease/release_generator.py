#! /usr/bin/env python2.5
import datetime
import re
import subprocess


getFullUsernameScript = r"whoami | xargs finger -l | grep 'Name: ' | awk -F 'Name: ' '{print $2}'"
getCurrentActivityScript = r"cleartool lsact -cact -short"
getFilesFromCurrentActivity = r"cleartool lsact -long -cact | grep '@@' | awk -F '@@' '{print $1}' | sort | uniq"

'''
Generates example of release note in given group.
'''
class DefaultReleaseNoteGenerator:

	def __init__(self, enableFileListGenerating):
		self._enableFileListGenerating = enableFileListGenerating

	'''
	Converts ConfigItem to relevant string of release note format.
	'''
	def _tagToString(self, item):
		if item.tagtype == "text" or item.tagtype == "multiline":
			# text fields can have default value
			default = item.default
			# generating current date
			if default == "now()":
				default = datetime.datetime.now().strftime("%b %d, %Y")
			elif default == "user()":
				default = ""
				
				try:
				 	unix_whoami = subprocess.Popen( ["-c", getFullUsernameScript], stdout=subprocess.PIPE, stderr=open('/dev/null', 'w'), shell=True )
					default = unix_whoami.communicate()[0].strip()
				except:
					# simply skip generating username and leave field blank
					pass
			
			elif default == "activity()":
				default = ""
				
				try:
				 	ctactivity = subprocess.Popen( ["-c", getCurrentActivityScript], stdout=subprocess.PIPE, stderr=open('/dev/null', 'w'), shell=True )
					default = ctactivity.communicate()[0].strip()
				except Exception as e:
					# simply skip generating activity and leave field blank
					pass

			elif default == "files()":
				default = ""
				if self._enableFileListGenerating:
					try:
					 	ctfiles = subprocess.Popen( ["-c", getFilesFromCurrentActivity], stdout=subprocess.PIPE, stderr=open('/dev/null', 'w'), shell=True )
						files = ctfiles.communicate()[0].strip()
						if files:
							default = "\n\t\t" + re.sub(r'\n\s*', r'\n\t\t', files)
					except Exception as e:
						# simply skip generating files and leave field blank
						print e
						pass

			return '{0}: {1}'.format(item.value, default)
		elif item.tagtype == "checkbox":
			# checkbox and input-checkbox can have default value 'checked' or 'unchecked'
			default = "X" if item.default == "checked" else " "
			return '[{0}] {1}'.format(default, item.value)
		elif item.tagtype == "input-checkbox":
			default = "X" if item.default == "checked" else " "
			return '[{0}] {1}: '.format(default, item.value)
		elif item.tagtype == "fixed-content":
			return '{0}:'.format(item.value)
		else:
			return item.value

	'''
		Needs name of group and ConfigGroup instance of this group.
	'''
	def generate(self, groupName, configGroup):
		doc = ""
		doc += "[{0}]\n\n".format(groupName)

		for item in configGroup.list:
			doc += self._tagToString(item)
			doc += '\n'

			for child in item.children:
				doc += '\t\t'
				doc += self._tagToString(child)
				doc += '\n'

		return doc
	
