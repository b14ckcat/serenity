/* 
 * Copyright (c) 2022, blackcat <b14ckcat@protonmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <Kernel/Bus/USB/USBController.h>
#include <Kernel/Bus/USB/USBDescriptors.h>
#include <Kernel/Bus/USB/USBDevice.h>
#include <Kernel/Bus/USB/USBPipe.h>
#include <Kernel/Bus/USB/USBTransfer.h>
#include <Kernel/Devices/DeviceManagement.h>
#include <Kernel/Devices/HID/KeyboardDevice.h>

namespace Kernel {

class USBKeyboardDevice final : public KeyboardDevice {
    friend class DeviceManagement;
public:
    static ErrorOr<NonnullLockRefPtr<USBKeyboardDevice>> create(NonnullLockRefPtr<USB::Device> device, USB::USBHIDDescriptor hid_descriptor, NonnullLockRefPtr<USB::Transfer> transfer);

    virtual void enable_interrupts() override {}

    void handle_key_press(u8 key);

private:
    USBKeyboardDevice(NonnullLockRefPtr<USB::Device> device, USB::USBHIDDescriptor hid_descriptor, NonnullLockRefPtr<USB::Transfer> transfer);

    NonnullLockRefPtr<USB::Device> m_device;
    USB::USBHIDDescriptor m_hid_descriptor;
    NonnullLockRefPtr<USB::Transfer> m_transfer;
};

}

