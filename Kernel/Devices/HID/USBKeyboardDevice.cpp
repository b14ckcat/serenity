#include <Kernel/Devices/HID/USBKeyboardDevice.h>

ErrorOr<NonnullLockRefPtr<USBKeyboardDevice>> USBKeyboardDevice::create(USB::Device const& device, NonnullOwnPtr<USB::InterruptPipe> int_pipe)
{
    auto keyboard_device = TRY(DeviceManagement::try_create_device<USBKeyboardDevice>(device, move(int_pipe)));
    return keyboard_device;
}

USBKeyboardDevice::USBKeyboardDevice(USB::Device const& device, NonnullOwnPtr<USB::InterruptPipe> int_pipe)
    : KeyboardDevice()
    , m_device(device)
    , m_int_pipe(move(int_pipe))
{
    auto buf = kmalloc(64);
    auto res = m_int_pipe->interrupt_transfer(64, buf, 10, &USBKeyboardDevice::handle_usb_interrupt_transfer);
    if (res.is_error()) {
        dbgln("Error");
    }
}

void USBKeyboardDevice::handle_usb_interrupt_transfer()
{
    dbgln("Callback!");
}
