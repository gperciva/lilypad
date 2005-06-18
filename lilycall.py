#!/usr/bin/python

#
# Pythonic interface to LilyPond call. 
#


import os
import sys
import string
import re
import subprocess

debug = 0

################################################################
# general utils

def file_is_newer (f1, f2):
	return os.stat (f1).st_mtime >  os.stat (f2).st_mtime

def writable_directory (dir):
	writable = 0
	try:
		testfile = os.path.join (dir,".testwrite")
		f = open (testfile, 'w')
		f.close()
		writable = 1
		os.unlink (testfile)
	except IOError:
		pass

	return writable


def quote_quotes (str):
	return re.sub ('"', '\\"', str)


################################################################
# lily call utils

def set_pango_paths (appdir):
	prefix = appdir + '/Contents/Resources' 
	pango_modules_file = prefix + '/etc/pango/pango.modules'
	
	pangorc = ('''[Pango]\nModuleFiles = "%s"\nModulesPath = "%s/lib/pango/1.4.0/modules"\n'''
		   % (quote_quotes (pango_modules_file),
		      quote_quotes (prefix)))

	pangorc_file = prefix + '/etc/pango/pangorc'

	if debug:
		print 'writing ', pangorc_file
		
	open (pangorc_file, 'w'). write(pangorc)

	pango_conf = open (pango_modules_file + '.in').read ()
	pango_conf = re.sub ('//', '/', pango_conf)

	ls = string.split (pango_conf, '\n')
	pango_conf = ''
	for l in ls: 
	   l = re.sub (r'^[^#].*build\.app(.*\.so)', r'"%s\1"' % appdir, l)
	   pango_conf += l + '\n'

	if debug:
		print 'writing ', pango_modules_file

	open (pango_modules_file, 'w').write (pango_conf)


def check_fontconfig (appdir):
	prefix = appdir + '/Contents/Resources' 
	my_sysfont_dir = prefix + '/share/SystemFonts'
	sysfont_dir = '/System/Library/Fonts'

	fc_cache = (prefix + '/font.cache-1')
	local_fc_conf = open (prefix + '/etc/fonts/local.conf','w')
	local_fc_conf.write ('''<?xml version="1.0"?>
<!DOCTYPE fontconfig SYSTEM "fonts.dtd">
<!-- /etc/fonts/local.conf file for local customizations -->
<fontconfig>
<cache>%s</cache> 
</fontconfig>''' % fc_cache)
	return not os.path.exists (fc_cache)

def get_env (prefix):
	p = ''
	try:
		p = os.environ['DYLD_LIBRARY_PATH']
	except KeyError:
		pass

	env = {} 
	env['DYLD_LIBRARY_PATH'] = prefix + '/lib' + ':' + p
	env['FONTCONFIG_PATH'] = prefix + '/etc/fonts/'
	env['GS_LIB'] = prefix + '/share/ghostscript/8.15/lib/'
	env['GUILE_LOAD_PATH'] = prefix + '/share/guile/1.6'
	env['LILYPONDPREFIX'] = prefix + '/share/lilypond/current'
	env['PANGO_RC_FILE'] = prefix + '/etc/pango/pangorc'
	env['PATH'] = prefix + '/bin/' + ':' + os.environ['PATH']
	return env


def get_command_line (appdir, args):
	binary = appdir + '/Contents/Resources/bin/lilypond'
	new_args = [binary,
		'-ddelete-intermediate-files']


	cwd = os.getcwd ()
	for a in args:
		if a[0] not in ['-','/']:
			a = os.path.join (cwd, a) 
		new_args.append (a)

	return new_args

def get_dest_dir (arguments):
	try:
		dest_dir = os.environ['LILYPOND_DESTDIR']
	except KeyError:
		dest_dir = ''
		
	for a in arguments:
		if (not dest_dir
		    and a and a[0] == '/'):
			dest_dir = os.path.split (a)[0]
			break

	if not dest_dir or not writable_directory (dest_dir):
		dest_dir = os.environ['HOME']+  '/Desktop'

	return dest_dir

################################################################
# the class

class Call:
	def check_app_dir (self, dir):
		self.error_string = ''
		if not writable_directory (dir):
			self.error_string = ('Cannot write program directory\n'
					     +'This program must be installed before using.')
			
		elif dir[0] <> '/':
			self.error_string = ("Application directory should be absolute. "
					     + "Was: %s\n"
					     % dir)

	def __init__ (self, appdir, args):
		self.check_app_dir (appdir)
		self.appdir = appdir
		self.env = get_env (appdir + '/Contents/Resources')
		self.args = get_command_line (appdir, args)
		self.executable = self.args[0]
		self.args[0] = os.path.split (self.args[0])[1]
		self.cwd = get_dest_dir (self.args[1:])
		self.need_fc_update = check_fontconfig (appdir)
		set_pango_paths (appdir);
		self.reroute_output = False

	def print_env (self):
		for (k,v) in self.env.items ():
			print 'export %s="%s"' % (k,v)

	def get_process (self, executable, args):
		out = None
		err = None
		if self.reroute_output:
			out = subprocess.PIPE
			err = subprocess.STDOUT

		if debug:
			print self.__dict__
			print 'args: ', args
			print 'executable: ', executable
		process = subprocess.Popen (args,
					    executable = executable,
					    cwd = self.cwd,
					    env = self.env,
					    stdout = out,
					    stderr = err,
					    shell = False)
		
		return process
	
	def get_lilypond_process (self):
		return self.get_process (self.executable, self.args)
	
	def get_pdfs (self):
		names = []
		for a in self.args:
			pdf = os.path.splitext (a)[0] + '.pdf'
			if os.path.exists (pdf):
				names.append (pdf)

		return names

	# data - for callbacks.
	def open_pdfs (self, data = None):
		pdfs = self.get_pdfs ()
		if pdfs:
		    b = '/usr/bin/open'
		    args = [b] + pdfs
		    if debug:
			    print 'invoking ', args
		    os.spawnv (os.P_NOWAIT, b, [b] + pdfs)  

################################################################
# default: wrap a LP call, open PDFs.  

if __name__ == '__main__':
	invocation = sys.argv[0]
	appdir_dir = sys.argv[1]

	argv = sys.argv[2:]
	
	if '--debug' in argv:
		debug = 1
		argv.remove ('--debug')
		
	call = Call(appdir_dir, argv)

	if debug:
		call.print_env ()

	if call.error_string:
		sys.stderr.write (call.error_string)
		sys.exit (2)

	if call.need_fc_update:
		sys.stderr.write ('Rebuilding font cache\nThis may take a few minutes.')
	
	p = call.get_lilypond_process ()
	code = p.wait ()

	if not sys.stdin.isatty (): # GUI
		call.open_pdfs ()
	
	sys.exit (code)
