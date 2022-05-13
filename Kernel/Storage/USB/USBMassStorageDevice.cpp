/*
 * Copyright (c) 2022, blackcat <b14ckcat@protonmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/BitCast.h>
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

    auto scsi_metadata = get_metadata(usb_msc_handle);

    auto device_or_error = DeviceManagement::try_create_device<USBMassStorageDevice>(move(usb_msc_handle), minor_number, 512, 1, move(device_name));
    VERIFY(!device_or_error.is_error());
    return device_or_error.release_value();
}

USBMassStorageDevice::USBMassStorageDevice(OwnPtr<USB::MassStorageHandle> usb_msc_handle, MinorNumber minor_number, u32 sector_size, u32 num_sectors, NonnullOwnPtr<KString> device_name)
    : StorageDevice(StorageManagement::storage_type_major_number(), minor_number, sector_size, num_sectors, move(device_name))
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

ErrorOr<OwnPtr<SCSIMetadata>> USBMassStorageDevice::get_metadata(OwnPtr<USB::MassStorageHandle> &usb_msc_handle)
{
    auto metadata = TRY(adopt_nonnull_own_or_enomem(new SCSIMetadata()));

    // Get number of logical units on device, only LUN 0 supported currently
    metadata->max_lun = TRY(usb_msc_handle->get_max_lun());
    if (metadata->max_lun != 0)
        dbgln("Multi-LUN USB MSC device detected. Only LUN 0 is currently supported.");

    // Retrieve the name/label of the logical unit
    InquiryResponse inq_res;
    TRY(usb_msc_handle->try_scsi_command<CommandDescriptorBlock6>(CDB_INQUIRY, 0, USB::Pipe::Direction::In, sizeof(InquiryResponse), &inq_res));
    memcpy(metadata->vendor_id, inq_res.vendor_id, 8);
    memcpy(metadata->product_id, inq_res.product_id, 16);

    // Read capacity = get sector size & number of sectors on logical unit
    ReadCapacityResponse cap_res;
    TRY(usb_msc_handle->try_scsi_command<CommandDescriptorBlock10>(CDB_READ_CAPACITY, 0, USB::Pipe::Direction::In, sizeof(ReadCapacityResponse), &cap_res));
    metadata->sector_size = AK::convert_between_host_and_big_endian(cap_res.sector_size);
    metadata->num_sectors = AK::convert_between_host_and_big_endian(cap_res.num_sectors);

    return metadata;
}

}
