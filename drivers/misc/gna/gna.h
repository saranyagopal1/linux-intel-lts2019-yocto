/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/* Copyright(c) 2017-2020 Intel Corporation */

#ifndef _UAPI_GNA_H_
#define _UAPI_GNA_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <linux/types.h>
#include <linux/ioctl.h>

#ifndef __user
#define __user
#endif

/* Request processing flags */

/* Operation modes */
#define GNA_MODE_GMM	0
#define GNA_MODE_XNN	1

#define GNA_PARAM_DEVICE_ID		1
#define GNA_PARAM_RECOVERY_TIMEOUT	2
#define GNA_PARAM_DEVICE_TYPE		3
#define GNA_PARAM_INPUT_BUFFER_S	4

#define GNA_STS_SCORE_COMPLETED		(1 <<  0)
#define GNA_STS_STATISTICS_VALID	(1 <<  3)
#define GNA_STS_PCI_MMU_ERR		(1 <<  4)
#define GNA_STS_PCI_DMA_ERR		(1 <<  5)
#define GNA_STS_PCI_UNEXCOMPL_ERR	(1 <<  6)
#define GNA_STS_PARAM_OOR		(1 <<  7)
#define GNA_STS_VA_OOR			(1 <<  8)
#define GNA_STS_OUTBUF_FULL		(1 << 16)
#define GNA_STS_SATURATE		(1 << 17)

#define GNA_ERROR (GNA_STS_PCI_DMA_ERR | \
		   GNA_STS_PCI_MMU_ERR | \
		   GNA_STS_PCI_UNEXCOMPL_ERR | \
		   GNA_STS_PARAM_OOR | \
		   GNA_STS_VA_OOR)

#define GNA_DEV_TYPE_0_9	0x09
#define GNA_DEV_TYPE_1_0	0x10
#define GNA_DEV_TYPE_2_0	0x20

/**
 * Structure describes part of memory to be overwritten before starting GNA
 */
struct gna_memory_patch {
	/* offset from targeted memory */
	__u64 offset;

	__u64 size;
	union {
		__u64 value;
		void __user *user_ptr;
	};
};

struct gna_buffer {
	__u64 memory_id;

	__u64 offset;
	__u64 size;

	__u64 patch_count;
	__u64 patches_ptr;
};

struct gna_drv_perf {
	__u64 pre_processing;	// driver starts pre-processing
	__u64 processing;	// hw starts processing
	__u64 hw_completed;	// hw finishes processing
	__u64 completion;	// driver finishes post-processing
};

struct gna_hw_perf {
	__u64 total;
	__u64 stall;
};

struct gna_compute_cfg {
	__u32 layer_base;
	__u32 layer_count;

	/* List of GNA memory buffers */
	__u64 buffers_ptr;
	__u64 buffer_count;

	__u8 active_list_on;
	__u8 gna_mode;
	__u8 hw_perf_encoding;
};

union gna_parameter {
	struct {
		__u64 id;
	} in;

	struct {
		__u64 value;
	} out;
};

union gna_memory_map {
	struct {
		__u64 address;
		__u32 size;
	} in;

	struct {
		__u64 memory_id;
	} out;
};

union gna_compute {
	struct {
		struct gna_compute_cfg config;
	} in;

	struct {
		__u64 request_id;
	} out;
};

union gna_wait {
	struct {
		__u64 request_id;
		__u32 timeout;
	} in;

	struct {
		__u32 hw_status;
		struct gna_drv_perf drv_perf;
		struct gna_hw_perf hw_perf;
	} out;
};

#define GNA_IOCTL_PARAM_GET	_IOWR('C', 0x01, union gna_parameter)
#define GNA_IOCTL_MEMORY_MAP	_IOWR('C', 0x02, union gna_memory_map)
#define GNA_IOCTL_MEMORY_UNMAP	_IOWR('C', 0x03, __u64)
#define GNA_IOCTL_COMPUTE	_IOWR('C', 0x04, union gna_compute)
#define GNA_IOCTL_WAIT		_IOWR('C', 0x05, union gna_wait)

#if defined(__cplusplus)
}
#endif

#endif /* _UAPI_GNA_H_ */
