#include <Kernel/Devices/HID/USBKeyboardDevice.h>

ErrorOr<NonnullLockRefPtr<USBKeyboardDevice>> USBKeyboardDevice::create(USB::Device const& device, NonnullOwnPtr<USB::InterruptPipe> int_pipe)
{
    dbgln("POINT 1");
    auto keyboard_device = TRY(DeviceManagement::try_create_device<USBKeyboardDevice>(device, move(int_pipe)));
    dbgln("POINT 3");
    return keyboard_device;
}

USBKeyboardDevice::USBKeyboardDevice(USB::Device const& device, NonnullOwnPtr<USB::InterruptPipe> int_pipe)
    : KeyboardDevice()
    , m_device(device)
    , m_int_pipe(move(int_pipe))
{
    dbgln("POINT 2");
}

void USBKeyboardDevice::handle_usb_interrupt_transfer()
{
}
