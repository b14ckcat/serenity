/*
 * Copyright (c) 2022, blackcat <b14ckcat@protonmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/FixedArray.h>
#include <AK/OwnPtr.h>
#include <Kernel/KString.h>
#include <Kernel/Bus/USB/USBMassStorageClass.h>
#include <Kernel/Storage/StorageDevice.h>

namespace Kernel {

struct SCSIMetadata {
    const u8 lun = 0; // Important! Multiple LUNs are not currently supported
    u8 max_lun;
    unsigned char vendor_id[8];
    unsigned char product_id[16];
    u16 block_size;
    u32 num_blocks;
    bool read_only;
};

class USBMassStorageDevice final : public StorageDevice {
    friend class DeviceManagement;

public:
    static ErrorOr<NonnullRefPtr<USBMassStorageDevice>> create(OwnPtr<USB::MassStorageHandle>);
    virtual ~USBMassStorageDevice() override;

    virtual void start_request(AsyncBlockDeviceRequest&) override;

    virtual CommandSet command_set() const override { return CommandSet::SCSI; }
    virtual StringView class_name() const override;

private:
    USBMassStorageDevice(OwnPtr<USB::MassStorageHandle>, MinorNumber, OwnPtr<SCSIMetadata>, NonnullOwnPtr<KString>);

    static ErrorOr<OwnPtr<SCSIMetadata>> get_metadata(OwnPtr<USB::MassStorageHandle> &usb_msc_handle);

    ErrorOr<void> read(u32 lba, void * buf);
    ErrorOr<void> write(u32 lba, void * buf);

    OwnPtr<SCSIMetadata> m_metadata;
    OwnPtr<USB::MassStorageHandle> m_usb_msc_handle;
};

}
