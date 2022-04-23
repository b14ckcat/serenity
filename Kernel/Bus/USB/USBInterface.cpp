#include <Kernel/Bus/USB/USBClasses.h>
#include <Kernel/Bus/USB/USBConfiguration.h>
#include <Kernel/Bus/USB/USBInterface.h>
#include <Kernel/Bus/USB/USBMassStorageClass.h>
#include <Kernel/Storage/USB/USBMassStorageDevice.h>

namespace Kernel::USB
{

// Temporary function to load drivers for certain USB device classes
void USBInterface::load_drivers()
{
    // Mass storage class
    if (m_descriptor.interface_class_code == USB_CLASS_MASS_STORAGE &&
        m_descriptor.interface_sub_class_code == USB_MSC_SUBCLASS_SCSI_TRANSPARENT_COMMAND_SET &&
	m_descriptor.interface_protocol == USB_MSC_PROTOCOL_BULK_ONLY) {
            dbgln("Testing testing");
    }
}

}
