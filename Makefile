LGPM ?= lgpm
LOGOS_DATA_DIR ?= $(HOME)/.local/share/Logos/LogosAppNix

.PHONY: build install-lgx

build:
	nix build .#lgx

install-lgx: build
	$(LGPM) install --file result/zone-sequencer.lgx --modules-dir $(LOGOS_DATA_DIR)/modules
