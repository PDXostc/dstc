.PHONY: all clean install uninstall
all:
	(cd print_name_and_age; make)
	(cd dynamic_data; make)
	(cd struct; make)
	(cd callback; make)
	(cd callback_dynamic_data; make)
	(cd stress; make)
	(cd chat; make)

clean:
	(cd print_name_and_age; make clean)
	(cd dynamic_data; make clean)
	(cd struct; make clean)
	(cd callback; make clean)
	(cd callback_dynamic_data; make clean)
	(cd stress; make clean)
	(cd chat; make clean)

install:
	(cd print_name_and_age; make DESTDIR=${DESTDIR} install)
	(cd dynamic_data; make DESTDIR=${DESTDIR} install)
	(cd struct; make DESTDIR=${DESTDIR} install)
	(cd callback; make DESTDIR=${DESTDIR} install)
	(cd callback_dynamic_data; make DESTDIR=${DESTDIR} install)
	(cd stress; make DESTDIR=${DESTDIR} install)
	(cd chat; make DESTDIR=${DESTDIR} install)

uninstall:
	(cd print_name_and_age; make DESTDIR=${DESTDIR} uninstall)
	(cd dynamic_data; make DESTDIR=${DESTDIR} uninstall)
	(cd struct; make DESTDIR=${DESTDIR} uninstall)
	(cd callback; make DESTDIR=${DESTDIR} uninstall)
	(cd callback_dynamic_data; make DESTDIR=${DESTDIR} uninstall)
	(cd stress; make DESTDIR=${DESTDIR} uninstall)
	(cd chat; make DESTDIR=${DESTDIR} uninstall)
