SHELL := /bin/bash 

#ROOT = $(word 1,$(subst /1_test,/1_test ,$(shell pwd)))

include $(ROOT)/include.mk

sources = $(wildcard *.c)
headers = $(wildcard *.h)
objects = $(sources:.c=.o)
depens = $(objects:.o=.d)

target = libdbg.a

$(target): $(objects)
	$(AR) rv $@ $^; \
	cp $(target) $(LIB_PATH); \
	cp $(headers) $(HEADER_PATH)

%.o:%.c
	$(CC) -o $@ -c $<

include $(depens)

define gen_depens
set -e; \
$(RM) $(depens); \
$(CC) -MM $< > $@.$$$$; \
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
	$(RM) $(target) $(objects) $(depens); \
	cd $(LIB_PATH) && $(RM) $(target); \
	cd $(HEADER_PATH) && $(RM) $(headers)
