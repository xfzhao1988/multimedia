SHELL := /bin/bash

ROOT := $(word 1,$(subst /1_test,/1_test ,$(shell pwd)))

export ROOT

include $(ROOT)/include.mk

all:
	@cd $(SRC_PATH); \
	$(MAKE)

clean:
	@cd $(SRC_PATH); \
	$(MAKE) clean

echo:
	@echo "$(ROOT)"; \
	echo "$(SRC_PATH)"; \
	echo "$(MAKE)"; \
	cd $(SRC_PATH); \
	$(MAKE) echo
	

.PHONY: all clean echo

