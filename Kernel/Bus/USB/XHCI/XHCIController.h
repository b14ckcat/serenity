/*
 * Copyright (c) 2022, blackcat <b14ckcat@protonmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <Kernel/Bus/PCI/Device.h>
#include <Kernel/Bus/USB/USBController.h>
#include <Kernel/IOWindow.h>
#include <Kernel/Interrupts/IRQHandler.h>

namespace Kernel::USB {

class XHCIController final
    : public USBController
    , public PCI::Device
    , public IRQHandler {
public:
    static ErrorOr<NonnullLockRefPtr<XHCIController>> create(PCI::DeviceIdentifier const& pci_device_identifier);
    virtual ~XHCIController() override;

    virtual StringView purpose() const override { return "XHCI"sv; }

    virtual ErrorOr<void> initialize() override;
    virtual ErrorOr<void> reset() override;
    virtual ErrorOr<void> stop() override;
    virtual ErrorOr<void> start() override;

    virtual ErrorOr<size_t> submit_control_transfer(Transfer& transfer) override;
    virtual ErrorOr<size_t> submit_bulk_transfer(Transfer& transfer) override;

private:
    XHCIController(PCI::DeviceIdentifier const& pci_device_identifier, NonnullOwnPtr<IOWindow> registers_io_window);

    virtual bool handle_irq(RegisterState const&) override;

    NonnullOwnPtr<IOWindow> m_registers_io_window;
};

}
