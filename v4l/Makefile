ifeq ($(wildcard ../linux/include/uapi/linux/videodev2.h),)

# No kernel source! User needs to download one
def:
	@echo 'No kernel files. You need to run "make download untar" or "make dir DIR=<your_git_source_dir>" first!'
	exit 1
endif

ifeq ($(obj),)
obj = .
endif

#################################################
# Take version info from last module build if available
# if .version doesn't exist, make will create it for us and restart
-include $(obj)/.version

ifneq ($(SRCDIR),)
	OUTDIR ?= $(SRCDIR)
endif
OUTDIR ?= /lib/modules/$(KERNELRELEASE)/build
SRCDIR ?= /lib/modules/$(KERNELRELEASE)/source
ifeq ($(wildcard $(SRCDIR)/Makefile),)
	# No kernel source, but headers should be in OUTDIR
	SRCDIR := $(OUTDIR)
endif

ifneq ($(origin KDIR), undefined)
ifneq ($(KDIR),$(SRCDIR))
KERNELRELEASE :=
override DIR := $(KDIR)
.PHONY: $(obj)/.version
endif
endif

#################################################
# default compilation rule

default:: prepare firmware
	@echo Kernel build directory is $(OUTDIR)
	$(MAKE) -C ../linux apply_patches
	$(MAKE) -C $(OUTDIR) M=$(PWD) $(MYCFLAGS) modules
	./scripts/rmmod.pl check

mismatch:: prepare firmware
	@echo Kernel build directory is $(OUTDIR)
	$(MAKE) -Wfatal-errors -C $(OUTDIR) M=$(PWD) $(MYCFLAGS) CONFIG_DEBUG_SECTION_MISMATCH=y modules
	./scripts/rmmod.pl check

# Anything in this target shouldn't be build in parallel.
prepare:: config-compat.h Makefile.media links

firmware::
	$(MAKE) -C firmware prep
	$(MAKE) -C firmware

#################################################
# Object specific rules

# Targets which don't need Makefile.media's rules
no-makefile-media-targets := %config clean distclean \
	links start insmod load stop rmmod unload reload \
	help debug

# Targets which don't need .myconfig to exist, to keep us from including it
no-dot-config-targets := $(no-makefile-media-targets) %install remove

dot-config := 1
ifneq ($(filter $(no-dot-config-targets), $(MAKECMDGOALS)),)
	ifeq ($(filter-out $(no-dot-config-targets), $(MAKECMDGOALS)),)
		dot-config := 0
	endif
endif

makefile-media := 1
ifneq ($(filter $(no-makefile-media-targets), $(MAKECMDGOALS)),)
	ifeq ($(filter-out $(no-makefile-media-targets), $(MAKECMDGOALS)),)
		makefile-media := 0
	endif
endif

# If version not yet detected, we can't create/have these files yet
ifeq ($(KERNELRELEASE),)
	makefile-media := 0
	dot-config := 0
endif

ifeq ($(dot-config),1)
-include $(obj)/.myconfig
endif

ifeq ($(makefile-media),1)
-include $(obj)/Makefile.media
-include $(obj)/Makefile.sound
endif

-include $(obj)/.kconfig.dep

# make will try to automatically rebuild the Makefile and all includes,
# and if any of them change, make will start over with the updated files

$(obj)/.myconfig: $(obj)/.config $(obj)/Kconfig
	./scripts/make_myconfig.pl

# If it doesn't exist, this rule will build Makefile.media.  If it does
# exist, it will have dependency information in it, and only be rebuilt
# when necessary.
Makefile.media:
	scripts/make_makefile.pl

#################################################
# CFLAGS configuration

EXTRA_CFLAGS += -g

EXTRA_CFLAGS += $(if $(wildcard $(srctree)/.mm), -DMM_KERNEL)

# Make sure to use the backport header for common stuff
EXTRA_CFLAGS += -include $(obj)/compat.h

# gives too much warnings (false positives) -> disable it
EXTRA_CFLAGS += -Wno-format-truncation

#################################################
# Directories to be used

PWD		:= $(shell pwd)
KDIR26		:= /lib/modules/$(KERNELRELEASE)/updates/extra/media

#################################################
# Compiler fixup rules

HOSTCC:=$(CC)
CC += -I$(obj)

LINUXINCLUDE    := -I$(M)/../linux/include -I$(M)/../linux/include/uapi $(LINUXINCLUDE) -I$(M)/

