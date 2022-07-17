/*
 * $Id: compat.h,v 1.44 2006/01/15 09:35:16 mchehab Exp $
 */

#ifndef _COMPAT_H
#define _COMPAT_H

#include <linux/version.h>

#include "config-compat.h"
/*
 * config-mycompat.h is for use with kernels/distros whose maintainers
 * have integrated various backports, which the media_build system does
 * not pick up on for whatever reason. At that point there are options
 * defined in config-compat.h, which enable backports here, in compat.h,
 * but which already exist in the target kernel. This allows disabling of
 * specific backports for a particular build, allowing compliation to succeed.

 * For example, if the following three statements exist in config-mycompat.h:

 * #undef NEED_WRITEL_RELAXED
 * #undef NEED_PM_RUNTIME_GET
 * #undef NEED_PFN_TO_PHYS

 * Those three media_build backports will be disabled in this file and
 * compilation on a problematic kernel will succeed without issue.
 * conifg-mycompat.h should be used strictly for disabling media_build
 * backports causing compilation issues. It will typically be left empty.
 *
 * WARNING: v4l/config-mycompat.h is removed by distclean, the file
 * should be saved externally and copied into v4l/ when required.
 */
#include "config-mycompat.h"

#include <linux/compiler.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0)
/* we got a lot of warnings for Kernels older than 4.16 because strscpy has
 * been declared with "__must_check" prior to 4.16. In fact it is really not
 * necessary to check the return value of strscpy, so we clear the
 * "__must_check" definition.
 */
#undef __must_check
#define __must_check
#endif

#include <linux/input.h>
#include <linux/init.h>
#include <linux/idr.h>
#include <linux/kernel.h>
#include "../linux/kernel_version.h"

#ifdef RETPOLINE
#ifndef __noretpoline
#define __noretpoline __attribute__((indirect_branch("keep")))
#endif
#endif

#undef __devinitconst
#define __devinitconst

#ifndef uninitialized_var
#define uninitialized_var(x) x = x
#endif

#ifdef NEED_POLL_T
typedef unsigned __poll_t;
/* Epoll event masks */
#define EPOLLIN		(__force __poll_t)0x00000001
#define EPOLLPRI	(__force __poll_t)0x00000002
#define EPOLLOUT	(__force __poll_t)0x00000004
#define EPOLLERR	(__force __poll_t)0x00000008
#define EPOLLHUP	(__force __poll_t)0x00000010
#define EPOLLNVAL	(__force __poll_t)0x00000020
#define EPOLLRDNORM	(__force __poll_t)0x00000040
#define EPOLLRDBAND	(__force __poll_t)0x00000080
#define EPOLLWRNORM	(__force __poll_t)0x00000100
#define EPOLLWRBAND	(__force __poll_t)0x00000200
#define EPOLLMSG	(__force __poll_t)0x00000400
#define EPOLLRDHUP	(__force __poll_t)0x00002000
#endif

#ifndef KEY_FULL_SCREEN
#define KEY_FULL_SCREEN		0x174   /* AC View Toggle */
#endif

#ifdef NEED_VZALLOC
#include <linux/vmalloc.h>

static inline void *vzalloc(unsigned long size)
{
	void *p = vmalloc(size);
	if (!p)
		return NULL;
	memset (p, '\0', size);
	return p;
}

#endif

#ifdef NEED_KVZALLOC
#include <linux/vmalloc.h>
#include <linux/gfp.h>

static inline void *kvzalloc(size_t size, gfp_t flags)
{
	return vzalloc(size);
}

static inline void *kvmalloc(size_t size, gfp_t flags)
{
	return (flags & __GFP_ZERO) ? vzalloc(size) : vmalloc(size);
}

static inline void *kvmalloc_array(size_t n, size_t size, gfp_t flags)
{
	if (size != 0 && n > SIZE_MAX / size)
		return NULL;

	return kvmalloc(n * size, flags);
}
#endif

#ifdef NEED_KVCALLOC
#include <linux/mm.h>
static inline void *kvcalloc(size_t n, size_t size, gfp_t flags)
{
	return kvmalloc_array(n, size, flags | __GFP_ZERO);
}
#endif

#ifdef NEED_USB_ENDPOINT_MAXP_MULT
#define USB_EP_MAXP_MULT_SHIFT  11
#define USB_EP_MAXP_MULT_MASK   (3 << USB_EP_MAXP_MULT_SHIFT)
#define USB_EP_MAXP_MULT(m) \
	        (((m) & USB_EP_MAXP_MULT_MASK) >> USB_EP_MAXP_MULT_SHIFT)

#define usb_endpoint_maxp_mult(epd) (USB_EP_MAXP_MULT(usb_endpoint_maxp(epd)) + 1)
#endif

#ifdef NEED_FRAME_VECTOR
#include <linux/mm.h>

#define frame_vector media_frame_vector
#define frame_vector_create media_frame_vector_create
#define frame_vector_destroy media_frame_vector_destroy
#define get_vaddr_frames media_get_vaddr_frames
#define put_vaddr_frames media_put_vaddr_frames
#define frame_vector_to_pages media_frame_vector_to_pages
#define frame_vector_to_pfns media_frame_vector_to_pfns
#define frame_vector_count media_frame_vector_count
#define frame_vector_pages media_frame_vector_pages
#define frame_vector_pfns media_frame_vector_pfns

