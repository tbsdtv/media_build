/*
 * $Id: compat.h,v 1.44 2006/01/15 09:35:16 mchehab Exp $
 */

#ifndef _COMPAT_H
#define _COMPAT_H

#include <linux/version.h>
#include <linux/compiler.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/idr.h>
#include "config-compat.h"
#include "../linux/kernel_version.h"

#undef __devinitconst
#define __devinitconst

#ifndef uninitialized_var
#define uninitialized_var(x) x = x
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0)
#include <linux/sizes.h>
#endif

/* In v2.6.19-rc6-118-g52bad64 struct work_struct was was changed to be only for
 * non-delayed work and struct delayed_work was created for delayed work.  This
 * will rename the structures.  Hopefully no one will decide to name something
 * delayed_work in the same context as something named work_struct.  */
#ifdef NEED_DELAYED_WORK
#define delayed_work work_struct
#define INIT_DELAYED_WORK(a,b,c)	INIT_WORK(a,b,c)
#endif

#define EXPERIMENTAL_TREE

#ifdef NEED_I2C_SMBUS_WORD_SWAPPED
#include <linux/i2c.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 38)
static inline s32
i2c_smbus_read_word_swapped(struct i2c_client *client, u8 command)
{
	s32 value = i2c_smbus_read_word_data(client, command);

	return (value < 0) ? value : swab16(value);
}

static inline s32
i2c_smbus_write_word_swapped(struct i2c_client *client, u8 command, u16 value)
{
	return i2c_smbus_write_word_data(client, command, swab16(value));
}
#else
static inline s32
i2c_smbus_read_word_swapped(const struct i2c_client *client, u8 command)
{
	s32 value = i2c_smbus_read_word_data(client, command);

	return (value < 0) ? value : swab16(value);
}

static inline s32
i2c_smbus_write_word_swapped(const struct i2c_client *client,
			     u8 command, u16 value)
{
	return i2c_smbus_write_word_data(client, command, swab16(value));
}
#endif
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 39)
#include <asm/bitops.h>
static inline unsigned long find_next_zero_bit_le(const void *addr,
		unsigned long size, unsigned long offset)
{
	return find_next_zero_bit(addr, size, offset);
}

static inline unsigned long find_next_bit_le(const void *addr,
		unsigned long size, unsigned long offset)
{
	return find_next_bit(addr, size, offset);
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 34)
#define usb_alloc_coherent(dev, size, mem_flags, dma) usb_buffer_alloc(dev, size, mem_flags, dma)
#define usb_free_coherent(dev, size, addr, dma) usb_buffer_free(dev, size, addr, dma)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24)
#define	KERN_CONT	""
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 32)
#define ir_register_class(a) input_register_device(a)
#define ir_unregister_class(a) input_unregister_device(a)
#endif

/* To allow I2C compatibility code to work */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24)
#include <linux/i2c-dev.h>
#endif

#ifdef NEED_HEX_TO_BIN
#include <linux/ctype.h>
static inline int hex_to_bin(char ch)
{
	if ((ch >= '0') && (ch <= '9'))
		return ch - '0';
	ch = tolower(ch);
	if ((ch >= 'a') && (ch <= 'f'))
		return ch - 'a' + 10;
	return -1;
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 34)
#define	usb_enable_autosuspend(udev) udev->autosuspend_disabled = 0;
#endif

#ifndef FBIO_WAITFORVSYNC
#define FBIO_WAITFORVSYNC      _IOW('F', 0x20, __u32)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 30)
#include <linux/mm.h>
#include <asm/uaccess.h>
#include <linux/err.h>
static inline void *memdup_user(const void __user *src, size_t len)
{
	void *p;

	/*
	 * Always use GFP_KERNEL, since copy_from_user() can sleep and
	 * cause pagefault, which makes it pointless to use GFP_NOFS
	 * or GFP_ATOMIC.
	 */
	p = kmalloc_track_caller(len, GFP_KERNEL);
	if (!p)
		return ERR_PTR(-ENOMEM);

	if (copy_from_user(p, src, len)) {
		kfree(p);
		return ERR_PTR(-EFAULT);
	}

	return p;
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 29)
static inline int pci_msi_enabled(void)
{
#ifndef CONFIG_PCI_MSI
	return 0;
#else
	return 1;
#endif
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 26)
#ifdef CONFIG_PROC_FS
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/device.h>
#endif
#endif

#ifndef KEY_NUMERIC_0
#define KEY_NUMERIC_0           0x200   /* used by phones, remote controls, */
#define KEY_NUMERIC_1           0x201   /* and other keypads */
#define KEY_NUMERIC_2           0x202
#define KEY_NUMERIC_3           0x203
#define KEY_NUMERIC_4           0x204
#define KEY_NUMERIC_5           0x205
#define KEY_NUMERIC_6           0x206
#define KEY_NUMERIC_7           0x207
#define KEY_NUMERIC_8           0x208
#define KEY_NUMERIC_9           0x209
#define KEY_NUMERIC_STAR        0x20a
#define KEY_NUMERIC_POUND       0x20b
#endif

#ifndef KEY_IMAGES
#define KEY_IMAGES           0x1ba   /* AL Image Browser */
#endif

#ifdef NEED_DEFINE_PCI_DEVICE_TABLE
#define DEFINE_PCI_DEVICE_TABLE(_table) \
	const struct pci_device_id _table[]
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 34)
#define netdev_mc_count(dev) ((dev)->mc_count)
#define netdev_mc_empty(dev) (netdev_mc_count(dev) == 0)
#define netdev_for_each_mc_addr(mclist, dev) \
	for (mclist = dev->mc_list; mclist; mclist = mclist->next)
#endif

#ifndef KEY_CONTEXT_MENU
#define KEY_CONTEXT_MENU        0x1b6   /* GenDesc - system context menu */
#endif

#ifndef BIT_MASK
#define BIT_MASK(nr)            (1UL << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr)            ((nr) / BITS_PER_LONG)
#endif

#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,32) && 0
#include <linux/ctype.h>
#undef _P
static inline char *skip_spaces(const char *str)
{
	while (isspace(*str))
		++str;
	return (char *)str;
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,25)
#define valtype	unsigned long
static inline int strict_strtoul(const char *cp, unsigned int base, valtype *res)
{
	char *tail;
	valtype val;
	size_t len;

	*res = 0;
	len = strlen(cp);
	if (len == 0)
		return -EINVAL;

	val = simple_strtoul(cp, &tail, base);
	if ((*tail == '\0') ||
		((len == (size_t)(tail - cp) + 1) && (*tail == '\n'))) {
		*res = val;
		return 0;
	}

	return -EINVAL;
}
#endif

#ifndef KEY_DASHBOARD
#define KEY_DASHBOARD           204     /* AL Dashboard */
#endif
/* To allow alsa code to work */
#ifdef NEED_SOUND_DRIVER_H
#include <sound/driver.h>
#endif

#ifdef NEED_SND_CARD_CREATE
#include <sound/core.h>
#endif

