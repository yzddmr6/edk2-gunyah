/** @file
  ARM FADT Table Helpers

  Copyright (c) 2017 - 2023, Arm Limited. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent

  @par Reference(s):
  - ACPI 6.5 Specification, Aug 29, 2022

**/

#include <Library/AcpiLib.h>
#include <Library/DebugLib.h>
#include <Protocol/AcpiTable.h>

// Module specific include files.
#include <AcpiTableGenerator.h>
#include <ConfigurationManagerObject.h>
#include <ConfigurationManagerHelper.h>
#include <Library/TableHelperLib.h>
#include <Protocol/ConfigurationManagerProtocol.h>
#include "FadtGenerator.h"

/** ARM Standard FADT Generator

Requirements:
  The following Configuration Manager Object(s) are required by
  this Generator:
  - EArmObjBootArchInfo
*/

/** This macro expands to a function that retrieves the Boot
    Architecture Information from the Configuration Manager.
*/
GET_OBJECT_LIST (
  EObjNameSpaceArm,
  EArmObjBootArchInfo,
  CM_ARM_BOOT_ARCH_INFO
  );

/** This macro defines the FADT flag options for ARM Platforms.

    RESET_REG_SUP is set because DroidVM advertises an ACPI reset register (see
    below); an ACPI-only guest (Windows-on-ARM) needs it to reboot.
*/
#define FADT_FLAGS  (EFI_ACPI_6_5_HW_REDUCED_ACPI |          \
                     EFI_ACPI_6_5_LOW_POWER_S0_IDLE_CAPABLE | \
                     EFI_ACPI_6_5_RESET_REG_SUP)

//
// DroidVM ACPI power/reset controller MMIO contract. These addresses MUST match
// the crosvm PmReset device (crosvm_mod: aarch64 AARCH64_PMRESET_ADDR = 0x5000,
// devices/src/pmreset.rs). Windows-on-ARM has no PSCI system-off/reset path; it
// drives whole-system power state through these hardware-reduced-ACPI registers:
// S5 soft-off writes SLEEP_CONTROL_REG, restart writes RESET_REG. A Linux guest
// keeps using PSCI and never touches these.
//
#define DROIDVM_PMRESET_SLEEP_CONTROL_ADDR  0x5000
#define DROIDVM_PMRESET_SLEEP_STATUS_ADDR   0x5004
#define DROIDVM_PMRESET_RESET_REG_ADDR      0x5008
#define DROIDVM_PMRESET_RESET_VALUE         0x42

/** Updates the Architecture specific information in the FADT Table.

  @param [in]  CfgMgrProtocol Pointer to the Configuration Manager
                              Protocol Interface.
  @param [in, out] Fadt       Pointer to the constructed ACPI Table.

  @retval EFI_SUCCESS           Success.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_NOT_FOUND         The required object was not found.
  @retval EFI_BAD_BUFFER_SIZE   The size returned by the Configuration
                                Manager is less than the Object size for the
                                requested object.
**/
STATIC
EFI_STATUS
EFIAPI
ArmFadtBootArchInfoUpdate (
  IN  CONST EDKII_CONFIGURATION_MANAGER_PROTOCOL  *CONST  CfgMgrProtocol,
  IN   OUT EFI_ACPI_6_5_FIXED_ACPI_DESCRIPTION_TABLE      *Fadt
  )
{
  EFI_STATUS             Status;
  CM_ARM_BOOT_ARCH_INFO  *BootArchInfo;

  ASSERT (CfgMgrProtocol != NULL);
  ASSERT (Fadt != NULL);

  // Get the Boot Architecture flags from the Platform Configuration Manager
  Status = GetEArmObjBootArchInfo (
             CfgMgrProtocol,
             CM_NULL_TOKEN,
             &BootArchInfo,
             NULL
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: FADT: Failed to get Boot Architecture flags. Status = %r\n",
      Status
      ));
    return Status;
  }

  DEBUG ((
    DEBUG_INFO,
    "FADT BootArchFlag = 0x%x\n",
    BootArchInfo->BootArchFlags
    ));

  Fadt->ArmBootArch = BootArchInfo->BootArchFlags;

  return Status;
}

/** Updates the Architecture specific information in the FADT Table.

  @param [in]  CfgMgrProtocol Pointer to the Configuration Manager
                              Protocol Interface.
  @param [in, out] Fadt       Pointer to the constructed ACPI Table.

  @retval EFI_SUCCESS           Success.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_NOT_FOUND         The required object was not found.
  @retval EFI_BAD_BUFFER_SIZE   The size returned by the Configuration
                                Manager is less than the Object size for the
                                requested object.
**/
EFI_STATUS
EFIAPI
FadtArchUpdate (
  IN  CONST EDKII_CONFIGURATION_MANAGER_PROTOCOL  *CONST  CfgMgrProtocol,
  IN   OUT EFI_ACPI_6_5_FIXED_ACPI_DESCRIPTION_TABLE      *Fadt
  )
{
  ASSERT (CfgMgrProtocol != NULL);
  ASSERT (Fadt != NULL);

  Fadt->Flags = FADT_FLAGS;

  //
  // Advertise the DroidVM ACPI power/reset controller so an ACPI-only guest
  // (Windows) can power off (S5 via SLEEP_CONTROL_REG / SLEEP_STATUS_REG) and
  // reboot (RESET_REG + ResetValue). All are byte-wide SystemMemory registers
  // backed by the crosvm PmReset MMIO device; RESET_REG_SUP is set in
  // FADT_FLAGS above. Populate field-by-field (no compound literals) to stay
  // clear of the EDK2 warning-as-error build settings.
  //
  Fadt->SleepControlReg.AddressSpaceId    = EFI_ACPI_6_5_SYSTEM_MEMORY;
  Fadt->SleepControlReg.RegisterBitWidth  = 8;
  Fadt->SleepControlReg.RegisterBitOffset = 0;
  Fadt->SleepControlReg.AccessSize        = EFI_ACPI_6_5_BYTE;
  Fadt->SleepControlReg.Address           = DROIDVM_PMRESET_SLEEP_CONTROL_ADDR;

  Fadt->SleepStatusReg.AddressSpaceId    = EFI_ACPI_6_5_SYSTEM_MEMORY;
  Fadt->SleepStatusReg.RegisterBitWidth  = 8;
  Fadt->SleepStatusReg.RegisterBitOffset = 0;
  Fadt->SleepStatusReg.AccessSize        = EFI_ACPI_6_5_BYTE;
  Fadt->SleepStatusReg.Address           = DROIDVM_PMRESET_SLEEP_STATUS_ADDR;

  Fadt->ResetReg.AddressSpaceId    = EFI_ACPI_6_5_SYSTEM_MEMORY;
  Fadt->ResetReg.RegisterBitWidth  = 8;
  Fadt->ResetReg.RegisterBitOffset = 0;
  Fadt->ResetReg.AccessSize        = EFI_ACPI_6_5_BYTE;
  Fadt->ResetReg.Address           = DROIDVM_PMRESET_RESET_REG_ADDR;

  Fadt->ResetValue = DROIDVM_PMRESET_RESET_VALUE;

  return ArmFadtBootArchInfoUpdate (CfgMgrProtocol, Fadt);
}
