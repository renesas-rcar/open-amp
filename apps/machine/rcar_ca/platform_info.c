/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 * Copyright (c) 2017 Xilinx, Inc.
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <metal/alloc.h>
#include <metal/atomic.h>
#include <metal/io.h>
#include <metal/irq.h>
#include <metal/device.h>
#include <metal/utilities.h>
#include <openamp/remoteproc.h>
#include <openamp/rpmsg_virtio.h>
#include <openamp/rsc_table_parser.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/un.h>
#include "platform_info.h"

#define UCIE_D2D_BASE0		0x20000000000ULL
#define UCIE_D2D_BASE1		0x24000000000ULL

#ifdef VDK_ENV
struct remoteproc_priv rproc_priv[] = {
	{
		.shm_name		= "81000000.rpmsg_shm",
		.shm_bus_name		= "platform",
		.ipi_name		= "18800000.rpmsg_ipi",
		.ipi_bus_name		= "platform",
		.rsc_mem_pa		= 0x81000000UL,
		.rsc_mem_size		= 0x1000UL,
		.vring_mem_pa		= 0x81001000UL,
		.vring_mem_offset	= 0x4000UL,
		.shared_buf_pa		= 0x81009000UL,
		.shared_buf_size	= 0x40000UL,
	}, {
		.shm_name		= "81050000.rpmsg_shm",
		.shm_bus_name		= "platform",
		.ipi_name		= "18801000.rpmsg_ipi",
		.ipi_bus_name		= "platform",
		.rsc_mem_pa		= 0x81050000UL,
		.rsc_mem_size		= 0x1000UL,
		.vring_mem_pa		= 0x81051000UL,
		.vring_mem_offset	= 0x4000UL,
		.shared_buf_pa		= 0x81059000UL,
		.shared_buf_size	= 0x40000UL,
	}, {
		.shm_name		= "810a0000.rpmsg_shm",
		.shm_bus_name		= "platform",
		.ipi_name		= "18802000.rpmsg_ipi",
		.ipi_bus_name		= "platform",
		.rsc_mem_pa		= 0x810a0000UL,
		.rsc_mem_size		= 0x1000UL,
		.vring_mem_pa		= 0x810a1000UL,
		.vring_mem_offset	= 0x4000UL,
		.shared_buf_pa		= 0x810a9000UL,
		.shared_buf_size	= 0x40000UL,
	}, {
		/* Terminator */
	}
};
#elif defined(RFS2X_ENV)	/* RFS2X_ENV */
struct remoteproc_priv rproc_priv[] = {
	{
		.shm_name		= "96600000.rpmsg_shm",
		.shm_bus_name		= "platform",
		.ipi_name		= "18800000.rpmsg_ipi",
		.ipi_bus_name		= "platform",
		.rsc_mem_pa		= 0x96600000UL,
		.rsc_mem_size		= 0x1000UL,
		.vring_mem_pa		= 0x96601000UL,
		.vring_mem_offset	= 0x4000UL,
		.shared_buf_pa		= 0x96609000UL,
		.shared_buf_size	= 0x40000UL,
	}, {
		.shm_name		= "96650000.rpmsg_shm",
		.shm_bus_name		= "platform",
		.ipi_name		= "18801000.rpmsg_ipi",
		.ipi_bus_name		= "platform",
		.rsc_mem_pa		= 0x96650000UL,
		.rsc_mem_size		= 0x1000UL,
		.vring_mem_pa		= 0x96651000UL,
		.vring_mem_offset	= 0x4000UL,
		.shared_buf_pa		= 0x96659000UL,
		.shared_buf_size	= 0x40000UL,
	}, {
		.shm_name		= "966a0000.rpmsg_shm",
		.shm_bus_name		= "platform",
		.ipi_name		= "18802000.rpmsg_ipi",
		.ipi_bus_name		= "platform",
		.rsc_mem_pa		= 0x966a0000UL,
		.rsc_mem_size		= 0x1000UL,
		.vring_mem_pa		= 0x966a1000UL,
		.vring_mem_offset	= 0x4000UL,
		.shared_buf_pa		= 0x966a9000UL,
		.shared_buf_size	= 0x40000UL,
	}, {
		.shm_name		= "90000000.rpmsg_shm",
		.shm_bus_name		= "platform",
		.rsc_mem_pa		= 0x90000000UL,
		.rsc_mem_size		= 0x1000UL,
		.vring_mem_pa		= 0x90001000UL,
		.vring_mem_offset	= 0x4000UL,
		.shared_buf_pa		= 0x90010000UL,
		.shared_buf_size	= 0x40000UL,
	}, {
		.shm_name		= "90050000.rpmsg_shm",
		.shm_bus_name		= "platform",
		.rsc_mem_pa		= 0x90050000UL,
		.rsc_mem_size		= 0x1000UL,
		.vring_mem_pa		= 0x90051000UL,
		.vring_mem_offset	= 0x4000UL,
		.shared_buf_pa		= 0x90060000UL,
		.shared_buf_size	= 0x40000UL,
	}, {
		.shm_name		= "90200000.rpmsg_shm",
		.shm_bus_name		= "platform",
		.rsc_mem_pa		= 0x90200000UL,
		.rsc_mem_size		= 0x1000UL,
		.vring_mem_pa		= 0x90201000UL,
		.vring_mem_offset	= 0x4000UL,
		.shared_buf_pa		= 0x90210000UL,
		.shared_buf_size	= 0x40000UL,
	}, {
		.shm_name		= "90250000.rpmsg_shm",
		.shm_bus_name		= "platform",
		.rsc_mem_pa		= 0x90250000UL,
		.rsc_mem_size		= 0x1000UL,
		.vring_mem_pa		= 0x90251000UL,
		.vring_mem_offset	= 0x4000UL,
		.shared_buf_pa		= 0x90260000UL,
		.shared_buf_size	= 0x40000UL,
	}, {
		/* Terminator */
	}
};
#else	/* HIL_ENV */
struct remoteproc_priv rproc_priv[] = {
	{
		.shm_name		= "96600000.rpmsg_shm",
		.shm_bus_name		= "platform",
		.ipi_name		= "18800000.rpmsg_ipi",
		.ipi_bus_name		= "platform",
		.rsc_mem_pa		= 0x96600000UL,
		.rsc_mem_size		= 0x1000UL,
		.vring_mem_pa		= 0x96601000UL,
		.vring_mem_offset	= 0x4000UL,
		.shared_buf_pa		= 0x96609000UL,
		.shared_buf_size	= 0x40000UL,
	}, {
		.shm_name		= "96650000.rpmsg_shm",
		.shm_bus_name		= "platform",
		.ipi_name		= "18801000.rpmsg_ipi",
		.ipi_bus_name		= "platform",
		.rsc_mem_pa		= 0x96650000UL,
		.rsc_mem_size		= 0x1000UL,
		.vring_mem_pa		= 0x96651000UL,
		.vring_mem_offset	= 0x4000UL,
		.shared_buf_pa		= 0x96659000UL,
		.shared_buf_size	= 0x40000UL,
	}, {
		.shm_name		= "966a0000.rpmsg_shm",
		.shm_bus_name		= "platform",
		.ipi_name		= "18802000.rpmsg_ipi",
		.ipi_bus_name		= "platform",
		.rsc_mem_pa		= 0x966a0000UL,
		.rsc_mem_size		= 0x1000UL,
		.vring_mem_pa		= 0x966a1000UL,
		.vring_mem_offset	= 0x4000UL,
		.shared_buf_pa		= 0x966a9000UL,
		.shared_buf_size	= 0x40000UL,
	}, {
		.shm_name		= "2008e600000.rpmsg_shm",
		.shm_bus_name		= "platform",
		.rsc_mem_pa		= UCIE_D2D_BASE0 + 0x8e600000UL,
		.rsc_mem_size		= 0x1000UL,
		.vring_mem_pa		= UCIE_D2D_BASE0 + 0x8e601000UL,
		.vring_mem_offset	= 0x4000UL,
		.shared_buf_pa		= UCIE_D2D_BASE0 + 0x8e609000UL,
		.shared_buf_size	= 0x40000UL,
	}, {
		.shm_name		= "2008e650000.rpmsg_shm",
		.shm_bus_name		= "platform",
		.rsc_mem_pa		= UCIE_D2D_BASE0 + 0x8e650000UL,
		.rsc_mem_size		= 0x1000UL,
		.vring_mem_pa		= UCIE_D2D_BASE0 + 0x8e651000UL,
		.vring_mem_offset	= 0x4000UL,
		.shared_buf_pa		= UCIE_D2D_BASE0 + 0x8e659000UL,
		.shared_buf_size	= 0x40000UL,
	}, {
		.shm_name		= "2008e6a0000.rpmsg_shm",
		.shm_bus_name		= "platform",
		.rsc_mem_pa		= UCIE_D2D_BASE0 + 0x8e6a0000UL,
		.rsc_mem_size		= 0x1000UL,
		.vring_mem_pa		= UCIE_D2D_BASE0 + 0x8e6a1000UL,
		.vring_mem_offset	= 0x4000UL,
		.shared_buf_pa		= UCIE_D2D_BASE0 + 0x8e6a9000UL,
		.shared_buf_size	= 0x40000UL,
	}, {
		.shm_name		= "2408e600000.rpmsg_shm",
		.shm_bus_name		= "platform",
		.rsc_mem_pa		= UCIE_D2D_BASE1 + 0x8e600000UL,
		.rsc_mem_size		= 0x1000UL,
		.vring_mem_pa		= UCIE_D2D_BASE1 + 0x8e601000UL,
		.vring_mem_offset	= 0x4000UL,
		.shared_buf_pa		= UCIE_D2D_BASE1 + 0x8e609000UL,
		.shared_buf_size	= 0x40000UL,
	}, {
		.shm_name		= "2408e650000.rpmsg_shm",
		.shm_bus_name		= "platform",
		.rsc_mem_pa		= UCIE_D2D_BASE1 + 0x8e650000UL,
		.rsc_mem_size		= 0x1000UL,
		.vring_mem_pa		= UCIE_D2D_BASE1 + 0x8e651000UL,
		.vring_mem_offset	= 0x4000UL,
		.shared_buf_pa		= UCIE_D2D_BASE1 + 0x8e659000UL,
		.shared_buf_size	= 0x40000UL,
	}, {
		.shm_name		= "2408e6a0000.rpmsg_shm",
		.shm_bus_name		= "platform",
		.rsc_mem_pa		= UCIE_D2D_BASE1 + 0x8e6a0000UL,
		.rsc_mem_size		= 0x1000UL,
		.vring_mem_pa		= UCIE_D2D_BASE1 + 0x8e6a1000UL,
		.vring_mem_offset	= 0x4000UL,
		.shared_buf_pa		= UCIE_D2D_BASE1 + 0x8e6a9000UL,
		.shared_buf_size	= 0x40000UL,
	}, {
		/* Terminator */
	}
};
#endif	/* VDK_ENV */

