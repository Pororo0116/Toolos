/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. 
 * Copyright (c) 2026, "뿌댕이"
 * Project: ToolOS
 * File: ToolOSBootLoader.c
 * Version : 0.1.1
 * * Description:
 * The 0.1.1 bootloader focused on improving code efficiency and readability. 
 * All structure definitions were moved to BootSpec.h, 
 * and frequently called memory map extraction functions were created as separate custom functions.
 */

#include <Uefi.h>
#include <Uefi/UefiBaseType.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Guid/Acpi.h>
#include <Guid/FileInfo.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/LoadedImage.h>
#include "BootSpec.h"

#define _1MB 1048576
#define GOP_GUID gEfiGraphicsOutputProtocolGuid
#define ACPI_GUID gEfiAcpi20TableGuid
#define LOADIMAGE_GUID gEfiLoadedImageProtocolGuid
#define FileSystem_GUID gEfiSimpleFileSystemProtocolGuid
#define CPU_HALT __asm__ __volatile__ ("cli; hlt")

CHAR8 ACPI_TABLE_Signature[8] = {'R', 'S', 'D', ' ', 'P', 'T', 'R', ' '};

// Kernel Entry Pointer
typedef VOID (*KernelStart)(TOOLOS_GRAPHICS_INFO, TOOLOS_MEMORYMAP_INFO*);
// Function
void GetMemoryMap(EFI_STATUS* Status, MemoryMapSpec* Memory);

