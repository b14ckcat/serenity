//
// Created by quaker on 17/5/22.
//

#pragma once

#include <Kernel/Bus/USB/Drivers/USBDriver.h>

namespace Kernel::USB
{

class FT232 : public Driver
{
public:
    FT232();
    ~FT232();

    virtual ErrorOr<void> probe(USBInterface const& device) override;
    virtual void disconnect() override;
    virtual StringView const& name() const override { return m_driver_name; }

    static void init();
};

}
