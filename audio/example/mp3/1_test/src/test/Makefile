SHELL := /bin/bash

#ROOT = $(word 1,$(subst /1_test,/1_test ,$(shell pwd)))

include $(ROOT)/include.mk

sources = $(wildcard *.c)
headers = $(wildcard *.h)
objects = $(sources:.c=.o)
depens = $(objects:.o=.d)

target = zxf

$(target): $(objects)
	$(CC) -o $@ $^ $(LIBFLAGS) -ldbg -lcJson; \
	cp $(target) $(BIN_PATH); \
	cp $(headers) $(HEADER_PATH); \
	$(RM) $(target) $(objects) $(depens)

%.o:%.c
	$(CC) -o $@ -c $< $(CCFLAGS)

include $(depens)

define gen_depens
set -e; \
$(RM) $($@); \
$(CC) -MM $(CCFLAGS) $< > $@.$$$$; \
sed 's,\($*\)\.o[ :]*,\1.o $@: ,g' < $@.$$$$ > $@; \
$(RM) $@.$$$$
endef

%.d:%.c
	$(gen_depens)


.PHONY: clean echo

echo:
	@echo "$(sources)"
	@echo "$(objects)"
	@echo "$(depens)"
	@echo "$(AR)"
	@echo "$(CP)"
	@echo "$(LIB_PATH)"
	@echo "$(HEADER_PATH)"

clean:
	$(RM) $(target) $(objects) $(depens) $(depens)*; \
	cd $(BIN_PATH) && $(RM) $(target); \
	cd $(HEADER_PATH) && $(RM) $(headers)