EFI_STATUS EFIAPI UefiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
  gST->ConOut->ClearScreen(gST->ConOut);
  
  EFI_STATUS Status;
  KernelStart JumpToKernel;

  EFI_GRAPHICS_OUTPUT_PROTOCOL* GOP = NULL;
  TOOLOS_ACPI_XSDT_TABLE* ACPISpecTable = NULL;
  MemoryMapSpec Memory = {0};
  ELF_READER ELF;
  KernelInfo Kernel = {0, FALSE, 0, 0};
  FileSystemSpec FileSystem;
  ELF.HeaderSize = sizeof(ELFHeader);
  ELF.ProgramHeaderSize = sizeof(ELFProgramHeader);

  // ---------------Information Extraction Area---------------

  // Get Graphics Out Protocol Info
  Status = gBS->LocateProtocol(
    &GOP_GUID,
    NULL,
    (VOID **)&GOP
  );
  if (EFI_ERROR(Status)) {
    Print(L"[ERROR]\r\nFailed to load graphics information. Error Code: %r\r\nSolution: Try rebooting your system. If the issue persists, recommend updating your UEFI firmware.\r\nIt is safe to shut down by pressing the power button.", Status);
    CPU_HALT;
  }

  // Get ACPI Table Address
  for (UINTN i = 0; i < SystemTable->NumberOfTableEntries; i++) {
    if (CompareGuid(&SystemTable->ConfigurationTable[i].VendorGuid, &ACPI_GUID)) {
      ACPISpecTable = SystemTable->ConfigurationTable[i].VendorTable;
      break;
    }
  }

  // ----------------Information Verification Area----------------
  
  // Verify that the GOP memory is logically consistent.
  if (GOP->Mode->FrameBufferSize < (GOP->Mode->Info->PixelsPerScanLine * GOP->Mode->Info->VerticalResolution * 4)) {
    Print(L"[ERROR] Error Code: UNRELIABLE_FRAME_BUFFER\r\nSolution: If you have external graphics, try booting with the integrated graphics, if not, do the opposite.\r\nIt is safe to shut down by pressing the power button.");
    CPU_HALT;
  }
  
  // Checks if the ACPI address pointer is empty or has an invalid value.
  if (ACPISpecTable == NULL) {
    Print(L"[ERROR] Error Code: ACPI_NOT_FOUND\r\nSolution: Check if your hardware supports ACPI 2.0 or if the firmware is corrupted.\r\nIt is safe to shut down by pressing the power button.");
    CPU_HALT;
  }
  
  // Verify that the ACPI tables are compliant.
  // TODO: Add a solution message
  for (UINTN i = 0; i < 8; i++) {
    if (ACPISpecTable->Signature[i] != ACPI_TABLE_Signature[i]) {
      Print(L"[ERROR] Error Code: UNRELIABLE_ACPI_TABLE\r\nSolution: (TODO)\r\nIt is safe to shut down by pressing the power button.");
      CPU_HALT;
    }
  }
  

  // ----------------------Kernel Load Area----------------------
  Status = gBS->HandleProtocol(
    ImageHandle,
    &LOADIMAGE_GUID,
    (VOID**)&FileSystem.Loaded_Image
  );
  if (EFI_ERROR(Status)) {
    Print(L"[ERROR] Failed to retrieve device information. Error Code: %r\r\nSolution: Please check that the boot device is properly connected and that there is no physical damage. If the issue persists, recommend updating your UEFI firmware.\r\nIt is safe to shut down by pressing the power button.", Status);
    CPU_HALT;
  }

  Status = gBS->HandleProtocol(
    FileSystem.Loaded_Image->DeviceHandle,
    &FileSystem_GUID,
    (VOID**)&FileSystem.FS
  );
  if (EFI_ERROR(Status)) {
    Print(L"[ERROR] Unable to check file system information on the device. Error Code: %r\r\nSolution: Boot from the installation media and run the \"/fix filesystem\" command.\r\nIt is safe to shut down by pressing the power button.", Status);
    CPU_HALT;
  }

  Status = FileSystem.FS->OpenVolume(
    FileSystem.FS,
    &FileSystem.Root
  );
  if (EFI_ERROR(Status)) {
    Print(L"[ERROR] Failed to activate file system. Error Code: %r\r\nSolution: Boot from the installation media and run the \"/fix filesystem\" command.\r\nIt is safe to shut down by pressing the power button.", Status);
    CPU_HALT;
  }

  Status = FileSystem.Root->Open(
    FileSystem.Root,
    &FileSystem.KernelFile,
    L"TOSKernel.elf",
    EFI_FILE_MODE_READ,
    0
  );
  if (EFI_ERROR(Status)) {
    Print(L"[ERROR] Cannot open kernel file. Error Code: %r\r\nSolution: Boot from the installation media and run the \"/check kernelfile\" command.\r\nIt is safe to shut down by pressing the power button.", Status);
    CPU_HALT;
  }

  Status = FileSystem.KernelFile->Read(
    FileSystem.KernelFile,
    &ELF.HeaderSize,
    (VOID*)&ELF.Header
  );
  if (EFI_ERROR(Status)) {
    Print(L"[ERROR] Unable to read kernel file. Error Code: %r\r\nSolution: Try rebooting your system. If the issue persists, recommend updating your UEFI firmware.\r\nIt is safe to shut down by pressing the power button.", Status);
    CPU_HALT;
  }

  if (ELF.Header.e_ident[0] != 0x7F || ELF.Header.e_ident[1] != 'E' || ELF.Header.e_ident[2] != 'L' || ELF.Header.e_ident[3] != 'F') {
    Print(L"[ERROR] Error Code: UNRELIABLE_KERNEL_FILE\r\nSolution: Boot from the installation media and run the \"/check kernelfile\" command.\r\nIt is safe to shut down by pressing the power button.");
    CPU_HALT;
  }
  if (ELF.Header.e_ident[4] != 2) {
    Print(L"[ERROR] Kernel is Not 64bit. Error Code: UNRELIABLE_KERNEL_FILE\r\nSolution: Boot from the installation media and run the \"/check kernelfile\" command.\r\nIt is safe to shut down by pressing the power button.");
    CPU_HALT;
  }
  if (ELF.Header.e_ident[5] != 1) {
    Print(L"[ERROR] The kernel file was not compiled as little endian. Error Code: UNRELIABLE_KERNEL_FILE\r\nSolution: Boot from the installation media and run the \"/check kernelfile\" command.\r\nIt is safe to shut down by pressing the power button.");
    CPU_HALT;
  }

  Status = FileSystem.KernelFile->SetPosition(
    FileSystem.KernelFile,
    ELF.Header.e_phoff
  );

  ELF.ALLProgramHeaderSize = ELF.ProgramHeaderSize * ELF.Header.e_phnum;
  Status = gBS->AllocatePool(
    EfiLoaderData,
    ELF.ALLProgramHeaderSize,
    (VOID**)&ELF.Program_Header
  );
  if (EFI_ERROR(Status)) {
    Print(L"[ERROR] A problem occurred while reading the kernel file. Error Code: %r\r\nSolution: Try rebooting your system. If the issue persists, recommend updating your UEFI firmware.\r\nIt is safe to shut down by pressing the power button.", Status);
    CPU_HALT;
  }
  Status = FileSystem.KernelFile->Read(
    FileSystem.KernelFile,
    &ELF.ALLProgramHeaderSize,
    (VOID*)ELF.Program_Header
  );
  if (EFI_ERROR(Status)) {
    Print(L"[ERROR] A problem occurred while reading the kernel file. Error Code: %r\r\nSolution: Try rebooting your system. If the issue persists, recommend updating your UEFI firmware.\r\nIt is safe to shut down by pressing the power button.", Status);
    CPU_HALT;
  }

  for (UINTN i = 0; i < ELF.Header.e_phnum; i++) {
    if (ELF.Program_Header[i].p_type == 1) {
      FileSystem.KernelFile->SetPosition(
        FileSystem.KernelFile,
        ELF.Program_Header[i].p_offset
      );
      Kernel.NeedMemory += ELF.Program_Header[i].p_memsz;
    }
  }

  GetMemoryMap(&Status, &Memory);

  Memory.Info.MapPTR = Memory.Map;
  Memory.Info.MapNums = Memory.MapSize / Memory.DescriptorSize;
  for (UINTN i = 0; i < Memory.Info.MapNums; i++) {
    if (Memory.Info.MapPTR->Type == EfiConventionalMemory && _1MB < Memory.Info.MapPTR->PhysicalStart && Kernel.NeedMemory < (Memory.Info.MapPTR->NumberOfPages * 4096)) {
      Kernel.LoadSafe = TRUE;
      Kernel.LoadAddress = Memory.Info.MapPTR->PhysicalStart;
      break;
    }
    Memory.Info.MapPTR = (EFI_MEMORY_DESCRIPTOR*)((UINT8*)Memory.Info.MapPTR + Memory.DescriptorSize);
  }
  
  if (!Kernel.LoadSafe) {
    Print(L"[ERROR] Not enough contiguous memory found at 1MB+ to load kernel. Error Code: KERNEL_LOAD_NOT_SAFE\r\nSolution: Try increasing system RAM. If the issue persists, recommend updating your UEFI firmware.\r\nIt is safe to shut down by pressing the power button.");
    CPU_HALT;
  }

  gBS->FreePool(Memory.Map);

  Status = gBS->AllocatePages(
    EfiLoaderData,
    AllocateAddress,
    Memory.Info.MapPTR->NumberOfPages,
    &Kernel.LoadAddress
  );
  if (EFI_ERROR(Status)) {
    Print(L"[ERROR] Failed to reserve physical pages for kernel image. Error Code: %r\r\nSolution: Try increasing system RAM. If the issue persists, recommend updating your UEFI firmware.\r\nIt is safe to shut down by pressing the power button.", Status);
    CPU_HALT;
  }

  for (UINTN i = 0; i < ELF.Header.e_phnum; i++) {
    if (ELF.Program_Header[i].p_type == 1) {
      FileSystem.KernelFile->SetPosition(
        FileSystem.KernelFile,
        ELF.Program_Header[i].p_offset
      );
      UINTN Size = ELF.Program_Header[i].p_filesz;
      FileSystem.KernelFile->Read(
        FileSystem.KernelFile,
        &Size,
        (VOID*)(Kernel.LoadAddress + ELF.Program_Header[i].p_vaddr)
      );

      if (ELF.Program_Header[i].p_filesz < ELF.Program_Header[i].p_memsz) {
        UINTN Fill = ELF.Program_Header[i].p_memsz - ELF.Program_Header[i].p_filesz;
        gBS->SetMem(
          (VOID *)(Kernel.LoadAddress + ELF.Program_Header[i].p_vaddr + ELF.Program_Header[i].p_filesz),
          Fill,
          0
        );
      }
    }
  }

  // ------------Information Transmission Area-------------
  
  JumpToKernel = (KernelStart)(Kernel.LoadAddress + ELF.Header.e_entry);
  TOOLOS_GRAPHICS_INFO TGI;
  TOOLOS_MEMORYMAP_INFO* TMI;

  TGI.FrameBufferBase = GOP->Mode->FrameBufferBase;
  TGI.FrameBufferSize = GOP->Mode->FrameBufferSize;
  TGI.HorizontalResolution = GOP->Mode->Info->HorizontalResolution;
  TGI.VerticalResolution = GOP->Mode->Info->VerticalResolution;
  TGI.PixelsPerScanLine = GOP->Mode->Info->PixelsPerScanLine;

  GetMemoryMap(&Status, &Memory);
  Memory.Info.KernelMapSize = (Memory.Info.MapNums + 2) * sizeof(TOOLOS_MEMORYMAP_INFO);
  Status = gBS->AllocatePool(
    EfiLoaderData,
    Memory.Info.KernelMapSize,
    (VOID **)&TMI
  );
  if (EFI_ERROR(Status)) {
    Print(L"[ERROR] Failed to rent dedicated memory map space. Error Code: %r\r\nSolution: Try rebooting your system. If the issue persists, recommend updating your UEFI firmware.\r\nIt is safe to shut down by pressing the power button.", Status);
    CPU_HALT;
  }
  gBS->FreePool(Memory.Map);

  GetMemoryMap(&Status, &Memory);
  Memory.Info.MapNums = Memory.MapSize / Memory.DescriptorSize;
  Memory.Info.MapPTR = Memory.Map;
  TMI->TotalNums = Memory.Info.MapNums;

  for (UINTN i = 1; i < Memory.Info.MapNums; i++) {
    TMI[i].Type = Memory.Info.MapPTR->Type;
    TMI[i].PhysicalStart = Memory.Info.MapPTR->PhysicalStart;
    TMI[i].NumberOfPages = Memory.Info.MapPTR->NumberOfPages;
    Memory.Info.MapPTR = (EFI_MEMORY_DESCRIPTOR*)((UINT8*)Memory.Info.MapPTR + Memory.DescriptorSize);
  }
  gBS->FreePool(Memory.Map);

  // ------------Loader Clearance Area-------------
  GetMemoryMap(&Status, &Memory);
  Status = gBS->ExitBootServices(
    ImageHandle,
    Memory.MapKey
  );
  if (EFI_ERROR(Status)) {
    GetMemoryMap(&Status, &Memory);
    Status = gBS->ExitBootServices(ImageHandle, Memory.MapKey);
    if (EFI_ERROR(Status)) {
      Print(L"[ERROR] Failed to exit UEFI environment. Error Code: %r\r\nSolution: Try rebooting your system. If the issue persists, recommend updating your UEFI firmware.\r\nIt is safe to shut down by pressing the power button.", Status);
      CPU_HALT;
    }
  }

  JumpToKernel(TGI, TMI);
  return EFI_SUCCESS;
}

