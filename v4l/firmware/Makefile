FW_DIR  = $(DESTDIR)/lib/firmware

####

.PHONY = default clean distclean install rminstall prep

default:

clean:

distclean: clean


install: default
	@echo -n "Installing firmwares at $(FW_DIR): "
	-@for i in $(wildcard *.fw*); do echo -n "$$i "; cp $$i $(FW_DIR)/$$i; done
	@echo

rminstall:
	@echo "Removing firmwares at $(FW_DIR) "

######

prep:

