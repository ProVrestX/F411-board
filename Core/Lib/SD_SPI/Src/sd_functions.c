/******************************************************************************
 *  File        : sd_functions.c
 *  Author      : ControllersTech
 *  Website     : https://controllerstech.com
 *  Date        : June 26, 2025
 *  Updated on  : Sep 27, 2025
 *
 *  Description :
 *    This file is part of a custom STM32/Embedded tutorial series.
 *    For documentation, updates, and more examples, visit the website above.
 *
 *  Note :
 *    This code is written and maintained by ControllersTech.
 *    You are free to use and modify it for learning and development.
 ******************************************************************************/


#include "sd_functions.h"
#include "fatfs.h"
#include "sd_diskio_spi.h"
#include "sd_spi.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ff.h"
#include "ffconf.h"

char SD_path[4];
FATFS fs;

static uint8_t SD_state = 0;

//int SD_format(void) {
//	// Pre-mount required for legacy FatFS
//	f_mount(&fs, SD_path, 0);
//
//	FRESULT res;
//	res = f_mkfs(SD_path, 1, 0);
//	if (res != FR_OK) {
//		printf("Format failed: f_mkfs returned %d\r\n", res);
//	}
//		return res;
//}

int SD_GetSpace_KB(void) {
	if(!SD_state) {
		printf("SD card is not mounted\r\n");
		return FR_NOT_READY;
	}

	FATFS *pfs;
	DWORD fre_clust, tot_sect, fre_sect, total_kb, free_kb;
	FRESULT res = f_getfree(SD_path, &fre_clust, &pfs);
	if (res != FR_OK) return res;

	tot_sect = (pfs->n_fatent - 2) * pfs->csize;
	fre_sect = fre_clust * pfs->csize;
	total_kb = tot_sect / 2;
	free_kb = fre_sect / 2;
	printf("<SD>  Total: %lu KB, Free: %lu KB\r\n", total_kb, free_kb);
	return FR_OK;
}

int SD_GetSpace(void) {
	if(!SD_state) {
//		printf("SD card is not mounted\r\n");
		return 0;
	}

	FATFS *pfs;
	DWORD fre_clust, tot_sect, total_kb;
	FRESULT res = f_getfree(SD_path, &fre_clust, &pfs);
	if (res != FR_OK) return res;

	tot_sect = (pfs->n_fatent - 2) * pfs->csize;
	total_kb = tot_sect / 2;
	return total_kb/1024;
}

int SD_Mount(void) {
	if(SD_state) {
		printf("SD card is already mounted\r\n");
		return FR_OK;
	}

	FRESULT res;
	extern uint8_t SD_IsSdhc(void);

	printf("Linking SD driver...\r\n");
	if (FATFS_LinkDriver(&SD_Driver, SD_path) != 0) {
		printf("FATFS_LinkDriver failed\n");
		return FR_DISK_ERR;
	}

	printf("Initializing disk...\r\n");
	DSTATUS stat = disk_initialize(0);
	if (stat != 0) {
		printf("disk_initialize failed: 0x%02X\n", stat);
		printf("FR_NOT_READY\tTry Hard Reset or Check Connection/Power\r\n");
		printf("Make sure \"MX_FATFS_Init\" is not being called in the main function\n"\
				"You need to disable its call in CubeMX->Project Manager->Advance Settings->Uncheck Generate code for MX_FATFS_Init\r\n");
		return FR_NOT_READY;
	}

	printf("Attempting mount at %s...\r\n", SD_path);
	res = f_mount(&fs, SD_path, 1);
	if (res == FR_OK)
	{
		SD_state = 1;
		printf("SD card mounted successfully at %s\r\n", SD_path);
		printf("Card Type: %s\r\n", SD_IsSdhc() ? "SDHC/SDXC" : "SDSC");

		// Capacity and free space reporting
		SD_GetSpace_KB();
		printf("\n");
		return FR_OK;
	}

	/* Many users were having issues with f_mkfs, so I have disabled it
	 * You need to format SD card in FAT FileSysytem before inserting it
	 */
//	 Handle no filesystem by creating one
//	if (res == FR_NO_FILESYSTEM)
//	{
//		printf("No filesystem found on SD card. Attempting format...\r\nThis will create 32MB Partition (Most Probably)\r\n");
//		printf("If you need the full sized SD card, use the computer to format into FAT32\r\n");
//		SD_format();
//
//		printf("Retrying mount after format...\r\n");
//		res = f_mount(&fs, SD_path, 1);
//		if (res == FR_OK) {
//			printf("SD card formatted and mounted successfully.\r\n");
//			printf("Card Type: %s\r\n", SD_is_sdhc() ? "SDHC/SDXC" : "SDSC");
//
//			// Report capacity after format
//			SD_get_space_kb();
//		}
//		else {
//			printf("Mount failed even after format: %d\r\n", res);
//		}
//		return res;
//	}

	// Any other mount error
	printf("Mount failed with code: %d\r\n", res);
	return res;
}


