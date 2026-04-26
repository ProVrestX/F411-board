/******************************************************************************
 *  File        : sd_functions.h
 *  Author      : ControllersTech
 *  Website     : https://controllerstech.com
 *  Date        : June 26, 2025
 *
 *  Description :
 *    This file is part of a custom STM32/Embedded tutorial series.
 *    For documentation, updates, and more examples, visit the website above.
 *
 *  Note :
 *    This code is written and maintained by ControllersTech.
 *    You are free to use and modify it for learning and development.
 ******************************************************************************/

#ifndef __SD_FUNCTIONS_H__
#define __SD_FUNCTIONS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "fatfs.h"
#include <stdint.h>

extern char sd_path[];

// Mount and unmount
int SD_Mount(void);
int SD_Unmount(void);

// Basic file operations
int SD_WriteFile(const char *filename, const char *text);
int SD_AppendFile(const char *filename, const char *text);
int SD_ReadFile(const char *filename, char *buffer, UINT bufsize, UINT *bytes_read);
int SD_DeleteFile(const char *filename);
int SD_RenameFile(const char *oldname, const char *newname);

// Directory handling
FRESULT SD_CreateDir(const char *path);
void SD_ListDirRecursive(const char *path, int depth);
void SD_ListFiles(void);

// Space information
int SD_GetSpace_KB(void);
int SD_GetSpace(void);

//csv File operations
// CSV Record structure
typedef struct CsvRecord {
    char field1[32];
    char field2[32];
    int value;
} CsvRecord;

// CSV reader (caller defines record array)
int SD_ReadCsv(const char *filename, CsvRecord *records, int max_records, int *record_count);

int SD_CheckState(void);
int SD_GetState(void);
uint8_t SD_ListDir(const char *path, char *output, uint16_t output_size);

#ifdef __cplusplus
}
#endif

#endif // __SD_FUNCTIONS_H__
