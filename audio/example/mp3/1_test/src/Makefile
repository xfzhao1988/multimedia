SHELL := /bin/bash

#ROOT = $(word 1,$(subst /1_test,/1_test ,$(shell pwd)))

include $(ROOT)/include.mk

MAKE_ALL_RULE = $(SRC_PATH)/cJson $(SRC_PATH)/dbg $(SRC_PATH)/test

all:
	for i in $(MAKE_ALL_RULE);\
	do \
	cd $$i && $(MAKE); \
	done

MAKE_CLEAN_RULE := $(SRC_PATH)/test $(SRC_PATH)/dbg $(SRC_PATH)/cJson

clean:
	for i in $(MAKE_CLEAN_RULE);\
	do \
	cd $$i && $(MAKE) clean; \
	done

echo:
	@echo "$(MAKE_ALL_RULE)"
	@echo "$(MAKE_CLEAN_RULE)"
	@echo "$(MAKE)"

.PHONY: all clean echo

