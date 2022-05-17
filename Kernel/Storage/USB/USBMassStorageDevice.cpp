/*
 * Copyright (c) 2022, blackcat <b14ckcat@protonmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/Endian.h>
#include <AK/FixedArray.h>
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

using namespace AK;
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
}

USBMassStorageDevice::~USBMassStorageDevice() = default;

StringView USBMassStorageDevice::class_name() const
{
    return "USBMassStorageDevice"sv;
}

void USBMassStorageDevice::start_request(AsyncBlockDeviceRequest& request)
{
    MutexLocker locker(m_lock);

    if (request.block_index() + request.block_count() > m_metadata->num_blocks) {
        request.complete(AsyncDeviceRequest::Failure);
        return;
    }

    auto res = FixedArray<u8>::try_create(m_metadata->block_size);
    if (res.is_error()) {
        request.complete(AsyncDeviceRequest::Failure);
        return;
    }
    auto buf = res.release_value();

    if (request.request_type() == AsyncBlockDeviceRequest::Read) {
        for (u32 i = 0; i < request.block_count(); i++) {
            if (read(request.block_index()+i, buf.data()).is_error()) {
                request.complete(AsyncDeviceRequest::Failure);
                return;
            }
            if (request.buffer().write(buf.data(), buf.size()*i, buf.size()).is_error()) {
                request.complete(AsyncDeviceRequest::Failure);
                return;
            }
	}
    } else if (request.request_type() == AsyncBlockDeviceRequest::Write) {
        for (u32 i = 0; i < request.block_count(); i++) {
            if (request.buffer().read(buf.data(), buf.size()*i, buf.size()).is_error()) {
                request.complete(AsyncDeviceRequest::Failure);
		return;
            }
            if (write(request.block_index()+i, buf.data()).is_error()) {
                request.complete(AsyncDeviceRequest::Failure);
                return;
            }
        }
    }

    request.complete(AsyncDeviceRequest::Success);
}

ErrorOr<OwnPtr<SCSIMetadata>> USBMassStorageDevice::get_metadata(OwnPtr<USB::MassStorageHandle> &usb_msc_handle)
{
    constexpr u8 MODE_SENSE_PAGE_LEN = 192;
    constexpr u8 MODE_SENSE_PAGE_ADDR = 0x3F;

    SCSICommand6 test_unit_ready(SCSI_TEST_UNIT_READY);
    SCSICommand6 inquiry(SCSI_INQUIRY, sizeof(InquiryResponse));
    SCSICommand10 read_capacity(SCSI_READ_CAPACITY_10, sizeof(ReadCapacityResponse));
    SCSICommand6 mode_sense(SCSI_MODE_SENSE_6, MODE_SENSE_PAGE_LEN, MODE_SENSE_PAGE_ADDR); // Used to determine whether or not the device is read-only

    auto metadata = TRY(adopt_nonnull_own_or_enomem(new SCSIMetadata()));
    CSWStatus res;

    // Check if device is ready to recieve SCSI commands
    res = TRY(usb_msc_handle->try_scsi_command<SCSICommand6>(test_unit_ready, 0, Pipe::Direction::Out, nullptr));
    if (res != CSWStatus::PASSED)
        return EBUSY;

    // Get number of logical units on device, only LUN 0 supported currently
    metadata->max_lun = TRY(usb_msc_handle->get_max_lun());
    if (metadata->max_lun != 0)
        dbgln("Multi-LUN USB MSC device detected. Only LUN 0 is currently supported.");

    // Retrieve the name/label of the device
    InquiryResponse inq_res;
    TRY(usb_msc_handle->try_scsi_command<SCSICommand6>(inquiry, 0, Pipe::Direction::In, &inq_res));
    memcpy(metadata->vendor_id, inq_res.vendor_id, 8);
    memcpy(metadata->product_id, inq_res.product_id, 16);

    // Read capacity = get block size & number of blocks on device
    ReadCapacityResponse cap_res;
    TRY(usb_msc_handle->try_scsi_command<SCSICommand10>(read_capacity, 0, Pipe::Direction::In, &cap_res));
    metadata->block_size = AK::convert_between_host_and_big_endian(cap_res.block_size);
    metadata->num_blocks = AK::convert_between_host_and_big_endian(cap_res.num_blocks);

    // Determine if device is read-only
    u8 page[MODE_SENSE_PAGE_LEN];
    TRY(usb_msc_handle->try_scsi_command<SCSICommand6>(mode_sense, 0, Pipe::Direction::In, page));
    metadata->read_only = page[1] & (1 << 6);

    return metadata;
}

ErrorOr<void> USBMassStorageDevice::read(u32 lba, void * buf)
{
    SCSICommand10 read(SCSI_READ_10, m_metadata->block_size, lba);
    TRY(m_usb_msc_handle->try_scsi_command<SCSICommand10>(read, 0, Pipe::Direction::In, buf));

    return {};
}

ErrorOr<void> USBMassStorageDevice::write(u32 lba, void * buf)
{
    SCSICommand10 write(SCSI_WRITE_10, m_metadata->block_size, lba);
    TRY(m_usb_msc_handle->try_scsi_command<SCSICommand10>(write, 0, Pipe::Direction::Out, buf));

    return {};
}

}
