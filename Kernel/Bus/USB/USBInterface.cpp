/*
 * Copyright (c) 2022, blackcat <b14ckcat@protonmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <Kernel/Bus/USB/USBClasses.h>
#include <Kernel/Bus/USB/USBConfiguration.h>
#include <Kernel/Bus/USB/USBInterface.h>
#include <Kernel/Bus/USB/USBMassStorageClass.h>
#include <Kernel/Storage/StorageManagement.h>
#include <Kernel/Storage/USB/USBMassStorageDevice.h>

namespace Kernel::USB
{

// Temporary function to load drivers for certain USB device classes
ErrorOr<void> USBInterface::load_drivers()
{
    auto device_descriptor = m_configuration.device().device_descriptor();

    // QEMU flakiness
    // Mass storage class workaround
    if (device_descriptor.vendor_id == USB_MSC_QEMU_VID &&
        device_descriptor.product_id == USB_MSC_QEMU_PID) {
        if constexpr (USB_DEBUG)
            dbgln("QEMU virtual MSC device detected.");
        auto msc_device = TRY(USBMassStorageDevice::create(m_configuration.device()));
        StorageManagement::the().attach_hotplug_device(msc_device);
    }

    // Mass storage class
    else if (m_descriptor.interface_class_code == USB_CLASS_MASS_STORAGE &&
        m_descriptor.interface_sub_class_code == USB_MSC_SUBCLASS_SCSI_TRANSPARENT_COMMAND_SET &&
	m_descriptor.interface_protocol == USB_MSC_PROTOCOL_BULK_ONLY) {
        if constexpr (USB_DEBUG)
            dbgln("MSC device detected.");
    }

    return {};
}

}