#endif

#ifdef NEED_KTIME_GET_NS
static inline u64 ktime_get_ns(void)
{
        return ktime_to_ns(ktime_get());
}
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)
#ifdef NEED_LED_SET_BRIGHTNESS
#include <linux/leds.h>
void led_trigger_remove(struct led_classdev *led_cdev);
static inline int led_set_brightness_sync(struct led_classdev *led_cdev,
					  enum led_brightness value)
{
	int ret = 0;

	led_cdev->brightness = min(value, led_cdev->max_brightness);

	if (!(led_cdev->flags & LED_SUSPENDED))
		ret = led_cdev->brightness_set_sync(led_cdev,
						    led_cdev->brightness);
	return ret;
}
#endif
#endif

#ifdef NEED_BUS_CEC
#define BUS_CEC			0x1E
/* Diagonal movement keys */
#define KEY_RIGHT_UP			0x266
#define KEY_RIGHT_DOWN			0x267
#define KEY_LEFT_UP			0x268
#define KEY_LEFT_DOWN			0x269

#define KEY_ROOT_MENU			0x26a /* Show Device's Root Menu */
/* Show Top Menu of the Media (e.g. DVD) */
#define KEY_MEDIA_TOP_MENU		0x26b
#define KEY_NUMERIC_11			0x26c
#define KEY_NUMERIC_12			0x26d
/*
 * Toggle Audio Description: refers to an audio service that helps blind and
 * visually impaired consumers understand the action in a program. Note: in
 * some countries this is referred to as "Video Description".
 */
#define KEY_AUDIO_DESC			0x26e
#define KEY_3D_MODE			0x26f
#define KEY_NEXT_FAVORITE		0x270
#define KEY_STOP_RECORD			0x271
#define KEY_PAUSE_RECORD		0x272
#define KEY_VOD				0x273 /* Video on Demand */
#define KEY_UNMUTE			0x274
#define KEY_FASTREVERSE			0x275
#define KEY_SLOWREVERSE			0x276
/*
 * Control a data application associated with the currently viewed channel,
 * e.g. teletext or data broadcast application (MHEG, MHP, HbbTV, etc.)
 */
#define KEY_DATA			0x275
#endif

#define SERIO_PULSE8_CEC     0x40
#define SERIO_RAINSHADOW_CEC 0x41

#ifdef NEED_KTHREAD_INIT_WORKER
#define __kthread_init_worker __init_kthread_worker
#define kthread_init_worker init_kthread_worker
#define kthread_init_work init_kthread_work
#define kthread_insert_work insert_kthread_work
#define kthread_queue_work queue_kthread_work
#define kthread_flush_work flush_kthread_work
#define kthread_flush_worker flush_kthread_worker
#endif

#ifdef NEED_PRINT_HEX_DUMP_DEBUG
#define print_hex_dump_debug(prefix_str, prefix_type, rowsize,          \
			     groupsize, buf, len, ascii)                \
	print_hex_dump(KERN_DEBUG, prefix_str, prefix_type, rowsize,    \
		       groupsize, buf, len, ascii)
#endif

#ifdef NEED_MIN3
#define min3(x, y, z) min((typeof(x))min(x, y), z)
#define max3(x, y, z) max((typeof(x))max(x, y), z)
#endif

#ifdef NEED_RCU_POINTER_HANDOFF
#define rcu_pointer_handoff(p) (p)
#endif

#ifdef NEED_REGMAP_READ_POLL_TIMEOUT
#define regmap_read_poll_timeout(map, addr, val, cond, sleep_us, timeout_us) \
({ \
	ktime_t timeout = ktime_add_us(ktime_get(), timeout_us); \
	int pollret; \
	might_sleep_if(sleep_us); \
	for (;;) { \
		pollret = regmap_read((map), (addr), &(val)); \
		if (pollret) \
			break; \
		if (cond) \
			break; \
		if (timeout_us && ktime_compare(ktime_get(), timeout) > 0) { \
			pollret = regmap_read((map), (addr), &(val)); \
			break; \
		} \
		if (sleep_us) \
			usleep_range((sleep_us >> 2) + 1, sleep_us); \
	} \
	pollret ?: ((cond) ? 0 : -ETIMEDOUT); \
})
#endif

#define of_node_cmp(s1, s2)          strcasecmp((s1), (s2))

#ifndef BIT_ULL
#define BIT_ULL(nr)        (1ULL << (nr))
#define BIT_ULL_MASK(nr)   (1ULL << ((nr) % BITS_PER_LONG_LONG))
#define BIT_ULL_WORD(nr)   ((nr) / BITS_PER_LONG_LONG)
#endif

#ifdef NEED_DMA_COERCE_MASK
#include <linux/dma-mapping.h>
static inline int dma_coerce_mask_and_coherent(struct device *dev, u64 mask)
{
	dev->dma_mask = &dev->coherent_dma_mask;
	return dma_set_mask_and_coherent(dev, mask);
}
#endif

#ifdef NEED_CDEV_DEVICE
#include <linux/cdev.h>

static inline void cdev_set_parent(struct cdev *p, struct kobject *kobj)
{
	WARN_ON(!kobj->state_initialized);
	p->kobj.parent = kobj;
}

