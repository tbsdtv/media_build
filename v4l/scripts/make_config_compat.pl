#!/usr/bin/perl
use strict;

my $kdir=shift or die "should specify a kernel dir";
my $infile=shift or die "should specify an input config file";
my $outfile=shift or die "should specify an output config file";

my $out;

sub check_spin_lock()
{
	my $file = "$kdir/include/linux/netdevice.h";
	my $old_syntax = 1;

	open INNET, "<$file" or die "File not found: $file";
	while (<INNET>) {
		if (m/netif_tx_lock_bh/) {
			$old_syntax = 0;
			last;
		}
	}

	if ($old_syntax) {
		$out.= "\n#define OLD_XMIT_LOCK 1\n";
	}
	close INNET;
}

sub check_sound_driver_h()
{
	my $file = "$kdir/include/sound/driver.h";
	my $old_syntax = 1;

	open INNET, "<$file" or return;
	while (<INNET>) {
		if (m/This file is deprecated/) {
			$old_syntax = 0;
			last;
		}
	}

	if ($old_syntax) {
		$out.= "\n#define NEED_SOUND_DRIVER_H 1\n";
	}
	close INNET;
}

sub check_snd_pcm_rate_to_rate_bit()
{
	my $file = "$kdir/include/sound/pcm.h";
	my $old_syntax = 1;

	open INNET, "<$file" or die "File not found: $file";
	while (<INNET>) {
		if (m/snd_pcm_rate_to_rate_bit/) {
			$old_syntax = 0;
			last;
		}
	}

	if ($old_syntax) {
		$out.= "\n#define COMPAT_PCM_TO_RATE_BIT 1\n";
	}
	close INNET;
}

sub check_snd_pcm_stop_xrun()
{
	my $file = "$kdir/include/sound/pcm.h";
	my $old_syntax = 1;

	open INNET, "<$file" or die "File not found: $file";
	while (<INNET>) {
		if (m/snd_pcm_stop_xrun/) {
			$old_syntax = 0;
			last;
		}
	}

	if ($old_syntax) {
		$out.= "\n#define NEED_PCM_STOP_XRUN 1\n";
	}
	close INNET;
}

sub check_snd_ctl_boolean_mono_info()
{
	my $file = "$kdir/include/sound/control.h";
	my $old_syntax = 1;

	open INNET, "<$file" or die "File not found: $file";
	while (<INNET>) {
		if (m/snd_ctl_boolean_mono_info/) {
			$old_syntax = 0;
			last;
		}
	}

	if ($old_syntax) {
		$out.= "\n#define COMPAT_SND_CTL_BOOLEAN_MONO 1\n";
	}
	close INNET;
}

sub check_bool()
{
	my $file = "$kdir/include/linux/types.h";
	my $old_syntax = 1;

	open INDEP, "<$file" or die "File not found: $file";
	while (<INDEP>) {
		if (m/^\s*typedef.*bool;/) {
			$old_syntax = 0;
			last;
		}
	}

	if ($old_syntax) {
		$out.= "\n#define NEED_BOOL_TYPE 1\n";
	}
	close INDEP;
}

sub check_is_singular()
{
	my $file = "$kdir/include/linux/list.h";
	my $need_compat = 1;

	open INNET, "<$file" or die "File not found: $file";
	while (<INNET>) {
		if (m/list_is_singular/) {
			$need_compat = 0;
			last;
		}
	}

	if ($need_compat) {
		$out.= "\n#define NEED_IS_SINGULAR 1\n";
	}
	close INNET;
}

sub check_clamp()
{
	my $file = "$kdir/include/linux/kernel.h";
	my $need_compat = 1;

	open INNET, "<$file" or die "File not found: $file";
	while (<INNET>) {
		if (m/define\s+clamp/) {
			$need_compat = 0;
			last;
		}
	}

	if ($need_compat) {
		$out.= "\n#define NEED_CLAMP 1\n";
	}
	close INNET;
}

sub check_proc_create()
{
	my $file = "$kdir/include/linux/proc_fs.h";
	my $need_compat = 1;

	open INNET, "<$file" or die "File not found: $file";
	while (<INNET>) {
		if (m/proc_create/) {
			$need_compat = 0;
			last;
		}
	}

	if ($need_compat) {
		$out.= "\n#define NEED_PROC_CREATE 1\n";
	}
	close INNET;
}

