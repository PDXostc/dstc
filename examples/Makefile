
SUBDIRS =                 \
	print_name_and_age    \
	dynamic_data          \
	struct                \
	callback              \
	callback_dynamic_data \
    stress                \
	chat                  \
	cpp                   \

.PHONY: all clean install uninstall cpp


all: $(SUBDIRS)
	@for dir in $(SUBDIRS); \
	do \
		$(MAKE) MAKEFLAGS=$(MAKEFLAGS) -C $$dir $(MAKECMDGOALS); \
	done

cpp:
	(cd cpp; cmake .; make)

clean: $(SUBDIRS)
	@for dir in $(SUBDIRS); \
	do \
	    $(MAKE) MAKEFLAGS=$(MAKEFLAGS) -C $$dir clean; \
	done


install: $(SUBDIRS)
	@for dir in $(SUBDIRS); \
	do \
	    $(MAKE) MAKEFLAGS=$(MAKEFLAGS) DESTDIR=${DESTDIR} -C $$dir install; \
	done


uninstall: $(SUBDIRS)
	@for dir in $(SUBDIRS); \
	do \
	    $(MAKE) MAKEFLAGS=$(MAKEFLAGS) DESTDIR=${DESTDIR} -C $$dir uninstall; \
	done