#ifdef NEED_ALGO_CONTROL
#include <linux/i2c.h>
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)
#define time_is_after_eq_jiffies(a) (time_before_eq(jiffies, a))
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,23)
#define set_freezable()
#define cancel_delayed_work_sync cancel_rearming_delayed_work
#define DEFAULT_POLLMASK (POLLIN | POLLOUT | POLLRDNORM | POLLWRNORM)
#endif

#ifndef __pure
#  define __pure __attribute__((pure))
#endif

/* device_create/destroy added in 2.6.18 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18)
/* on older kernels, class_device_create will in turn be a compat macro */
# define device_create(a, b, c, d, e, f, g) class_device_create(a, NULL, c, b, d, e, f, g)
# define device_destroy(a, b) class_device_destroy(a, b)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18)
#define IRQF_SHARED		SA_SHIRQ
#define IRQF_DISABLED		SA_INTERRUPT
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#define PCIAGP_FAIL 0
#define vmalloc_32_user(a) vmalloc_32(a)
#endif

#ifndef DIV_ROUND_CLOSEST
#define DIV_ROUND_CLOSEST(x, divisor)(                  \
{                                                       \
	typeof(divisor) __divisor = divisor;            \
	(((x) + ((__divisor) / 2)) / (__divisor));      \
}                                                       \
)
#endif

#ifdef NEED_BOOL_TYPE
/* bool type and enum-based definition of true and false was added in 2.6.19 */
typedef int bool;
enum {
	false   = 0,
	true    = 1
};
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22)
#define sony_pic_camera_command(a,b) sonypi_camera_command(a,b)

#define SONY_PIC_COMMAND_SETCAMERAAGC        SONYPI_COMMAND_SETCAMERAAGC
#define SONY_PIC_COMMAND_SETCAMERABRIGHTNESS SONYPI_COMMAND_SETCAMERABRIGHTNESS
#define SONY_PIC_COMMAND_SETCAMERACOLOR      SONYPI_COMMAND_SETCAMERACOLOR
#define SONY_PIC_COMMAND_SETCAMERACONTRAST   SONYPI_COMMAND_SETCAMERACONTRAST
#define SONY_PIC_COMMAND_SETCAMERAHUE        SONYPI_COMMAND_SETCAMERAHUE
#define SONY_PIC_COMMAND_SETCAMERAPICTURE    SONYPI_COMMAND_SETCAMERAPICTURE
#define SONY_PIC_COMMAND_SETCAMERASHARPNESS  SONYPI_COMMAND_SETCAMERASHARPNESS
#define SONY_PIC_COMMAND_SETCAMERA           SONYPI_COMMAND_SETCAMERA
#endif

/* pci_dev got a new revision field in 2.6.23-rc1 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,23) && defined(LINUX_PCI_H)
/* Just make it easier to subsitute pci_dev->revision with
 * v4l_compat_pci_rev(pci_dev).  It's too bad there isn't some kind of context
 * sensitive macro in C that could do this for us.  */
static inline u8 v4l_compat_pci_rev(struct pci_dev *pci)
{ u8 rev; pci_read_config_byte(pci, PCI_REVISION_ID, &rev); return rev; }
#endif

#if defined(COMPAT_PCM_TO_RATE_BIT) && defined(__SOUND_PCM_H)
/* New alsa core utility function */
static inline unsigned int snd_pcm_rate_to_rate_bit(unsigned int rate)
{
	static const unsigned int rates[] = { 5512, 8000, 11025, 16000, 22050,
		32000, 44100, 48000, 64000, 88200, 96000, 176400, 192000 };
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(rates); i++)
		if (rates[i] == rate)
			return 1u << i;
	return SNDRV_PCM_RATE_KNOT;
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
# define task_pid_nr(current) ((current)->pid)

# define sg_init_table(a,b)
# define sg_page(p) (sg->page)
# define sg_set_page(sglist,pg,sz,off)					\
do {									\
	struct scatterlist *p=sglist;					\
	p->page   = pg;							\
	p->length = sz;							\
	p->offset = off;						\
} while (0)
#endif

#ifndef pr_err
#define pr_err(fmt, arg...) \
	printk(KERN_ERR fmt, ##arg)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,37)
#define noop_llseek NULL
#endif

#ifndef pr_warn
#define pr_warn(fmt, arg...) \
	printk(KERN_WARNING fmt, ##arg)
#endif

#ifndef BIT_MASK
# define BIT_MASK(nr)            (1UL << ((nr) % BITS_PER_LONG))
# define BIT_WORD(nr)            ((nr) / BITS_PER_LONG)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
#define i2c_verify_client(dev)	\
	((dev->bus == &i2c_bus_type) ? to_i2c_client(dev) : NULL)
#elif LINUX_VERSION_CODE < KERNEL_VERSION(2,6,25)
#define i2c_verify_client(dev) \
	((dev->bus && 0 == strcmp(dev->bus->name, "i2c")) ? to_i2c_client(dev) : NULL)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 31)
/* Construct an I2C_CLIENT_END-terminated array of i2c addresses */
#define I2C_ADDRS(addr, addrs...) \
	((const unsigned short []){ addr, ## addrs, I2C_CLIENT_END })
#endif

#ifndef USB_DEVICE_AND_INTERFACE_INFO
# define USB_DEVICE_AND_INTERFACE_INFO(vend,prod,cl,sc,pr) \
	.match_flags = USB_DEVICE_ID_MATCH_INT_INFO \
		| USB_DEVICE_ID_MATCH_DEVICE, \
	.idVendor = (vend), .idProduct = (prod), \
	.bInterfaceClass = (cl), \
	.bInterfaceSubClass = (sc), .bInterfaceProtocol = (pr)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 26)
#define get_unaligned_be16(a)					\
	be16_to_cpu(get_unaligned((unsigned short *)(a)))
#define put_unaligned_be16(r, a)				\
	put_unaligned(cpu_to_be16(r), ((unsigned short *)(a)))
#define get_unaligned_le16(a)					\
	le16_to_cpu(get_unaligned((unsigned short *)(a)))
#define put_unaligned_le16(r, a)				\
	put_unaligned(cpu_to_le16(r), ((unsigned short *)(a)))
#define get_unaligned_be32(a)					\
	be32_to_cpu(get_unaligned((u32 *)(a)))
#define put_unaligned_be32(r, a)				\
	put_unaligned(cpu_to_be32(r), ((u32 *)(a)))
#define get_unaligned_le32(a)					\
	le32_to_cpu(get_unaligned((u32 *)(a)))
#define put_unaligned_le32(r, a)				\
	put_unaligned(cpu_to_le32(r), ((u32 *)(a)))
#define get_unaligned_le64(a)					\
	le64_to_cpu(get_unaligned((u64 *)(a)))
#define put_unaligned_le64(r, a)				\
	put_unaligned(cpu_to_le64(r), ((u64 *)(a)))
#endif

#ifdef NEED_PROC_CREATE
#ifdef CONFIG_PROC_FS
static inline struct proc_dir_entry *proc_create(const char *a,
	mode_t b, struct proc_dir_entry *c, const struct file_operations *d)
{
	struct proc_dir_entry *e;

	e = create_proc_entry(a, b, c);
	if (e) {
		e->owner = THIS_MODULE;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 17)
		e->proc_fops = d;
#else
		e->proc_fops = (struct file_operations *)d;
#endif
	}
	return e;
}
#endif
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 26)
#ifdef CONFIG_PROC_FS
static inline struct proc_dir_entry *proc_create_data(const char *a,
	mode_t b, struct proc_dir_entry *c, const struct file_operations *d,
	void *f)
{
	struct proc_dir_entry *e;

	e = create_proc_entry(a, b, c);
	if (e) {
		e->owner = THIS_MODULE;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 17)
		e->proc_fops = d;
#else
		e->proc_fops = (struct file_operations *)d;
#endif
		e->data = f;
	}
	return e;
}
#endif
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 17)
#define hweight64(x)  generic_hweight64(x)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24)
typedef unsigned long uintptr_t;
#endif

