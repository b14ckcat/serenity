/*
 * Copyright (c) 2022, blackcat <b14ckcat@protonmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/Try.h>
#include <AK/StringView.h>
#include <Kernel/Bus/USB/USBController.h>
#include <Kernel/Bus/USB/USBDevice.h>
#include <Kernel/Devices/DeviceManagement.h>
#include <Kernel/FileSystem/OpenFileDescription.h>
#include <Kernel/Storage/StorageManagement.h>
#include <Kernel/Storage/USB/USBMassStorageDevice.h>
#include <Kernel/Storage/SCSI/SCSI.h>

namespace Kernel {

ErrorOr<NonnullRefPtr<USBMassStorageDevice>> USBMassStorageDevice::create(OwnPtr<USB::MassStorageHandle> usb_msc_handle)
{
    auto minor_number = StorageManagement::generate_storage_minor_number();
    auto device_name = MUST(KString::formatted("USBMassStorageDevice"));

    auto max_lun = TRY(usb_msc_handle->get_max_lun());

    if (max_lun == 0) {
        auto test_unit_ready = CommandDescriptorBlock6 {
            .opcode = SCSI_INQUIRY,
    	    .misc = 0x00,
    	    .logical_block_addr = 0x00,
	    .len = 0,
	    .control = 0x00
	};

	TRY(usb_msc_handle->try_scsi_command<CommandDescriptorBlock6>(test_unit_ready));
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