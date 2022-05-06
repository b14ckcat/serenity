/*
 * Copyright (c) 2022, blackcat <b14ckcat@protonmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/Error.h>
#include <Kernel/Bus/USB/PacketTypes.h>
#include <Kernel/Bus/USB/USBController.h>
#include <Kernel/Bus/USB/USBDevice.h>
#include <Kernel/Bus/USB/USBMassStorageClass.h>

namespace Kernel::USB {

MassStorageHandle::MassStorageHandle(Device const& usb_device, USBInterface const& usb_interface, OwnPtr<Pipe> bulk_in, OwnPtr<Pipe> bulk_out)
    : m_usb_device(usb_device)
    , m_usb_interface(usb_interface)
    , m_bulk_in(move(bulk_in))
    , m_bulk_out(move(bulk_out))
{
    
}

ErrorOr<u8> MassStorageHandle::get_max_lun() {
    u8 buf[1];
    auto transfer_length = TRY(const_cast<Device&>(m_usb_device).control_transfer(0xA1, USB_MSC_REQUEST_BULK_ONLY_GET_MAX_LUN, 0, m_usb_interface.get_id(), 1, buf));
    if (!transfer_length)
        return EPROTO;
    if constexpr (USB_DEBUG)
        dbgln("Max LUN: {}", buf[0]);
    return buf[0];
}

}
