# From mm/Makefile

kver_ge = $(shell \
echo test | awk ' \
{if ($(VERSION) < $(1)) {print 0} else { \
    if ($(VERSION) > $(1)) {print 1} else { \
        if ($(PATCHLEVEL) < $(2)) {print 0} else { \
            if ($(PATCHLEVEL) >= $(2)) {print 1} \
}}}}' \
)

ifeq ($(call kver_ge,4,3),0)
# Kernels prior to 4.3 require always frame_vector.ko
CONFIG_FRAME_VECTOR := m
endif

obj-$(CONFIG_FRAME_VECTOR) += frame_vector.o

KDIRA          := /lib/modules/$(KERNELRELEASE)/kernel

mm-install install-mm::
	@dir="mm"; \
	files='frame_vector.ko'; \
	if [ -f $$files ]; then \
	echo -e "\nInstalling $(KDIRA)/$$dir files:"; \
	install -d $(KDIRA)/$$dir; \
	for i in $$files;do if [ -e $$i ]; then echo -n "$$i "; \
	install -m 644 -c $$i $(KDIRA)/$$dir; fi; done; echo; fi
