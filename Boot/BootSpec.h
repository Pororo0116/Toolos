/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * Copyright (c) 2026, "뿌댕이"
 * Project: ToolOS
 * File: BootSpec.h
 * * Description:
 * ToolOS BootLoader structure
 * 
*/

#ifndef BootSpec_h
#define BootSpec_h

#include <Uefi.h>

// 16 + 2 + 2 + 4 + 8 + 8 + 8 + 4 + 2 + 2 + 2 + 2 + 2 + 2 = 64byte
// ELF Header
#pragma pack(1)
typedef struct {
  CHAR8 e_ident[16];
  UINT16 e_type;
  UINT16 e_machine;
  UINT32 e_version;
  UINT64 e_entry;
  UINT64 e_phoff;
  UINT64 e_shoff;
  UINT32 e_flags;
  UINT16 e_ehsize;
  UINT16 e_phentsize;
  UINT16 e_phnum;
  UINT16 e_shentsize;
  UINT16 e_shnum;
  UINT16 e_shstrndx;
} ELFHeader;
#pragma pack()

// 4 + 4 + 8 + 8 + 8 + 8 + 8 + 8 = 56byte
// ELF Program Header
#pragma pack(1)
typedef struct {
  UINT32 p_type;
  UINT32 p_flags;
  UINT64 p_offset;
  UINT64 p_vaddr;
  UINT64 p_paddr;
  UINT64 p_filesz;
  UINT64 p_memsz;
  UINT64 p_align;
} ELFProgramHeader;
#pragma pack()

// 1 + 1 + 1 + 1 + 4 + 4 + 8 + 1 + 1 = 20byte
// ACPI TABLE
#pragma pack(1)
typedef struct {
  CHAR8 Signature[8];
  UINT8 Checksum;
  CHAR8 OEMID[6];
  UINT8 Revision;
  UINT32 RsdtAddress;
  UINT32 Length;
  UINT64 XsdtAddress;
  UINT8 ExtendedChecksum;
  UINT8 Reserved[3];
} TOOLOS_ACPI_XSDT_TABLE;
#pragma pack()

typedef struct {
  UINTN Size;
} _ELFProgramHeaderInfo;

typedef struct {
  ELFHeader Header;
  ELFProgramHeader* Program_Header;
  UINTN HeaderSize;
  UINTN ProgramHeaderSize;
  UINTN ALLProgramHeaderSize;
  _ELFProgramHeaderInfo ProgramHeader_Info;
} ELF_READER;

typedef struct {
  EFI_MEMORY_DESCRIPTOR* MapPTR;
  UINT64 MapNums;
  UINTN KernelMapSize;
} _MemoryMapInfo;

typedef struct {
  UINTN MapSize;
  EFI_MEMORY_DESCRIPTOR* Map;
  UINTN MapKey;
  UINTN DescriptorSize;
  UINT32 DescriptorVersion;
  _MemoryMapInfo Info;
} MemoryMapSpec;

typedef struct {
  UINT64 LoadAddress;
  BOOLEAN LoadSafe;
  UINT64 InfoSize;
  UINT64 NeedMemory;
} KernelInfo;

typedef struct {
  EFI_LOADED_IMAGE* Loaded_Image;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* FS;
  EFI_FILE_PROTOCOL* Root;
  EFI_FILE_PROTOCOL* KernelFile;
} FileSystemSpec;

// 8 + 8 + 8 + 8 = 32byte
typedef struct {
  UINT64 PhysicalStart;
  UINT64 NumberOfPages;
  UINT64 Type;
  UINT64 TotalNums;
} TOOLOS_MEMORYMAP_INFO;

// 8 + 8 + 4 + 4 + 4 + 4 = 32byte
typedef struct  {
  UINT64 FrameBufferBase;
  UINT64 FrameBufferSize;
  UINT32 VerticalResolution;
  UINT32 HorizontalResolution;
  UINT32 Reserved;
  UINT32 PixelsPerScanLine; 
} TOOLOS_GRAPHICS_INFO;

#endif