static inline int cdev_device_add(struct cdev *cdev, struct device *dev)
{
	int rc = 0;

	if (dev->devt) {
		cdev_set_parent(cdev, &dev->kobj);

		rc = cdev_add(cdev, dev->devt, 1);
		if (rc)
			return rc;
	}

	rc = device_add(dev);
	if (rc)
		cdev_del(cdev);

	return rc;
}

static inline void cdev_device_del(struct cdev *cdev, struct device *dev)
{
	device_del(dev);
	if (dev->devt)
		cdev_del(cdev);
}
#endif

#ifdef NEED_MODULE_PARAM_HW
#include <linux/moduleparam.h>
#define module_param_hw(n, t, hwtype, p) module_param(n, t, p)
#define module_param_hw_named(n, v, t, hwtype, p) module_param_named(n, v, t, p)
#define module_param_hw_array(n, t, hwtype, m, p) module_param_array(n, t, m, p)
#endif

#ifdef NEED_FWNODE

/* Minimum stuff for drivers to build without OF support */
#define of_fwnode_handle(node) NULL

struct fwnode_endpoint {
	unsigned int port;
	unsigned int id;
	const struct fwnode_handle *local_fwnode;
};

static inline struct fwnode_handle *fwnode_get_parent(struct fwnode_handle *fwnode)
{
        return NULL;
}

static inline struct fwnode_handle *fwnode_get_next_parent(struct fwnode_handle *fwnode)
{
        return NULL;
}

static inline struct fwnode_handle *fwnode_graph_get_next_endpoint(
	struct fwnode_handle *fwnode, struct fwnode_handle *prev)
{
	return NULL;
}

static inline struct fwnode_handle *
fwnode_graph_get_remote_endpoint(struct fwnode_handle *fwnode)
{
        return NULL;
}

static inline struct fwnode_handle *dev_fwnode(struct device *dev)
{
	return NULL;
}

static inline int fwnode_graph_parse_endpoint(struct fwnode_handle *fwnode,
                                struct fwnode_endpoint *endpoint)
{
	if (endpoint)
		endpoint->port = endpoint->id = 0;
	return 0;
}

static inline void fwnode_handle_get(struct fwnode_handle *fwnode)
{
}

static inline void fwnode_handle_put(struct fwnode_handle *fwnode)
{
}

#endif

#ifdef NEED_FWNODE_GRAPH_GET_ENDPOINT_BY_ID

#define FWNODE_GRAPH_ENDPOINT_NEXT      BIT(0)
#define FWNODE_GRAPH_DEVICE_DISABLED    BIT(1)

static inline struct fwnode_handle *
fwnode_graph_get_endpoint_by_id(const struct fwnode_handle *fwnode,
                                u32 port, u32 endpoint, unsigned long flags)
{
	return NULL;
}

#endif

#ifdef NEED_TO_OF_NODE
static inline struct device_node *to_of_node(struct fwnode_handle *fwnode)
{
	return NULL;
}
#endif

#ifdef NEED_IS_OF_NODE
static inline bool is_of_node(struct fwnode_handle *fwnode)
{
	return false;
}
#endif

#ifdef NEED_SKB_PUT_DATA
#include <linux/skbuff.h>
static inline void *skb_put_data(struct sk_buff *skb, const void *data,
                                 unsigned int len)
{
        void *tmp = skb_put(skb, len);

        memcpy(tmp, data, len);

        return tmp;
}
#endif

#ifdef NEED_PM_RUNTIME_GET
static inline int pm_runtime_get_if_in_use(struct device *dev)
{
	unsigned long flags;
	int retval;

	spin_lock_irqsave(&dev->power.lock, flags);
	retval = dev->power.disable_depth > 0 ? -EINVAL :
		dev->power.runtime_status == RPM_ACTIVE
			&& atomic_inc_not_zero(&dev->power.usage_count);
	spin_unlock_irqrestore(&dev->power.lock, flags);
	return retval;
}
#endif

#ifdef NEED_KEY_APPSELECT
#define KEY_APPSELECT         0x244   /* AL Select Task/Application */
#endif

#ifndef __GFP_RETRY_MAYFAIL
#define __GFP_RETRY_MAYFAIL __GFP_REPEAT
#endif

#ifdef NEED_PCI_DEVICE_SUB
#define PCI_DEVICE_SUB(vend, dev, subvend, subdev) \
	.vendor = (vend), .device = (dev), \
	.subvendor = (subvend), .subdevice = (subdev)
#endif


#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 16, 0)

#include <linux/jiffies.h>

/*
 * copied from kernel/time/time.c
 */
static inline u64 nsecs_to_jiffies64_static(u64 n)
{
#if (NSEC_PER_SEC % HZ) == 0
    /* Common case, HZ = 100, 128, 200, 250, 256, 500, 512, 1000 etc. */
    return div_u64(n, NSEC_PER_SEC / HZ);
#elif (HZ % 512) == 0
    /* overflow after 292 years if HZ = 1024 */
    return div_u64(n * HZ / 512, NSEC_PER_SEC / 512);
#else
    /*
     * Generic case - optimized for cases where HZ is a multiple of 3.
     * overflow after 64.99 years, exact for HZ = 60, 72, 90, 120 etc.
     */
    return div_u64(n * 9, (9ull * NSEC_PER_SEC + HZ / 2) / HZ);
#endif
}

