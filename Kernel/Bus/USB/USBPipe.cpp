/*
 * Copyright (c) 2021, Jesse Buhagiar <jooster669@gmail.com>
 * Copyright (c) 2022, blackcat <b14ckcat@protonmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/StdLibExtras.h>
#include <Kernel/Bus/USB/PacketTypes.h>
#include <Kernel/Bus/USB/UHCI/UHCIController.h>
#include <Kernel/Bus/USB/USBPipe.h>
#include <Kernel/Bus/USB/USBTransfer.h>

namespace Kernel::USB {

Pipe::Pipe(USBController const& controller, Type type, Direction direction, u8 endpoint_address, u16 max_packet_size, i8 device_address, NonnullOwnPtr<Memory::BufferPool> buffer_pool)
    : m_controller(controller)
    , m_type(type)
    , m_direction(direction)
    , m_device_address(device_address)
    , m_endpoint_address(endpoint_address)
    , m_max_packet_size(max_packet_size)
    , m_data_toggle(false)
    , m_buffer_pool(move(buffer_pool))
{
}

ErrorOr<NonnullOwnPtr<ControlPipe>> ControlPipe::create(USBController const& controller, u8 endpoint_address, u16 max_packet_size, i8 device_address)
{
    auto buffer_pool = TRY(Memory::BufferPool::create(16, 64, Memory::BufferPool::Type::DMA));
    return adopt_nonnull_own_or_enomem(new (nothrow) ControlPipe(controller, endpoint_address, max_packet_size, device_address, move(buffer_pool)));
}

ControlPipe::ControlPipe(USBController const& controller, u8 endpoint_address, u16 max_packet_size, i8 device_address, NonnullOwnPtr<Memory::BufferPool> buffer_pool)
    : Pipe(controller, Type::Control, Direction::Bidirectional, endpoint_address, max_packet_size, device_address, move(buffer_pool))
{
}

ErrorOr<size_t> ControlPipe::control_transfer(u8 request_type, u8 request, u16 value, u16 index, u16 length, void* data)
{
    USBRequestData usb_request;

    usb_request.request_type = request_type;
    usb_request.request = request;
    usb_request.value = value;
    usb_request.index = index;
    usb_request.length = length;

    VirtualAddress vaddr; 
    PhysicalAddress paddr;
    TRY(m_buffer_pool->take(vaddr, paddr));
    auto transfer = TRY(Transfer::try_create(*this, length, vaddr, paddr));
    transfer->set_setup_packet(usb_request);

    dbgln_if(USB_DEBUG, "Pipe: Transfer allocated @ {}", transfer->buffer_physical());
    auto transfer_length = TRY(m_controller->submit_control_transfer(*transfer));

    // TODO: Check transfer for completion and copy data from transfer buffer into data
    if (length > 0)
        memcpy(reinterpret_cast<u8*>(data), transfer->buffer().as_ptr() + sizeof(USBRequestData), length);

    dbgln_if(USB_DEBUG, "Pipe: Control Transfer complete!");
    m_buffer_pool->release(vaddr);
    return transfer_length;
}

ErrorOr<NonnullOwnPtr<BulkInPipe>> BulkInPipe::create(USBController const& controller, u8 endpoint_address, u16 max_packet_size, i8 device_address)
{
    auto buffer_pool = TRY(Memory::BufferPool::create(16, 64, Memory::BufferPool::Type::DMA));
    return adopt_nonnull_own_or_enomem(new (nothrow) BulkInPipe(controller, endpoint_address, max_packet_size, device_address, move(buffer_pool)));
}

BulkInPipe::BulkInPipe(USBController const& controller, u8 endpoint_address, u16 max_packet_size, i8 device_address, NonnullOwnPtr<Memory::BufferPool> buffer_pool)
    : Pipe(controller, Pipe::Type::Bulk, Pipe::Direction::In, endpoint_address, max_packet_size, device_address, move(buffer_pool))
{
}

ErrorOr<size_t> BulkInPipe::bulk_in_transfer(u16 length, void* data)
{
    size_t transfer_length = 0;
    VirtualAddress vaddr; 
    PhysicalAddress paddr;
    TRY(m_buffer_pool->take(vaddr, paddr));
    auto transfer = TRY(Transfer::try_create(*this, length, vaddr, paddr));

    dbgln_if(USB_DEBUG, "Pipe: Bulk in transfer allocated @ {}", transfer->buffer_physical());
    transfer_length = TRY(m_controller->submit_bulk_transfer(*transfer));
    memcpy(data, transfer->buffer().as_ptr(), min(length, transfer_length));
    dbgln_if(USB_DEBUG, "Pipe: Bulk in transfer complete!");

    m_buffer_pool->release(vaddr);
    return transfer_length;
}

ErrorOr<NonnullOwnPtr<BulkOutPipe>> BulkOutPipe::create(USBController const& controller, u8 endpoint_address, u16 max_packet_size, i8 device_address)
{
    auto buffer_pool = TRY(Memory::BufferPool::create(16, 64, Memory::BufferPool::Type::DMA));
    return adopt_nonnull_own_or_enomem(new (nothrow) BulkOutPipe(controller, endpoint_address, max_packet_size, device_address, move(buffer_pool)));
}

BulkOutPipe::BulkOutPipe(USBController const& controller, u8 endpoint_address, u16 max_packet_size, i8 device_address, NonnullOwnPtr<Memory::BufferPool> buffer_pool)
    : Pipe(controller, Pipe::Type::Bulk, Pipe::Direction::Out, endpoint_address, max_packet_size, device_address, move(buffer_pool))
{
}

ErrorOr<size_t> BulkOutPipe::bulk_out_transfer(u16 length, void* data)
{
    size_t transfer_length = 0;
    VirtualAddress vaddr; 
    PhysicalAddress paddr;
    TRY(m_buffer_pool->take(vaddr, paddr));
    auto transfer = TRY(Transfer::try_create(*this, length, vaddr, paddr));

    TRY(transfer->write_buffer(length, data));
    dbgln_if(USB_DEBUG, "Pipe: Bulk out transfer allocated @ {}", transfer->buffer_physical());
    transfer_length = TRY(m_controller->submit_bulk_transfer(*transfer));
    dbgln_if(USB_DEBUG, "Pipe: Bulk out transfer complete!");

    m_buffer_pool->release(vaddr);
    return transfer_length;
}

ErrorOr<NonnullOwnPtr<InterruptInPipe>> InterruptInPipe::create(USBController const& controller, u8 endpoint_address, u16 max_packet_size, i8 device_address, u8 poll_interval)
{
    auto buffer_pool = TRY(Memory::BufferPool::create(16, 64, Memory::BufferPool::Type::DMA));
    return adopt_nonnull_own_or_enomem(new (nothrow) InterruptInPipe(controller, endpoint_address, max_packet_size, poll_interval, device_address, move(buffer_pool)));
}

InterruptInPipe::InterruptInPipe(USBController const& controller, u8 endpoint_address, u16 max_packet_size, i8 device_address, u8 poll_interval, NonnullOwnPtr<Memory::BufferPool> buffer_pool)
    : Pipe(controller, Pipe::Type::Interrupt, Pipe::Direction::In, endpoint_address, max_packet_size, device_address, move(buffer_pool))
    , m_poll_interval(poll_interval)
{
}

ErrorOr<NonnullOwnPtr<InterruptOutPipe>> InterruptOutPipe::create(USBController const& controller, u8 endpoint_address, u16 max_packet_size, i8 device_address, u8 poll_interval)
{
    auto buffer_pool = TRY(Memory::BufferPool::create(16, 64, Memory::BufferPool::Type::DMA));
    return adopt_nonnull_own_or_enomem(new (nothrow) InterruptOutPipe(controller, endpoint_address, max_packet_size, poll_interval, device_address, move(buffer_pool)));

}

InterruptOutPipe::InterruptOutPipe(USBController const& controller, u8 endpoint_address, u16 max_packet_size, i8 device_address, u8 poll_interval, NonnullOwnPtr<Memory::BufferPool> buffer_pool)
    : Pipe(controller, Pipe::Type::Interrupt, Pipe::Direction::Out, endpoint_address, max_packet_size, device_address, move(buffer_pool))
    , m_poll_interval(poll_interval)
{
}

}
