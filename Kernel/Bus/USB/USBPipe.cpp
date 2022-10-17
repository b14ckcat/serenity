/*
 * Copyright (c) 2021, Jesse Buhagiar <jooster669@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/StdLibExtras.h>
#include <Kernel/Bus/USB/PacketTypes.h>
#include <Kernel/Bus/USB/UHCI/UHCIController.h>
#include <Kernel/Bus/USB/USBPipe.h>
#include <Kernel/Bus/USB/USBTransfer.h>

namespace Kernel::USB {

ErrorOr<NonnullOwnPtr<Pipe>> Pipe::create(USBController const& controller, Pipe::Type type, Pipe::Direction direction, u8 endpoint_address, u16 max_packet_size, i8 device_address, size_t buffer_size, u16 poll_interval)
{
    auto dma_region = TRY(MM.allocate_dma_buffer_pages(TRY(Memory::page_round_up(buffer_size)), "USB device DMA buffer"sv, Memory::Region::Access::ReadWrite));
    return adopt_nonnull_own_or_enomem(new (nothrow) Pipe(controller, type, direction, endpoint_address, max_packet_size, poll_interval, device_address, move(dma_region)));

}

Pipe::Pipe(USBController const& controller, Type type, Direction direction, u8 endpoint_address, u16 max_packet_size, u16 poll_interval, i8 device_address, NonnullOwnPtr<Memory::Region> dma_buffer)
    : m_controller(controller)
    , m_type(type)
    , m_direction(direction)
    , m_device_address(device_address)
    , m_endpoint_address(endpoint_address)
    , m_max_packet_size(max_packet_size)
    , m_poll_interval(poll_interval)
    , m_data_toggle(false)
    , m_dma_buffer(move(dma_buffer))
{
}

ErrorOr<size_t> Pipe::control_transfer(u8 request_type, u8 request, u16 value, u16 index, size_t length, void* data)
{
    MutexLocker lock(m_dma_buffer_lock);

    USBRequestData usb_request;

    usb_request.request_type = request_type;
    usb_request.request = request;
    usb_request.value = value;
    usb_request.index = index;
    usb_request.length = length;

    auto transfer = TRY(Transfer::create(*this, length, *m_dma_buffer));
    transfer->set_setup_packet(usb_request);

    dbgln_if(USB_DEBUG, "Pipe: Transfer allocated @ {}", transfer->buffer_physical());
    auto transfer_length = TRY(m_controller->submit_control_transfer(*transfer));

    // TODO: Check transfer for completion and copy data from transfer buffer into data
    if (length > 0)
        memcpy(reinterpret_cast<u8*>(data), transfer->buffer().as_ptr() + sizeof(USBRequestData), length);

    dbgln_if(USB_DEBUG, "Pipe: Control Transfer complete!");
    return transfer_length;
}

ErrorOr<size_t> Pipe::bulk_in_transfer(size_t length, void* data)
{
    MutexLocker lock(m_dma_buffer_lock);

    size_t transfer_length = 0;
    auto transfer = TRY(Transfer::create(*this, length, *m_dma_buffer));

    dbgln_if(USB_DEBUG, "Pipe: Bulk in transfer allocated @ {}", transfer->buffer_physical());
    transfer_length = TRY(m_controller->submit_bulk_transfer(*transfer));
    memcpy(data, transfer->buffer().as_ptr(), min(length, transfer_length));
    dbgln_if(USB_DEBUG, "Pipe: Bulk in transfer complete!");

    return transfer_length;
}

ErrorOr<size_t> Pipe::bulk_out_transfer(size_t length, void* data)
{
    MutexLocker lock(m_dma_buffer_lock);

    size_t transfer_length = 0;
    auto transfer = TRY(Transfer::create(*this, length, *m_dma_buffer));

    TRY(transfer->write_buffer(length, data));
    dbgln_if(USB_DEBUG, "Pipe: Bulk out transfer allocated @ {}", transfer->buffer_physical());
    transfer_length = TRY(m_controller->submit_bulk_transfer(*transfer));
    dbgln_if(USB_DEBUG, "Pipe: Bulk out transfer complete!");

    return transfer_length;
}

ErrorOr<NonnullLockRefPtr<Transfer>> Pipe::interrupt_transfer(size_t length, u16 ms_interval, usb_async_callback callback)
{
    VERIFY(length <= m_dma_buffer->size());

    auto transfer = TRY(Transfer::create(*this, length, *m_dma_buffer, move(callback)));
    dbgln_if(USB_DEBUG, "Pipe: Interrupt in transfer allocated @ {}", transfer->buffer_physical());
    TRY(m_controller->submit_async_interrupt_transfer(transfer, ms_interval));
    return transfer;
}

void Pipe::cancel_async_transfer(Transfer const& transfer)
{
    m_controller->cancel_async_transfer(transfer);
}

}
