#include <AK/Format.h>
#include <Kernel/Bus/USB/USBClasses.h>
#include <Kernel/Bus/USB/USBConfiguration.h>
#include <Kernel/Bus/USB/USBController.h>
#include <Kernel/Bus/USB/USBDescriptors.h>
#include <Kernel/Bus/USB/USBEndpoint.h>
#include <Kernel/Bus/USB/USBInterface.h>
#include <Kernel/Bus/USB/USBPipe.h>
#include <Kernel/Devices/HID/USBKeyboardDevice.h>
#include <Kernel/Devices/HID/HIDManagement.h>

namespace Kernel::USB {

// Temporary, replace this with actual driver probing code later
void USBInterface::driver_probe()
{
    if (m_descriptor.interface_class_code == USB_CLASS_HID) {
        if (m_descriptor.interface_protocol == USB_HID_PROTOCOL_KEYBOARD) {
            for (auto endpoint_desc : m_endpoint_descriptors) {
	        if (endpoint_desc.endpoint_attributes_bitmap & USBEndpoint::ENDPOINT_ATTRIBUTES_TRANSFER_TYPE_BULK) {
                    auto err_or_int_pipe = InterruptPipe::try_create_pipe(m_configuration.device().controller(), Pipe::Direction::Bidirectional, endpoint_desc.endpoint_address, endpoint_desc.max_packet_size, m_configuration.device().address(), 10);
		    if (err_or_int_pipe.is_error()) {
                        dbgln("Error initializing USB keyboard driver");
			return;
		    }
                    
		    auto usb_device = USBKeyboardDevice::create(m_configuration.device(), err_or_int_pipe.release_value());
                    if (usb_device.is_error()) {
                        dbgln("Error initializing USB keyboard driver");
			return;
		    }
		    HIDManagement::the().attach_hotplug_device(usb_device.release_value());
                }
            }
	}
    }
}

}