int SD_Unmount(void) {
	if(!SD_state) {
		printf("SD card is not mounted\r\n");
		return FR_OK;
	}

	FRESULT res = f_mount(NULL, SD_path, 1);
	printf("SD card unmounted: %s\r\n", (res == FR_OK) ? "OK" : "Failed");
	SD_state = 0;
	return res;
}

int SD_WriteFile(const char *filename, const char *text) {
	if(!SD_state) {
		printf("SD card is not mounted\r\n");
		return FR_NOT_READY;
	}

	FIL file;
	UINT bw;
	FRESULT res = f_open(&file, filename, FA_CREATE_ALWAYS | FA_WRITE);
	if (res == FR_NO_PATH)
		printf("Write error: No path (%s)\r\n", filename);
	if (res != FR_OK) return res;

	res = f_write(&file, text, strlen(text), &bw);
	f_close(&file);
	printf("Write %u bytes to %s\r\n", bw, filename);
	return (res == FR_OK && bw == strlen(text)) ? FR_OK : FR_DISK_ERR;
}

int SD_AppendFile(const char *filename, const char *text) {
	if(!SD_state) {
		printf("SD card is not mounted\r\n");
		return FR_NOT_READY;
	}

	FIL file;
	UINT bw;
	FRESULT res = f_open(&file, filename, FA_OPEN_ALWAYS | FA_WRITE);
	if (res == FR_NO_PATH)
		printf("Write error: No path (%s)\r\n", filename);
	if (res != FR_OK) return res;

	res = f_lseek(&file, f_size(&file));
	if (res != FR_OK) {
		f_close(&file);
		return res;
	}

	res = f_write(&file, text, strlen(text), &bw);
	f_close(&file);
	printf("Appended %u bytes to %s\r\n", bw, filename);
	return (res == FR_OK && bw == strlen(text)) ? FR_OK : FR_DISK_ERR;
}

int SD_ReadFile(const char *filename, char *buffer, UINT bufsize, UINT *bytes_read) {
	if(!SD_state) {
		printf("SD card is not mounted\r\n");
		return FR_NOT_READY;
	}

	FIL file;
	*bytes_read = 0;

	FRESULT res = f_open(&file, filename, FA_READ);
	if (res != FR_OK) {
		printf("f_open failed with code: %d\r\n", res);
		return res;
	}

	printf("<READ>  Reading file: %s\r\n", filename);

	res = f_read(&file, buffer, bufsize - 1, bytes_read);
	if (res != FR_OK) {
		printf("f_read failed with code: %d\r\n", res);
		f_close(&file);
		return res;
	}

	buffer[*bytes_read] = '\0';

	res = f_close(&file);
	if (res != FR_OK) {
		printf("f_close failed with code: %d\r\n", res);
		return res;
	}

	printf("Read %u bytes from %s\r\n", *bytes_read, filename);
	return FR_OK;
}

int SD_ReadCsv(const char *filename, CsvRecord *records, int max_records, int *record_count) {
	if(!SD_state) {
		printf("SD card is not mounted\r\n");
		return FR_NOT_READY;
	}

	FIL file;
	char line[128];
	*record_count = 0;

	FRESULT res = f_open(&file, filename, FA_READ);
	if (res != FR_OK) {
		printf("Failed to open CSV: %s (%d)", filename, res);
		return res;
	}

	printf("<READ>  Reading CSV: %s\r\n", filename);
	while (f_gets(line, sizeof(line), &file) && *record_count < max_records) {
		char *token = strtok(line, ",");
		if (!token) continue;
		strncpy(records[*record_count].field1, token, sizeof(records[*record_count].field1));

		token = strtok(NULL, ",");
		if (!token) continue;
		strncpy(records[*record_count].field2, token, sizeof(records[*record_count].field2));

		token = strtok(NULL, ",");
		if (token)
			records[*record_count].value = atoi(token);
		else
			records[*record_count].value = 0;

		(*record_count)++;
	}

	f_close(&file);

	// Print parsed data
	for (int i = 0; i < *record_count; i++) {
		printf("[%d] %s | %s | %d", i,
				records[i].field1,
				records[i].field2,
				records[i].value);
	}

	return FR_OK;
}

