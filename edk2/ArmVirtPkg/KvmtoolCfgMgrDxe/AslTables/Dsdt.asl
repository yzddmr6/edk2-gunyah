/** @file
  Differentiated System Description Table Fields (DSDT)

  Copyright (c) 2021 - 2022, ARM Ltd. All rights reserved.<BR>
    SPDX-License-Identifier: BSD-2-Clause-Patent

**/

DefinitionBlock ("DsdtTable.aml", "DSDT", 2, "ARMLTD", "ARM-KVMT", 1) {
  Scope (_SB) {
    //
    // Most ACPI tables for Kvmtool firmware are
    // dynamically generated. The AML code is also
    // generated at runtime for most components in
    // appropriate SSDTs.
    // Although there may not be much to describe
    // in the DSDT, the DSDT table is mandatory.
    // Therefore, add an empty stub for DSDT.
    //
  } // Scope (_SB)

  //
  // System sleep state S5 (soft off). Required by an ACPI-only guest
  // (Windows-on-ARM) so it knows the SLP_TYP value to write to the FADT
  // SLEEP_CONTROL_REG when powering off. SLP_TYPa = 0 matches the crosvm
  // PmReset device, which powers the VM off (VmEventType::Exit) on an S5
  // (SLP_TYP == 0) write to SLEEP_CONTROL_REG.
  //
  Name (\_S5, Package (0x02) {
    0x00, // PM1a_CNT.SLP_TYP
    0x00, // PM1b_CNT.SLP_TYP
  })
}