#ifdef NEED_IS_SINGULAR
static inline int list_is_singular(const struct list_head *head)
{
	return !list_empty(head) && (head->next == head->prev);
}
#endif

#ifdef NEED_CLAMP
#define clamp( x, l, h )        max_t( __typeof__( x ),		\
				      ( l ),			\
				      min_t( __typeof__( x ),	\
					     ( h ),        	\
					     ( x ) ) )
#define clamp_val(val, min, max) ({		\
	typeof(val) __val = (val);		\
	typeof(val) __min = (min);		\
	typeof(val) __max = (max);		\
	__val = __val < __min ? __min : __val;	\
	__val > __max ? __max : __val; })
#endif

#ifdef NEED_ALGO_CONTROL
static inline int dummy_algo_control(struct i2c_adapter *adapter,
			     unsigned int cmd, unsigned long arg)
{
	return 0;
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 26)
#define div64_u64(a,b) div64_64(a,b)

#define dev_name(dev)	((dev)->bus_id)

#define dev_set_name(dev, fmt, arg...) ({	\
	snprintf((dev)->bus_id, sizeof((dev)->bus_id), fmt , ## arg); \
	0;					\
})
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 27)
#define current_uid() (current->uid)
#endif

#ifndef WARN
#define WARN(condition, format...) ({					\
	int __ret_warn_on = !!(condition);				\
	if (unlikely(__ret_warn_on))					\
		printk(KERN_WARNING format);				\
	unlikely(__ret_warn_on);					\
})
#endif

#ifdef NEED_SND_BUG_ON
#define snd_BUG_ON(cond)	WARN((cond), "BUG? (%s)\n", __stringify(cond))
#endif

#ifdef NEED_BITOPS
#define BIT(x)  (1UL<<((x)%BITS_PER_LONG))
#endif

#ifndef PCI_DEVICE_ID_MARVELL_88ALP01_CCIC
#define PCI_DEVICE_ID_MARVELL_88ALP01_CCIC     0x4102
#endif

#ifdef __LINUX_USB_H
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19)
/*
 * usb_endpoint_* functions
 *
 * Included in Linux 2.6.19
 * Backported to 2.6.18 in Red Hat Enterprise Linux 5.2
 */

#ifdef RHEL_RELEASE_CODE
#if RHEL_RELEASE_CODE >= RHEL_RELEASE_VERSION(5, 2)
#define RHEL_HAS_USB_ENDPOINT
#endif
#endif

#ifndef RHEL_HAS_USB_ENDPOINT
static inline int
usb_endpoint_dir_in(const struct usb_endpoint_descriptor *epd)
{
	return (epd->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN;
}

static inline int
usb_endpoint_xfer_int(const struct usb_endpoint_descriptor *epd)
{
	return (epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
		USB_ENDPOINT_XFER_INT;
}

static inline int
usb_endpoint_xfer_isoc(const struct usb_endpoint_descriptor *epd)
{
	return (epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
		USB_ENDPOINT_XFER_ISOC;
}

static inline int
usb_endpoint_xfer_bulk(const struct usb_endpoint_descriptor *epd)
{
	return (epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
		USB_ENDPOINT_XFER_BULK;
}

static inline int
usb_endpoint_is_int_in(const struct usb_endpoint_descriptor *epd)
{
	return usb_endpoint_xfer_int(epd) && usb_endpoint_dir_in(epd);
}

static inline int usb_endpoint_dir_out(
				const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_OUT);
}

static inline int usb_endpoint_is_bulk_in(
				const struct usb_endpoint_descriptor *epd)
{
	return (usb_endpoint_xfer_bulk(epd) && usb_endpoint_dir_in(epd));
}

static inline int usb_endpoint_is_bulk_out(
				const struct usb_endpoint_descriptor *epd)
{
	return (usb_endpoint_xfer_bulk(epd) && usb_endpoint_dir_out(epd));
}

static inline int usb_endpoint_is_int_out(
				const struct usb_endpoint_descriptor *epd)
{
	return (usb_endpoint_xfer_int(epd) && usb_endpoint_dir_out(epd));
}

static inline int usb_endpoint_is_isoc_in(
				const struct usb_endpoint_descriptor *epd)
{
	return (usb_endpoint_xfer_isoc(epd) && usb_endpoint_dir_in(epd));
}

static inline int usb_endpoint_is_isoc_out(
				const struct usb_endpoint_descriptor *epd)
{
	return (usb_endpoint_xfer_isoc(epd) && usb_endpoint_dir_out(epd));
}
#endif /* RHEL_HAS_USB_ENDPOINT */
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 21)
#ifndef RHEL_HAS_USB_ENDPOINT
static inline int usb_endpoint_xfer_control(
				const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
		USB_ENDPOINT_XFER_CONTROL);
}
#endif /* RHEL_HAS_USB_ENDPOINT */
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24)
static inline int usb_endpoint_num(const struct usb_endpoint_descriptor *epd)
{
	return epd->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
}
#endif

#ifdef NEED_USB_ENDPOINT_TYPE
static inline int usb_endpoint_type(const struct usb_endpoint_descriptor *epd)
{
	return epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
}
#endif
#endif /* __LINUX_USB_H */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 22)
/*
 * Linked list API
 */
#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)

/*
 * uninitialized_var() macro
 */
#define uninitialized_var(x) x = x
#endif

#ifdef NEED_NETDEV_PRIV
#define netdev_priv(dev)	((dev)->priv)
#endif

#ifdef NEED_PCI_IOREMAP_BAR
#define pci_ioremap_bar(pdev, bar) \
	 ioremap_nocache(pci_resource_start(pdev, bar),	\
			 pci_resource_len(pdev, bar))
#endif

#ifdef NEED_POLL_SCHEDULE
#define  poll_schedule(pwq, task)			\
	do {						\
		set_current_state(task);		\
		schedule();				\
		set_current_state(TASK_RUNNING);	\
	} while (0)
