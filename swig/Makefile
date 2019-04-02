#
#   Makefile to generate python interface code for DSTC
#
#	Note: Several of the commands below need to be run with "sudo" because
#	they are manipulating files in system directories that belong to "root".
#

CFLAGS += -ggdb

#
#	Define the directory that the compiled C files will go into.
#
OBJ_DIR = "build/temp.linux-x86_64-3.6"

$(OBJ_DIR)/dstc_swig_wrap.o: dstc_swig_wrap.c
	python3 setup.py build_ext --inplace

dstc_swig_wrap.c: dstc_swig.i ../dstc.h
	swig -I/usr/include -python -includeall dstc_swig.i

.PHONY: install uninstall clean

#
#	The install option will install the python package that has been built
#	here into the systemwide user created package directory.  Since the
#	installation is done into some system owned directories, this "install"
#	option should be run with "sudo", e.g. "sudo make install"
#
#	After the package has been installed, python users can utilize this
#	package by merely adding a "import dstc" command to their programs.
#
#	After installation of the package, the files used to generate the package
#	can be removed with:
#
#      sudo make clean
#
install: $(OBJ_DIR)/dstc_swig_wrap.o
	python3 setup.py install

#
#	This rule will uninstall the dstc package from the system python3
#	directories.  This procedure originated at:
#
#	   https://wiki.python.org/moin/Distutils/Proposals/UninstallCommand
#
#	The recommended procedure is a bit ugly and may leave some empty
#	directories laying around at times but it's about all that's available.
#
#	Since the uninstall is done from some system owned directories, this
#	"uninstall" option should be run with "sudo", e.g. "sudo make uninstall"
#
uninstall:
	@sudo python setup.py install --record build/files.$$$$; \
	 cat build/files.$$$$ | xargs sudo rm -rf; \
	 $(MAKE) clean

#
#	This "clean" rule will remove all of the files that were generated to
#	create the package.  Doing this will not effect the installed package, it
#	will only clean up the current directory.
#
#	Since the "clean" is done from some system owned directories, this option
#	should be run with "sudo", e.g. "sudo make clean"
#
clean:
	rm -rf _dstc*.so build dstc_swig.py dstc_swig_wrap.* *.pyc __pycache__
