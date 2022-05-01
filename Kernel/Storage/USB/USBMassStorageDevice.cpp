/*
 * Copyright (c) 2022, blackcat <b14ckcat@protonmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/StringView.h>
#include <Kernel/Bus/USB/USBDevice.h>
#include <Kernel/Devices/DeviceManagement.h>
#include <Kernel/FileSystem/OpenFileDescription.h>
#include <Kernel/Storage/USB/USBMassStorageDevice.h>

namespace Kernel {

ErrorOr<NonnullRefPtr<USBMassStorageDevice>> USBMassStorageDevice::create(USB::Device const& usb_device)
{
    auto device_name = MUST(KString::formatted("storage device"));
    auto device_or_error = DeviceManagement::try_create_device<USBMassStorageDevice>(usb_device, move(device_name));
    VERIFY(!device_or_error.is_error());
    return device_or_error.release_value();
}

USBMassStorageDevice::USBMassStorageDevice(USB::Device const& usb_device, NonnullOwnPtr<KString> device_name)
    : StorageDevice(0x13, 0x37, 512, 512*8, move(device_name))
    , m_attached_usb_device(usb_device)
{
    
}

USBMassStorageDevice::~USBMassStorageDevice() = default;

StringView USBMassStorageDevice::class_name() const
{
    return "USBMassStorageDevice"sv;
}

void USBMassStorageDevice::start_request(AsyncBlockDeviceRequest& request)
{
    if (!request.block_count())
    {
        return;
    }
}

}
