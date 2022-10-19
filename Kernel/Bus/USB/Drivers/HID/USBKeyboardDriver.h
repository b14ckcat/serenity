/* 
 * Copyright (c) 2022, blackcat <b14ckcat@protonmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <Kernel/Bus/USB/Drivers/USBDriver.h>
#include <Kernel/Bus/USB/USBController.h>
#include <Kernel/Bus/USB/USBDescriptors.h>
#include <Kernel/Bus/USB/USBDevice.h>
#include <Kernel/Bus/USB/USBPipe.h>
#include <Kernel/Bus/USB/USBTransfer.h>
#include <Kernel/Devices/DeviceManagement.h>
#include <Kernel/Devices/HID/USBKeyboardDevice.h>

namespace Kernel::USB
{

class USBKeyboardDriver final : public Driver {
    friend class USBKeyboardDevice;
public:
    USBKeyboardDriver();
    ~USBKeyboardDriver();

    virtual ErrorOr<void> probe(USBInterface const& device) override;
    virtual void disconnect() override;
    virtual StringView const& name() const override { return m_driver_name; }

    static void init();

    static void handle_usb_interrupt_transfer(Driver* driver, USB::Transfer* transfer);

private:
    NonnullLockRefPtrVector<USBKeyboardDevice> m_active_devices;
    ErrorOr<USBHIDDescriptor> read_hid_descriptor(USBInterface const& interface);

};

}
