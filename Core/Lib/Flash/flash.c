#include "flash.h"


extern SPI_HandleTypeDef W25Q_SPI;


// #define W25Q_Select() HAL_GPIO_WritePin(Flash_CS_GPIO_Port, Flash_CS_Pin, GPIO_PIN_RESET)
// #define W25Q_Unselect() HAL_GPIO_WritePin(Flash_CS_GPIO_Port, Flash_CS_Pin, GPIO_PIN_SET)
#define W25Q_Select()  (Flash_CS_GPIO_Port->BSRR = ((uint32_t)Flash_CS_Pin << 16))
#define W25Q_Unselect() (Flash_CS_GPIO_Port->BSRR = Flash_CS_Pin)


#ifdef W25Q_SPI_DMA
    volatile uint8_t W25Q_dma_state = 0;
#endif


static void W25Q_Transmit(uint8_t *p_buf, uint16_t size);
static void W25Q_Receive(uint8_t *p_buf, uint16_t size);
#ifdef W25Q_SPI_DMA
    static void W25Q_Receive_DMA(uint8_t *p_buf, uint16_t size);
#endif


static void W25Q_Transmit(uint8_t *p_buf, uint16_t size) {
    HAL_SPI_Transmit(&W25Q_SPI, p_buf, size, 100);
}

static void W25Q_Receive(uint8_t *p_buf, uint16_t size) {
    HAL_SPI_Receive(&W25Q_SPI, p_buf, size, 100);
}

#ifdef W25Q_SPI_DMA
    static void W25Q_Receive_DMA(uint8_t *p_buf, uint16_t size) {
        W25Q_dma_state = 1;
        HAL_SPI_Receive_DMA(&W25Q_SPI, p_buf, size);
        while(W25Q_dma_state);
    }
#endif


uint8_t W25Q_Init(void) {
    HAL_Delay(100);
	W25Q_Reset();
	HAL_Delay(100);

	uint32_t id = W25Q_GetJedecId();

	if(id != W25Q_JEDEC_ID) {
		return 1;
	}

    return 0;
}

void W25Q_Reset(void) {
    uint8_t cmd[2];

	W25Q_Select();
	cmd[0] = W25Q_ENABLE_RESET;
	cmd[1] = W25Q_RESET;

	W25Q_Transmit(cmd, 2);
	W25Q_Unselect();
}

uint16_t W25Q_GetId(void) {
    uint8_t cmd[4];
	uint8_t id[2];

	W25Q_Select();
	cmd[0] = W25Q_GET_ID;
	cmd[1] = 0;
	cmd[2] = 0;
	cmd[3] = 0;

	W25Q_Transmit(cmd, 4);
	W25Q_Receive(id, 2);
	W25Q_Unselect();

	return (id[0] << 8) | id[1];
}

uint32_t W25Q_GetJedecId(void) {
    uint8_t cmd[1];
	uint8_t id[3];

	W25Q_Select();
	cmd[0] = W25Q_GET_JEDEC_ID;

	W25Q_Transmit(cmd, 1);
	W25Q_Receive(id, 3);
	W25Q_Unselect();

	return (id[0] << 16) | (id[1] << 8) | id[2];
}

void W25Q_WriteEnable(void) {
	uint8_t cmd[1];

	W25Q_Select();
	cmd[0] = W25Q_WRITE_ENABLE;

	W25Q_Transmit(cmd, 1);
	W25Q_Unselect();
}

void W25Q_WriteDisable(void) {
	uint8_t cmd[1];

	W25Q_Select();
	cmd[0] = W25Q_WRITE_DISABLE;

	W25Q_Transmit(cmd, 1);
	W25Q_Unselect();
}

void W25Q_WaitOperation(void) {
    uint8_t cmd[1];
    uint8_t reg_status;

    W25Q_Select();
    cmd[0] = W25Q_READ_STATUS_1;
    W25Q_Transmit(cmd, 1);

    do {
        W25Q_Receive(&reg_status, 1);
        HAL_Delay(0);
    }
    while((reg_status & 0x01) == 0x01);
    W25Q_Unselect();
}


void W25Q_EraseSector(uint32_t sector_num) {
	W25Q_WaitOperation();
	W25Q_WriteEnable();

	uint8_t cmd[4];
	uint32_t addr = sector_num * W25Q_SECTOR_SIZE;

	W25Q_Select();
	cmd[0] = W25Q_SECTOR_ERASE;
	cmd[1] = (addr >> 16) & 0xFF;
	cmd[2] = (addr >> 8) & 0xFF;
	cmd[3] = addr & 0xFF;

	W25Q_Transmit(cmd, 4);
	W25Q_Unselect();

	W25Q_WaitOperation();
}

void W25Q_EraseBlock(uint32_t block_num) {
	W25Q_WaitOperation();
	W25Q_WriteEnable();

	uint8_t cmd[4];
	uint32_t addr = block_num * W25Q_BLOCK_SIZE;

	W25Q_Select();
	cmd[0] = W25Q_BLOCK_ERASE;
	cmd[1] = (addr >> 16) & 0xFF;
	cmd[2] = (addr >> 8) & 0xFF;
	cmd[3] = addr & 0xFF;

	W25Q_Transmit(cmd, 4);
	W25Q_Unselect();

	W25Q_WaitOperation();
}

