/*
 * Copyright (c) 2022, blackcat <b14ckcat@protonmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <Kernel/Bus/USB/USBClasses.h>
#include <Kernel/Bus/USB/USBConfiguration.h>
#include <Kernel/Bus/USB/USBInterface.h>
#include <Kernel/Bus/USB/USBMassStorageClass.h>
#include <Kernel/Storage/USB/USBMassStorageDevice.h>

namespace Kernel::USB
{

// Temporary function to load drivers for certain USB device classes
void USBInterface::load_drivers()
{
    // QEMU flakiness
    // Mass storage class workaround
    auto m_device_descriptor = m_configuration.device().device_descriptor();
    if (m_device_descriptor.vendor_id == USB_MSC_QEMU_VID &&
        m_device_descriptor.product_id == USB_MSC_QEMU_PID) {
            dbgln("Testing testing");
	}

    // Mass storage class
    if (m_descriptor.interface_class_code == USB_CLASS_MASS_STORAGE &&
        m_descriptor.interface_sub_class_code == USB_MSC_SUBCLASS_SCSI_TRANSPARENT_COMMAND_SET &&
	m_descriptor.interface_protocol == USB_MSC_PROTOCOL_BULK_ONLY) {
            dbgln("Testing testing");
    }
}

}
