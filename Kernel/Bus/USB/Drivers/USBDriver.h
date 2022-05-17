/*
* Copyright (c) 2022, Jesse Buhagiar <jesse.buhagiar@serenityos.org>
*
* SPDX-License-Identifier: BSD-2-Clause
*/

#pragma once

#include <AK/Error.h>
#include <AK/NonnullRefPtr.h>
#include <Kernel/Bus/USB/USBDevice.h>

namespace Kernel::USB {

class Driver : public RefCounted<Driver> {
public:
    Driver(StringView const& name)
        : m_driver_name(name)
    {
    }
    
    virtual ~Driver() = default;

    virtual ErrorOr<void> probe(USBInterface const& interface) = 0;
    virtual void disconnect() = 0;
    virtual StringView const& name() const = 0;

protected:
    StringView const m_driver_name;
};
}