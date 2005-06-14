"""TinyTinyEdit -- A minimal Document-based Cocoa application."""


from PyObjCTools import NibClassBuilder, AppHelper
from Foundation import NSBundle


NibClassBuilder.extractClasses("TinyTinyDocument")


import URLHandlerClass
import lilycall
import subprocess
import os

from ProcessLog import ProcessLogWindowController

# class defined in TinyTinyDocument.nib
class TinyTinyDocument(NibClassBuilder.AutoBaseClass):
    # the actual base class is NSDocument
    # The following outlets are added to the class:
    # textView

    startupPath = None  # fallback if instance has no startupPath.
    
    def windowNibName(self):
        return "TinyTinyDocument"

    def readFromFile_ofType_(self, path, tp):
        if self.textView is None:
            # we're not yet fully loaded
            self.startupPath = path
        else:
            # "revert"
            self.readFromUTF8(path)
            
        return True

    def writeToFile_ofType_(self, path, tp):
        f = file(path, "w")
        text = self.textView.string()
        f.write(text.encode("utf8"))
        f.close()

        return True

    def windowControllerDidLoadNib_(self, controller):
        if self.startupPath:
            self.readFromUTF8 (self.startupPath)
        else:
            appdir = NSBundle.mainBundle().bundlePath()
            prefix = appdir + "/Contents/Resources"
            self.readFromUTF8 (prefix + '/Welcome-to-LilyPond-MacOS.ly')

    def readFromUTF8(self, path):
        f = file(path)
        text = unicode(f.read(), "utf8")
        f.close()
        self.textView.setString_(text)

    def compileFile_ (self, sender):
        if 0:
            self.saveDocumentWithDelegate_didSaveSelector_contextInfo_ (self,
                                                                        "compileDidSaveSelector:",
                                                                        None)
        if self.fileName():
            self.compileMe ()
        else:
            self.saveDocument_ (None)
            

    def compileDidSaveSelector_ (doc, didSave, info):
        print "I'm here"
        if didSave:
            doc.compileMe ()
            
    def compileMe (self):
        bundle =  NSBundle.mainBundle ()         
        appdir = NSBundle.mainBundle().bundlePath()

        call = lilycall.Call (appdir, [self.fileName()])
        call.reroute_output = 1
        
        pyproc = call.get_process()

        if not self.processLogWindowController:
            self.processLogWindowController = ProcessLogWindowController()
        else:
            self.processLogWindowController.showWindow_ (None)
            
        wc = self.processLogWindowController
        wc.setWindowTitle_ ('LilyPond -- ' + self.fileName())
        wc.runProcessWithCallback (pyproc, lambda y: call.open_pdfs ())

if __name__ == "__main__":
    AppHelper.runEventLoop()
