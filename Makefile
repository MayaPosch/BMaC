# Central Makefile for building BMaC firmware images and test servers.

export TOP := $(CURDIR)

all: server node

server:
	# Trigger server build script.
	$(MAKE) -C test
	
node:
	# Trigger node build script.
	$(MAKE) -f Makefile.node
	
clean: clean-server clean-node

clean-server:
	$(MAKE) -C test clean
	
clean-node:
	$(MAKE) -f Makefile.node clean
