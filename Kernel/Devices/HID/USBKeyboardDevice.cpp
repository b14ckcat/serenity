/*
 * Copyright (c) 2022, blackcat <b14ckcat@protonmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <Kernel/Devices/HID/USBKeyboardDevice.h>

ErrorOr<NonnullLockRefPtr<USBKeyboardDevice>> USBKeyboardDevice::create(NonnullLockRefPtr<USB::Device> device, USB::USBHIDDescriptor hid_descriptor, NonnullLockRefPtr<USB::Transfer> transfer)
{
    auto keyboard_device = TRY(DeviceManagement::try_create_device<USBKeyboardDevice>(device, hid_descriptor, transfer));
    return keyboard_device;
}

USBKeyboardDevice::USBKeyboardDevice(NonnullLockRefPtr<USB::Device> device, USB::USBHIDDescriptor hid_descriptor, NonnullLockRefPtr<USB::Transfer> transfer)
    : KeyboardDevice()
    , m_device(device)
    , m_hid_descriptor(hid_descriptor)
    , m_transfer(transfer)
{
}

void USBKeyboardDevice::handle_key_press(u8 key)
{
    dbgln("KEY: {}", key);
    key_state_changed(key, true);
}