static inline unsigned long nsecs_to_jiffies_static(u64 n)
{
    return (unsigned long)nsecs_to_jiffies64_static(n);
}

/*
 * linux/jiffies.h defines nsecs_to_jiffies64 and nsecs_to_jiffies
 * as externals. To get rid of the compiler error, we redefine the
 * functions to the static variant just defined above.
 */
#define nsecs_to_jiffies64(_n) nsecs_to_jiffies64_static(_n)
#define nsecs_to_jiffies(_n) nsecs_to_jiffies_static(_n)

#endif

#ifdef NEED_U32_MAX
#define U32_MAX     ((u32)~0U)
#endif

#ifdef NEED_U16_MAX
#define U16_MAX     ((u16)~0U)
#endif

#ifdef NEED_BSEARCH
static inline void *bsearch(const void *key, const void *base, size_t num, size_t size,
                            int (*cmp)(const void *key, const void *elt))
{
    const char *pivot;
    int result;

    while (num > 0) {
        pivot = base + (num >> 1) * size;
        result = cmp(key, pivot);

        if (result == 0)
            return (void *)pivot;

        if (result > 0) {
            base = pivot + size;
            num--;
        }
        num >>= 1;
    }

    return NULL;
}
#endif

#ifdef NEED_SETUP_TIMER
#define __setup_timer(_timer, _fn, _data, _flags)                       \
        do {                                                            \
                init_timer(_timer);                                     \
                (_timer)->function = (_fn);                             \
                (_timer)->data = (_data);                               \
        } while (0)
#endif

#ifdef NEED_TIMER_SETUP
#define TIMER_DATA_TYPE                unsigned long
#define TIMER_FUNC_TYPE                void (*)(TIMER_DATA_TYPE)

static inline void timer_setup(struct timer_list *timer,
                              void (*callback)(struct timer_list *),
                              unsigned int flags)
{
       __setup_timer(timer, (TIMER_FUNC_TYPE)callback,
                     (TIMER_DATA_TYPE)timer, flags);
}

#define from_timer(var, callback_timer, timer_fieldname) \
       container_of(callback_timer, typeof(*var), timer_fieldname)

#endif

#ifdef NEED_FWNODE_REF_ARGS
#define NR_FWNODE_REFERENCE_ARGS 8
struct fwnode_handle;
struct fwnode_reference_args {
	struct fwnode_handle *fwnode;
	unsigned int nargs;
	unsigned int args[NR_FWNODE_REFERENCE_ARGS];
};

static inline int fwnode_property_get_reference_args(const struct fwnode_handle *fwnode,
				       const char *prop, const char *nargs_prop,
				       unsigned int nargs, unsigned int index,
				       struct fwnode_reference_args *args)
{
	return -ENOENT;
}
#endif

#ifdef NEED_FWNODE_FOR_EACH_CHILD_NODE
static inline struct fwnode_handle *
fwnode_get_next_child_node(struct fwnode_handle *fwnode,
			   struct fwnode_handle *child)
{
	return NULL;
}

#define fwnode_for_each_child_node(fwnode, child)                       \
        for (child = fwnode_get_next_child_node(fwnode, NULL); child;   \
             child = fwnode_get_next_child_node(fwnode, child))

static inline struct fwnode_handle *
fwnode_graph_get_remote_port_parent(const struct fwnode_handle *fwnode)
{
	return NULL;
}
#endif

#ifdef NEED_FWNODE_GRAPH_GET_PORT_PARENT
static inline struct fwnode_handle *
fwnode_graph_get_port_parent(const struct fwnode_handle *fwnode)
{
	return NULL;
}
static inline bool fwnode_device_is_available(struct fwnode_handle *fwnode)
{
	return false;
}
#endif

#ifdef NEED_PROP_COUNT

#ifdef NEED_PROP_READ_U32_ARRAY
static inline int fwnode_property_read_u32_array(struct fwnode_handle *fwnode,
						 const char *propname,
						 u32 *val, size_t nval)
{
	return -ENODATA;
}

static inline int fwnode_property_read_u64_array(struct fwnode_handle *fwnode,
						 const char *propnam,
						 u64 *val, size_t nval)
{
	return -ENODATA;
}
#else
#include <linux/property.h>
#endif

static inline int fwnode_property_count_u32(struct fwnode_handle *fwnode,
					    const char *propname)
{
	return fwnode_property_read_u32_array(fwnode, propname, NULL, 0);
}

static inline int fwnode_property_count_u64(struct fwnode_handle *fwnode,
					    const char *propname)
{
	return fwnode_property_read_u64_array(fwnode, propname, NULL, 0);
}
#endif

#ifdef NEED_FWNODE_GETNAME
static inline const char *fwnode_get_name(const struct fwnode_handle *fwnode)
{
	return "name";
}
#endif

#ifdef NEED_TIMER_SETUP_ON_STACK
#define timer_setup_on_stack(timer, callback, flags)        \
        setup_timer_on_stack((timer), (TIMER_FUNC_TYPE)(callback), (flags))
#endif

#ifdef NEED_TIME64_TO_TM
#define time64_to_tm(totalsecs, offset, result) time_to_tm((time_t)totalsecs, offset, result)
#endif

#ifdef NEED_READ_ONCE
#define READ_ONCE(x)  ACCESS_ONCE(x)
#endif