static struct remoteproc rproc_inst;

/* External functions */
extern int init_system(void);
extern void cleanup_system(void);

#define _rproc_wait() metal_cpu_yield()

extern const struct remoteproc_ops rcar_ca_linux_proc_ops;

/* RPMsg virtio shared buffer pool */
static struct rpmsg_virtio_shm_pool shpool;

static struct remoteproc *
platform_create_proc(int proc_index, int rsc_index)
{
	void *rsc_table;
	int rsc_size;
	int ret;
	metal_phys_addr_t pa;
	struct remoteproc_priv *priv = &rproc_priv[proc_index];

	(void)rsc_index;
	rsc_size = priv->rsc_mem_size;

	/* Initialize remoteproc instance */
	if (!remoteproc_init(&rproc_inst, &rcar_ca_linux_proc_ops, priv))
		return NULL;
	printf("Successfully initialized remoteproc\r\n");

	/* Mmap resource table */
	pa = priv->rsc_mem_pa;
	printf("Calling mmap resource table.\r\n");
	rsc_table = remoteproc_mmap(&rproc_inst, &pa, NULL, rsc_size,
				    0, NULL);
	if (!rsc_table) {
		fprintf(stderr, "ERROR: Failed to mmap resource table.\r\n");
		return NULL;
	}
	printf("Successfully mmap resource table.\r\n");

	/* parse resource table to remoteproc */
	ret = remoteproc_set_rsc_table(&rproc_inst, rsc_table, rsc_size);
	if (ret) {
		printf("Failed to initialize remoteproc\r\n");
		remoteproc_remove(&rproc_inst);
		return NULL;
	}
	printf("Successfully set resource table to remoteproc.\r\n");

	return &rproc_inst;
}