sub check_pcm_lock()
{
	my $file = "$kdir/include/sound/pcm.h";
	my $need_compat = 1;

	open INNET, "<$file" or die "File not found: $file";
	while (<INNET>) {
		if (m/pcm_stream_lock/) {
			$need_compat = 0;
			last;
		}
	}

	if ($need_compat) {
		$out.= "\n#define NO_PCM_LOCK 1\n";
	}
	close INNET;
}

sub check_algo_control()
{
	my $file = "$kdir/include/linux/i2c.h";
	my $need_compat = 0;

	open INNET, "<$file" or die "File not found: $file";
	while (<INNET>) {
		if (m/algo_control/) {
			$need_compat = 1;
			last;
		}
	}

	if ($need_compat) {
		$out.= "\n#define NEED_ALGO_CONTROL 1\n";
	}
	close INNET;
}

sub check_net_dev()
{
	my $file = "$kdir/include/linux/netdevice.h";
	my $need_compat = 1;

	open INNET, "<$file" or die "File not found: $file";
	while (<INNET>) {
		if (m/netdev_priv/) {
			$need_compat = 0;
			last;
		}
	}

	if ($need_compat) {
		$out.= "\n#define NEED_NETDEV_PRIV 1\n";
	}
	close INNET;
}

sub check_usb_endpoint_type()
{
	my $nfiles = 0;
	my @files = ( "$kdir/include/linux/usb.h",
		      "$kdir/include/linux/usb/ch9.h",
		      "$kdir/include/uapi/linux/usb/ch9.h" );

	foreach my $file ( @files ) {
		open IN, "<$file" or next;
		$nfiles++;
		while (<IN>) {
			if (m/usb_endpoint_type/) {
				close IN;
				# definition found. No need for compat
				return;
			}
		}
		close IN;
	}

	die "Usb headers not found" if (!$nfiles);

	# definition not found. This means that we need compat
	$out.= "\n#define NEED_USB_ENDPOINT_TYPE 1\n";
}

sub check_pci_ioremap_bar()
{
	my $file = "$kdir/include/linux/pci.h";
	my $need_compat = 1;

	open INNET, "<$file" or die "File not found: $file";
	while (<INNET>) {
		if (m/pci_ioremap_bar/) {
			$need_compat = 0;
			last;
		}
	}

	if ($need_compat) {
		$out.= "\n#define NEED_PCI_IOREMAP_BAR 1\n";
	}
	close INNET;
}

sub check_snd_card_create()
{
	my $file = "$kdir/include/sound/core.h";
	my $need_compat = 1;

	open IN, "<$file" or die "File not found: $file";
	while (<IN>) {
		if (m/snd_card_create/) {
			$need_compat = 0;
			last;
		}
	}

	if ($need_compat) {
		$out.= "\n#define NEED_SND_CARD_CREATE\n";
	}
	close IN;
}

sub check_poll_schedule()
{
	my @files = ( "$kdir/include/linux/poll.h" );

	foreach my $file ( @files ) {
		open IN, "<$file" or die "File not found: $file";
		while (<IN>) {
			if (m/poll_schedule/) {
				close IN;
				# definition found. No need for compat
				return;
			}
		}
		close IN;
	}

	# definition not found. This means that we need compat
	$out.= "\n#define NEED_POLL_SCHEDULE 1\n";
}

sub check_snd_BUG_ON()
{
	my @files = ( "$kdir/include/sound/core.h" );

	foreach my $file ( @files ) {
		open IN, "<$file" or die "File not found: $file";
		while (<IN>) {
			if (m/snd_BUG_ON/) {
				close IN;
				# definition found. No need for compat
				return;
			}
		}
		close IN;
	}

	# definition not found. This means that we need compat
	$out.= "\n#define NEED_SND_BUG_ON 1\n";
}

sub check_bitops()
{
	my @files = ( "$kdir/include/linux/bitops.h" );

	foreach my $file ( @files ) {
		open IN, "<$file" or next;
		while (<IN>) {
			if (m/#define\s+BIT\(/) {
				close IN;
				# definition found. No need for compat
				return;
			}
		}
		close IN;
	}

	# definition not found. This means that we need compat
	$out.= "\n#define NEED_BITOPS 1\n";
}

