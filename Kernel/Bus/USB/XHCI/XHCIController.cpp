/*
 * Copyright (c) 2022, blackcat <b14ckcat@protonmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <Kernel/Bus/PCI/API.h>
#include <Kernel/Bus/USB/XHCI/XHCIController.h>

namespace Kernel::USB {

ErrorOr<NonnullLockRefPtr<XHCIController>> XHCIController::create(PCI::DeviceIdentifier const& pci_device_identifier)
{
    auto registers_io_window = TRY(IOWindow::create_for_pci_device_bar(pci_device_identifier, PCI::HeaderType0BaseRegister::BAR0));
    auto controller = TRY(adopt_nonnull_lock_ref_or_enomem(new (nothrow) XHCIController(pci_device_identifier, move(registers_io_window))));

    dmesgln("XHCI: Controller found {} @ {}", PCI::get_hardware_id(controller->pci_address()), controller->pci_address());
    dmesgln("XHCI: I/O base {}", controller->m_registers_io_window);
    dmesgln("XHCI: Interrupt line: {}", controller->interrupt_number());

    TRY(controller->reset());
    TRY(controller->start());

    return controller;
}

UNMAP_AFTER_INIT XHCIController::XHCIController(PCI::DeviceIdentifier const& pci_device_identifier, NonnullOwnPtr<IOWindow> registers_io_window)
    : PCI::Device(pci_device_identifier.address())
    , IRQHandler(pci_device_identifier.interrupt_line().value())
    , m_registers_io_window(move(registers_io_window))
{
}

UNMAP_AFTER_INIT XHCIController::~XHCIController() = default;

ErrorOr<void> XHCIController::initialize()
{

    return {};
}

ErrorOr<void> XHCIController::reset()
{

    return {};
}

ErrorOr<void> XHCIController::stop()
{

    return {};
}

ErrorOr<void> XHCIController::start()
{

    return {};
}

ErrorOr<size_t> XHCIController::submit_control_transfer(Transfer& transfer)
{
    (void)transfer;
    return 0;
}

ErrorOr<size_t> XHCIController::submit_bulk_transfer(Transfer& transfer)
{
    (void)transfer;
    return 0;
}

bool XHCIController::handle_irq(RegisterState const&)
{

    return false;
}

};
