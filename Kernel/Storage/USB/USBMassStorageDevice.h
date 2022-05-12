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

class USBMassStorageDevice final : public StorageDevice {
    friend class DeviceManagement;

public:
    static ErrorOr<NonnullRefPtr<USBMassStorageDevice>> create(OwnPtr<USB::MassStorageHandle>);
    virtual ~USBMassStorageDevice() override;

    virtual void start_request(AsyncBlockDeviceRequest&) override;

    virtual CommandSet command_set() const override { return CommandSet::SCSI; }
    virtual StringView class_name() const override;

private:
    USBMassStorageDevice(OwnPtr<USB::MassStorageHandle>, MinorNumber, size_t, u64, NonnullOwnPtr<KString>);

    ErrorOr<void> get_metadata(u8 lun);

    u8 m_max_lun;
    OwnPtr<USB::MassStorageHandle> m_usb_msc_handle;
};

}