#endif

/* This macro was added in commit v2.6.23-5792-g34c6538 */
#ifndef DMA_BIT_MASK
#define DMA_BIT_MASK(n)	(((n) == 64) ? ~0ULL : ((1ULL<<(n))-1))
#endif

/* __fls() was added for generic code in 2.6.29, existed for just 64-bit arches
 * since 2.6.26 (v2.6.25-5228-g56a6b1e), and was x86-64 only before then.  We
 * only want this compat code when __fls doesn't exist, which 2.6.29 or later,
 * non x86-64, and non 64-bit that's 2.6.26 or later. */
#if !(LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29) || \
	defined(__x86_64__) || \
	(BITS_PER_LONG == 64 && LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 26)))
/* This define will prevent breakage if __fls was already defined. */
#undef __fls
#define __fls v4l_compat_fls
static inline unsigned long v4l_compat_fls(unsigned long x)
{
	return fls(x) - 1;
}
#endif

/*
 * Compatibility code for hrtimer API
 * This will make hrtimer usable for kernels 2.6.22 and later.
 * For earlier kernels, not all required functions are exported
 * so there's nothing we can do.
 */

#ifdef _LINUX_HRTIMER_H
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 25) && \
	LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 22)
/* Forward a hrtimer so it expires after the hrtimer's current now */
static inline unsigned long hrtimer_forward_now(struct hrtimer *timer,
						ktime_t interval)
{
	return hrtimer_forward(timer, timer->base->get_time(), interval);
}
#endif
#endif /* _LINUX_HRTIMER_H */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 22)
#define strncasecmp(a, b, c) strnicmp(a, b, c)
#define strcasecmp(a, b) strnicmp(a, b, sizeof(a))
#endif

#ifdef NEED_FW_CSR_STRING

#include <linux/firewire.h>
#include <linux/firewire-constants.h>

static inline int fw_csr_string(u32 *directory, int search_key, char *buf, size_t size)
{
	struct fw_csr_iterator ci;
	int last_key = 0, key, value;
	const u32 *block = NULL;
	unsigned int quadlets, i;

	fw_csr_iterator_init(&ci, directory);
	while (fw_csr_iterator_next(&ci, &key, &value)) {
		if (last_key == search_key &&
		    key == (CSR_DESCRIPTOR | CSR_LEAF)) {
			block = ci.p - 1 + value;
			break;
		}

		last_key = key;
	}

	if (!block)
		return -ENOENT;

	quadlets = min(block[0] >> 16, 256U);
	if (quadlets < 2)
		return -ENODATA;

	if (block[1] != 0 || block[2] != 0)
		/* unknown language/character set */
		return -ENODATA;

	block += 3;
	quadlets -= 2;
	for (i = 0; i < quadlets * 4 && i < size - 1; i++) {
		char c = block[i / 4] >> (24 - 8 * (i % 4));
		if (c == '\0')
			break;
		buf[i] = c;
	}
	buf[i] = '\0';

	return i;
}
#endif

#ifdef NEED_POLL_REQUESTED_EVENTS
#include <linux/poll.h>

static inline bool poll_does_not_wait(const poll_table *p)
{
	return p == NULL;
}

/* Note: this function was introduced in kernel 3.4. That version relies
   on changes in the core poll code in order to be able to get the right
   value. For older kernels we use this compatibility function which will
   not always get it right: if you pass multiple file descriptors to the
   select() call, and if one of the earlier fds found an event, then for any
   subsequent file descriptors the poll_table pointer is set to NULL and you
   loose the key information.

   You can't make this fool-proof other than by going to kernel 3.4.
 */
static inline unsigned long poll_requested_events(const poll_table *p)
{
	return p ? p->key : ~0UL;
}
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

#ifdef NEED_FLUSH_WORK_SYNC
#define flush_work_sync(dev)
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 32)

#ifdef NEED_AUTOSUSPEND_DELAY
#define pm_runtime_set_autosuspend_delay(dev, delay) {	\
	struct usb_device *udev = to_usb_device(dev);	\
	udev->autosuspend_delay = delay;		\
}
#elif LINUX_VERSION_CODE == KERNEL_VERSION(2, 6, 37)
/* usb.h includes this from 2.6.38 onwards */
#include <linux/pm_runtime.h>
#endif
#endif

#ifndef KEY_10CHANNELSUP
#define KEY_10CHANNELSUP        0x1b8   /* 10 channels up (10+) */
#define KEY_10CHANNELSDOWN      0x1b9   /* 10 channels down (10-) */
#endif

#ifdef NEED_SND_CTL_ENUM_INFO
#include <sound/asound.h>
static inline int snd_ctl_enum_info(struct snd_ctl_elem_info *info, unsigned int channels,
		      unsigned int items, const char *const names[])
{
	info->type = SNDRV_CTL_ELEM_TYPE_ENUMERATED;
	info->count = channels;
	info->value.enumerated.items = items;
	if (info->value.enumerated.item >= items)
		info->value.enumerated.item = items - 1;
	strlcpy(info->value.enumerated.name,
		names[info->value.enumerated.item],
		sizeof(info->value.enumerated.name));
	return 0;
}
#endif

#ifdef NEED_SYSFS_ATTR_INIT
#define sysfs_attr_init(a)
#endif

#ifdef NEED_USLEEP_RANGE
#define usleep_range(min, max) msleep(min/1000)
#endif

#ifdef NEED_IS_ERR_OR_NULL
#define IS_ERR_OR_NULL(ptr) (!(ptr) || IS_ERR_VALUE((unsigned long)(ptr)))
#endif

