from distutils.core import setup
import py2app

plist = dict( 
   NSAppleScriptEnabled = 'YES',
   CFBundleIndentifier = 'org.lilypond.lilypond',
   CFBundleVersion = "0.0",
   LSMinimumSystemVersion = "10.2",
   CFBundleGetInfoString = "Copyright 2005 LilyPond Software Design",
   CFBundleURLTypes = [
	dict(CFBundleURLSchemes = ['textedit'],
	     CFBundleURLName = "text editor via url", 
	     CFBundleTypeRole = 'Editor',
             NSDocumentClass = "TinyTinyDocument",
	     ),
	],
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
		'/sw/lib/python2.4/subprocess.py',
		'URLHandle.scriptSuite',
		'URLHandle.scriptTerminology',
		'Welcome-to-LilyPond-MacOS.ly'
		],
    options=dict(py2app=dict(plist=plist)),
)