int platform_init(int argc, char *argv[], void **platform)
{
	unsigned long proc_id = 0;
	unsigned long rsc_id = 0;
	struct remoteproc *rproc;

	if (!platform) {
		fprintf(stderr, "Failed to initialize platform, NULL pointer"
			" to store platform data.\r\n");
		return -EINVAL;
	}
	/* Initialize HW system components */
	init_system();

	if (argc >= 2) {
		unsigned long num_proc = sizeof(rproc_priv) / sizeof(rproc_priv[0]) - 1;

		proc_id = strtoul(argv[1], NULL, 0);
		if (proc_id >= num_proc) {
			fprintf(stderr, "Failed to initialize platform,"
				" the rproc ID is not supported.\r\n");
			return -EINVAL;
		}
	}

	if (argc >= 3) {
		rsc_id = strtoul(argv[2], NULL, 0);
	}

	rproc = platform_create_proc(proc_id, rsc_id);
	if (!rproc) {
		fprintf(stderr, "Failed to create remoteproc device.\r\n");
		return -EINVAL;
	}
	*platform = rproc;
	return 0;
}

void platform_update_vring_addr(struct remoteproc *rproc, unsigned int vdev_id, int role)
{
	char *rsc_table = rproc->rsc_table;
	struct fw_rsc_vdev *vdev_rsc;
	size_t vdev_rsc_offset;
	unsigned int num_vrings, i;

	if (role == VIRTIO_DEV_DEVICE)
		return;

	metal_assert(rproc);
	metal_mutex_acquire(&rproc->lock);

	vdev_rsc_offset = find_rsc(rsc_table, RSC_VDEV, vdev_id);
	if (!vdev_rsc_offset)
		goto err;

	vdev_rsc = (struct fw_rsc_vdev *)(rsc_table + vdev_rsc_offset);
	num_vrings = vdev_rsc->num_of_vrings;

	for (i = 0; i < num_vrings; i++) {
		struct fw_rsc_vdev_vring *vring_rsc;
		struct remoteproc_priv *priv = rproc->priv;

		vring_rsc = &vdev_rsc->vring[i];

		if (vring_rsc->da == FW_RSC_U32_ADDR_ANY)
			vring_rsc->da = priv->vring_mem_pa + i * priv->vring_mem_offset;
	}

err:
	metal_mutex_release(&rproc->lock);
}

