#include <debug.h>
#include <string.h>
#include <syscall.h>
#include <types.h>

#include "bios.h"
#include "nvram.h"

#define CMOS_INDEX_PORT		0x70
#define CMOS_DATA_PORT		0x71

/* memory size in KB from 1 MB ~ 16 MB */
#define CMOS_EXTMEM_LOW		0x30	/* Bits 7 ~ 0 */
#define CMOS_EXTMEM_HIGH	0x31	/* Bits 15 ~ 8 */
/* memory size in byte from 16 MB ~ 4 GB */
#define CMOS_EXTMEM2_LOW	0x34	/* Bits 23 ~ 16 */
#define CMOS_EXTMEM2_HIGH	0x35	/* Bits 31 ~ 24 */
/* memory size in byte above 4GB */
#define CMOS_HIGHMEM_LOW	0x5b	/* Bits 23 ~ 16 */
#define CMOS_HIGHMEM_MID	0x5c	/* Bits 31 ~ 24 */
#define CMOS_HIGHMEM_HIGH	0x5d	/* Bits 39 ~ 32 */

#define CMOS_NMI_DISABLE_BIT	(1 << 7)

#ifdef DEBUG_VNVRAM

#define VNVRAM_DEBUG(fmt, ...)			\
	do {					\
		DEBUG(fmt, ##__VA_ARGS__);	\
	} while (0)

#else

#define VNVRAM_DEBUG(fmt...)			\
	do {					\
	} while (0)

#endif

static uint8_t
vnvram_ioport_read(struct vnvram *nvram, uint16_t port)
{
	ASSERT(nvram != NULL);
	ASSERT(port == CMOS_DATA_PORT);

	if (nvram->data_valid == TRUE) {
		return nvram->data;
	} else {
		uint8_t data;
		/* DEBUG("Passthrough\n"); */
		sys_read_ioport(port, SZ8, &data);
		return data;
	}
}

static void
vnvram_ioport_write(struct vnvram *nvram, uint16_t port, uint8_t data)
{
	ASSERT(nvram != NULL);
	ASSERT(port == CMOS_INDEX_PORT);

	uint8_t idx = data & ~(uint8_t) CMOS_NMI_DISABLE_BIT;

	switch (idx) {
	case CMOS_EXTMEM_LOW:
		nvram->data = (nvram->extmem_size >> 10) & 0xFF;
		VNVRAM_DEBUG("Read ExtMem.LOW: %d.\n", nvram->data);
		break;

	case CMOS_EXTMEM_HIGH:
		nvram->data = (nvram->extmem_size >> 18) & 0xFF;
		VNVRAM_DEBUG("Read ExtMem.HIGH: %d.\n", nvram->data);
		break;

	case CMOS_EXTMEM2_LOW:
		nvram->data = (nvram->extmem2_size >> 16) & 0xFF;
		VNVRAM_DEBUG("Read ExtMem2.LOW: %d.\n", nvram->data);
		break;

	case CMOS_EXTMEM2_HIGH:
		nvram->data = (nvram->extmem2_size >> 24) & 0xFF;
		VNVRAM_DEBUG("Read ExtMem2.HIGH: %d.\n", nvram->data);
		break;

	case CMOS_HIGHMEM_LOW:
		nvram->data = (nvram->highmem_size >> 16) & 0xFF;
		VNVRAM_DEBUG("Read HighMem.LOW: %d.\n", nvram->data);
		break;

	case CMOS_HIGHMEM_MID:
		nvram->data = (nvram->highmem_size >> 24) & 0xFF;
		VNVRAM_DEBUG("Read HighMem.MID: %d.\n", nvram->data);
		break;

	case CMOS_HIGHMEM_HIGH:
		nvram->data = (nvram->highmem_size >> 32) & 0xFF;
		VNVRAM_DEBUG("Read HighMem.HIGH: %d.\n", nvram->data);
		break;

	default:
		VNVRAM_DEBUG("Passthrough index 0x%x.\n", idx);
		nvram->data_valid = FALSE;
		sys_write_ioport(port, SZ8, data);
		return;
	}

	nvram->data_valid = TRUE;
}

static void
_vnvram_ioport_read(struct vnvram *nvram, uint16_t port, void *data)
{
	ASSERT(nvram != NULL && data != NULL);
	ASSERT(port == CMOS_DATA_PORT);

	*(uint8_t *) data = vnvram_ioport_read(nvram, port);

	VNVRAM_DEBUG("Read I/O port %d, val 0x%x.\n", port, *(uint8_t *) data);
}

static void
_vnvram_ioport_write(struct vnvram *nvram, uint16_t port, void *data)
{
	ASSERT(nvram != NULL && data != NULL);
	ASSERT(port == CMOS_INDEX_PORT);

	VNVRAM_DEBUG("Write I/O port %d, val 0x%08x.\n", port, *(uint8_t *) data);

	vnvram_ioport_write(nvram, port, *(uint8_t *) data);
}

int
main(int argc, char **argv)
{
	struct vnvram vnvram;

	int parent_chid;

	uint8_t buf[1024];
	size_t size;

	struct ioport_rw_req *rw_req;
	struct ioport_read_ret *ret;
	struct device_ready *dev_rdy;

	uint64_t phy_mem_size;

	memset(&vnvram, 0, sizeof(struct vnvram));

	vnvram.data_valid = FALSE;

	sys_guest_mem_size(&phy_mem_size);
	if (phy_mem_size > 0x100000000ULL) { /* above 4 GB */
		vnvram.highmem_size = phy_mem_size - 0x100000000ULL;
		phy_mem_size -= vnvram.highmem_size;
	}
	if (phy_mem_size > 0x1000000) { /* 16 MB ~ 4G */
		vnvram.extmem2_size = phy_mem_size - 0x1000000;
		phy_mem_size -= vnvram.extmem2_size;
	}
	if (phy_mem_size > 0x100000) { /* 1 MB ~ 16 MB */
		vnvram.extmem_size = phy_mem_size - 0x100000;
		phy_mem_size -= vnvram.extmem_size;
	}

	VNVRAM_DEBUG("ExtMem %d bytes, ExtMem2 %d bytes, HighMem %lld bytes.\n",
		     vnvram.extmem_size,
		     vnvram.extmem2_size,
		     vnvram.highmem_size);

	sys_register_ioport(CMOS_DATA_PORT, SZ8, 0);
	sys_register_ioport(CMOS_DATA_PORT, SZ8, 1);
	sys_register_ioport(CMOS_INDEX_PORT, SZ8, 0);
	sys_register_ioport(CMOS_INDEX_PORT, SZ8, 1);

	parent_chid = sys_getpchid();

	dev_rdy = (struct device_ready *) buf;
	dev_rdy->magic = MAGIC_DEVICE_READY;
	sys_send(parent_chid, dev_rdy, sizeof(struct device_ready));

	while (1) {
		if (sys_recv(parent_chid, buf, &size, TRUE))
			continue;

		switch (((uint32_t *) buf)[0]) {
		case MAGIC_IOPORT_RW_REQ:
			rw_req = (struct ioport_rw_req *) buf;

			if (rw_req->port != CMOS_DATA_PORT &&
			    rw_req->port != CMOS_INDEX_PORT) {
				VNVRAM_DEBUG("Ignore unknown port %d.\n",
					     rw_req->port);
				continue;
			}

			if (rw_req->width != SZ8) {
				VNVRAM_DEBUG("Ignore unknown data width %d.\n",
					     rw_req->width);
				continue;
			}

			VNVRAM_DEBUG("Receive IOPORT_RW request: "
				     "port %d, write %d.\n",
				     rw_req->port, rw_req->write);

			ret = (struct ioport_read_ret *) buf;

			if (rw_req->write && rw_req->port == CMOS_INDEX_PORT) {
				_vnvram_ioport_write(&vnvram,
						     rw_req->port,
						     &rw_req->data);
			} else if (rw_req->write == 0 &&
				   rw_req->port == CMOS_DATA_PORT) {
				_vnvram_ioport_read(&vnvram,
						    rw_req->port,
						    &ret->data);
				ret->magic = MAGIC_IOPORT_READ_RET;
				sys_send(parent_chid,
					 ret, sizeof(struct ioport_read_ret));
			}

			continue;

		default:
			continue;
		}
	}

	return 0;
}