MYCFLAGS :=

#################################################
# locales seem to cause trouble sometimes.
LC_ALL = POSIX
export LC_ALL


#################################################
# Generic wildcard rules

%.asm: %.o
	objdump -S $< > $@

#################################################
# all file compilation rule

all:: default

#################################################
# installation invocation rules
INSTALLDEPS :=
ifeq ($(makefile-media),1)
INSTALLDEPS += media-install
endif
modules_install install:: $(INSTALLDEPS) firmware_install

remove rminstall:: media-rminstall

firmware_install::
	make -C firmware install

#################################################
# Compiling preparation rules

$(obj)/.version:
ifneq ($(DIR),)
	@echo "Searching in $(DIR)/Makefile for kernel version."
	@perl \
	-e '$$d="$(DIR)"; ' \
	-e 'S: open IN,"$$d/Makefile"; ' \
	-e 'while (<IN>) {' \
	-e '	if (/^VERSION\s*=\s*(\d+)/){ $$version=$$1; }' \
	-e '	elsif (/^PATCHLEVEL\s*=\s*(\d+)/){ $$level=$$1; }' \
	-e '	elsif (/^SUBLEVEL\s*=\s*(\d+)/){ $$sublevel=$$1; }' \
	-e '	elsif (/^EXTRAVERSION\s*=\s*(\d+)\n/){ $$extra=".$$1"; }' \
	-e '	elsif (/^EXTRAVERSION\s*=\s*(\S+)\n/){ $$extra=$$1; }' \
	-e '    elsif (/^KERNELSRC\s*:=\s*(\S.*)\n/ || /^MAKEARGS\s*:=\s*-C\s*(\S.*)\n/)' \
	-e '        { $$o=$$d; $$d=$$1; goto S; }' \
	-e '};' \
	-e 'printf ("VERSION=%s\nPATCHLEVEL:=%s\nSUBLEVEL:=%s\nKERNELRELEASE:=%s.%s.%s%s\n",' \
	-e '	$$version,$$level,$$sublevel,$$version,$$level,$$sublevel,$$extra);' \
	-e 'print "OUTDIR:=$$o\n" if($$o);' \
	-e 'print "SRCDIR:=$$d\n";' > $(obj)/.version
	@cat .version|grep KERNELRELEASE:|sed s,'KERNELRELEASE:=','Forcing compiling to version ',

	@if [ ! -f $(DIR)/scripts/kallsyms ]; then \
		echo "*** Warning: You should configure and build kernel before compiling V4L"; \
	fi
else
ifneq ($(VER),)
	@echo "Forcing kernel version to $(VER)."
	@echo $(VER)|perl -ne 'if (/^([0-9]*)\.([0-9]*)\.([0-9]*)(.*)$$/) { printf ("VERSION=%s\nPATCHLEVEL:=%s\nSUBLEVEL:=%s\nKERNELRELEASE:=%s.%s.%s%s\n",$$1,$$2,$$3,$$1,$$2,$$3,$$4); };' > $(obj)/.version
else
	@echo No version yet, using `uname -r`
	@uname -r|perl -ne 'if (/^([0-9]*)\.([0-9]*)\.?([0-9]*)(.*)$$/) { $$ver = $$1; $$patch = $$2; $$sub = $$3; if ($$ver == 2 && $$patch == 6 && $$sub >= 40) { $$ver = 3; $$patch = $$sub - 40; $$sub = 0; }; printf ("VERSION=%s\nPATCHLEVEL:=%s\nSUBLEVEL:=%s\nKERNELRELEASE:=%s",$$ver,$$patch,$$sub==""?"0":$$sub,$$_); };' > $(obj)/.version
endif
endif

links::
	@echo creating symbolic links...
	@find ../linux/drivers/media -name '*.[ch]' -type f -print0 | xargs -0n 255 ln -sf --target-directory=.
	@find ../linux/sound -name '*.[ch]' -type f -print0 | xargs -0n 255 ln -sf --target-directory=.
	@if [ -f ../linux/mm/frame_vector.c ]; then find ../linux/mm -name '*.[ch]' -type f -print0 | xargs -0n 255 ln -sf --target-directory=. ; fi
	@find ../linux/drivers/staging -name '*.[ch]' -type f -print0 | xargs -0n 255 ln -sf --target-directory=.
	@find ../linux/drivers/misc -name '*.[ch]' -type f -print0 | xargs -0n 255 ln -sf --target-directory=.

