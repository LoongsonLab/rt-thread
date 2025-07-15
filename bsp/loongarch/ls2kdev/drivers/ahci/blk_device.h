#ifndef __LS2K_BLK_DEVICE_H__
#define __LS2K_BLK_DEVICE_H__

#include <ahci_platform.h>

#define DEV_TYPE_UNKNOWN  0xff  // not connected
#define DEV_TYPE_HARDDISK 0x00  // harddisk
#define DEV_TYPE_TAPE     0x01  // Tape
#define DEV_TYPE_CDROM    0x05  // CD-ROM
#define DEV_TYPE_OPDISK   0x07  // optical disk

struct blk_device
{
    struct rt_device parent;
    struct ahci_uc_priv *ahci_device;

    uint8_t target;
    uint8_t lun;
    uint8_t type;

    bool lba48;
    uint64_t lba;
    uint64_t blksz;
    uint32_t log2blksz;

    char vendor[41];
    char product[21];
    char revision[9];
};

#endif // __LS2K_BLK_DEVICE_H__