#ifdef NEED_USB_EP_CHECK
static inline int usb_urb_ep_type_check(void *urb)
{
	/* This is for security. Backward compat may survive without that */
	return 0;
}
#endif

/* prototype of get_user_pages changed in Kernel 4.9. For older Kernels
 * this will not compile */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)
#ifdef NEED_GET_USER_PAGES_LONGTERM
#include <linux/mm.h>
static inline long get_user_pages_longterm(unsigned long start,
                unsigned long nr_pages, unsigned int gup_flags,
                struct page **pages, struct vm_area_struct **vmas)
{
        return get_user_pages(start, nr_pages, gup_flags, pages, vmas);
}
#endif
#endif

#ifdef NEED_PCI_EXP_DEVCTL2_COMP_TIMEOUT
#define  PCI_EXP_DEVCTL2_COMP_TIMEOUT     0x000f
#endif

#ifdef NEED_PFN_TO_PHYS
#include <linux/pfn.h>
#define __pfn_to_phys(pfn)  PFN_PHYS(pfn)
#endif

#ifdef NEED_NEXT_PSEUDO_RANDOM32
static inline u32 next_pseudo_random32(u32 seed)
{
	return seed * 1664525 + 1013904223;
}
#endif

#ifdef NEED_MEMDUP_USER_NUL
#include <linux/compat.h>
#include <linux/slab.h>
static inline void *memdup_user_nul(const void __user *src, size_t len)
{
	char *p;

	/*
	 * Always use GFP_KERNEL, since copy_from_user() can sleep and
	 * cause pagefault, which makes it pointless to use GFP_NOFS
	 * or GFP_ATOMIC.
	 */
	p = kmalloc_track_caller(len + 1, GFP_KERNEL);
	if (!p)
		return ERR_PTR(-ENOMEM);

	if (copy_from_user(p, src, len)) {
		kfree(p);
		return ERR_PTR(-EFAULT);
	}
	p[len] = '\0';

	return p;
}
#endif

#ifdef NEED_KEY_SCREENSAVER
#define KEY_SCREENSAVER     0x245   /* AL Screen Saver */
#endif

#ifdef NEED_STACK_FRAME_NON_STANDARD
#define STACK_FRAME_NON_STANDARD(func)
#else
/* be sure STACK_FRAME_NON_STANDARD is defined */
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
#include <linux/frame.h>
#else
#include <linux/objtool.h>
#endif
#endif

#ifdef NEED_PCI_FREE_IRQ_VECTORS
#include <linux/pci.h>
static inline void pci_free_irq_vectors(struct pci_dev *dev)
{
}
#endif

#ifdef NEED_PCI_IRQ_VECTOR
#include <linux/pci.h>
static inline int pci_irq_vector(struct pci_dev *dev, unsigned int nr)
{
        if (WARN_ON_ONCE(nr > 0))
                    return -EINVAL;
            return dev->irq;
}
#endif

#ifdef NEED_U8_MAX
#define U8_MAX      ((u8)~0U)
#endif

#ifdef NEED_KTHREAD_FREEZABLE_SHOULD_STOP
#include <linux/kthread.h>
#include <linux/freezer.h>
static inline bool kthread_freezable_should_stop(bool *was_frozen)
{
	bool frozen = false;

	might_sleep();

	if (unlikely(freezing(current))) {
		/* __refrigerator is not available for Kernels older than 3.3
		 * so we can only sa false */
		frozen = false;
	}

	if (was_frozen)
		*was_frozen = frozen;

	return kthread_should_stop();
}
#endif

#ifdef NEED_VM_FAULT_T
typedef int vm_fault_t;
#endif

/* header location for of_find_i2c_[device,adapter]_by_node */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 12, 0)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 5, 0)
#include <linux/i2c.h>
#include <linux/of_i2c.h>
#endif
#endif

#ifdef NEED_ARRAY_INDEX_NOSPEC
#define array_index_nospec(index, size)    index
#else
/* Some older Kernels got a backport, but we removed the include of
 * "linux/nospec.h" with patch "v4.13_remove_nospec_h.patch". Thus
 * including it again.
 */
#include <linux/nospec.h>
#endif

#ifdef NEED_LIST_FIRST_ENTRY_OR_NULL
#define list_first_entry_or_null(ptr, type, member) \
        (!list_empty(ptr) ? list_first_entry(ptr, type, member) : NULL)
#endif

#ifdef NEED_STRUCT_SIZE

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0)
/* This older Kernels define "is_signed_type" different (and wrong)
 * in "ftrace_event.h". "overflow.h" will correct this, which results
 * in a warning because of different definitions. We include the file
 * with the wrong definition first at this place and undefine the wrong
 * definition, so that overflow can set it correctly without a warning.
 * Moreover, "ftrace_event.h" will be never included again, because it
 * is already included here. Thus, we will get no warning, if another
 * file does include "ftrace_event.h".
 */
#include <linux/ftrace_event.h>
#undef is_signed_type
#endif

#include <linux/overflow.h>
#endif

#ifdef NEED_LIST_LAST_ENTRY
#define list_last_entry(ptr, type, member) \
        list_entry((ptr)->prev, type, member)
#endif

