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
#include <Kernel/Bus/USB/USBMassStorageClass.h>
#include <Kernel/Bus/USB/USBPipe.h>
#include <Kernel/Devices/DeviceManagement.h>
#include <Kernel/FileSystem/OpenFileDescription.h>
#include <Kernel/Storage/StorageManagement.h>
#include <Kernel/Storage/USB/SCSI.h>
#include <Kernel/Storage/USB/USBMassStorageDevice.h>

namespace Kernel {

using namespace USB;

ErrorOr<NonnullRefPtr<USBMassStorageDevice>> USBMassStorageDevice::create(OwnPtr<USB::MassStorageHandle> usb_msc_handle)
{
    auto minor_number = StorageManagement::generate_storage_minor_number();
    auto device_name = MUST(KString::formatted("USBMassStorageDevice"));

    auto metadata = TRY(get_metadata(usb_msc_handle));

    auto device_or_error = DeviceManagement::try_create_device<USBMassStorageDevice>(move(usb_msc_handle), minor_number, move(metadata), move(device_name));
    VERIFY(!device_or_error.is_error());
    return device_or_error.release_value();
}

USBMassStorageDevice::USBMassStorageDevice(OwnPtr<USB::MassStorageHandle> usb_msc_handle, MinorNumber minor_number, OwnPtr<SCSIMetadata> metadata, NonnullOwnPtr<KString> device_name)
    : StorageDevice(StorageManagement::storage_type_major_number(), minor_number, metadata->block_size, metadata->num_blocks, move(device_name))
    , m_metadata(move(metadata))
    , m_usb_msc_handle(move(usb_msc_handle))
{
    u8 buf[512] = {'\0'};
    u8 print[17] = {'\0'};
    for (int j = 0; j < 64; j++) {
        dbgln("{}", read(0, buf).is_error());
        for (int i = 0; i < 64; i++) {
            memcpy(print, buf+(i*16), 16);
            dbgln("{}", print);
        }
    }
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
    CSWStatus res;

    // Check if device is ready to recieve SCSI commands
    res = TRY(usb_msc_handle->try_scsi_command<CommandDescriptorBlock6>(CDB_TEST_UNIT_READY, 0, Pipe::Direction::Out, 0, nullptr));
    if (res != CSWStatus::PASSED)
        return EBUSY;

    // Get number of logical units on device, only LUN 0 supported currently
    metadata->max_lun = TRY(usb_msc_handle->get_max_lun());
    if (metadata->max_lun != 0)
        dbgln("Multi-LUN USB MSC device detected. Only LUN 0 is currently supported.");

    // Retrieve the name/label of the device
    InquiryResponse inq_res;
    TRY(usb_msc_handle->try_scsi_command<CommandDescriptorBlock6>(CDB_INQUIRY, 0, Pipe::Direction::In, sizeof(InquiryResponse), &inq_res));
    memcpy(metadata->vendor_id, inq_res.vendor_id, 8);
    memcpy(metadata->product_id, inq_res.product_id, 16);

    // Read capacity = get block size & number of blocks on device
    ReadCapacityResponse cap_res;
    TRY(usb_msc_handle->try_scsi_command<CommandDescriptorBlock10>(CDB_READ_CAPACITY, 0, Pipe::Direction::In, sizeof(ReadCapacityResponse), &cap_res));
    metadata->block_size = AK::convert_between_host_and_big_endian(cap_res.block_size);
    metadata->num_blocks = AK::convert_between_host_and_big_endian(cap_res.num_blocks);

    // Determine if device is read-only
    u8 page[MODE_SENSE_PAGE_LEN];
    TRY(usb_msc_handle->try_scsi_command<CommandDescriptorBlock6>(CDB_MODE_SENSE, 0, Pipe::Direction::In, MODE_SENSE_PAGE_LEN, page));
    metadata->read_only = page[1] & (1 << 6);

    return metadata;
}

ErrorOr<void> USBMassStorageDevice::read(u32 lba, void * buf)
{
    CommandDescriptorBlock10 cdb_read = {
        .opcode = SCSI_READ_10,
        .misc_and_service = 0x00,
        .logical_block_addr = lba,
        .misc_continued = 0x00,
        .len = m_metadata->block_size,
        .control = 0x00
    };

    TRY(m_usb_msc_handle->try_scsi_command<CommandDescriptorBlock10>(cdb_read, 0, Pipe::Direction::In, m_metadata->block_size, buf));

    return {};
}

ErrorOr<void> USBMassStorageDevice::write(u32 lba, void * buf)
{
    CommandDescriptorBlock10 cdb_write = {
        .opcode = SCSI_WRITE_10,
        .misc_and_service = 0x00,
        .logical_block_addr = lba,
        .misc_continued = 0x00,
        .len = m_metadata->block_size,
        .control = 0x00
    };

    TRY(m_usb_msc_handle->try_scsi_command<CommandDescriptorBlock10>(cdb_write, 0, Pipe::Direction::Out, m_metadata->block_size, buf));

    return {};
}

}
