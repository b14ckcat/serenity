/*
 * Copyright (c) 2022, blackcat <b14ckcat@protonmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

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

    virtual StringView class_name() const override;

private:
    USBMassStorageDevice(OwnPtr<USB::MassStorageHandle>, MinorNumber, NonnullOwnPtr<KString>);

    virtual void start_request(AsyncBlockDeviceRequest&) override;

    virtual CommandSet command_set() const override { return CommandSet::SCSI; }

    OwnPtr<USB::MassStorageHandle> m_usb_msc_handle;
    RefPtr<Thread> m_polling_thread;
};

}
