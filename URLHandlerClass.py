
from PyObjCTools import NibClassBuilder, AppHelper
from Foundation import *
from AppKit import *
import re
import string
import urllib

NibClassBuilder.extractClasses("MainMenu")

class URLHandlerClass(NibClassBuilder.AutoBaseClass):  
    def performDefaultImplementation(self):
        urlString = self.directParameter()
        self.openURL(urlString)
        return None

    def openURL (self, urlString):
        urlString = urllib.unquote (urlString)
        m = re.match ("^textedit://([^:]*):?([0-9]*):?([0-9]*):?([0-9]*)$", urlString)
        if m == None:
            NSLog ("URL doesn't match")
            return None
        
        line = 1
        char = 1
        column = 1
        try:
            line = string.atoi (m.group (2))
            char = string.atoi (m.group (3))
            column = string.atoi (m.group (4))
        except ValueError:
            pass
        
        path = m.group (1)
        self.jumpFile (path, line, char)

    def charCount (self, str, line, char):
        line -= 1
        char -= 1

        lines = string.split (str, '\n')
        lineChars = sum (map (lambda x: len (x) + 1, lines[:line]))
        if line < len (lines):
            lineChars += min (char, len (lines[line]))
        return lineChars
        
    def jumpFile (self, path, line, char):
        NSLog ("Jumping to %s %d %d\n" % (path, line, char))

        controller = NSDocumentController.sharedDocumentController ()
        doc = controller.openDocumentWithContentsOfFile_display_ (path, True)
        textView = doc.textView
        range = NSRange()
        str = doc.textView.string()
        range.location = self.charCount (str, line, char)
        range.location = min (range.location, len (str))
        range.length = 1
        
        textView.setSelectedRange_ (range)
        textView.scrollRangeToVisible_ (range)
        return None

if __name__ == "__main__":
    AppHelper.runEventLoop()


