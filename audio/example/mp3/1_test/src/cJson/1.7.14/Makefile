SHELL := /bin/bash

#ROOT = $(word 1,$(subst /1_test,/1_test ,$(shell pwd)))

include $(ROOT)/include.mk

CFLAGS = -fpic -I.
LD_FLAGS = -shared
headrs := $(wildcard *.h)
sources := $(wildcard *.c)
objects := $(sources:.c=.o)
depens := $(objects:.o=.d)

TARGET := libcJson.so

$(TARGET):$(objects)
	$(CC) $(LD_FLAGS) $^ -o $@; \
	$(CP) $(headrs) $(ROOT)/include; \
	$(CP) $(TARGET) $(ROOT)/lib

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

include $(depens)

define gen_depens
set -e; \
$(RM) $@; \
$(CC) -MM $< > $@.$$$$; \
sed 's,\($*\)\.o[ :]*,\1.o $@: ,g' < $@.$$$$ > $@; \
$(RM) $@.$$$$
endef

%.d: %.c
	$(gen_depens)

.PHONY: echo clean

echo:
	@echo "$(sources)"
	@echo "$(objects)"
	@echo "$(depens)"
	@echo "$(ROOT)"

clean:
	-$(RM) $(TARGET) $(objects) $(depens); \
	cd $(ROOT)/include && $(RM) $(headrs); \
	cd $(ROOT)/lib && $(RM) $(TARGET)