int SD_DeleteFile(const char *filename) {
	if(!SD_state) {
		printf("SD card is not mounted\r\n");
		return FR_NOT_READY;
	}

	FRESULT res = f_unlink(filename);
	printf("Delete %s: %s\r\n", filename, (res == FR_OK ? "OK" : "Failed"));
	return res;
}

int SD_RenameFile(const char *oldname, const char *newname) {
	if(!SD_state) {
		printf("SD card is not mounted\r\n");
		return FR_NOT_READY;
	}

	FRESULT res = f_rename(oldname, newname);
	printf("Rename %s to %s: %s\r\n", oldname, newname, (res == FR_OK ? "OK" : "Failed"));
	return res;
}

FRESULT SD_CreateDir(const char *path) {
	if(!SD_state) {
		printf("SD card is not mounted\r\n");
		return FR_NOT_READY;
	}

	FRESULT res = f_mkdir(path);
//	printf("Create directory %s: %s\r\n", path, (res == FR_OK ? "OK" : "Failed"));
	if (res == FR_OK) {
		printf("Directory created: %s\r\n", path);
	} else if (res == FR_EXIST) {
		printf("Directory already exists: %s\r\n", path);
		res = FR_OK; // Не считаем это ошибкой
	} else {
		printf("Failed to create directory %s: %d\r\n", path, res);
	}
	return res;
}

void SD_ListDirRecursive(const char *path, int depth) {
	if(!SD_state) {
		printf("SD card is not mounted\r\n");
		return;
	}

	DIR dir;
	FILINFO fno = {0};
	FRESULT res = f_opendir(&dir, path);
	if (res != FR_OK) {
		printf("%*s[ERR] Cannot open: %s\r\n", depth * 2, "", path);
		return;
	}

	while (1) {
		res = f_readdir(&dir, &fno);
		if (res != FR_OK || fno.fname[0] == 0) break;

		const char *name = (*fno.fname) ? fno.fname : fno.fname;

		if (fno.fattrib & AM_DIR) {
			if (strcmp(name, ".") && strcmp(name, "..")) {
				printf("%*s<DIR>%s\r\n", depth * 2, "", name);
				char newpath[128];
				snprintf(newpath, sizeof(newpath), "%s/%s", path, name);
				SD_ListDirRecursive(newpath, depth + 1);
			}
		} else {
			printf("%*s<FILE>%s  (%lu bytes)\r\n", depth * 2, "", name, (unsigned long)fno.fsize);
		}
	}
	f_closedir(&dir);
}

void SD_ListFiles(void) {
	if(!SD_state) {
		printf("SD card is not mounted\r\n");
		return;
	}

	printf("<LIST>  Files on SD Card:\r\n");
	SD_ListDirRecursive(SD_path, 1);
	printf("\n");
}

int SD_CheckState(void) {
	char label[32];
	DWORD vsn;
	SD_state = (f_getlabel(SD_path, label, &vsn) == FR_OK)? 1: 0;
	return SD_state;
}

int SD_GetState(void) {
	return SD_state;
}

uint8_t SD_ListDir(const char *path, char *output, uint16_t output_size) {
	if(output == NULL) {
		return 0;
	}
	if(!SD_state) {
		sprintf(output, "SD card is not mounted\r\n");
		return 0;
	}

	DIR dir;
	FILINFO fno = {0};
	FRESULT res = f_opendir(&dir, path);
	uint16_t output_pos = 0;
	uint16_t max_pos = output_size - 2;
	uint8_t item_count = 0;

	if (res != FR_OK) {
		sprintf(output, "[ERR] Cannot open: %s\r\n", path);
		return 0;
	}


	while (max_pos - output_pos) {
		res = f_readdir(&dir, &fno);
		if (res != FR_OK || fno.fname[0] == 0) break;

		const char *name = (*fno.fname) ? fno.fname : fno.fname;

		if (fno.fattrib & AM_DIR) {
			if (strcmp(name, ".") && strcmp(name, "..")) {
				output_pos += snprintf(output + output_pos, max_pos - output_pos, "<d>%s\n", name);
			}
		} else {
			output_pos += snprintf(output + output_pos, max_pos - output_pos, "<f>%s (%lu b)\n", name, (unsigned long)fno.fsize);
		}

		item_count++;
	}
	f_closedir(&dir);

	output[max_pos+1] = 0;

	return item_count;
}
