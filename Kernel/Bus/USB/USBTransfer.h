/*
 * Copyright (c) 2021, Jesse Buhagiar <jooster669@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/AtomicRefCounted.h>
#include <AK/OwnPtr.h>
#include <Kernel/Bus/USB/PacketTypes.h>
#include <Kernel/Bus/USB/USBPipe.h>
#include <Kernel/Library/LockRefPtr.h>
#include <Kernel/Memory/AnonymousVMObject.h>
#include <Kernel/Memory/PhysicalPage.h>
#include <Kernel/Memory/Region.h>

namespace Kernel::USB {

class Transfer : public AtomicRefCounted<Transfer> {
public:
    static ErrorOr<NonnullLockRefPtr<Transfer>> try_create(Pipe&, u16 length, VirtualAddress buffer_vaddr, PhysicalAddress buffer_paddr, usb_async_callback completion_callback=nullptr);

    Transfer() = delete;

    void set_setup_packet(USBRequestData const& request);
    void set_complete() { m_complete = true; }
    void set_error_occurred() { m_error_occurred = true; }

    ErrorOr<void> write_buffer(u16 len, void* data);

    // `const` here makes sure we don't blow up by writing to a physical address
    USBRequestData const& request() const { return m_request; }
    Pipe const& pipe() const { return m_pipe; }
    Pipe& pipe() { return m_pipe; }
    VirtualAddress buffer() const { return m_buffer_vaddr; }
    PhysicalAddress buffer_physical() const { return m_buffer_paddr; }
    u16 transfer_data_size() const { return m_transfer_data_size; }
    bool complete() const { return m_complete; }
    bool error_occurred() const { return m_error_occurred; }
    void invoke_async_callback();

private:
    Transfer(Pipe& pipe, u16 len, VirtualAddress buffer_vaddr, PhysicalAddress buffer_paddr, usb_async_callback completion_callback = nullptr);
    Pipe& m_pipe;                    // Pipe that initiated this transfer
    USBRequestData m_request;        // USB request
    u16 m_transfer_data_size { 0 };  // Size of the transfer's data stage
    bool m_complete { false };       // Has this transfer been completed?
    bool m_error_occurred { false }; // Did an error occur during this transfer?
    VirtualAddress m_buffer_vaddr;
    PhysicalAddress m_buffer_paddr;
    usb_async_callback m_completion_callback;
};

}