void W25Q_EraseChip(void) {
	W25Q_WaitOperation();
	W25Q_WriteEnable();

	uint8_t cmd[1];

	W25Q_Select();
	cmd[0] = W25Q_CHIP_ERASE;

	W25Q_Transmit(cmd, 1);
	W25Q_Unselect();

	W25Q_WaitOperation();
}


void W25Q_ReadData(uint32_t addr, uint8_t* p_data, uint32_t size) {
	uint8_t cmd[4];

	W25Q_Select();
	cmd[0] = W25Q_READ;
	cmd[1] = (addr >> 16) & 0xFF;
	cmd[2] = (addr >> 8) & 0xFF;
	cmd[3] = addr & 0xFF;

	W25Q_Transmit(cmd, 4);
	W25Q_Receive(p_data, size);
	W25Q_Unselect();
}

void W25Q_ReadDataFast(uint32_t addr, uint8_t* p_data, uint32_t size) {
	uint8_t cmd[5];

	W25Q_Select();
	cmd[0] = W25Q_FAST_READ;
	cmd[1] = (addr >> 16) & 0xFF;
	cmd[2] = (addr >> 8) & 0xFF;
	cmd[3] = addr & 0xFF;
	cmd[4] = 0;

	W25Q_Transmit(cmd, 5);

    #ifdef W25Q_SPI_DMA
        W25Q_Receive_DMA(p_data, size);
    #else
	    W25Q_Receive(p_data, size);
    #endif

    W25Q_Unselect();
}

void W25Q_ReadPage(uint32_t page_num, uint32_t offset, uint8_t* p_data, uint16_t size) {
	if(offset + size > W25Q_PAGE_SIZE)
		return;

	uint32_t addr = page_num * W25Q_PAGE_SIZE + offset;

	// W25Q_ReadData(addr, p_data, size);
	W25Q_ReadDataFast(addr, p_data, size);
}

void W25Q_WriteData(uint32_t addr, uint8_t* p_data, uint32_t size) {
    uint8_t cmd[4];
    uint32_t bytes_written = 0;

    while (bytes_written < size) {
        W25Q_WaitOperation();
        W25Q_WriteEnable();

        uint32_t page_offset = addr & 0xFF;
        uint32_t bytes_to_write = 256 - page_offset;
        if (bytes_to_write > (size - bytes_written)) {
            bytes_to_write = size - bytes_written;
        }

        W25Q_Select();
        cmd[0] = W25Q_PAGE_PROGRAMM;
        cmd[1] = (addr >> 16) & 0xFF;
        cmd[2] = (addr >> 8) & 0xFF;
        cmd[3] = addr & 0xFF;

        W25Q_Transmit(cmd, 4);
        W25Q_Transmit(&p_data[bytes_written], bytes_to_write);
        W25Q_Unselect();

        addr += bytes_to_write;
        bytes_written += bytes_to_write;
    }

    W25Q_WriteDisable();
}

void W25Q_WritePage(uint32_t page_num, uint32_t offset, uint8_t* p_data, uint16_t size) {
	if(size == 0 || p_data == NULL)
		return;
	if(offset + size > W25Q_PAGE_SIZE)
		return;

	W25Q_WaitOperation();
	W25Q_WriteEnable();

	uint32_t addr = page_num * W25Q_PAGE_SIZE + offset;

	W25Q_WriteData(addr, p_data, size);
}




