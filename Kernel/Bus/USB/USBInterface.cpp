/*
 * Copyright (c) 2022, blackcat <b14ckcat@protonmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/RefPtr.h>
#include <Kernel/Bus/USB/USBController.h>
#include <Kernel/Bus/USB/USBClasses.h>
#include <Kernel/Bus/USB/USBConfiguration.h>
#include <Kernel/Bus/USB/USBInterface.h>
#include <Kernel/Bus/USB/USBEndpoint.h>
#include <Kernel/Bus/USB/USBPipe.h>
#include <Kernel/Bus/USB/USBMassStorageClass.h>
#include <Kernel/Storage/StorageManagement.h>
#include <Kernel/Storage/USB/USBMassStorageDevice.h>

namespace Kernel::USB
{

// Temporary function to load drivers for certain USB interface classes
ErrorOr<void> USBInterface::load_driver()
{
    auto device_descriptor = m_configuration.device().device_descriptor();

    // Sorry about this huge conditional statement but its necessary to check for QEMU's
    // special case since it doesn't play nicely and use the proper interface descriptor 
    // values that specify USB class
    if ((device_descriptor.vendor_id == USB_MSC_QEMU_VID &&  // QEMU flakiness which needs
        device_descriptor.product_id == USB_MSC_QEMU_PID) || // to be handled manually
	(m_descriptor.interface_class_code == USB_CLASS_MASS_STORAGE &&
        m_descriptor.interface_sub_class_code == USB_MSC_SUBCLASS_SCSI_TRANSPARENT_COMMAND_SET &&
	m_descriptor.interface_protocol == USB_MSC_PROTOCOL_BULK_ONLY)) {

        if constexpr (USB_DEBUG)
            dbgln("MSC device detected.");

	OwnPtr<Pipe> bulk_in, bulk_out;
        for (auto endpoint_desc : m_endpoint_descriptors) {
	    if (endpoint_desc.endpoint_attributes_bitmap & USBEndpoint::ENDPOINT_ATTRIBUTES_TRANSFER_TYPE_BULK) {
                if (endpoint_desc.endpoint_address & USBEndpoint::ENDPOINT_ADDRESS_DIRECTION_IN)
                    bulk_in = TRY(Pipe::try_create_pipe(m_configuration.device().controller(), 
                              Pipe::Type::Control, 
                              Pipe::Direction::In, 
                              endpoint_desc.endpoint_address, 
                              endpoint_desc.max_packet_size, 
                              m_configuration.device().address(), 
                              10));
		else
                    bulk_out = TRY(Pipe::try_create_pipe(m_configuration.device().controller(), 
                               Pipe::Type::Control, 
                               Pipe::Direction::Out, 
                               endpoint_desc.endpoint_address, 
                               endpoint_desc.max_packet_size, 
                               m_configuration.device().address(), 
                               10));
	    }
	}

	auto usb_msc_handle = USBMassStorageHandle(m_configuration.device(), move(bulk_in), move(bulk_out));
        auto usb_mass_storage_device = TRY(USBMassStorageDevice::create(m_configuration.device()));
        StorageManagement::the().attach_hotplug_device(usb_mass_storage_device);
    }

    return {};
}

}
