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

#ifdef NEED_KVCALLOC
#include <linux/mm.h>
static inline void *kvcalloc(size_t n, size_t size, gfp_t flags)
{
	return kvmalloc_array(n, size, flags | __GFP_ZERO);
}
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

#ifndef __GFP_RETRY_MAYFAIL
#define __GFP_RETRY_MAYFAIL __GFP_REPEAT
#endif

#ifdef NEED_PROP_COUNT
#include <linux/property.h>
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

#ifdef NEED_STACK_FRAME_NON_STANDARD
/* be sure STACK_FRAME_NON_STANDARD is defined */
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
#include <linux/frame.h>
#else
#include <linux/objtool.h>
#endif
#endif

#ifdef NEED_VM_FAULT_T
typedef int vm_fault_t;
#endif

#ifdef NEED_ARRAY_INDEX_NOSPEC
/* Some older Kernels got a backport, but we removed the include of
 * "linux/nospec.h" with patch "v4.13_remove_nospec_h.patch". Thus
 * including it again.
 */
#include <linux/nospec.h>
#endif

#ifdef NEED_STRUCT_SIZE
#include <linux/overflow.h>
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
