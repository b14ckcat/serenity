/*
 * Copyright (c) 2022, Jesse Buhagiar <jesse.buhagiar@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/Format.h>
#include <Kernel/Bus/USB/USBClasses.h>
#include <Kernel/Bus/USB/USBConfiguration.h>
#include <Kernel/Bus/USB/USBController.h>
#include <Kernel/Bus/USB/USBDescriptors.h>
#include <Kernel/Bus/USB/USBEndpoint.h>
#include <Kernel/Bus/USB/USBInterface.h>
#include <Kernel/Bus/USB/USBPipe.h>
#include <Kernel/Devices/HID/HIDManagement.h>

namespace Kernel::USB {

ErrorOr<void> USBInterface::open()
{
    for (auto const& interface_descriptor : m_endpoint_descriptors) {
        auto endpoint = TRY(USBEndpoint::create(m_configuration.device(), interface_descriptor));
        m_endpoints.append(move(endpoint));
    }

    return {};
}

void USBInterface::close()
{
    m_endpoints.clear();
}

ErrorOr<NonnullLockRefPtr<Transfer>> USBInterface::async_read_endpoint(u8 endpoint_address, size_t count, usb_async_callback callback)
{
    for (auto &endpoint : m_endpoints) {
        if (endpoint.endpoint_address() == endpoint_address) {
            return endpoint.read_async(count, move(callback));
	}
    }

    return EINVAL;
}

}