#ifdef NEED_LIST_NEXT_ENTRY
#define list_next_entry(pos, member) \
	list_entry((pos)->member.next, typeof(*(pos)), member)
#endif

#ifdef NEED_XA_LOCK_IRQSAVE
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 17, 0)
#define xa_lock_irqsave(xa, flags) (void)flags
#define xa_unlock_irqrestore(xa, flags) (void)flags
#else
#define xa_lock_irqsave(xa, flags) \
				spin_lock_irqsave(&(xa)->xa_lock, flags)
#define xa_unlock_irqrestore(xa, flags) \
				spin_unlock_irqrestore(&(xa)->xa_lock, flags)
#endif
#endif


#ifdef NEED_IDA_ALLOC_MIN
#include <linux/idr.h>
static inline
int ida_alloc_range(struct ida *ida, unsigned int min, unsigned int max,
			gfp_t gfp)
{
	int id = 0, err;
	unsigned long flags;

	if ((int)min < 0)
		return -ENOSPC;

	if ((int)max < 0)
		max = INT_MAX;

again:
	xa_lock_irqsave(&ida->ida_rt, flags);
	err = ida_get_new_above(ida, min, &id);
	if (err < 0)
		id = err;
	if (id > (int)max) {
		ida_remove(ida, id);
		id = -ENOSPC;
	}
	xa_unlock_irqrestore(&ida->ida_rt, flags);

	if (unlikely(id == -EAGAIN)) {
		if (!ida_pre_get(ida, gfp))
			return -ENOMEM;
		goto again;
	}

	return id;
}

static inline int ida_alloc_min(struct ida *ida, unsigned int min, gfp_t gfp)
{
	return ida_alloc_range(ida, min, ~0, gfp);
}

static inline
void ida_free(struct ida *ida, unsigned int id)
{
	unsigned long flags;

	BUG_ON((int)id < 0);
	xa_lock_irqsave(&ida->ida_rt, flags);
	ida_remove(ida, id);
	xa_unlock_irqrestore(&ida->ida_rt, flags);
}
#endif

#ifdef NEED_I2C_LOCK_BUS

#include <linux/i2c.h>

#define I2C_LOCK_ROOT_ADAPTER 0
#define I2C_LOCK_SEGMENT      1

static inline void
i2c_lock_bus(struct i2c_adapter *adapter, unsigned int flags)
{
	/* there is no bus implementation for Kernels < 4.7
	 * fallback to adapter locking */
	i2c_lock_adapter(adapter);
}

static inline void
i2c_unlock_bus(struct i2c_adapter *adapter, unsigned int flags)
{
	/* there is no bus implementation for Kernels < 4.7
	 * fallback to adapter unlocking */
	i2c_unlock_adapter(adapter);
}
#endif

#ifdef NEED_STRSCPY
#include <linux/string.h>
static inline
ssize_t strscpy(char *dest, const char *src, size_t count)
{
	long res = 0;

	if (count == 0)
		return -E2BIG;

	while (count) {
		char c;

		c = src[res];
		dest[res] = c;
		if (!c)
			return res;
		res++;
		count--;
	}

	/* Hit buffer length without finding a NUL; force NUL-termination. */
	if (res)
		dest[res-1] = '\0';

	return -E2BIG;
}
#endif

#ifdef NEED_STRCHRNUL
#include <linux/string.h>
static inline char *strchrnul(const char *s, int c)
{
        while (*s && *s != (char)c)
                s++;
        return (char *)s;
}
#endif

#ifdef NEED_FWNODE_GRAPH_FOR_EACH_ENDPOINT
#define fwnode_graph_for_each_endpoint(fwnode, child)			\
	for (child = NULL;						\
	     (child = fwnode_graph_get_next_endpoint(fwnode, child)); )
#endif

#ifdef NEED_LOCKDEP_ASSERT_IRQS
#define lockdep_assert_irqs_enabled() do { } while (0)
#define lockdep_assert_irqs_disabled() do { } while (0)
#endif


#ifdef NEED_OF_NODE_NAME_EQ
#include <linux/of.h>
#include <linux/string.h>
static inline
bool of_node_name_eq(const struct device_node *np, const char *name)
{
	const char *node_name;
	size_t len;

	if (!np)
		return false;

	node_name = kbasename(np->full_name);
	len = strchrnul(node_name, '@') - node_name;

	return (strlen(name) == len) && (strncmp(node_name, name, len) == 0);
}
#endif

#ifdef NEED_I2C_8BIT_ADDR_FROM_MSG
#include <linux/i2c.h>
static inline u8 i2c_8bit_addr_from_msg(const struct i2c_msg *msg)
{
	return (msg->addr << 1) | (msg->flags & I2C_M_RD ? 1 : 0);
}
#endif

#ifdef NEED_FOLL_LONGTERM
#define FOLL_LONGTERM 0
#endif

#ifdef NEED_STREAM_OPEN
#define stream_open nonseekable_open
#endif

#ifdef NEED_I2C_NEW_DUMMY_DEVICE
#define i2c_new_dummy_device(adap, addr) (i2c_new_dummy(adap, addr) ? : (struct i2c_client *)ERR_PTR(-ENODEV))
#endif

#ifdef NEED_I2C_NEW_ANCILLARY_DEVICE
#include <linux/i2c.h>

