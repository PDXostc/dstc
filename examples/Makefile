
SUBDIRS =                 \
	print_name_and_age    \
	dynamic_data          \
	string_data          \
	print_struct          \
	callback              \
	callback_dyndata      \
	no_argument	      \
	stress                \
	loopback              \
	chat                  \
	thread_stress         \
	many_arguments        \
	cpp                   \

.PHONY: all clean install uninstall

export USE_POLL

all: $(SUBDIRS)
	@for dir in $(SUBDIRS); \
	do \
		$(MAKE) -C $$dir; \
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