// GetMemoryMap is often used redundantly, so it has been separated into a separate function.
void GetMemoryMap(EFI_STATUS* Status, MemoryMapSpec* Memory) {
  *Status = gBS->GetMemoryMap(
    &Memory->MapSize,
    NULL,
    &Memory->MapKey,
    &Memory->DescriptorSize,
    &Memory->DescriptorVersion
  );
  if (*Status != EFI_BUFFER_TOO_SMALL) {
    Print(L"[ERROR]\r\nFailed to determine the size of memory required to hold all memory information.\r\nSolution: Try rebooting your system. If the issue persists, recommend updating your UEFI firmware.\r\nIt is safe to shut down by pressing the power button.", Status);
    CPU_HALT;
  }

  *Status = gBS->AllocatePool(
    EfiLoaderData,
    Memory->MapSize,
    (VOID **)&Memory->Map
  );
  if (EFI_ERROR(*Status)) {
    Print(L"[ERROR]\r\nFailed to allocate memory space to hold full memory information.\r\nSolution: Try rebooting your system. If the issue persists, recommend updating your UEFI firmware.\r\nIt is safe to shut down by pressing the power button.", Status);
    CPU_HALT;
  }

  *Status = gBS->GetMemoryMap(
    &Memory->MapSize,
    Memory->Map,
    &Memory->MapKey,
    &Memory->DescriptorSize,
    &Memory->DescriptorVersion
  );
  if (EFI_ERROR(*Status)) {
    Print(L"[ERROR]\r\nFailed to load full memory information.\r\nSolution: Try rebooting your system. If the issue persists, recommend updating your UEFI firmware.\r\nIt is safe to shut down by pressing the power button. [1/2]", Status);
    CPU_HALT;
  }
}