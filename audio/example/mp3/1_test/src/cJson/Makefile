SHELL := /bin/bash

#ROOT := $(word 1,$(subst /1_test,/1_test ,$(shell pwd)))
PWD := $(shell pwd)

include $(ROOT)/include.mk

all:
	@cd $(PWD)/1.7.14/; \
	$(MAKE)

clean:
	@cd $(PWD)/1.7.14/; \
	$(MAKE) clean

echo:
	@echo "$(ROOT)"; \
	echo "$(PWD)"
	

.PHONY: all clean echo

