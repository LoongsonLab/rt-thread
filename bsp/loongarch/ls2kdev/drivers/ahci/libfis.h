#ifndef __LS2K_LIBFIS_H__
#define __LS2K_LIBFIS_H__

// Register - Host to Device FIS
typedef struct sata_fis_h2d
{
    uint8_t fis_type;
    uint8_t pm_port_c;
    uint8_t command;
    uint8_t features;
    uint8_t lba_low;
    uint8_t lba_mid;
    uint8_t lba_high;
    uint8_t device; // 7
    uint8_t lba_low_exp;
    uint8_t lba_mid_exp;
    uint8_t lba_high_exp;
    uint8_t features_exp;
    uint8_t sector_count; // 12
    uint8_t sector_count_exp; // 13
    uint8_t res1;
    uint8_t control;
    uint8_t res2[4];
} __attribute__((packed)) sata_fis_h2d_t;


// Register - Host to Device FIS for read/write FPDMA queued
typedef struct sata_fis_h2d_ncq
{
    uint8_t fis_type;
    uint8_t pm_port_c;
    uint8_t command;
    uint8_t sector_count_low;
    uint8_t lba_low;
    uint8_t lba_mid;
    uint8_t lba_high;
    uint8_t device;
    uint8_t lba_low_exp;
    uint8_t lba_mid_exp;
    uint8_t lba_high_exp;
    uint8_t sector_count_high;
    uint8_t tag;
    uint8_t res1;
    uint8_t res2;
    uint8_t control;
    uint8_t res3[4];
} __attribute__((packed)) sata_fis_h2d_ncq_t;


// Register - Device to Host FIS
typedef struct sata_fis_d2h
{
    uint8_t fis_type;
    uint8_t pm_port_i;
    uint8_t status;
    uint8_t error;
    uint8_t lba_low;
    uint8_t lba_mid;
    uint8_t lba_high;
    uint8_t device;
    uint8_t lba_low_exp;
    uint8_t lba_mid_exp;
    uint8_t lba_high_exp;
    uint8_t res1;
    uint8_t sector_count;
    uint8_t sector_count_exp;
    uint8_t res2[2];
    uint8_t res3[4];
} __attribute__((packed)) sata_fis_d2h_t;


// DMA Setup - Device to Host or Host to Device FIS
typedef struct sata_fis_dma_setup
{
    uint8_t fis_type;
    uint8_t pm_port_dir_int_act;
    uint8_t res1;
    uint8_t res2;
    uint32_t dma_buffer_id_low;
    uint32_t dma_buffer_id_high;
    uint32_t res3;
    uint32_t dma_buffer_offset;
    uint32_t dma_transfer_count;
    uint32_t res4;
} __attribute__((packed)) sata_fis_dma_setup_t;


// PIO Setup - Device to Host FIS
typedef struct sata_fis_pio_setup
{
    uint8_t fis_type;
    uint8_t pm_port_dir_int;
    uint8_t status;
    uint8_t error;
    uint8_t lba_low;
    uint8_t lba_mid;
    uint8_t lba_high;
    uint8_t res1;
    uint8_t lba_low_exp;
    uint8_t lba_mid_exp;
    uint8_t lba_high_exp;
    uint8_t res2;
    uint8_t sector_count;
    uint8_t sector_count_exp;
    uint8_t res3;
    uint8_t e_status;
    uint16_t transfer_count;
    uint16_t res4;
} __attribute__((packed)) sata_fis_pio_setup_t;


// Data - Host to Device or Device to Host FIS
typedef struct sata_fis_data
{
    uint8_t fis_type;
    uint8_t pm_port;
    uint8_t res1;
    uint8_t res2;
    uint32_t data[2048];
} __attribute__((packed)) sata_fis_data_t;


// fis_type - SATA FIS type
enum sata_fis_type
{
    SATA_FIS_TYPE_REGISTER_H2D = 0x27,
    SATA_FIS_TYPE_REGISTER_D2H = 0x34,
    SATA_FIS_TYPE_DMA_ACT_D2H = 0x39,
    SATA_FIS_TYPE_DMA_SETUP_BI = 0x41,
    SATA_FIS_TYPE_DATA_BI = 0x46,
    SATA_FIS_TYPE_BIST_ACT_BI = 0x58,
    SATA_FIS_TYPE_PIO_SETUP_D2H = 0x5F,
    SATA_FIS_TYPE_SET_DEVICE_BITS_D2H = 0xA1,
};

#endif // __LS2K_LIBFIS_H__
