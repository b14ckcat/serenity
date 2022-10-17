/*
 * Copyright (c) 2022, blackcat <b14ckcat@protonmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <Kernel/Devices/HID/USBKeyboardDevice.h>

ErrorOr<NonnullLockRefPtr<USBKeyboardDevice>> USBKeyboardDevice::create(USB::Device const& device, NonnullOwnPtr<USB::InterruptInPipe> int_pipe)
{
    auto keyboard_device = TRY(DeviceManagement::try_create_device<USBKeyboardDevice>(device, move(int_pipe)));
    return keyboard_device;
}

USBKeyboardDevice::USBKeyboardDevice(USB::Device const& device, NonnullOwnPtr<USB::InterruptInPipe> int_pipe)
    : KeyboardDevice()
    , m_device(device)
    , m_int_pipe(move(int_pipe))
{
    auto res = m_int_pipe->interrupt_in_transfer(8, 10, &USBKeyboardDevice::handle_usb_interrupt_transfer);
    if (res.is_error()) {
        dbgln("Error");
    }
}

void USBKeyboardDevice::handle_usb_interrupt_transfer(USB::Transfer *transfer)
{
    for (int i = 0; i < 8; i++) {
        dbgln("{}", transfer->buffer().as_ptr()[i]);
    }
}

