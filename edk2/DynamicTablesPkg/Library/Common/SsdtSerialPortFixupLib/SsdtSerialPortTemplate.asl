/** @file
  SSDT Serial Template

  Copyright (c) 2019 - 2024, Arm Limited. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

  @par Reference(s):
  - Arm Server Base Boot Requirements (SBBR), s4.2.1.8 "SPCR".

  @par Glossary:
    - {template} - Data fixed up using AML Fixup APIs.
    - {codegen}  - Data generated using AML Codegen APIs.
**/

DefinitionBlock ("SsdtSerialPortTemplate.aml", "SSDT", 2, "ARMLTD", "SERIAL", 1) {
  Scope (_SB) {
    // UART PL011
    Device (COM0) {                                       // {template}
      Name (_UID, 0x0)                                    // {template}
      Name (_HID, "HID0000")                              // {template}
      Name (_CID, "CID0000")                              // {template}

      Method(_STA) {
        Return(0xF)
      }

      // Device-properties _DSD. Real PL011 platforms (e.g. Raspberry Pi UEFI)
      // carry one; Windows' SerCx2/SerPL011 queries the ACPI device properties
      // at start, and a device with no _DSD fails that query
      // (STATUS_ACPI_INVALID_DATA) and never starts.
      Name (_DSD, Package () {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package () {
          Package (2) { "SerCx-FriendlyName", "UART0" }
        }
      })

      // This QWordMemory is a placeholder only, present so the ResourceTemplate
      // buffer is well-formed (it must carry an EndTag for AmlLib). FixupCrs()
      // deletes it and re-emits the register block as a Memory32Fixed descriptor:
      // Windows' SerCx2/SerPL011 rejects the QWordMemory form (it reaches the
      // driver with an extra CmResourceTypeMemoryLarge companion, failing start
      // with STATUS_ACPI_INVALID_DATA), whereas real PL011 ACPI (RPi, QEMU virt)
      // uses Memory32Fixed. The Interrupt is likewise generated via AmlCodegen.
      Name (_CRS, ResourceTemplate() {
        QWordMemory (
          , , , , , ReadWrite,
          0x0,                // AddressGranularity
          0xA0000000,         // AddressMinimum
          0xAFFFFFFF,         // AddressMaximum
          0,                  // AddressTranslation
          0x10000000          // RangeLength
        )
      }) // Name
    } // Device
  } // Scope (_SB)
}