#ifndef pr_err_ratelimited
#if defined(DEBUG)
#define pr_err_ratelimited(fmt, ...)					\
	printk_ratelimited(KERN_ERR pr_fmt(fmt), ##__VA_ARGS__)
#else
#define pr_err_ratelimited(fmt, ...)
#endif
#endif /*pr_debug_ratelimited */

#ifdef NEED_DEV_DBG_RATELIMITED
#define dev_dbg_ratelimited dev_dbg
#define dev_err_ratelimited dev_err
#define dev_notice_ratelimited dev_notice
#endif

#ifdef NEED_MODULE_USB_DRIVER
#define module_usb_driver(drv)			\
static int __init usb_mod_init(void)		\
{						\
	return usb_register(&drv);		\
}						\
static void __exit usb_mod_exit(void)		\
{						\
	usb_deregister(&drv);			\
}						\
module_init(usb_mod_init);			\
module_exit(usb_mod_exit);
#endif /* module_usb_driver */

#ifdef NEED_MODULE_PLATFORM_DRIVER
#define module_platform_driver(drv)		\
static int __init plat_mod_init(void)		\
{						\
	return platform_driver_register(&drv);	\
}						\
static void __exit plat_mod_exit(void)		\
{						\
	platform_driver_unregister(&drv);	\
}						\
module_init(plat_mod_init);			\
module_exit(plat_mod_exit);
#endif /* module_platform_driver */

#ifdef NEED_DMA_TRANSFER_DIRECTION
#include <linux/dmaengine.h>
#define DMA_DEV_TO_MEM DMA_FROM_DEVICE
#define DMA_MEM_TO_DEV DMA_TO_DEVICE
#endif

#ifndef module_driver
#define module_driver(__driver, __register, __unregister) \
static int __init __driver##_init(void) \
{ \
	return __register(&(__driver)); \
} \
module_init(__driver##_init); \
static void __exit __driver##_exit(void) \
{ \
	__unregister(&(__driver)); \
} \
module_exit(__driver##_exit);
#endif

#ifndef module_i2c_driver
#define module_i2c_driver(__i2c_driver) \
       module_driver(__i2c_driver, i2c_add_driver, \
		       i2c_del_driver)
#endif

#ifdef NEED_KMALLOC_ARRAY
#include <linux/slab.h>
static inline void *kmalloc_array(size_t n, size_t size, gfp_t flags)
{
	if (size != 0 && n > ULONG_MAX / size)
		return NULL;
	return __kmalloc(n * size, flags);
}
#endif

#ifdef NEED_DMAENGINE_PREP_SLAVE_SG
#include <linux/dmaengine.h>
static inline struct dma_async_tx_descriptor *dmaengine_prep_slave_sg(
	struct dma_chan *chan, struct scatterlist *sgl,	unsigned int sg_len,
	int dir, unsigned long flags)
{
	return chan->device->device_prep_slave_sg(chan, sgl, sg_len,
						  dir, flags);
}
#endif

#ifdef NEED_SET_SYSTEM_SLEEP_PM_OPS
#ifdef CONFIG_PM_SLEEP
#define SET_SYSTEM_SLEEP_PM_OPS(suspend_fn, resume_fn) \
	.suspend = suspend_fn, \
	.resume = resume_fn, \
	.freeze = suspend_fn, \
	.thaw = resume_fn, \
	.poweroff = suspend_fn, \
	.restore = resume_fn,
#else
#define SET_SYSTEM_SLEEP_PM_OPS(suspend_fn, resume_fn)
#endif
#endif

#ifdef NEED_I2C_CLIENT_SCCB
#define I2C_CLIENT_SCCB 0x9000
#endif

#ifdef NEED_UNLOCK_I2C_XFER
#include <linux/i2c.h>

static inline int __i2c_transfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
{
 unsigned long orig_jiffies;
 int ret, try;

 /* Retry automatically on arbitration loss */
 orig_jiffies = jiffies;
 for (ret = 0, try = 0; try <= adap->retries; try++) {
	 ret = adap->algo->master_xfer(adap, msgs, num);
	 if (ret != -EAGAIN)
		 break;
	 if (time_after(jiffies, orig_jiffies + adap->timeout))
		 break;
 }

 return ret;
}
#endif

#ifdef NEED_KSTRTOU16
#include <linux/kernel.h>

static inline int kstrtou16(const char *s, unsigned int base, u16 *res)
{
	unsigned long val;
	int ret;

	ret = strict_strtoul(s, base, &val);
	*res = (u16)val;
	return ret;
}
#endif

#ifdef NEED_KSTRTOUL
#include <linux/kernel.h>

#define kstrtoul strict_strtoul

#endif

#ifdef NEED_MEMWEIGHT
static inline size_t memweight(const void *ptr, size_t bytes)
{
	size_t ret = 0;
	size_t longs;
	const unsigned char *bitmap = ptr;

	for (; bytes > 0 && ((unsigned long)bitmap) % sizeof(long);
			bytes--, bitmap++)
		ret += hweight8(*bitmap);

	longs = bytes / sizeof(long);
	if (longs) {
		BUG_ON(longs >= INT_MAX / BITS_PER_LONG);
		ret += bitmap_weight((unsigned long *)bitmap,
				longs * BITS_PER_LONG);
		bytes -= longs * sizeof(long);
		bitmap += longs * sizeof(long);
	}
	/*
	 * The reason that this last loop is distinct from the preceding
	 * bitmap_weight() call is to compute 1-bits in the last region smaller
	 * than sizeof(long) properly on big-endian systems.
	 */
	for (; bytes > 0; bytes--, bitmap++)
		ret += hweight8(*bitmap);

	return ret;
}
#endif

#ifdef NEED_USB_ENDPOINT_MAXP
#define usb_endpoint_maxp(epd) __le16_to_cpu((epd)->wMaxPacketSize)
#endif

#ifdef NEED_USB_ENDPOINT_MAXP_MULT
#define USB_EP_MAXP_MULT_SHIFT  11
#define USB_EP_MAXP_MULT_MASK   (3 << USB_EP_MAXP_MULT_SHIFT)
#define USB_EP_MAXP_MULT(m) \
	        (((m) & USB_EP_MAXP_MULT_MASK) >> USB_EP_MAXP_MULT_SHIFT)

#define usb_endpoint_maxp_mult(epd) (USB_EP_MAXP_MULT(usb_endpoint_maxp(epd)) + 1)
#endif

#ifdef NEED_PRINTK_RATELIMITED
#define printk_ratelimited printk
#endif

#ifndef module_pci_driver
#define module_pci_driver(__pci_driver) \
       module_driver(__pci_driver, pci_register_driver, \
		       pci_unregister_driver)
#endif

#ifdef NEED_LOCK_ADAPTER
static inline void i2c_lock_adapter(struct i2c_adapter *adapter)
{
	mutex_lock(&adapter->bus_lock);
}
static inline void i2c_unlock_adapter(struct i2c_adapter *adapter)
{
	mutex_unlock(&adapter->bus_lock);
}
#endif

#ifdef NEED_I2C_PROBE_FUNC_QUICK_READ
static inline int i2c_probe_func_quick_read(struct i2c_adapter *adap, unsigned short addr)
{
	return i2c_smbus_xfer(adap, addr, 0, I2C_SMBUS_READ, 0,
				      I2C_SMBUS_QUICK, NULL) >= 0;
}
#endif

#ifdef NEED_ABS64
#define abs64(x) ({				\
		s64 __x = (x);			\
		(__x < 0) ? -__x : __x;		\
	})
#endif

#ifndef module_pci_driver
#define module_pci_driver(__pci_driver) \
       module_driver(__pci_driver, pci_register_driver, \
		       pci_unregister_driver)
#endif

#ifdef NEED_DONTDUMP
#ifdef NEED_NODUMP
/*
 * If NODUMP doesn't exist, then this flag was actually called VM_ALWAYSDUMP
 * and we have to invert the meaning of the flag.
 * So NODUMP == !VM_ALWAYSDUMP == 0.
 */
#define VM_NODUMP (0)
#endif
#define VM_DONTDUMP VM_NODUMP
#endif

#ifdef NEED_IS_ENABLED
#define __LINUX_KCONFIG_H
#undef IS_ENABLED
#define __ARG_PLACEHOLDER_1 0,
#define config_enabled(cfg) _config_enabled(cfg)
#define _config_enabled(value) __config_enabled(__ARG_PLACEHOLDER_##value)
#define __config_enabled(arg1_or_junk) ___config_enabled(arg1_or_junk 1, 0)
#define ___config_enabled(__ignored, val, ...) val
#define IS_ENABLED(option) \
		(config_enabled(option) || config_enabled(option##_MODULE))
#endif

#ifdef NEED_IS_REACHABLE
#define IS_REACHABLE(option) (config_enabled(option) || \
		 (config_enabled(option##_MODULE) && config_enabled(MODULE)))
#endif

#ifdef NEED_USB_TRANSLATE_ERRORS
static inline int usb_translate_errors(int error_code)
{
	switch (error_code) {
	case 0:
	case -ENOMEM:
	case -ENODEV:
	case -EOPNOTSUPP:
		return error_code;
	default:
		return -EIO;
	}
}
#endif

#ifdef NEED_PTR_RET
static inline int __must_check PTR_RET(const void *ptr)
{
	if (IS_ERR(ptr))
		return PTR_ERR(ptr);
	else
		return 0;
}
#endif

#ifdef NEED_FILE_INODE
#include <linux/fs.h>
static inline struct inode *file_inode(struct file *f)
{
	return f->f_path.dentry->d_inode;
}
#endif

#ifdef NEED_ETH_P_802_3_MIN
#define ETH_P_802_3_MIN   0x0600
#endif

#ifdef NEED_PROC_SET_SIZE
#ifdef CONFIG_PROC_FS
#include <linux/proc_fs.h>
static inline void proc_set_size(struct proc_dir_entry *de, loff_t size)
{
	de->size = size;
}
static inline void *PDE_DATA(const struct inode *inode)
{
	return PDE(inode)->data;
}
#else
static inline void proc_set_size(struct proc_dir_entry *de, loff_t size) {}
#endif
#endif

#ifdef NEED_SIMPLE_DEV_PM_OPS
#define SIMPLE_DEV_PM_OPS(name, suspend_fn, resume_fn) \
	struct dev_pm_ops name = { \
			.suspend = suspend_fn, \
			.resume = resume_fn, \
			.freeze = suspend_fn, \
			.thaw = resume_fn, \
			.poweroff = suspend_fn, \
			.restore = resume_fn, \
	}
#endif

#ifdef NEED_VM_IOMAP_MEMORY
#include <linux/mm.h>
static inline int vm_iomap_memory(struct vm_area_struct *vma, phys_addr_t start, unsigned long len)
{
	unsigned long vm_len, pfn, pages;

	/* Check that the physical memory area passed in looks valid */
	if (start + len < start)
		return -EINVAL;
	/*
	 * You *really* shouldn't map things that aren't page-aligned,
	 * but we've historically allowed it because IO memory might
	 * just have smaller alignment.
	 */
	len += start & ~PAGE_MASK;
	pfn = start >> PAGE_SHIFT;
	pages = (len + ~PAGE_MASK) >> PAGE_SHIFT;
	if (pfn + pages < pfn)
		return -EINVAL;

	/* We start the mapping 'vm_pgoff' pages into the area */
	if (vma->vm_pgoff > pages)
		return -EINVAL;
	pfn += vma->vm_pgoff;
	pages -= vma->vm_pgoff;

	/* Can we fit all of the mapping? */
	vm_len = vma->vm_end - vma->vm_start;
	if (vm_len >> PAGE_SHIFT > pages)
		return -EINVAL;

	/* Ok, let it rip */
	return io_remap_pfn_range(vma, vma->vm_start, pfn, vm_len, vma->vm_page_prot);
}
#endif

#ifdef NEED_DEVICE_LOCK
#include <linux/device.h>
static inline void device_lock(struct device *dev)
{
	down(&dev->sem);
}

static inline int device_trylock(struct device *dev)
{
	return down_trylock(&dev->sem);
}

static inline void device_unlock(struct device *dev)
{
	up(&dev->sem);
}
#endif

#ifdef NEED_PTR_ERR_OR_ZERO
static inline int __must_check PTR_ERR_OR_ZERO(__force const void *ptr)
{
	if (IS_ERR(ptr))
		return PTR_ERR(ptr);
	else
		return 0;
}
#endif

#ifdef NEED_SG_ALLOC_TABLE_FROM_PAGES
#include <linux/scatterlist.h>
/**
 * sg_alloc_table_from_pages - Allocate and initialize an sg table from
 *			       an array of pages
 * @sgt:	The sg table header to use
 * @pages:	Pointer to an array of page pointers
 * @n_pages:	Number of pages in the pages array
 * @offset:     Offset from start of the first page to the start of a buffer
 * @size:       Number of valid bytes in the buffer (after offset)
 * @gfp_mask:	GFP allocation mask
 *
 *  Description:
 *    Allocate and initialize an sg table from a list of pages. Contiguous
 *    ranges of the pages are squashed into a single scatterlist node. A user
 *    may provide an offset at a start and a size of valid data in a buffer
 *    specified by the page array. The returned sg table is released by
 *    sg_free_table.
 *
 * Returns:
 *   0 on success, negative error on failure
 */
static inline int sg_alloc_table_from_pages(struct sg_table *sgt,
	struct page **pages, unsigned int n_pages,
	unsigned long offset, unsigned long size,
	gfp_t gfp_mask)
{
	unsigned int chunks;
	unsigned int i;
	unsigned int cur_page;
	int ret;
	struct scatterlist *s;

	/* compute number of contiguous chunks */
	chunks = 1;
	for (i = 1; i < n_pages; ++i)
		if (page_to_pfn(pages[i]) != page_to_pfn(pages[i - 1]) + 1)
			++chunks;

	ret = sg_alloc_table(sgt, chunks, gfp_mask);
	if (unlikely(ret))
		return ret;

	/* merging chunks and putting them into the scatterlist */
	cur_page = 0;
	for_each_sg(sgt->sgl, s, sgt->orig_nents, i) {
		unsigned long chunk_size;
		unsigned int j;

		/* look for the end of the current chunk */
		for (j = cur_page + 1; j < n_pages; ++j)
			if (page_to_pfn(pages[j]) !=
			    page_to_pfn(pages[j - 1]) + 1)
				break;

		chunk_size = ((j - cur_page) << PAGE_SHIFT) - offset;
		sg_set_page(s, pages[cur_page], min(size, chunk_size), offset);
		size -= chunk_size;
		offset = 0;
		cur_page = j;
	}

	return 0;
}
#endif

#ifdef NEED_REPLACE_FOPS
#define replace_fops(f, fops) \
	do {	\
		struct file *__file = (f); \
		fops_put(__file->f_op); \
		BUG_ON(!(__file->f_op = (fops))); \
	} while(0)
#endif

#ifdef NEED_REINIT_COMPLETION
#include <linux/completion.h>
static inline void reinit_completion(struct completion *x)
{
	x->done = 0;
}
#endif

#ifdef NEED_DMA_SET_MASK_AND_COHERENT
#include <linux/dma-mapping.h>

#ifdef NEED_DMA_SET_COHERENT_MASK
static inline int dma_set_coherent_mask(struct device *dev, u64 mask)
{
	if (!dma_supported(dev, mask))
		return -EIO;
	dev->coherent_dma_mask = mask;
	return 0;
}
#endif

static inline int dma_set_mask_and_coherent(struct device *dev, u64 mask)
{
	int rc = dma_set_mask(dev, mask);
	if (rc == 0)
		dma_set_coherent_mask(dev, mask);
	return rc;
}
#endif

#ifdef NEED_BITMAP_CLEAR

#include <linux/bitmap.h>

#define BITMAP_FIRST_WORD_MASK(start) (~0UL << ((start) % BITS_PER_LONG))

static inline void bitmap_set(unsigned long *map, int start, int nr)
{
	unsigned long *p = map + BIT_WORD(start);
	const int size = start + nr;
	int bits_to_set = BITS_PER_LONG - (start % BITS_PER_LONG);
	unsigned long mask_to_set = BITMAP_FIRST_WORD_MASK(start);

	while (nr - bits_to_set >= 0) {
		*p |= mask_to_set;
		nr -= bits_to_set;
		bits_to_set = BITS_PER_LONG;
		mask_to_set = ~0UL;
		p++;
	}
	if (nr) {
		mask_to_set &= BITMAP_LAST_WORD_MASK(size);
		*p |= mask_to_set;
	}
}

static inline void bitmap_clear(unsigned long *map, int start, int nr)
{
	unsigned long *p = map + BIT_WORD(start);
	const int size = start + nr;
	int bits_to_clear = BITS_PER_LONG - (start % BITS_PER_LONG);
	unsigned long mask_to_clear = BITMAP_FIRST_WORD_MASK(start);

	while (nr - bits_to_clear >= 0) {
		*p &= ~mask_to_clear;
		nr -= bits_to_clear;
		bits_to_clear = BITS_PER_LONG;
		mask_to_clear = ~0UL;
		p++;
	}
	if (nr) {
		mask_to_clear &= BITMAP_LAST_WORD_MASK(size);
		*p &= ~mask_to_clear;
	}
}
#endif

#ifdef NEED_DEVM_KMALLOC
#define devm_kmalloc devm_kzalloc
#endif

#ifdef NEED_USB_SPEED_STRING
#include <linux/usb.h>
/* undefine err & dbg since they clash with err/dbg defines from drivers. */
#undef err
#undef dbg
static inline const char *usb_speed_string(enum usb_device_speed speed)
{
	return "UNKNOWN";
}
#endif

#ifdef NEED_USB_SPEED_WIRELESS
#define USB_SPEED_WIRELESS USB_SPEED_VARIABLE
#endif

#ifdef NEED_ETHER_ADDR_EQUAL
static inline bool ether_addr_equal(const u8 *addr1, const u8 *addr2)
{
	const u16 *a = (const u16 *)addr1;
	const u16 *b = (const u16 *)addr2;

	return ((a[0] ^ b[0]) | (a[1] ^ b[1]) | (a[2] ^ b[2])) == 0;
}
#endif

#ifdef NEED_SND_CARD_NEW
#define snd_card_new(p, idx, id, module, extra_size, ret) \
	snd_card_create(idx, id, module, extra_size, ret)
#endif

#ifdef NEED_COMPAT_PUT_TIMESPEC
#define compat_put_timespec put_compat_timespec
#endif

#ifdef NEED_SMP_MB_AFTER_ATOMIC
#define smp_mb__after_atomic smp_mb__after_clear_bit
#endif

#ifdef NEED_DEVM_KMALLOC_ARRAY
static inline void *devm_kmalloc_array(struct device *dev,
				       size_t n, size_t size, gfp_t flags)
{
	if (size != 0 && n > (~(size_t)0) / size)
		return NULL;
	return devm_kmalloc(dev, n * size, flags);
}
#endif

#ifdef NEED_PCI_ZALLOC_CONSISTENT
#include <linux/pci.h>
#include <linux/dma-mapping.h>

static inline void *
pci_zalloc_consistent(struct pci_dev *hwdev, size_t size,
		dma_addr_t *dma_handle)
{
	return dma_alloc_coherent(hwdev == NULL ? NULL : &hwdev->dev,
			size, dma_handle, GFP_ATOMIC | __GFP_ZERO);
}
#endif

#ifdef NEED_KREF_GET_UNLESS_ZERO
#include <linux/kref.h>
static inline int __must_check kref_get_unless_zero(struct kref *kref)
{
	return atomic_add_unless(&kref->refcount, 1, 0);
}
#endif

#ifdef NEED_PRANDOM_U32_MAX
#include <linux/random.h>
#ifdef NEED_PRANDOM_U32
#define prandom_u32 random32
#endif

static inline u32 prandom_u32_max(u32 ep_ro)
{
	return (u32)(((u64) prandom_u32() * ep_ro) >> 32);
}
#endif

#ifdef NEED_GENMASK
#define GENMASK(h, l)           (((U32_C(1) << ((h) - (l) + 1)) - 1) << (l))
#endif

#ifdef NEED_MULT_FRAC
#define mult_frac(x, numer, denom)(			\
{							\
	typeof(x) quot = (x) / (denom);			\
	typeof(x) rem  = (x) % (denom);			\
	(quot * (numer)) + ((rem * (numer)) / (denom));	\
}							\
)
#endif

#ifdef NEED_CLK_HELPERS
#include <linux/clk.h>
static inline int clk_prepare_enable(struct clk *clk)
{
	int ret;

	ret = clk_prepare(clk);
	if (ret)
		return ret;
	ret = clk_enable(clk);
	if (ret)
		clk_unprepare(clk);

	return ret;
}

static inline void clk_disable_unprepare(struct clk *clk)
{
	clk_disable(clk);
	clk_unprepare(clk);
}
#endif

#ifdef NEED_PCM_STOP_XRUN
#include <sound/pcm.h>
static inline int snd_pcm_stop_xrun(struct snd_pcm_substream *substream)
{
	unsigned long flags;
	int ret = 0;

	snd_pcm_stream_lock_irqsave(substream, flags);
	if (snd_pcm_running(substream))
		ret = snd_pcm_stop(substream, SNDRV_PCM_STATE_XRUN);
	snd_pcm_stream_unlock_irqrestore(substream, flags);
	return ret;
}
#endif

#ifdef NEED_IS_MODULE
#define IS_MODULE(option)  defined(option ## _MODULE)
#endif

#ifdef NEED_DMA_ATTR_SKIP_CPU_SYNC
/*
 * Works fine for Intel architectures, but this can cause
 * problems on ARM.
 */
#define DMA_ATTR_SKIP_CPU_SYNC 0
#endif

#ifdef NEED_SIGN_EXTEND32
static inline __s32 sign_extend32(__u32 value, int index)
{
	__u8 shift = 31 - index;
	return (__s32)(value << shift) >> shift;
}
#endif

#ifdef NEED_NETDEV_DBG
#define netdev_dbg(__dev, format, args...)
#endif

#ifdef NEED_WRITEL_RELAXED
#define writel_relaxed writel
#endif

#ifdef NEED_GET_USER_PAGES_UNLOCKED
#include <linux/mm.h>
static inline long get_user_pages_unlocked(struct task_struct *tsk, struct mm_struct *mm,
			     unsigned long start, unsigned long nr_pages,
			     int write, int force, struct page **pages)
{
	long err;

	down_read(&mm->mmap_sem);
	err = get_user_pages(tsk, mm,
			start, nr_pages, write, force, pages, NULL);
	up_read(&mm->mmap_sem);
	return err;
}
#endif

#ifdef NEED_PR_WARN_ONCE
#define pr_warn_once pr_warn
#endif

#ifdef NEED_DEV_ERR_ONCE
#define dev_err_once dev_err
#endif

#ifdef NEED_DIV_ROUND_CLOSEST_ULL
#define DIV_ROUND_CLOSEST_ULL(x, divisor)(		\
{							\
	typeof(divisor) __d = divisor;			\
	unsigned long long _tmp = (x) + (__d) / 2;	\
	do_div(_tmp, __d);				\
	_tmp;						\
}							\
)
#endif

#ifdef NEED_PROP_READ_U64_ARRAY
struct device_node;
static inline int of_property_read_u64_array(const struct device_node *np,
					     const char *propname,
					     u64 *out_values, size_t sz)
{
	return -ENOSYS;
}
#endif

#ifdef NEED_MODULE_PNP_DRIVER
#define module_pnp_driver(__pnp_driver) \
	module_driver(__pnp_driver, pnp_register_driver, \
				    pnp_unregister_driver)
#endif

#include <linux/if_ether.h>
#ifdef NEED_ETH_ZERO_ADDR
static inline void eth_zero_addr(u8 *addr)
{
	memset(addr, 0x00, ETH_ALEN);
}
#endif

#ifdef NEED_KVFREE
#include <linux/mm.h>
#include <linux/vmalloc.h>
static inline void kvfree(const void *addr)
{
	if (is_vmalloc_addr(addr))
		vfree(addr);
	else
		kfree(addr);
}
#endif

#ifdef NEED_FRAME_VECTOR
#include <linux/mm.h>
#include <linux/err.h>
#include <linux/module.h>

/* Container for pinned pfns / pages */
struct frame_vector {
	unsigned int nr_allocated;	/* Number of frames we have space for */
	unsigned int nr_frames;	/* Number of frames stored in ptrs array */
	bool got_ref;		/* Did we pin pages by getting page ref? */
	bool is_pfns;		/* Does array contain pages or pfns? */
	void *ptrs[0];		/* Array of pinned pfns / pages. Use
				 * pfns_vector_pages() or pfns_vector_pfns()
				 * for access */
};

struct frame_vector *frame_vector_create(unsigned int nr_frames);
void frame_vector_destroy(struct frame_vector *vec);
int get_vaddr_frames(unsigned long start, unsigned int nr_pfns,
		     bool write, bool force, struct frame_vector *vec);
void put_vaddr_frames(struct frame_vector *vec);
int frame_vector_to_pages(struct frame_vector *vec);
void frame_vector_to_pfns(struct frame_vector *vec);

static inline unsigned int frame_vector_count(struct frame_vector *vec)
{
	return vec->nr_frames;
}

static inline struct page **frame_vector_pages(struct frame_vector *vec)
{
	if (vec->is_pfns) {
		int err = frame_vector_to_pages(vec);

		if (err)
			return ERR_PTR(err);
	}
	return (struct page **)(vec->ptrs);
}

static inline unsigned long *frame_vector_pfns(struct frame_vector *vec)
{
	if (!vec->is_pfns)
		frame_vector_to_pfns(vec);
	return (unsigned long *)(vec->ptrs);
}

#endif

#ifdef NEED_KTIME_COMPARE
static inline int ktime_compare(const ktime_t cmp1, const ktime_t cmp2)
{
	if (cmp1.tv64 < cmp2.tv64)
		return -1;
	if (cmp1.tv64 > cmp2.tv64)
		return 1;
	return 0;
}
#endif

#ifdef NEED_KTIME_BEFORE
static inline bool ktime_before(const ktime_t cmp1, const ktime_t cmp2)
{
	return ktime_compare(cmp1, cmp2) < 0;
}
#endif

#ifdef NEED_KTIME_GET_NS
static inline u64 ktime_get_ns(void)
{
        return ktime_to_ns(ktime_get());
}
#endif

#ifdef NEED_OF_NODE_FULL_NAME
#ifdef CONFIG_OF
static inline const char *of_node_full_name(const struct device_node *np)
{
	return np ? np->full_name : "<no-node>";
}
#else
static inline const char* of_node_full_name(const struct device_node *np)
{
	return "<no-node>";
}
#endif
#endif

#ifdef NEED_DIV64_U64_REM
#include <asm/div64.h>

#if BITS_PER_LONG == 64
static inline u64 div64_u64_rem(u64 dividend, u64 divisor, u64 *remainder)
{
	*remainder = dividend % divisor;
	return dividend / divisor;
}
#elif BITS_PER_LONG == 32
static inline u64 div64_u64_rem(u64 dividend, u64 divisor, u64 *remainder)
{
	u32 high = divisor >> 32;
	u64 quot;

	if (high == 0) {
		u32 rem32;
		quot = div_u64_rem(dividend, divisor, &rem32);
		*remainder = rem32;
	} else {
		int n = 1 + fls(high);
		quot = div_u64(dividend >> n, divisor >> n);

		if (quot != 0)
			quot--;

		*remainder = dividend - quot * divisor;
		if (*remainder >= divisor) {
			quot++;
			*remainder -= divisor;
		}
	}

	return quot;
}
#endif
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

#ifdef NEED_GENMASK_ULL
#define BITS_PER_LONG_LONG 64
#define GENMASK_ULL(h, l) \
	(((~0ULL) << (l)) & (~0ULL >> (BITS_PER_LONG_LONG - 1 - (h))))
#endif

#ifdef NEED_IDA_SIMPLE_REMOVE
#define ida_simple_remove ida_remove
#endif

#ifdef NEED_KTIME_GET_BOOTTIME
#define ktime_get_boottime ktime_get
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

#ifdef NEED_KTIME_MS_DELTA
static inline s64 ktime_ms_delta(const ktime_t later, const ktime_t earlier)
{
	return ktime_to_ms(ktime_sub(later, earlier));
}
#endif

#ifdef NEED_SMP_LOAD_ACQUIRE
#define smp_load_acquire(p) (*(p))
#define smp_store_release(p, a) (*p) = (a)
#endif

#define SERIO_PULSE8_CEC     0x40

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

#endif /*  _COMPAT_H */
