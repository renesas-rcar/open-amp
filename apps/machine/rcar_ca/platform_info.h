/*
 * Copyright (c) 2016 Xilinx, Inc. All rights reserved.
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_INFO_H_
#define PLATFORM_INFO_H_

#include <openamp/remoteproc.h>
#include <openamp/virtio.h>
#include <openamp/rpmsg.h>

#include "platform_info_common.h"

#if defined __cplusplus
extern "C" {
#endif

struct remoteproc_priv {
	const char *ipi_name; /**< IPI device name */
	const char *ipi_bus_name; /**< IPI bus name */
	const char *rsc_name; /**< rsc device name */
	const char *rsc_bus_name; /**< rsc bus name */
	const char *shm_name; /**< shared memory device name */
	const char *shm_bus_name; /**< shared memory bus name */
	metal_phys_addr_t rsc_mem_pa; /**< rsc table physical address */
	size_t rsc_mem_size; /**< Size of the rsc table */
	metal_phys_addr_t vring_mem_pa; /**< vring physical address */
	size_t vring_mem_offset; /**< Offset of each vring */
	metal_phys_addr_t shared_buf_pa; /**< Shared buffer physical address */
	size_t shared_buf_size; /**< Size of the shared buffer */
	struct metal_device *ipi_dev; /**< pointer to IPI device */
	struct metal_io_region *ipi_io; /**< pointer to IPI i/o region */
	struct metal_device *shm_dev; /**< pointer to shared memory device */
	struct metal_io_region *shm_io; /**< pointer to sh mem i/o region */

	struct remoteproc_mem shm_mem; /**< shared memory */
	unsigned int ipi_chn_mask; /**< IPI channel mask */
	atomic_int ipi_nokick;
};

#if defined __cplusplus
}
#endif

#endif /* PLATFORM_INFO_H_ */

