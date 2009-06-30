from setuptools import setup

import os

plist = dict( 
   NSAppleScriptEnabled = 'YES',
   CFBundleIdentifier = 'org.lilypond.lilypond',
   LSMinimumSystemVersion = "10.2",
   CFBundleURLTypes = [
	dict(CFBundleURLSchemes = ['textedit'],
	     CFBundleURLName = "text editor via url", 
	     CFBundleTypeRole = 'Editor',
             NSDocumentClass = "TinyTinyDocument",
	     ),
	],
   CFBundleShortVersionString = open ('VERSION').read (),
   CFBundleVersion = ('Build from '
		      + os.popen ('date +"%d-%m-%Y %H:%M"').read ()),
   NSHumanReadableCopyright = "(c) 2009 LilyPond Software Design\nLicensed under the GNU General Public License\nLilyPond comes with NO WARRANTY", 
   CFBundleDocumentTypes = [
        dict(
            CFBundleTypeExtensions=["ly",],
            CFBundleTypeName="LilyPond source",
            CFBundleTypeRole="Editor",
            NSDocumentClass="TinyTinyDocument",
        ),
    ]
)


# ugh , need to run lsregister manually

setup(
    app=["LilyPond.py"],
    data_files=["MainMenu.nib",
		"TinyTinyDocument.nib",
		'ProcessLog.nib',
		'lilycall.py',
		'ProcessLog.py',
		'/Library/Frameworks/Python.framework/Versions/2.6/lib/python2.6/shutil.py',
		'Credits.html',
		'URLHandle.scriptSuite',
		'URLHandle.scriptTerminology',
		'Welcome-to-LilyPond-MacOS.ly'
		],
    options=dict(py2app=dict(plist=plist)),
    setup_requires=["py2app"],
)


