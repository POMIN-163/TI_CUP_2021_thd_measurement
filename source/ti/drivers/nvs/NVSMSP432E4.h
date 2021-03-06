/*
 * Copyright (c) 2017-2019, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** ============================================================================
 *  @file       NVSMSP432E4.h
 *
 *  @brief      NVS driver implementation for MSP432E4 devices
 *
 *  This NVS driver implementation is designed to work with FLASH memory on
 *  MSP432E4 devices.  FLASH memory sectors on these devices are 16 kilobytes
 *  in size.
 *
 *  The NVS header file should be included in an application as follows:
 *  @code
 *  #include <ti/drivers/NVS.h>
 *  #include <ti/drivers/nvs/NVSMSP432E4.h>
 *  @endcode
 *
 *  ============================================================================
 */
/*
 *  ======== NVSMSP432E4.h ========
 */

#ifndef ti_drivers_nvs_NVSMSP432E4__include
#define ti_drivers_nvs_NVSMSP432E4__include

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if defined (__cplusplus)
extern "C" {
#endif

/*!
 *  @internal @brief NVS function pointer table
 *
 *  'NVSMSP432E4_fxnTable' is a fully populated function pointer table
 *  that can be referenced in the NVS_config[] array entries.
 *
 *  Users can minimize their application code size by providing their
 *  own custom NVS function pointer table that contains only those APIs
 *  used by the application.
 *
 *  An example of a custom NVS function table is shown below:
 *  @code
 *  //
 *  // Since the application does not use the
 *  // NVS_control(), NVS_lock(), and NVS_unlock() APIs,
 *  // these APIs are removed from the function
 *  // pointer table and replaced with NULL
 *  //
 *  const NVS_FxnTable myNVS_fxnTable = {
 *      NVSMSP432E4_close,
 *      NULL,     // remove NVSMSP432E4_control(),
 *      NVSMSP432E4_erase,
 *      NVSMSP432E4_getAttrs,
 *      NVSMSP432E4_init,
 *      NULL,     // remove NVSMSP432E4_lock(),
 *      NVSMSP432E4_open,
 *      NVSMSP432E4_read,
 *      NULL,     // remove NVSMSP432E4_unlock(),
 *      NVSMSP432E4_write
 *  };
 *  @endcode
 */
extern const NVS_FxnTable NVSMSP432E4_fxnTable;

/*!
 *  @brief      NVSMSP432E4 attributes
 *
 *  The 'regionBase' field must point to the base address of the region
 *  to be managed.
 *
 *  The regionSize must be an integer multiple of the flash sector size.
 *  For most MSP432E4 devices, the flash sector size is 16 kilobytes.
 *
 *  Care must be taken to ensure that the linker does not unintentionally
 *  place application content (e.g., code/data) in the flash regions.
 *
 *  For CCS and IAR tools, defining and reserving flash memory regions can
 *  be done entirely within the ti_drivers_config.c file. For GCC, additional
 *  content is required in the application's linker command file to achieve
 *  the same result.
 *
 *  The example below defines a char array 'flashBuf' and uses CCS and IAR
 *  compiler pragmas to place 'flashBuf' at a specific address within the flash
 *  memory.
 *
 *  For GCC, the 'flashBuf' array is placed into a named linker section.
 *  Corresponding linker commands are added to the application's linker
 *  command file to place the section at a specific flash memory address.
 *  The section placement command is carefully chosen to only RESERVE space
 *  for the 'flashBuf' array, and not to actually initialize it during
 *  the application load process, thus preserving the content of flash.
 *
 *  The 'regionBase' fields of the two HWAttrs region instances
 *  are initialized to point to the base address of 'flashBuf' and to some
 *  offset from the base of the char array.
 *
 *  The linker command syntax is carefully chosen to only RESERVE space
 *  for the char array and not to actually initialize it during application
 *  load.
 *
 *  @code
 *  #define SECTORSIZE 0x4000
 *  #define FLASH_REGION_BASE 0xF8000
 *
 *  //
 *  // Reserve flash sectors for NVS driver use
 *  // by placing an uninitialized byte array
 *  // at the desired flash address.
 *  //
 *  #if defined(__TI_COMPILER_VERSION__)
 *  //
 *  //  Place uninitialized array at FLASH_REGION_BASE
 *  //
 *  #pragma LOCATION(flashBuf, FLASH_REGION_BASE);
 *  #pragma NOINIT(flashBuf);
 *  char flashBuf[SECTORSIZE * 2];
 *
 *  #elif defined(__IAR_SYSTEMS_ICC__)
 *  //
 *  //  Place uninitialized array at FLASH_REGION_BASE
 *  //
 *  __no_init char flashBuf[SECTORSIZE * 2] @ FLASH_REGION_BASE;
 *
 *  #elif defined(__GNUC__)
 *
 *  //
 *  //  Place the flash buffers in the .nvs section created in the gcc linker file.
 *  //  The .nvs section enforces alignment on a sector boundary but may
 *  //  be placed anywhere in flash memory.  If desired the .nvs section can be set
 *  //  to a fixed address by changing the following in the gcc linker file:
 *
 * .nvs (FIXED_FLASH_ADDR) (NOLOAD) : AT (FIXED_FLASH_ADDR) {
 *      *(.nvs)
 * } > REGION_TEXT
 *
 *  __attribute__ ((section (".nvs")))
 *  char flashBuf[SECTORSIZE * 2];
 *
 *  #endif
 *
 *  NVSMSP432E4_HWAttrs NVSMSP432E4HWAttrs[2] = {
 *      //
 *      // region 0 is 1 flash sector in length.
 *      //
 *      {
 *          .regionBase = (void *)flashBuf,
 *          .regionSize = SECTORSIZE,
 *      },
 *      //
 *      // region 1 is 1 flash sectors in length.
 *      //
 *      {
 *          .regionBase = (void *)(flashBuf + SECTORSIZE),
 *          .regionSize = SECTORSIZE,
 *      }
 *  };
 *
 *  Example GCC linker command file content reserves flash space
 *  but does not initialize it:
 *
 *  MEMORY
 *  {
 *      FLASH (RX) : ORIGIN = 0x00000000, LENGTH = 0x00100000
 *      SRAM (RWX) : ORIGIN = 0x20000000, LENGTH = 0x00040000
 *  }
 *
 *  .nvs (0xF8000) (NOLOAD) : AT (0xF8000) {
 *      KEEP (*(.nvs))
 *  } > NVS
 *
 *
 *  @endcode
 */
typedef struct {
    void     *regionBase;    /*!< Base address of flash region */
    size_t    regionSize;    /*!< The size of the region in bytes */
} NVSMSP432E4_HWAttrs;

/*
 *  @brief      NVSMSP432E4 Object
 *
 *  The application must not access any member variables of this structure!
 */
typedef struct {
    bool        opened;             /* Has this region been opened */
} NVSMSP432E4_Object;

/*
 *  @cond NODOC
 *  NVSMSP432E4 driver public APIs
 */
extern void NVSMSP432E4_close(NVS_Handle handle);
extern int_fast16_t NVSMSP432E4_control(NVS_Handle handle, uint_fast16_t cmd,
    uintptr_t arg);
extern int_fast16_t NVSMSP432E4_erase(NVS_Handle handle, size_t offset,
    size_t size);
extern void NVSMSP432E4_getAttrs(NVS_Handle handle, NVS_Attrs *attrs);
extern void NVSMSP432E4_init();
extern int_fast16_t NVSMSP432E4_lock(NVS_Handle handle, uint32_t timeout);
extern NVS_Handle NVSMSP432E4_open(uint_least8_t index, NVS_Params *params);
extern int_fast16_t NVSMSP432E4_read(NVS_Handle handle, size_t offset,
    void *buffer, size_t bufferSize);
extern void NVSMSP432E4_unlock(NVS_Handle handle);
extern int_fast16_t NVSMSP432E4_write(NVS_Handle handle, size_t offset,
    void *buffer, size_t bufferSize, uint_fast16_t flags);
/*! @endcond */

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

/*@}*/
#endif /* ti_drivers_nvs_NVSMSP432E4__include */