#ifdef NEED_I2C_NEW_SECONDARY_DEV
static inline struct i2c_client *i2c_new_secondary_device(struct i2c_client *client,
							  const char *name,
							  u16 default_addr)
{
	struct device_node *np = client->dev.of_node;
	u32 addr = default_addr;
	int i;

	if (np) {
		i = of_property_match_string(np, "reg-names", name);
		if (i >= 0)
			of_property_read_u32_index(np, "reg", i, &addr);
	}

	dev_dbg(&client->adapter->dev, "Address for %s : 0x%x\n", name, addr);
	return i2c_new_dummy(client->adapter, addr);
}
#endif

#define i2c_new_ancillary_device(client, name, addr) \
	(i2c_new_secondary_device(client, name, addr) ? : (struct i2c_client *)ERR_PTR(-ENODEV))
#endif

#ifdef NEED_I2C_NEW_CLIENT_DEVICE
#include <linux/i2c.h>
static inline struct i2c_client *
i2c_new_client_device(struct i2c_adapter *adap, struct i2c_board_info const *info)
{
	struct i2c_client *ret;

	ret = i2c_new_device(adap, info);
	return ret ? : ERR_PTR(-ENOMEM);
}
#endif

#ifdef NEED_I2C_NEW_SCANNED_DEVICE
#include <linux/i2c.h>
static inline struct i2c_client *
i2c_new_scanned_device(struct i2c_adapter *adap,
		       struct i2c_board_info *info,
		       unsigned short const *addr_list,
		       int (*probe)(struct i2c_adapter *adap, unsigned short addr))
{
	struct i2c_client *client;

	client = i2c_new_probed_device(adap, info, addr_list, probe);
	return client ? : ERR_PTR(-ENOMEM);
}
#endif

#ifdef NEED_I2C_CLIENT_HAS_DRIVER
#include <linux/i2c.h>
static inline bool i2c_client_has_driver(struct i2c_client *client)
{
	return !IS_ERR_OR_NULL(client) && client->dev.driver;
}
#endif

#ifdef NEED_UNTAGGED_ADDR
#define untagged_addr(addr) (addr)
#endif

#ifdef NEED_COMPAT_PTR_IOCTL
#ifdef CONFIG_COMPAT
#include <linux/compat.h>

static inline long compat_ptr_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
        if (!file->f_op->unlocked_ioctl)
                return -ENOIOCTLCMD;

        return file->f_op->unlocked_ioctl(file, cmd, (unsigned long)compat_ptr(arg));
}
#else
#define compat_ptr_ioctl NULL
#endif
#endif

#ifdef NEED_TIMESPEC64
#define timespec64 timespec
#define ns_to_timespec64 ns_to_timespec
#endif

#ifdef NEED_SIZEOF_FIELD
#define sizeof_field(TYPE, MEMBER) sizeof((((TYPE *)0)->MEMBER))
#endif

#ifdef NEED_DEVM_PLATFORM_IOREMAP_RESOURCE
#include <linux/platform_device.h>
static inline void __iomem *devm_platform_ioremap_resource(struct platform_device *pdev,
							   unsigned int index)
{
        struct resource *res;

        res = platform_get_resource(pdev, IORESOURCE_MEM, index);
        return devm_ioremap_resource(&pdev->dev, res);
}
#endif

#ifdef NEED_CPU_LATENCY_QOS
#define cpu_latency_qos_add_request(req, val) \
	pm_qos_add_request((req), PM_QOS_CPU_DMA_LATENCY, (val));

#define cpu_latency_qos_remove_request pm_qos_remove_request
#endif

#ifdef NEED_FWNODE_PROPERTY_PRESENT
#define fwnode_property_present(fwnode, propname) 0
#endif

#ifdef NEED_FWNODE_GRAPH_IS_ENDPOINT
#define fwnode_graph_is_endpoint(fwnode) fwnode_property_present((fwnode), "remote-endpoint")
#endif

#ifdef NEED_FALLTHROUGH
#ifdef __has_attribute
#if __has_attribute(__fallthrough__)
# define fallthrough                    __attribute__((__fallthrough__))
#else
# define fallthrough                    do {} while (0)  /* fallthrough */
#endif
#else
# define fallthrough                    do {} while (0)  /* fallthrough */
#endif
#endif

#ifdef NEED_SCHED_SET_FIFO
#include <linux/sched.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
#include <uapi/linux/sched/types.h>
#endif
static inline void sched_set_fifo(struct task_struct *p)
{
	struct sched_param sp = { .sched_priority = 50 };
	sched_setscheduler(p, SCHED_FIFO, &sp);
}
#endif

#ifdef NEED_DMA_MAP_SGTABLE
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)
#include <linux/dma-mapping.h>

#define for_each_sgtable_sg(sgt, sg, i)		\
	for_each_sg(sgt->sgl, sg, sgt->orig_nents, i)

#define for_each_sgtable_dma_sg(sgt, sg, i)	\
	for_each_sg(sgt->sgl, sg, sgt->nents, i)

#define for_each_sgtable_page(sgt, piter, pgoffset)	\
	for_each_sg_page(sgt->sgl, piter, sgt->orig_nents, pgoffset)

#define for_each_sgtable_dma_page(sgt, dma_iter, pgoffset)	\
	for_each_sg_dma_page(sgt->sgl, dma_iter, sgt->nents, pgoffset)

