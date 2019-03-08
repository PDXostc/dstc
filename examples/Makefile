
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
		$(MAKE) -C $$dir $(MAKECMDGOALS); \
	done

clean: $(SUBDIRS)
	@for dir in $(SUBDIRS); \
	do \
	    $(MAKE) -C $$dir clean; \
	done


install: $(SUBDIRS)
	@for dir in $(SUBDIRS); \
	do \
	    $(MAKE) DESTDIR=${DESTDIR} -C $$dir install; \
	done \


uninstall: $(SUBDIRS)
	@for dir in $(SUBDIRS); \
	do \
	    $(MAKE) DESTDIR=${DESTDIR} -C $$dir uninstall; \
	done
