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

ErrorOr<u8> MassStorageHandle::get_max_lun() {
    u8 max_lun;
    auto transfer_length = TRY(const_cast<Device&>(m_usb_device).control_transfer(0xA1, USB_MSC_REQUEST_BULK_ONLY_GET_MAX_LUN, 0, m_usb_interface.get_id(), 1, &max_lun));
    if (!transfer_length)
        return EPROTO;
    dbgln_if(USB_DEBUG, "Max LUN: {}", max_lun);
    dbgln_if(USB_DEBUG, "Get max LUN control transfer length: {}", transfer_length);

    return max_lun;
}

}
