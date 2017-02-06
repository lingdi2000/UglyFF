
SUBDIRS = include test superserver

.PHONY:all

define make_subdir
	@for subdir in $(SUBDIRS); do\
	(cd $$subdir && make $1)\
	done;
endef

all:
	$(call make_subdir, all)

.PHONY:clean
clean:
	$(call make_subdir, clean)