static inline int dma_map_sgtable(struct device *dev, struct sg_table *sgt,
		enum dma_data_direction dir, unsigned long attrs)
{
	int nents;

	nents = dma_map_sg_attrs(dev, sgt->sgl, sgt->orig_nents, dir, attrs);
	if (nents <= 0)
		return -EINVAL;
	sgt->nents = nents;
	return 0;
}

static inline void dma_unmap_sgtable(struct device *dev, struct sg_table *sgt,
		enum dma_data_direction dir, unsigned long attrs)
{
	dma_unmap_sg_attrs(dev, sgt->sgl, sgt->orig_nents, dir, attrs);
}

static inline void dma_sync_sgtable_for_cpu(struct device *dev,
		struct sg_table *sgt, enum dma_data_direction dir)
{
	dma_sync_sg_for_cpu(dev, sgt->sgl, sgt->orig_nents, dir);
}

static inline void dma_sync_sgtable_for_device(struct device *dev,
		struct sg_table *sgt, enum dma_data_direction dir)
{
	dma_sync_sg_for_device(dev, sgt->sgl, sgt->orig_nents, dir);
}
#endif
#endif

#ifdef NEED_IN_COMPAT_SYSCALL
#include <linux/compat.h>

static inline bool in_compat_syscall(void) { return is_compat_task(); }

#endif

#ifdef NEED_DEV_ERR_PROBE
static inline int dev_err_probe(const struct device *dev, int err, const char *fmt, ...)
{
        struct va_format vaf;
        va_list args;

        va_start(args, fmt);
        vaf.fmt = fmt;
        vaf.va = &args;

        if (err != -EPROBE_DEFER)
                dev_err(dev, "error %pe: %pV", ERR_PTR(err), &vaf);
        else
                dev_dbg(dev, "error %pe: %pV", ERR_PTR(err), &vaf);

        va_end(args);

        return err;
}
#endif

#ifdef NEED_PM_RUNTIME_RESUME_AND_GET
#include <linux/pm_runtime.h>
static inline int pm_runtime_resume_and_get(struct device *dev)
{
	int ret;

	ret = __pm_runtime_resume(dev, RPM_GET_PUT);
	if (ret < 0) {
		pm_runtime_put_noidle(dev);
		return ret;
	}

	return 0;
}
#endif

#ifdef NEED_VMA_LOOKUP
#include <linux/mm.h>
static inline
struct vm_area_struct *vma_lookup(struct mm_struct *mm, unsigned long addr)
{
	struct vm_area_struct *vma = find_vma(mm, addr);

	if (vma && addr < vma->vm_start)
		vma = NULL;

	return vma;
}
#endif

#ifdef NEED_HZ_PER_MHZ
#define HZ_PER_MHZ 1000000UL
#endif

#ifdef NEED_DMA_VMAP_NONCONTIGUOUS
#include <linux/dma-mapping.h>
static inline struct sg_table *dma_alloc_noncontiguous(struct device *dev,
		size_t size, enum dma_data_direction dir, gfp_t gfp,
		unsigned long attrs)
{
	return NULL;
}
static inline void dma_free_noncontiguous(struct device *dev, size_t size,
		struct sg_table *sgt, enum dma_data_direction dir)
{
}
static inline void *dma_vmap_noncontiguous(struct device *dev, size_t size,
		struct sg_table *sgt)
{
	return NULL;
}
static inline void dma_vunmap_noncontiguous(struct device *dev, void *vaddr)
{
}
static inline int dma_mmap_noncontiguous(struct device *dev,
		struct vm_area_struct *vma, size_t size, struct sg_table *sgt)
{
	return -EINVAL;
}
#endif

#ifdef NEED_SYSFS_EMIT
#define sysfs_emit(buf, args...) snprintf(buf, PAGE_SIZE, ##args)
#endif

#ifdef NEED_ETH_HW_ADDR_SET
#define eth_hw_addr_set(dev, addr) memcpy((dev)->dev_addr, addr, ETH_ALEN)
#endif

#ifdef NEED_MODULE_IMPORT_NS
#define MODULE_IMPORT_NS(s)
#endif

#ifdef NEED_ALIGN_DOWN
#define __V4L_COMPAT_ALIGN_KERNEL(x, a)		__V4L_COMPAT_ALIGN_KERNEL_MASK(x, (typeof(x))(a) - 1)
#define __V4L_COMPAT_ALIGN_KERNEL_MASK(x, mask)	(((x) + (mask)) & ~(mask))
#define ALIGN_DOWN(x, a)			__V4L_COMPAT_ALIGN_KERNEL((x) - ((a) - 1), (a))
#endif

#ifdef NEED_BITMAP_ZALLOC
#define bitmap_zalloc(n, f) kcalloc((n) / BITS_PER_LONG, sizeof(long), (f))
#define bitmap_free(b) kfree(b)
#endif

#ifdef NEED_FIRMWARE_REQUEST_NOWARN
#define firmware_request_nowarn request_firmware
#endif

#ifdef NEED_LOCKDEP_ASSERT_NOT_HELD
#define lockdep_assert_not_held(l)
#endif

#ifdef NEED_DEV_IS_PLATFORM
#define dev_is_platform(dev) ((dev)->bus == &platform_bus_type)
#endif

#endif /*  _COMPAT_H */