config-compat.h:: $(obj)/.version .myconfig scripts/make_config_compat.pl
	[ ! -f "$(obj)/config-mycompat.h" ] && echo "/* empty config-mycompat.h */" > "$(obj)/config-mycompat.h" || true
	perl scripts/make_config_compat.pl $(SRCDIR) $(obj)/.myconfig $(obj)/config-compat.h


#################################################
# Cleaning rules

clean::
	@find . -name '*.c' -type l -exec rm '{}' \;
	@find . -name '*.h' -type l -exec rm '{}' \;
	-rm -f *~ *.o *.ko *.mod *.dwo .*.o.cmd .*.mod.cmd .*.ko.cmd *.mod.c av7110_firm.h fdump \
		config-compat.h Module.symvers Module.markers modules.order \
		*.unsigned .*.ko.unsigned.cmd
	$(MAKE) -C firmware $@

distclean:: clean
	-rm -f .version .*.o.flags .*.o.d *.mod.gcno Makefile.media \
		Kconfig Kconfig.kern .config .config.cmd .myconfig \
		.kconfig.dep config-mycompat.h
	-rm -rf .tmp_versions .tmp*.ver .tmp*.o .*.gcno .cache.mk
	@find .. -name '*.orig' -exec rm '{}' \;
	@find .. -name '*.rej' -exec rm '{}' \;
	$(MAKE) -C firmware $@

#################################################
# Kernel module insert/removal rules

start insmod load::
	scripts/rmmod.pl load

stop rmmod unload::
	scripts/rmmod.pl unload

reload::
	scripts/rmmod.pl reload

debug::
	scripts/rmmod.pl debug

#################################################
# Configuration rules

# Kernel config programs
QCONF := $(OUTDIR)/scripts/kconfig/qconf
GCONF := $(OUTDIR)/scripts/kconfig/gconf
MCONF := $(OUTDIR)/scripts/kconfig/mconf
CONF := $(OUTDIR)/scripts/kconfig/conf

# Ideally, some kind of oldconfig process would be used to update .config
# without destroying the user's configuration.  Lacking that, we will
# just touch .config if it already exists, and run make_kconfig if it
# doesn't.
$(obj)/.config: $(obj)/.version
	@echo Updating/Creating .config
	@if [ -e $(obj)/.config ]; then touch $(obj)/.config ; else \
		$(MAKE) -C ../linux apply_patches; \
		./scripts/make_kconfig.pl $(OUTDIR) $(SRCDIR); fi

$(obj)/Kconfig: $(obj)/.version
	@$(MAKE) -C ../linux apply_patches
	./scripts/make_kconfig.pl $(OUTDIR) $(SRCDIR)

# With make -j, it's possible that both the .config and Kconfig rules
# will run at the same time, running make_kconfig.pl twice.  There
# doesn't appear to be any way around this, but it doesn't cause a
# failure.  One make_kconfig.pl will just overwrite the other's output.

xconfig:: $(QCONF) $(obj)/Kconfig
	$(QCONF) $(obj)/Kconfig

gconfig:: $(GCONF) $(obj)/Kconfig
	$(GCONF) $(obj)/Kconfig

config:: $(CONF) $(obj)/Kconfig
	$(CONF) $(obj)/Kconfig

menuconfig:: $(MCONF) $(obj)/Kconfig
	$(MCONF) $(obj)/Kconfig

allyesconfig allmodconfig:: $(obj)/.version
	@$(MAKE) -C ../linux apply_patches
	./scripts/make_kconfig.pl $(OUTDIR) $(SRCDIR) 1

stagingconfig:: $(obj)/.version
	@$(MAKE) -C ../linux apply_patches
	./scripts/make_kconfig.pl $(OUTDIR) $(SRCDIR) 1 1

# rule to build kernel conf programs
KMAKEVARS := config-targets=1 mixed-targets=0 dot-config=0 config-build=1 need-config=0 SRCDIR=$(SRCDIR)
$(QCONF) $(GCONF) $(MCONF) $(CONF):
	$(MAKE) -C $(OUTDIR) -f $(PWD)/Makefile.kernel $(KMAKEVARS) v4l-$(notdir $@)

#################################################
# Help
help::
	@cat ../INSTALL

#################################################
# main rules

dir download apply-patches unapply-patches untar::
	make -C ../linux $@
