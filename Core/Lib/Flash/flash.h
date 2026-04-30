#ifndef INC_FLASH_H_
#define INC_FLASH_H_

#ifdef __cplusplus
extern C {
#endif

#include "main.h"


/**
 * @defgroup W25Q_Setup
 * @brief W25Q Setup parameters
 * @{
 */
#define W25Q_SPI hspi1
// #define 	W25Q_SPI_DMA

#define W25Q_ID 0xEF16
#define W25Q_JEDEC_ID 0xEF4017
/// @}


/**
 * @defgroup W25Q_Commands
 * @brief W25Q Chip commands from datasheet
 * @{
 */
#define W25Q_PAGE_SIZE 0x100
#define W25Q_SECTOR_SIZE 0x1000
#define W25Q_BLOCK_SIZE 0x10000

#define W25Q_ENABLE_RESET 0x66
#define W25Q_RESET 0x99
#define W25Q_GET_ID 0x90
#define W25Q_GET_JEDEC_ID 0x9F

#define W25Q_WRITE_DISABLE 0x04
#define W25Q_WRITE_ENABLE 0x06
#define W25Q_SECTOR_ERASE 0x20
#define W25Q_BLOCK_ERASE 0xD8
#define W25Q_CHIP_ERASE 0xC7

#define W25Q_READ_STATUS_1 0x05
#define W25Q_READ_STATUS_2 0x35
#define W25Q_READ_STATUS_3 0x15
#define W25Q_WRITE_STATUS_1 0x01
#define W25Q_WRITE_STATUS_2 0x31
#define W25Q_WRITE_STATUS_3 0x11

#define W25Q_READ 0x03
#define W25Q_FAST_READ 0x0B
#define W25Q_PAGE_PROGRAMM 0x02
/// @}


/**
 * @defgroup W25Q_Func
 * @brief W25Q Functions
 * @{
 */
#define W25Q_Select() (Flash_CS_GPIO_Port->BSRR = ((uint32_t)Flash_CS_Pin << 16))
#define W25Q_Unselect() (Flash_CS_GPIO_Port->BSRR = Flash_CS_Pin)

#ifdef W25Q_SPI_DMA
    uint8_t W25Q_WaitReady(void);
#endif

uint8_t W25Q_Init(void);
uint8_t W25Q_CheckState(void);
void W25Q_Reset(void);
uint16_t W25Q_GetId(void);
uint32_t W25Q_GetJedecId(void);
void W25Q_WriteEnable(void);
void W25Q_WriteDisable(void);
void W25Q_WaitOperation(void);

void W25Q_EraseSector(uint32_t sector_num);
void W25Q_EraseBlock(uint32_t block_num);
void W25Q_EraseChip(void);

void W25Q_ReadData(uint32_t addr, uint8_t* p_data, uint32_t size);
void W25Q_ReadDataFast(uint32_t addr, uint8_t* p_data, uint32_t size);
void W25Q_ReadPage(uint32_t page_num, uint32_t offset, uint8_t* p_data, uint16_t size);

void W25Q_WriteData(uint32_t addr, uint8_t* p_data, uint32_t size);
void W25Q_WritePage(uint32_t page_num, uint32_t offset, uint8_t* p_data, uint16_t size);
/// @}


#ifdef __cplusplus
}
#endif

#endif /* INC_FLASH_H_ */
