from distutils.core import setup
import py2app

import os

plist = dict( 
   NSAppleScriptEnabled = 'YES',
   CFBundleIndentifier = 'org.lilypond.lilypond',
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
   NSHumanReadableCopyright = "(c) 2005 LilyPond Software Design\nLicensed under the GNU General Public License\nLilyPond comes with NO WARRANTY", 
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
		'Credits.html',
		'/sw/lib/python2.4/subprocess.py',
		'URLHandle.scriptSuite',
		'URLHandle.scriptTerminology',
		'Welcome-to-LilyPond-MacOS.ly'
		],
    options=dict(py2app=dict(plist=plist)),
)


