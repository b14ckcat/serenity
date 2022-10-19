/*
 * Copyright (c) 2022, blackcat <b14ckcat@protonmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <Kernel/Bus/USB/Drivers/HID/USBKeyboardDriver.h>
#include <Kernel/Bus/USB/USBManagement.h>
#include <Kernel/Bus/USB/USBRequest.h>

namespace Kernel::USB
{

static constexpr u16 QEMU_KBD_VID = 0x0627u;
static constexpr u16 QEMU_KBD_PID = 0x0001u;

static AK::Array<Driver::DeviceId, 1> const s_support_device_list = {{
    USB_DEVICE_TABLE_IDENTIFIER(QEMU_KBD_VID, QEMU_KBD_PID)
}};

void USBKeyboardDriver::init(void)
{
    auto driver = adopt_nonnull_ref_or_enomem(new (nothrow) USBKeyboardDriver());
    if (driver.is_error())
    {
        dbgln("USBKeyboardDriver: Failed to create driver!");
        return;
    }

    USBManagement::the().register_driver(driver.release_value());
}

USBKeyboardDriver::USBKeyboardDriver() 
    : Driver("USB HID Driver"sv)
{
    for (auto const& device_id : s_support_device_list)
        m_supported_device_table.append(device_id);
}

USBKeyboardDriver::~USBKeyboardDriver()
{

}

ErrorOr<void> USBKeyboardDriver::probe(USBInterface const& interface)
{
    auto &device = interface.configuration().device();
    auto &non_const_interface = const_cast<USBInterface&>(interface);

    dbgln("Probing for USB keyboard device...");
    auto device_hid_descriptor = TRY(read_hid_descriptor(interface));
    for (size_t i = 0; i < sizeof(device_hid_descriptor); i++) {
        dbgln("{}", ((u8*)&device_hid_descriptor)[i]);
    }
    TRY(non_const_interface.open());
    for (auto const& endpoint: interface.endpoints()) {
        if (endpoint.is_interrupt()) {
	   auto transfer = TRY(non_const_interface.async_read_endpoint(endpoint.endpoint_address(), 8, handle_usb_interrupt_transfer));
	   transfer->m_driver = this;
           auto keyboard = TRY(USBKeyboardDevice::create(device, device_hid_descriptor, transfer));
           m_active_devices.append(keyboard);
	   return {};
	}
    }
    
    return EINVAL;
}

void USBKeyboardDriver::disconnect()
{

}

void USBKeyboardDriver::handle_usb_interrupt_transfer(Driver* driver, USB::Transfer* transfer)
{
    USBKeyboardDriver* driv = reinterpret_cast<USBKeyboardDriver*>(driver);
    u8 key = transfer->buffer().as_ptr()[2];
    driv->m_active_devices[0].handle_key_press(key);
}


ErrorOr<USBHIDDescriptor> USBKeyboardDriver::read_hid_descriptor(USBInterface const& interface)
{
    (void)interface;
    USBHIDDescriptor hid_descriptor;
    TRY(const_cast<Device&>(interface.configuration().device()).control_transfer(USB_REQUEST_TRANSFER_DIRECTION_DEVICE_TO_HOST | USB_REQUEST_RECIPIENT_INTERFACE, USB_REQUEST_GET_DESCRIPTOR, 0x2100, 0, sizeof(hid_descriptor), &hid_descriptor));
    return hid_descriptor;
}

USB_DEVICE_DRIVER(USBKeyboardDriver);

}

