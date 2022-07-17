#!/usr/bin/perl
use strict;

my $kdir=shift or die "should specify a kernel dir";
my $infile=shift or die "should specify an input config file";
my $outfile=shift or die "should specify an output config file";

my $out;

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

sub check_other_dependencies()
{
	check_files_for_func("frame_vector_create", "NEED_FRAME_VECTOR", "include/media/frame_vector.h");
	check_files_for_func("kvzalloc", "NEED_KVZALLOC", "include/linux/mm.h", "include/linux/slab.h");
	check_files_for_func("kvcalloc", "NEED_KVCALLOC", "include/linux/mm.h", "include/linux/slab.h");
	check_files_for_func("ktime_get_ns", "NEED_KTIME_GET_NS", "include/linux/timekeeping.h");
	check_files_for_func("led_set_brightness_sync", "NEED_LED_SET_BRIGHTNESS", "include/linux/leds.h");
	check_files_for_func("BUS_CEC", "NEED_BUS_CEC", "include/uapi/linux/input.h");
	check_files_for_func("kthread_init_worker", "NEED_KTHREAD_INIT_WORKER", "include/linux/kthread.h");
	check_files_for_func("print_hex_dump_debug", "NEED_PRINT_HEX_DUMP_DEBUG", "include/linux/printk.h");
	check_files_for_func("min3", "NEED_MIN3", "include/linux/kernel.h");
	check_files_for_func("rcu_pointer_handoff", "NEED_RCU_POINTER_HANDOFF", "include/linux/rcupdate.h");
	check_files_for_func("regmap_read_poll_timeout", "NEED_REGMAP_READ_POLL_TIMEOUT", "include/linux/regmap.h");
	check_files_for_func("dma_coerce_mask_and_coherent", "NEED_DMA_COERCE_MASK", "include/linux/dma-mapping.h");
	check_files_for_func("cdev_device_add", "NEED_CDEV_DEVICE", "include/linux/cdev.h");
	check_files_for_func("module_param_hw", "NEED_MODULE_PARAM_HW", "include/linux/moduleparam.h");
	check_files_for_func("of_fwnode_handle", "NEED_FWNODE", "include/linux/of.h");
	check_files_for_func("to_of_node", "NEED_TO_OF_NODE", "include/linux/of.h");
	check_files_for_func("is_of_node", "NEED_IS_OF_NODE", "include/linux/of.h");
	check_files_for_func("skb_put_data", "NEED_SKB_PUT_DATA", "include/linux/skbuff.h");
	check_files_for_func("pm_runtime_get_if_in_use", "NEED_PM_RUNTIME_GET", "include/linux/pm_runtime.h");
	check_files_for_func("KEY_APPSELECT", "NEED_KEY_APPSELECT", "include/uapi/linux/input-event-codes.h");
	check_files_for_func("PCI_DEVICE_SUB", "NEED_PCI_DEVICE_SUB", "include/linux/pci.h");
	check_files_for_func("U32_MAX", "NEED_U32_MAX", "include/linux/kernel.h", "include/linux/limits.h");
	check_files_for_func("U16_MAX", "NEED_U16_MAX", "include/linux/kernel.h", "include/linux/limits.h");
	check_files_for_func("bsearch", "NEED_BSEARCH", "include/linux/bsearch.h");
	check_files_for_func("timer_setup", "NEED_TIMER_SETUP", "include/linux/timer.h");
	check_files_for_func("__setup_timer", "NEED_SETUP_TIMER", "include/linux/timer.h");
	check_files_for_func("fwnode_reference_args", "NEED_FWNODE_REF_ARGS", "include/linux/fwnode.h");
	check_files_for_func("fwnode_for_each_child_node", "NEED_FWNODE_FOR_EACH_CHILD_NODE", "include/linux/property.h");
	check_files_for_func("fwnode_graph_for_each_endpoint", "NEED_FWNODE_GRAPH_FOR_EACH_ENDPOINT", "include/linux/property.h");
	check_files_for_func("fwnode_graph_get_port_parent", "NEED_FWNODE_GRAPH_GET_PORT_PARENT", "include/linux/property.h");
	check_files_for_func("fwnode_graph_get_endpoint_by_id", "NEED_FWNODE_GRAPH_GET_ENDPOINT_BY_ID", "include/linux/property.h");
	check_files_for_func("timer_setup_on_stack", "NEED_TIMER_SETUP_ON_STACK", "include/linux/timer.h");
	check_files_for_func("time64_to_tm", "NEED_TIME64_TO_TM", "include/linux/time.h");
	check_files_for_func("READ_ONCE", "NEED_READ_ONCE", "include/linux/compiler.h", "include/asm-generic/rwonce.h");
	check_files_for_func("usb_urb_ep_type_check", "NEED_USB_EP_CHECK", "include/linux/usb.h");
	check_files_for_func("get_user_pages_longterm", "NEED_GET_USER_PAGES_LONGTERM", "include/linux/mm.h");
	check_files_for_func("__pfn_to_phys", "NEED_PFN_TO_PHYS", "include/asm-generic/memory_model.h");
	check_files_for_func("next_pseudo_random32", "NEED_NEXT_PSEUDO_RANDOM32", "include/linux/random.h", "include/linux/prandom.h");
	check_files_for_func("memdup_user_nul", "NEED_MEMDUP_USER_NUL", "include/linux/string.h");
	check_files_for_func("STACK_FRAME_NON_STANDARD", "NEED_STACK_FRAME_NON_STANDARD", "include/linux/frame.h", "include/linux/objtool.h");
	check_files_for_func("pci_free_irq_vectors", "NEED_PCI_FREE_IRQ_VECTORS", "include/linux/pci.h");
	check_files_for_func(" pci_irq_vector", "NEED_PCI_IRQ_VECTOR", "include/linux/pci.h");
	check_files_for_func("U8_MAX", "NEED_U8_MAX", "include/linux/kernel.h");
	check_files_for_func("kthread_freezable_should_stop", "NEED_KTHREAD_FREEZABLE_SHOULD_STOP", "include/linux/kthread.h");
	check_files_for_func(" vm_fault_t;", "NEED_VM_FAULT_T", "include/linux/mm_types.h");
	check_files_for_func("array_index_nospec", "NEED_ARRAY_INDEX_NOSPEC", "include/linux/nospec.h");
	check_files_for_func("list_first_entry_or_null", "NEED_LIST_FIRST_ENTRY_OR_NULL", "include/linux/list.h");
	check_files_for_func("struct_size", "NEED_STRUCT_SIZE", "linux/overflow.h");
	check_files_for_func("list_last_entry", "NEED_LIST_LAST_ENTRY", "include/linux/list.h");
	check_files_for_func("list_next_entry", "NEED_LIST_NEXT_ENTRY", "include/linux/list.h");
	check_files_for_func("xa_lock_irqsave", "NEED_XA_LOCK_IRQSAVE", "include/linux/xarray.h");
	check_files_for_func("ida_alloc_min", "NEED_IDA_ALLOC_MIN", "include/linux/idr.h");
	check_files_for_func("i2c_lock_bus", "NEED_I2C_LOCK_BUS", "include/linux/i2c.h");
	check_files_for_func("strscpy", "NEED_STRSCPY", "include/linux/string.h");
	check_files_for_func("strchrnul", "NEED_STRCHRNUL", "include/linux/string.h");
	check_files_for_func("i2c_8bit_addr_from_msg", "NEED_I2C_8BIT_ADDR_FROM_MSG", "include/linux/i2c.h");
	check_files_for_func("lockdep_assert_irqs_enabled", "NEED_LOCKDEP_ASSERT_IRQS", "include/linux/lockdep.h");
	check_files_for_func("of_node_name_eq", "NEED_OF_NODE_NAME_EQ", "include/linux/of.h");
	check_files_for_func("FOLL_LONGTERM", "NEED_FOLL_LONGTERM", "include/linux/mm.h");
	check_files_for_func("stream_open", "NEED_STREAM_OPEN", "include/linux/fs.h");
	check_files_for_func("fwnode_property_read_u32_array", "NEED_PROP_READ_U32_ARRAY", "include/linux/property.h");
	check_files_for_func("fwnode_property_count_u32", "NEED_PROP_COUNT", "include/linux/property.h");
	check_files_for_func("fwnode_get_name\\(", "NEED_FWNODE_GETNAME", "include/linux/property.h");
	check_files_for_func("i2c_new_secondary_device", "NEED_I2C_NEW_SECONDARY_DEV", "include/linux/i2c.h");
	check_files_for_func("^i2c_new_dummy_device", "NEED_I2C_NEW_DUMMY_DEVICE", "include/linux/i2c.h");
	check_files_for_func("i2c_new_ancillary_device", "NEED_I2C_NEW_ANCILLARY_DEVICE", "include/linux/i2c.h");
	check_files_for_func("i2c_new_scanned_device", "NEED_I2C_NEW_SCANNED_DEVICE", "include/linux/i2c.h");
	check_files_for_func("i2c_new_client_device", "NEED_I2C_NEW_CLIENT_DEVICE", "include/linux/i2c.h");
	check_files_for_func("i2c_client_has_driver", "NEED_I2C_CLIENT_HAS_DRIVER", "include/linux/i2c.h");
	check_files_for_func("untagged_addr", "NEED_UNTAGGED_ADDR", "include/linux/mm.h");
	check_files_for_func("compat_ptr_ioctl", "NEED_COMPAT_PTR_IOCTL", "include/linux/fs.h");
	check_files_for_func("ns_to_timespec64", "NEED_TIMESPEC64", "include/linux/time64.h");
	check_files_for_func("sizeof_field", "NEED_SIZEOF_FIELD", "include/linux/stddef.h");
	check_files_for_func("devm_platform_ioremap_resource", "NEED_DEVM_PLATFORM_IOREMAP_RESOURCE", "include/linux/platform_device.h");
	check_files_for_func("cpu_latency_qos_add_request", "NEED_CPU_LATENCY_QOS", "include/linux/pm_qos.h");
	check_files_for_func("fwnode_property_present", "NEED_FWNODE_PROPERTY_PRESENT", "include/linux/property.h");
	check_files_for_func("fwnode_graph_is_endpoint", "NEED_FWNODE_GRAPH_IS_ENDPOINT", "include/linux/property.h");
	check_files_for_func("fallthrough", "NEED_FALLTHROUGH", "include/linux/compiler_attributes.h");
	check_files_for_func("sched_set_fifo", "NEED_SCHED_SET_FIFO", "include/linux/sched.h");
	check_files_for_func("dma_map_sgtable", "NEED_DMA_MAP_SGTABLE", "include/linux/dma-mapping.h");
	check_files_for_func("in_compat_syscall", "NEED_IN_COMPAT_SYSCALL", "include/linux/compat.h");
	check_files_for_func("dev_err_probe", "NEED_DEV_ERR_PROBE", "include/linux/device.h");
	check_files_for_func("pm_runtime_resume_and_get", "NEED_PM_RUNTIME_RESUME_AND_GET", "include/linux/pm_runtime.h");
	check_files_for_func("vma_lookup", "NEED_VMA_LOOKUP", "include/linux/mm.h");
	check_files_for_func("HZ_PER_MHZ", "NEED_HZ_PER_MHZ", "include/linux/units.h");
	check_files_for_func("dma_vmap_noncontiguous", "NEED_DMA_VMAP_NONCONTIGUOUS", "include/linux/dma-mapping.h");
	check_files_for_func("sysfs_emit", "NEED_SYSFS_EMIT", "include/linux/sysfs.h");
	check_files_for_func("eth_hw_addr_set", "NEED_ETH_HW_ADDR_SET", "include/linux/etherdevice.h");
	check_files_for_func("MODULE_IMPORT_NS", "NEED_MODULE_IMPORT_NS", "include/linux/module.h");
	check_files_for_func("ALIGN_DOWN", "NEED_ALIGN_DOWN", "include/linux/align.h", "include/linux/kernel.h");
	check_files_for_func("bitmap_zalloc", "NEED_BITMAP_ZALLOC", "include/linux/bitmap.h");
	check_files_for_func("firmware_request_nowarn", "NEED_FIRMWARE_REQUEST_NOWARN", "include/linux/firmware.h");
	check_files_for_func("lockdep_assert_not_held", "NEED_LOCKDEP_ASSERT_NOT_HELD", "include/linux/lockdep.h");
	check_files_for_func("dev_is_platform", "NEED_DEV_IS_PLATFORM", "include/linux/platform_device.h");

	# For tests for uapi-dependent logic
	check_files_for_func_uapi("usb_endpoint_maxp_mult", "NEED_USB_ENDPOINT_MAXP_MULT", "usb/ch9.h");
	check_files_for_func_uapi("PCI_EXP_DEVCTL2_COMP_TIMEOUT", "NEED_PCI_EXP_DEVCTL2_COMP_TIMEOUT", "pci_regs.h");
	check_files_for_func_uapi("__poll_t", "NEED_POLL_T", "types.h");
	check_files_for_func_uapi("KEY_SCREENSAVER", "NEED_KEY_SCREENSAVER", "input.h");
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
