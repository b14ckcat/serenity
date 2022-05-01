/*
 * Copyright (c) 2022, blackcat <b14ckcat@protonmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Try.h>
#include <Kernel/KString.h>
#include <Kernel/Storage/StorageDevice.h>

namespace Kernel {

class USBMassStorageDevice final : public StorageDevice {
    friend class DeviceManagement;

public:
    static ErrorOr<NonnullRefPtr<USBMassStorageDevice>> create(USB::Device const&);
    virtual ~USBMassStorageDevice() override;

    virtual StringView class_name() const override;

private:
    USBMassStorageDevice(USB::Device const&, MinorNumber, NonnullOwnPtr<KString>);

    virtual void start_request(AsyncBlockDeviceRequest&) override;

    virtual CommandSet command_set() const override { return CommandSet::SCSI; }

    RefPtr<USB::Device> m_attached_usb_device;
    RefPtr<Thread> m_polling_thread;
};

}