/*

#include <stdio.h>
#include <string.h>

#include "uart.h"


extern SPI_HandleTypeDef hspi1;


FlashInfo_t flash_info;


uint8_t cmd[cmd_SIZE] = {0};


void W25Q_Transmit(uint8_t *p_buf, uint16_t size) {
	HAL_SPI_Transmit(&hspi1, p_buf, size, 100);
}
void W25Q_Receive(uint8_t *p_buf, uint16_t size) {
	HAL_SPI_Receive(&hspi1, p_buf, size, 100);
}


void flash_init(void) {
	HAL_Delay(100);
	flash_reset();
	HAL_Delay(100);

	uint32_t id = flash_get_id();
	sprintf(uart1_tx_buffer,"ID:0x%lX\r\n",id);
	UART1_Transmit(uart1_tx_buffer, strlen(uart1_tx_buffer));

	if((id & 0xFFFF) == 0x4017) {
		flash_info.block_count=128;
		flash_info.page_size=256;
		flash_info.sector_size=0x1000;
		flash_info.sector_count=flash_info.block_count*16;
		flash_info.page_count=(flash_info.sector_count*flash_info.sector_size)/flash_info.page_size;
		flash_info.block_size=flash_info.sector_size*16;
		flash_info.capacity_KB=(flash_info.sector_count*flash_info.sector_size)/1024;
	} else {
		UART1_Transmit("Id error\r\n", strlen("Id error\r\n"));
	}
}


void flash_reset(void) {
	flash_cs_set();
	cmd[0] = W25_ENABLE_RESET;
	cmd[1] = W25_RESET;

	W25Q_Transmit(cmd, 2);
	flash_cs_reset();
}

uint32_t flash_get_id(void) {
	flash_cs_set();
	uint8_t id[3];
	cmd[0] = W25_GET_JEDEC_ID;

	W25Q_Transmit(cmd, 1);
	W25Q_Receive(id, 3);
	flash_cs_reset();

	return (id[0] << 16) | (id[1] << 8) | id[2];
}

void W25Q_WriteEnable(void) {
	flash_cs_set();
	cmd[0] = W25_WRITE_ENABLE;

	W25Q_Transmit(cmd, 1);
	flash_cs_reset();
}

void W25Q_WriteDisable(void) {
	flash_cs_set();
	cmd[0] = W25_WRITE_DISABLE;

	W25Q_Transmit(cmd, 1);
	flash_cs_reset();
}

void W25Q_WaitOperation(void) {
  uint8_t reg_status;

  flash_cs_set();
  cmd[0] = W25_READ_STATUS_1;
  W25Q_Transmit(cmd, 1);

  do {
	W25Q_Receive(&reg_status,1);
	flash_info.status_register_1 = reg_status;
	HAL_Delay(1);
  }
  while((flash_info.status_register_1 & 0x01) == 0x01);
  flash_cs_reset();
}


void flash_erase_sector(uint32_t sector_addr) {
	W25Q_WaitOperation();
	W25Q_WriteEnable();

	uint32_t addr = sector_addr * flash_info.sector_size;

	flash_cs_set();
	cmd[0] = W25_SECTOR_ERASE;
	cmd[1] = (addr >> 16) & 0xFF;
	cmd[2] = (addr >> 8) & 0xFF;
	cmd[3] = addr & 0xFF;

	W25Q_Transmit(cmd, 4);
	flash_cs_reset();

	W25Q_WaitOperation();
}

void flash_erase_block(uint32_t block_addr) {
	W25Q_WaitOperation();
	W25Q_WriteEnable();

	uint32_t addr = block_addr * flash_info.block_size;

	flash_cs_set();
	cmd[0] = W25_BLOCK_ERASE;
	cmd[1] = (addr >> 16) & 0xFF;
	cmd[2] = (addr >> 8) & 0xFF;
	cmd[3] = addr & 0xFF;

	W25Q_Transmit(cmd, 4);
	flash_cs_reset();

	W25Q_WaitOperation();
}

void flash_erase_chip(void) {
	W25Q_WaitOperation();
	W25Q_WriteEnable();

	flash_cs_set();
	cmd[0] = W25_CHIP_ERASE;

	W25Q_Transmit(cmd, 1);
	flash_cs_reset();

	W25Q_WaitOperation();
}


void flash_read_data(uint32_t addr, uint8_t* p_data, uint32_t size) {
	flash_cs_set();
	cmd[0] = W25_READ;
	cmd[1] = (addr >> 16) & 0xFF;
	cmd[2] = (addr >> 8) & 0xFF;
	cmd[3] = addr & 0xFF;

	W25Q_Transmit(cmd, 4);
	W25Q_Receive(p_data, size);
	flash_cs_reset();
}

void flash_fast_read_data(uint32_t addr, uint8_t* p_data, uint32_t size) {
	flash_cs_set();
	cmd[0] = W25_FAST_READ;
	cmd[1] = (addr >> 16) & 0xFF;
	cmd[2] = (addr >> 8) & 0xFF;
	cmd[3] = addr & 0xFF;
	cmd[4] = 0;

	W25Q_Transmit(cmd, 5);
	W25Q_Receive(p_data, size);
	flash_cs_reset();
}

void flash_read_page(uint32_t page_addr, uint32_t offset, uint8_t* p_data, uint16_t size) {
	if(page_addr > flash_info.page_count)
		return;
	if(offset + size > flash_info.page_size)
		return;

	uint32_t addr = page_addr * flash_info.page_size + offset;

	flash_read_data(addr, p_data, size);
//	flash_fast_read_data(addr, p_data, size);
}

void flash_write_data(uint32_t addr, uint8_t* p_data, uint32_t size) {
	W25Q_WaitOperation();
	W25Q_WriteEnable();

	flash_cs_set();
	cmd[0] = W25_PAGE_PROGRAMM;
	cmd[1] = (addr >> 16) & 0xFF;
	cmd[2] = (addr >> 8) & 0xFF;
	cmd[3] = addr & 0xFF;

	W25Q_Transmit(cmd, 4);
	W25Q_Transmit(p_data, size);
	flash_cs_reset();

	W25Q_WaitOperation();
	W25Q_WriteDisable();
}

void flash_write_page(uint32_t page_addr, uint32_t offset, uint8_t* p_data, uint16_t size) {
	if(size == 0 || p_data == NULL)
		return;
	if(page_addr > flash_info.page_count)
		return;
	if(offset + size > flash_info.page_size)
		return;

	W25Q_WaitOperation();
	W25Q_WriteEnable();

	uint32_t addr = page_addr * flash_info.page_size + offset;

	flash_write_data(addr, p_data, size);
}

*/