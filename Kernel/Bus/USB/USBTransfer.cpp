/*
 * Copyright (c) 2021, Jesse Buhagiar <jooster669@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <Kernel/Bus/USB/USBTransfer.h>
#include <Kernel/Memory/MemoryManager.h>

namespace Kernel::USB {

ErrorOr<NonnullLockRefPtr<Transfer>> Transfer::try_create(Pipe& pipe, u16 length, VirtualAddress buffer_vaddr, PhysicalAddress buffer_paddr, usb_async_callback completion_callback)
{
    return adopt_nonnull_lock_ref_or_enomem(new (nothrow) Transfer(pipe, length, buffer_vaddr, buffer_paddr, completion_callback));
}

Transfer::Transfer(Pipe& pipe, u16 len, VirtualAddress buffer_vaddr, PhysicalAddress buffer_paddr, usb_async_callback completion_callback)
    : m_pipe(pipe)
    , m_transfer_data_size(len)
    , m_buffer_vaddr(buffer_vaddr)
    , m_buffer_paddr(buffer_paddr)
    , m_completion_callback(completion_callback)
{
}

void Transfer::set_setup_packet(USBRequestData const& request)
{
    // Kind of a nasty hack... Because the kernel isn't in the business
    // of handing out physical pointers that we can directly write to,
    // we set the address of the setup packet to be the first 8 bytes of
    // the data buffer, which we then set to the physical address.
    auto* request_data = reinterpret_cast<USBRequestData*>(buffer().as_ptr());

    request_data->request_type = request.request_type;
    request_data->request = request.request;
    request_data->value = request.value;
    request_data->index = request.index;
    request_data->length = request.length;

    m_request = request;
}

ErrorOr<void> Transfer::write_buffer(u16 len, void* data)
{
    m_transfer_data_size = len;
    memcpy(buffer().as_ptr(), data, len);

    return {};
}

void Transfer::invoke_async_callback()
{
    if (m_completion_callback != nullptr)
        m_completion_callback();
    // TODO: Make this impossible without having to subclass and cast everywhere
    else
        dbgln_if(USB_DEBUG, "Error: attempted to invoke callback of synchronous transfer");
}

}
