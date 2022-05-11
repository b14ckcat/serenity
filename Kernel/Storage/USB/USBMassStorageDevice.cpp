/*
 * Copyright (c) 2022, blackcat <b14ckcat@protonmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/Try.h>
#include <AK/StringView.h>
#include <Kernel/Bus/USB/USBController.h>
#include <Kernel/Bus/USB/USBDevice.h>
#include <Kernel/Bus/USB/USBPipe.h>
#include <Kernel/Devices/DeviceManagement.h>
#include <Kernel/FileSystem/OpenFileDescription.h>
#include <Kernel/Storage/StorageManagement.h>
#include <Kernel/Storage/USB/SCSI.h>
#include <Kernel/Storage/USB/USBMassStorageDevice.h>

namespace Kernel {

ErrorOr<NonnullRefPtr<USBMassStorageDevice>> USBMassStorageDevice::create(OwnPtr<USB::MassStorageHandle> usb_msc_handle)
{
    auto minor_number = StorageManagement::generate_storage_minor_number();
    auto device_name = MUST(KString::formatted("USBMassStorageDevice"));

    auto max_lun = TRY(usb_msc_handle->get_max_lun());
    for (int i = 0; i <= max_lun; i++) {
        constexpr u8 inquiry_size = 36; // Unwritten standard for inquiry data size
        u8 buf[inquiry_size + 1] = {'\0'};
        TRY(usb_msc_handle->try_scsi_command<CommandDescriptorBlock6>(CDB_INQUIRY, 0, USB::Pipe::Direction::In, inquiry_size, inquiry_size+1, buf));
        dbgln("Data in: {}", buf);

        memset(buf, '\0', inquiry_size+1);
	constexpr u8 read_cap_size = 8;
        TRY(usb_msc_handle->try_scsi_command<CommandDescriptorBlock10>(CDB_READ_CAPACITY, 0, USB::Pipe::Direction::In, read_cap_size, inquiry_size+1, buf));
        dbgln("Data in: {}", buf);
    }

    auto device_or_error = DeviceManagement::try_create_device<USBMassStorageDevice>(move(usb_msc_handle), minor_number, 512, 1, move(device_name));
    VERIFY(!device_or_error.is_error());
    return device_or_error.release_value();
}

USBMassStorageDevice::USBMassStorageDevice(OwnPtr<USB::MassStorageHandle> usb_msc_handle, MinorNumber minor_number, size_t sector_size, u64 disk_size, NonnullOwnPtr<KString> device_name)
    : StorageDevice(StorageManagement::storage_type_major_number(), minor_number, sector_size, sector_size / disk_size, move(device_name))
    , m_usb_msc_handle(move(usb_msc_handle))
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
