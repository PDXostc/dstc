
SUBDIRS =                 \
	callback              \
	callback_dynamic_data \
	chat                  \
	dynamic_data          \
	print_name_and_age    \
	struct

.PHONY: $(SUBDIRS)

all: $(SUBDIRS)
	@for dir in $(SUBDIRS); \
	do \
		$(MAKE) MAKEFLAGS=$(MAKEFLAGS) -C $$dir $(MAKECMDGOALS); \
	done

clean: $(SUBDIRS)
	@for dir in $(SUBDIRS); \
	do \
	    $(MAKE) MAKEFLAGS=$(MAKEFLAGS) -C $$dir clean; \
	done

install: $(SUBDIRS)
	@for dir in $(SUBDIRS); \
	do \
	    $(MAKE) MAKEFLAGS=$(MAKEFLAGS) -C $$dir install; \
	done
