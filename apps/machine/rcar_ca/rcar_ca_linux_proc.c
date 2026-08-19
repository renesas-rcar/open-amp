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
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/un.h>
#include "platform_info.h"

/* IPI REGs OFFSET */
#define IPI_OBS_OFFSET  0x00000000 /* IPI observation register offset */
#define IPI_TRIG_OFFSET 0x00000004 /* IPI trigger register offset */

#define OFFSET_MEM_KICK_PA		0x500
#define OFFSET_MEM_CHECK_NOTIFY		0x04

int rcar_ca_linux_proc_mem_check(struct remoteproc_priv *prproc)
{
	uint32_t val;

	if (!prproc->shm_io)
		return 0;

	val = metal_io_read32(prproc->shm_io,
			      OFFSET_MEM_KICK_PA + OFFSET_MEM_CHECK_NOTIFY);
	if (val & (1U << 0)) {
		metal_io_write32(prproc->shm_io,
				 OFFSET_MEM_KICK_PA + OFFSET_MEM_CHECK_NOTIFY,
				 val & ~(1U << 0));
		return 1;
	}

	return 0;
}

static int rcar_ca_linux_proc_irq_handler(int vect_id, void *data)
{
	struct remoteproc *rproc = data;
	struct remoteproc_priv *prproc;
	unsigned int ipi_intr_status;

	(void)vect_id;
	if (!rproc)
		return METAL_IRQ_NOT_HANDLED;
	prproc = rproc->priv;
	ipi_intr_status = (unsigned int)metal_io_read32(prproc->ipi_io,
							IPI_OBS_OFFSET);
	if (ipi_intr_status) {
		atomic_flag_clear(&prproc->ipi_nokick);
		metal_io_write32(prproc->ipi_io, IPI_OBS_OFFSET,
				 0);
		return METAL_IRQ_HANDLED;
	}
	return METAL_IRQ_NOT_HANDLED;
}

static struct remoteproc *
rcar_ca_linux_proc_init(struct remoteproc *rproc,
			const struct remoteproc_ops *ops, void *arg)
{
	struct remoteproc_priv *prproc = arg;
	struct metal_device *dev;
	unsigned int irq_vect;
	metal_phys_addr_t mem_pa;
	int ret;

	(void)ops;
	if (!rproc || !prproc)
		return NULL;
	prproc->ipi_dev = NULL;
	prproc->shm_dev = NULL;

	/* Get shared memory device */
	ret = metal_device_open(prproc->shm_bus_name, prproc->shm_name,
				&dev);
	if (ret) {
		fprintf(stderr, "ERROR: failed to open shm device: %d.\r\n",
			ret);
		goto err1;
	}

	printf("Successfully open shm device.\r\n");
	prproc->shm_dev = dev;
	prproc->shm_io = metal_device_io_region(dev, 0);
	if (!prproc->shm_io)
		goto err2;

	mem_pa = metal_io_phys(prproc->shm_io, 0);
	/* Over the UCIe, the device address is the low 32-bit of physical addres */
	remoteproc_init_mem(&prproc->shm_mem, "shm", mem_pa,
			    mem_pa & 0xFFFFFFFFUL,
			    metal_io_region_size(prproc->shm_io),
			    prproc->shm_io);
	remoteproc_add_mem(rproc, &prproc->shm_mem);
	printf("Successfully added shared memory\r\n");

	if (prproc->ipi_name &&
	    !metal_device_open(prproc->ipi_bus_name, prproc->ipi_name, &dev)) {
		prproc->ipi_dev = dev;
		prproc->ipi_io = metal_device_io_region(dev, 0);
		if (prproc->ipi_io) {
			atomic_store(&prproc->ipi_nokick, 1);
			metal_io_write32(prproc->ipi_io, IPI_OBS_OFFSET, 0);
			irq_vect = (uintptr_t)dev->irq_info;
			metal_irq_register(irq_vect,
					   rcar_ca_linux_proc_irq_handler, rproc);
			metal_irq_enable(irq_vect);
			printf("Successfully initialized Linux remoteproc.\r\n");
			return rproc;
		}
		metal_device_close(dev);
		prproc->ipi_dev = NULL;
	}

	if (prproc->shm_io) {
		prproc->use_mem_kick = 1;
		atomic_store(&prproc->ipi_nokick, 1);
		metal_io_write32(prproc->shm_io,
				 OFFSET_MEM_KICK_PA + OFFSET_MEM_CHECK_NOTIFY, 0);
		printf("Successfully initialized memory-based kick.\r\n");
		return rproc;
	}

	printf("failed to find ipi device and shared memory to poll.\r\n");

err2:
	metal_device_close(prproc->shm_dev);
err1:
	return NULL;
}

static void rcar_ca_linux_proc_remove(struct remoteproc *rproc)
{
	struct remoteproc_priv *prproc;
	struct metal_device *dev;

	if (!rproc)
		return;

	prproc = rproc->priv;
	dev = prproc->ipi_dev;
	if (dev) {
		metal_irq_disable((uintptr_t)dev->irq_info);
		metal_irq_unregister((uintptr_t)dev->irq_info);
		metal_device_close(dev);
	}

	if (prproc->shm_dev)
		metal_device_close(prproc->shm_dev);
}

static void *
rcar_ca_linux_proc_mmap(struct remoteproc *rproc, metal_phys_addr_t *pa,
			metal_phys_addr_t *da, size_t size,
			unsigned int attribute, struct metal_io_region **io)
{
	struct remoteproc_priv *prproc;
	metal_phys_addr_t lpa, lda;
	struct metal_io_region *tmpio;

	(void)attribute;
	(void)size;
	if (!rproc)
		return NULL;
	prproc = rproc->priv;
	lpa = *pa;
	lda = *da;

	if (lpa == METAL_BAD_PHYS && lda == METAL_BAD_PHYS)
		return NULL;
	if (lpa == METAL_BAD_PHYS)
		lpa = lda;
	if (lda == METAL_BAD_PHYS)
		lda = lpa;
	tmpio = prproc->shm_io;
	if (!tmpio)
		return NULL;

	*pa = lpa;
	*da = lda;
	if (io)
		*io = tmpio;
	return metal_io_phys_to_virt(tmpio, lpa);
}

static int rcar_ca_linux_proc_notify(struct remoteproc *rproc, uint32_t id)
{
	struct remoteproc_priv *prproc;

	(void)id;
	if (!rproc)
		return -1;

	prproc = rproc->priv;

	if (prproc->use_mem_kick) {
		metal_io_write32(prproc->shm_io, OFFSET_MEM_KICK_PA, 1U << 0);
		return 0;
	}

	metal_io_write32(prproc->ipi_io, IPI_TRIG_OFFSET, 0x1);
	return 0;
}

const struct remoteproc_ops rcar_ca_linux_proc_ops = {
	.init = rcar_ca_linux_proc_init,
	.remove = rcar_ca_linux_proc_remove,
	.mmap = rcar_ca_linux_proc_mmap,
	.notify = rcar_ca_linux_proc_notify,
	.start = NULL,
	.stop = NULL,
	.shutdown = NULL,
};