struct  rpmsg_device *
platform_create_rpmsg_vdev(void *platform, unsigned int vdev_index,
			   unsigned int role,
			   void (*rst_cb)(struct virtio_device *vdev),
			   rpmsg_ns_bind_cb ns_bind_cb)
{
	struct remoteproc *rproc = platform;
	struct remoteproc_priv *priv = rproc->priv;
	struct rpmsg_virtio_device *rpmsg_vdev;
	struct virtio_device *vdev;
	void *shbuf;
	struct metal_io_region *shbuf_io;
	int ret;

	rpmsg_vdev = metal_allocate_memory(sizeof(*rpmsg_vdev));
	if (!rpmsg_vdev)
		return NULL;
	shbuf_io = remoteproc_get_io_with_pa(rproc, priv->shared_buf_pa);
	if (!shbuf_io)
		goto err1;
	shbuf = metal_io_phys_to_virt(shbuf_io, priv->shared_buf_pa);

	platform_update_vring_addr(rproc, vdev_index, role);

	printf("Creating virtio...\r\n");
	/* TODO: can we have a wrapper for the following two functions? */
	vdev = remoteproc_create_virtio(rproc, vdev_index, role, rst_cb);
	if (!vdev) {
		printf("failed remoteproc_create_virtio\r\n");
		goto err1;
	}
	printf("Successfully created virtio device.\r\n");

	/* Only RPMsg virtio driver needs to initialize the shared buffers pool */
	rpmsg_virtio_init_shm_pool(&shpool, shbuf, priv->shared_buf_size);

	printf("initializing rpmsg vdev\r\n");
	/* RPMsg virtio device can set shared buffers pool argument to NULL */
	ret = rpmsg_init_vdev(rpmsg_vdev, vdev, ns_bind_cb,
			      shbuf_io, &shpool);
	if (ret) {
		printf("failed rpmsg_init_vdev\r\n");
		goto err2;
	}
	return rpmsg_virtio_get_rpmsg_device(rpmsg_vdev);
err2:
	remoteproc_remove_virtio(rproc, vdev);
err1:
	metal_free_memory(rpmsg_vdev);
	return NULL;
}

int platform_poll(void *priv)
{
	struct remoteproc *rproc = priv;
	struct remoteproc_priv *prproc;
	unsigned int flags;
	int ret;

	prproc = rproc->priv;
	while (1) {
		if (prproc->use_mem_kick) {
			if (rcar_ca_linux_proc_mem_check(prproc)) {
				ret = remoteproc_get_notification(rproc,
								  RSC_NOTIFY_ID_ANY);
				if (ret)
					return ret;
				break;
			}
			_rproc_wait();
			continue;
		}

		flags = metal_irq_save_disable();
		if (!(atomic_flag_test_and_set(&prproc->ipi_nokick))) {
			metal_irq_restore_enable(flags);
			ret = remoteproc_get_notification(rproc,
							  RSC_NOTIFY_ID_ANY);
			if (ret)
				return ret;
			break;
		}
		_rproc_wait();
		metal_irq_restore_enable(flags);
	}

	return 0;
}

void platform_release_rpmsg_vdev(struct rpmsg_device *rpdev, void *platform)
{
	struct rpmsg_virtio_device *rpvdev;
	struct remoteproc *rproc;

	rpvdev = metal_container_of(rpdev, struct rpmsg_virtio_device, rdev);
	rproc = platform;

	rpmsg_deinit_vdev(rpvdev);
	remoteproc_remove_virtio(rproc, rpvdev->vdev);
}

void platform_cleanup(void *platform)
{
	struct remoteproc *rproc = platform;

	if (rproc)
		remoteproc_remove(rproc);
	cleanup_system();
}
