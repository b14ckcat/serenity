/*
 * Copyright (c) 2022, blackcat <b14ckcat@protonmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/Endian.h>
#include <AK/StringView.h>
#include <AK/Try.h>
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
    dbgln("{}", max_lun);
    for (int i = 0; i <= max_lun; i++) {
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

ErrorOr<void> USBMassStorageDevice::get_metadata(u8 lun)
{
    if (lun) {}

    u8 buf[SCSI_INQUIRY_DATA_LEN+1] = {'\0'};
    TRY(m_usb_msc_handle->try_scsi_command<CommandDescriptorBlock6>(CDB_INQUIRY, 0, USB::Pipe::Direction::In, SCSI_INQUIRY_DATA_LEN, SCSI_INQUIRY_DATA_LEN+1, buf));
    dbgln("Data in: {}", buf);

    memset(buf, 0, SCSI_INQUIRY_DATA_LEN+1);
    TRY(m_usb_msc_handle->try_scsi_command<CommandDescriptorBlock10>(CDB_READ_CAPACITY, 0, USB::Pipe::Direction::In, SCSI_READ_CAPACITY_DATA_LEN, SCSI_INQUIRY_DATA_LEN+1, buf));
    for (int i = 0; i < 8; i++)
        dbgln("{}\n", buf[i]);

    return {};
}

}
