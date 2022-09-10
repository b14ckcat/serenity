/*
 * Copyright (c) 2022, blackcat <b14ckcat@protonmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <Kernel/Bus/USB/USBController.h>
#include <Kernel/Bus/USB/USBDevice.h>
#include <Kernel/Bus/USB/USBPipe.h>
#include <Kernel/Bus/USB/USBTransfer.h>
#include <Kernel/Devices/DeviceManagement.h>
#include <Kernel/Devices/HID/KeyboardDevice.h>

namespace Kernel {

class USBKeyboardDevice final : public KeyboardDevice {
    friend class DeviceManagement;
public:
    static ErrorOr<NonnullLockRefPtr<USBKeyboardDevice>> create(USB::Device const& device, NonnullOwnPtr<USB::InterruptInPipe> int_pipe);

    virtual void enable_interrupts() override {}

    static void handle_usb_interrupt_transfer(USB::Transfer *transfer);

private:
    USBKeyboardDevice(USB::Device const& device, NonnullOwnPtr<USB::InterruptInPipe> int_pipe);

    USB::Device const& m_device;
    NonnullOwnPtr<USB::InterruptInPipe> m_int_pipe;
};

}