sub check_fw_csr_string()
{
	my @files = ( "$kdir/include/linux/firewire.h" );

	foreach my $file ( @files ) {
		open IN, "<$file" or next;
		while (<IN>) {
			if (m/fw_csr_string\(/) {
				close IN;
				# definition found. No need for compat
				return;
			}
		}
		close IN;
	}

	# definition not found. This means that we need compat
	$out.= "\n#define NEED_FW_CSR_STRING 1\n";
}


sub check_delayed_work()
{
	my @files = ( "$kdir//include/linux/workqueue.h" );

	foreach my $file ( @files ) {
		open IN, "<$file" or die "File not found: $file";
		while (<IN>) {
			if (m/struct\s+delayed_work/) {
				close IN;
				# definition found. No need for compat
				return;
			}
		}
		close IN;
	}

	# definition not found. This means that we need compat
	$out.= "\n#define NEED_DELAYED_WORK 1\n";
}

sub check_vzalloc()
{
	my @files = ( "$kdir/include/linux/vmalloc.h" );

	foreach my $file ( @files ) {
		open IN, "<$file" or die "File not found: $file";
		while (<IN>) {
			if (m/vzalloc/) {
				close IN;
				# definition found. No need for compat
				return;
			}
		}
		close IN;
	}

	# definition not found. This means that we need compat
	$out.= "\n#define NEED_VZALLOC 1\n";
}

sub check_flush_work_sync()
{
	my @files = ( "$kdir/include/linux/workqueue.h" );

	foreach my $file ( @files ) {
		open IN, "<$file" or die "File not found: $file";
		while (<IN>) {
			if (m/flush_work_sync/) {
				close IN;
				# definition found. No need for compat
				return;
			}
		}
		close IN;
	}

	# definition not found. This means that we need compat
	$out.= "\n#define NEED_FLUSH_WORK_SYNC 1\n";
}

sub check_autosuspend_delay()
{
	my @files = ( "$kdir/include/linux/pm_runtime.h" );

	foreach my $file ( @files ) {
		open IN, "<$file" or next;
		while (<IN>) {
			if (m/pm_runtime_set_autosuspend_delay/) {
				close IN;
				# definition found. No need for compat
				return;
			}
		}
		close IN;
	}

	# definition not found. This means that we need compat
	$out.= "\n#define NEED_AUTOSUSPEND_DELAY 1\n";
}

sub check_i2c_smbus_read_word_swapped()
{
	my @files = ( "$kdir/include/linux/i2c.h" );

	foreach my $file ( @files ) {
		open IN, "<$file" or next;
		while (<IN>) {
			if (m/i2c_smbus_read_word_swapped/) {
				close IN;
				# definition found. No need for compat
				return;
			}
		}
		close IN;
	}

	# definition not found. This means that we need compat
	$out.= "\n#define NEED_I2C_SMBUS_WORD_SWAPPED 1\n";
}

sub check_printk_ratelimited()
{
	my @files = ( "$kdir/include/linux/kernel.h" );

	foreach my $file ( @files ) {
		open IN, "<$file" or next;
		while (<IN>) {
			# It's either defined in kernel.h, or printk.h
			# is included from kernel.h
			if (m/printk.h/ || m/printk_ratelimited/) {
				close IN;
				# definition found. No need for compat
				return;
			}
		}
		close IN;
	}

	# definition not found. This means that we need compat
	$out.= "\n#define NEED_PRINTK_RATELIMITED 1\n";
}


sub check_files_for_func($$@)
{
	my $function = shift;
	my $define = shift;
	my @incfiles = @_;

	for my $incfile (@incfiles) {
		my @files = ( "$kdir/$incfile" );

		foreach my $file ( @files ) {
			open IN, "<$file" or next;
			while (<IN>) {
				if (m/($function)/) {
					close IN;
					# definition found. No need for compat
					return;
				}
			}
			close IN;
		}
	}

	# definition not found. This means that we need compat
	$out.= "\n#define $define 1\n";
}

sub check_files_for_func_uapi($$@)
{
	my $function = shift;
	my $define = shift;
	my @incfiles = @_;

	for my $incfile (@incfiles) {
		my @files = ( "$kdir/include/linux/$incfile",
			      "$kdir/include/uapi/linux/$incfile" );

		foreach my $file ( @files ) {
			open IN, "<$file" or next;
			while (<IN>) {
				if (m/($function)/) {
					close IN;
					# definition found. No need for compat
					return;
				}
			}
			close IN;
		}
	}

	# definition not found. This means that we need compat
	$out.= "\n#define $define 1\n";
}

sub check_gfp_kswapd_reclaim()
{
	my @files = ( "$kdir/include/linux/gfp.h" );

	foreach my $file ( @files ) {
		open IN, "<$file" or next;
		while (<IN>) {
			if (m/__GFP_KSWAPD_RECLAIM/) {
				close IN;
				# definition found. No need for compat
				return;
			}
		}
		close IN;
	}

	# definition not found. This means that we need compat
	$out.= "\n#define __GFP_KSWAPD_RECLAIM 0\n";
}

sub check_other_dependencies()
{
	check_spin_lock();
	check_sound_driver_h();
	check_snd_ctl_boolean_mono_info();
	check_snd_pcm_rate_to_rate_bit();
	check_snd_pcm_stop_xrun();
	check_bool();
	check_is_singular();
	check_clamp();
	check_proc_create();
	check_pcm_lock();
	check_algo_control();
	check_net_dev();
	check_usb_endpoint_type();
	check_pci_ioremap_bar();
	check_snd_card_create();
	check_poll_schedule();
	check_snd_BUG_ON();
	check_bitops();
	check_delayed_work();
	check_fw_csr_string();
	check_vzalloc();
	check_flush_work_sync();
	check_autosuspend_delay();
	check_i2c_smbus_read_word_swapped();
	check_printk_ratelimited();
	check_gfp_kswapd_reclaim();
	check_files_for_func("hex_to_bin", "NEED_HEX_TO_BIN", "include/linux/kernel.h");
	check_files_for_func("snd_ctl_enum_info", "NEED_SND_CTL_ENUM_INFO", "include/sound/control.h");
	check_files_for_func("sysfs_attr_init", "NEED_SYSFS_ATTR_INIT", "include/linux/sysfs.h");
	check_files_for_func("usleep_range", "NEED_USLEEP_RANGE", "include/linux/delay.h");
	check_files_for_func("IS_ERR_OR_NULL", "NEED_IS_ERR_OR_NULL", "include/linux/err.h");
	check_files_for_func("dma_transfer_direction", "NEED_DMA_TRANSFER_DIRECTION", "include/linux/dmaengine.h");
	check_files_for_func("poll_requested_events", "NEED_POLL_REQUESTED_EVENTS", "include/linux/poll.h");
	check_files_for_func("module_usb_driver", "NEED_MODULE_USB_DRIVER", "include/linux/usb.h");
	check_files_for_func("module_platform_driver", "NEED_MODULE_PLATFORM_DRIVER", "include/linux/platform_device.h");
	check_files_for_func("kmalloc_array", "NEED_KMALLOC_ARRAY", "include/linux/slab.h");
	check_files_for_func("dmaengine_prep_slave_sg", "NEED_DMAENGINE_PREP_SLAVE_SG", "include/linux/dmaengine.h");
	check_files_for_func("SET_SYSTEM_SLEEP_PM_OPS", "NEED_SET_SYSTEM_SLEEP_PM_OPS", "include/linux/pm.h");
	check_files_for_func("__i2c_transfer", "NEED_UNLOCK_I2C_XFER", "include/linux/i2c.h");
	check_files_for_func("I2C_CLIENT_SCCB", "NEED_I2C_CLIENT_SCCB", "include/linux/i2c.h");
	check_files_for_func("kstrtou16", "NEED_KSTRTOU16", "include/linux/kernel.h");
	check_files_for_func("kstrtoul", "NEED_KSTRTOUL", "include/linux/kernel.h");
	check_files_for_func("memweight", "NEED_MEMWEIGHT", "include/linux/string.h");
	check_files_for_func("dev_dbg_ratelimited", "NEED_DEV_DBG_RATELIMITED", "include/linux/device.h");
	check_files_for_func("i2c_lock_adapter", "NEED_LOCK_ADAPTER", "include/linux/i2c.h");
	check_files_for_func("i2c_probe_func_quick_read", "NEED_I2C_PROBE_FUNC_QUICK_READ", "include/linux/i2c.h");
	check_files_for_func("abs64", "NEED_ABS64", "include/linux/kernel.h");
	check_files_for_func("VM_DONTDUMP", "NEED_DONTDUMP", "include/linux/mm.h");
	check_files_for_func("VM_NODUMP", "NEED_NODUMP", "include/linux/mm.h");
	check_files_for_func("config_enabled", "NEED_IS_ENABLED", "include/linux/kconfig.h");
	check_files_for_func("IS_REACHABLE", "NEED_IS_REACHABLE", "include/linux/kconfig.h");
	check_files_for_func("DEFINE_PCI_DEVICE_TABLE", "NEED_DEFINE_PCI_DEVICE_TABLE", "include/linux/pci.h");
	check_files_for_func("usb_translate_errors", "NEED_USB_TRANSLATE_ERRORS", "include/linux/usb.h");
	check_files_for_func("PTR_RET", "NEED_PTR_RET", "include/linux/err.h");
	check_files_for_func("file_inode", "NEED_FILE_INODE", "include/linux/fs.h");
	check_files_for_func("ETH_P_802_3_MIN", "NEED_ETH_P_802_3_MIN", "include/uapi/linux/if_ether.h");
	check_files_for_func("proc_set_size", "NEED_PROC_SET_SIZE", "include/linux/proc_fs.h");
	check_files_for_func("SIMPLE_DEV_PM_OPS", "NEED_SIMPLE_DEV_PM_OPS", "include/linux/pm.h");
	check_files_for_func("vm_iomap_memory", "NEED_VM_IOMAP_MEMORY", "include/linux/mm.h");
	check_files_for_func("device_lock", "NEED_DEVICE_LOCK", "include/linux/device.h");
	check_files_for_func("PTR_ERR_OR_ZERO", "NEED_PTR_ERR_OR_ZERO", "include/linux/err.h");
	check_files_for_func("sg_alloc_table_from_pages", "NEED_SG_ALLOC_TABLE_FROM_PAGES", "include/linux/scatterlist.h");
	check_files_for_func("replace_fops", "NEED_REPLACE_FOPS", "include/linux/fs.h");
	check_files_for_func("reinit_completion", "NEED_REINIT_COMPLETION", "include/linux/completion.h");
	check_files_for_func("dma_set_mask_and_coherent", "NEED_DMA_SET_MASK_AND_COHERENT", "include/linux/dma-mapping.h");
	check_files_for_func("dma_set_coherent_mask", "NEED_DMA_SET_COHERENT_MASK", "include/linux/dma-mapping.h");
	check_files_for_func("bitmap_clear", "NEED_BITMAP_CLEAR", "include/linux/bitmap.h");
	check_files_for_func("devm_kmalloc", "NEED_DEVM_KMALLOC", "include/linux/device.h");
	check_files_for_func("devm_kmalloc_array", "NEED_DEVM_KMALLOC_ARRAY", "include/linux/device.h");
	check_files_for_func("usb_speed_string", "NEED_USB_SPEED_STRING", "include/linux/usb/ch9.h");
	check_files_for_func("USB_SPEED_WIRELESS", "NEED_USB_SPEED_WIRELESS", "include/linux/usb/ch9.h");
	check_files_for_func("ether_addr_equal", "NEED_ETHER_ADDR_EQUAL", "include/linux/etherdevice.h");
	check_files_for_func("snd_card_new", "NEED_SND_CARD_NEW", "include/sound/core.h");
	check_files_for_func("compat_put_timespec", "NEED_COMPAT_PUT_TIMESPEC", "include/linux/compat.h");
	check_files_for_func("mp_mb__after_atomic", "NEED_SMP_MB_AFTER_ATOMIC", "include/asm-generic/barrier.h");
	check_files_for_func("pci_zalloc_consistent", "NEED_PCI_ZALLOC_CONSISTENT", "include/asm-generic/pci-dma-compat.h", "include/linux/pci-dma-compat.h");
	check_files_for_func("kref_get_unless_zero", "NEED_KREF_GET_UNLESS_ZERO", "include/linux/kref.h");
	check_files_for_func("prandom_u32_max", "NEED_PRANDOM_U32_MAX", "include/linux/random.h");
	check_files_for_func("prandom_u32", "NEED_PRANDOM_U32", "include/linux/random.h");
	check_files_for_func("GENMASK", "NEED_GENMASK", "include/linux/bitops.h");
	check_files_for_func("mult_frac", "NEED_MULT_FRAC", "include/linux/kernel.h");
	check_files_for_func("clk_prepare_enable", "NEED_CLOCK_HELPERS", "include/linux/clk.h");
	check_files_for_func("IS_MODULE", "NEED_IS_MODULE", "include/linux/kconfig.h");
	check_files_for_func("DMA_ATTR_SKIP_CPU_SYNC", "NEED_DMA_ATTR_SKIP_CPU_SYNC", "include/linux/dma-attrs.h");
	check_files_for_func("sign_extend32", "NEED_SIGN_EXTEND32", "include/linux/bitops.h");
	check_files_for_func("netdev_dbg", "NEED_NETDEV_DBG", "include/linux/netdevice.h");
	check_files_for_func("writel_relaxed", "NEED_WRITEL_RELAXED", "include/asm-generic/io.h");
	check_files_for_func("get_user_pages_unlocked", "NEED_GET_USER_PAGES_UNLOCKED", "include/linux/mm.h");
	check_files_for_func("pr_warn_once", "NEED_PR_WARN_ONCE", "include/linux/printk.h");
	check_files_for_func("DIV_ROUND_CLOSEST_ULL", "NEED_DIV_ROUND_CLOSEST_ULL", "include/linux/kernel.h");
	check_files_for_func("of_property_read_u64_array", "NEED_PROP_READ_U64_ARRAY", "include/linux/of.h");
	check_files_for_func("module_pnp_driver", "NEED_MODULE_PNP_DRIVER", "include/linux/pnp.h");
	check_files_for_func("eth_zero_addr", "NEED_ETH_ZERO_ADDR", "include/linux/etherdevice.h");
	check_files_for_func("frame_vector_create", "NEED_FRAME_VECTOR", "include/linux/mm.h");
	check_files_for_func("kvfree", "NEED_KVFREE", "include/linux/mm.h");
	check_files_for_func("ktime_before", "NEED_KTIME_BEFORE", "include/linux/ktime.h");
	check_files_for_func("ktime_compare", "NEED_KTIME_COMPARE", "include/linux/ktime.h");
	check_files_for_func("ktime_ms_delta", "NEED_KTIME_MS_DELTA", "include/linux/ktime.h");
	check_files_for_func("of_node_full_name", "NEED_OF_NODE_FULL_NAME", "include/linux/of.h");
	check_files_for_func("ktime_get_ns", "NEED_KTIME_GET_NS", "include/linux/timekeeping.h");
	check_files_for_func("div64_u64_rem", "NEED_DIV64_U64_REM", "include/linux/math64.h");
	check_files_for_func("led_set_brightness_sync", "NEED_LED_SET_BRIGHTNESS", "include/linux/leds.h");
	check_files_for_func("GENMASK_ULL", "NEED_GENMASK_ULL", "include/linux/bitops.h");
	check_files_for_func("ida_simple_remove", "NEED_IDA_SIMPLE_REMOVE", "include/linux/idr.h");
	check_files_for_func("ktime_get_boottime", "NEED_KTIME_GET_BOOTTIME", "include/linux/hrtimer.h", "include/linux/timekeeping.h");
	check_files_for_func("BUS_CEC", "NEED_BUS_CEC", "include/uapi/linux/input.h");
	check_files_for_func("smp_load_acquire", "NEED_SMP_LOAD_ACQUIRE", "include/asm-generic/barrier.h");

	# For tests for uapi-dependent logic
	check_files_for_func_uapi("usb_endpoint_maxp", "NEED_USB_ENDPOINT_MAXP", "usb/ch9.h");
}

# Do the basic rules
open IN, "<$infile" or die "File not found: $infile";

$out.= "#ifndef __CONFIG_COMPAT_H__\n";
$out.= "#define __CONFIG_COMPAT_H__\n\n";

$out.= "#include <linux/version.h>\n\n";
$out.= "#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)\n";
$out.= "#include <generated/autoconf.h>\n";
$out.= "#else\n";
$out.= "#include <linux/autoconf.h>\n";
$out.= "#endif\n\n";

# mmdebug.h includes autoconf.h. So if this header exists,
# then include it before our config is set.
if (-f "$kdir/include/linux/mmdebug.h") {
	$out.= "#include <linux/mmdebug.h>\n\n";
}

while(<IN>) {
	next unless /^(\S+)\s*:= (\S+)$/;
	$out.= "#undef $1\n";
	$out.= "#undef $1_MODULE\n";
	if($2 eq "n") {
		next;
	} elsif($2 eq "m") {
		$out.= "#define $1_MODULE 1\n";
	} elsif($2 eq "y") {
		$out.= "#define $1 1\n";
	} else {
		$out.= "#define $1 $2\n";
	}
}
close IN;

check_other_dependencies();

open OUT, ">$outfile" or die 'Unable to write $outfile';
print OUT "$out\n#endif\n";
close OUT
