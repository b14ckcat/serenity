/*
 * Copyright (c) 2022, blackcat <b14ckcat@protonmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Error.h>
#include <AK/Types.h>
#include <AK/Memory.h>
#include <Kernel/Bus/USB/USBDevice.h>
#include <Kernel/Bus/USB/USBPipe.h>
#include <Kernel/Storage/USB/SCSI.h>

namespace Kernel::USB {

// Mass storage subclass codes
static constexpr u8 USB_MSC_SUBCLASS_SCSI_COMMAND_SET_NOT_REPORTED = 0x00;
static constexpr u8 USB_MSC_SUBCLASS_RBC = 0x01;
static constexpr u8 USB_MSC_SUBCLASS_MMC_5 = 0x02;
static constexpr u8 USB_MSC_SUBCLASS_UFI = 0x04;
static constexpr u8 USB_MSC_SUBCLASS_SCSI_TRANSPARENT_COMMAND_SET = 0x06;
static constexpr u8 USB_MSC_SUBCLASS_LSD_FS = 0x07;
static constexpr u8 USB_MSC_SUBCLASS_IEEE_1667 = 0x08;
static constexpr u8 USB_MSC_SUBCLASS_VENDOR_SPECIFIC = 0xFF;

// Mass storage transport protocol codes
static constexpr u8 USB_MSC_PROTOCOL_CBI_COMMAND_COMPLETION_INTERRUPT = 0x00;
static constexpr u8 USB_MSC_PROTOCOL_CBI_NO_COMMAND_COMPLETION_INTERRUPT = 0x01;
static constexpr u8 USB_MSC_PROTOCOL_BULK_ONLY = 0x50;
static constexpr u8 USB_MSC_PROTOCOL_UAS = 0x62;
static constexpr u8 USB_MSC_PROTOCOL_VENDOR_SPECIFIC = 0xFF;

// Class-specific request codes
static constexpr u8 USB_MSC_REQUEST_ADSC = 0x00;
static constexpr u8 USB_MSC_REQUEST_LSDFS_GET_REQUESTS = 0xFC;
static constexpr u8 USB_MSC_REQUEST_LSDFS_PUT_REQUESTS = 0xFC;
static constexpr u8 USB_MSC_REQUEST_BULK_ONLY_GET_MAX_LUN = 0xFE;
static constexpr u8 USB_MSC_REQUEST_BULK_ONLY_MASS_STORAGE_RESET = 0xFF;

// Class-specific descriptor codes
static constexpr u8 USB_MSC_PIPE_USAGE_CLASS_SPECIFIC_DESCRIPTOR = 0x24;

// QEMU virtual mass storage device magic numbers
// When QEMU inserts a virtual USB MSC device ("usb-storage"), it provides a device
// that return an interface descriptor that does not contain the proper data to
// identify it as a MSC device, so these values are checked as well as a workaround.
static constexpr u16 USB_MSC_QEMU_VID = 0x46F4;
static constexpr u16 USB_MSC_QEMU_PID = 0x0001;

static constexpr u32 USB_MSC_CBW_MAGIC_NUMBER = 0x43425355;
static constexpr u32 USB_MSC_CSW_MAGIC_NUMBER = 0x53425355;

enum CSWStatus : u8 {
    PASSED = 0x00,
    FAILED,
    PHASE_ERROR,
};

struct [[gnu::packed]] CommandBlockWrapper {
    u32 const dCBWSignature { USB_MSC_CBW_MAGIC_NUMBER };
    u32 dCBWTag; // Tag that allows CBW to be matched with its CSW
    u32 dCBWDataTransferLength; // Size in bytes of the data to be transfered on the bulk in/out endpoint (depending on direction bit in bmCBWFlags)
    u8 bmCBWFlags; // MSB indicates direction, 0 for host->device, 1 for device->host
    u8 bCBWLUN; // Lower 4 bits only, indicates device LUN to send command block to
    u8 bCBWCBLength; // Lower 5 bits only, command block length in bytes
    u8 CBWCB[16]; // Command block to be sent to device
};
static_assert(sizeof(CommandBlockWrapper) == 31);

struct [[gnu::packed]] CommandStatusWrapper {
    u32 const dCBWSignature { USB_MSC_CBW_MAGIC_NUMBER };
    u32 dCSWTag; // Tag that allows CSW to be matched with its CBW
    u32 dCSWDataResidue; // Indicates the difference between the amount of data expected (in bytes?) and the amount received
    CSWStatus bCSWStatus; // Indicates success/failure of command, 0 = success, 1 = failure, 2 = "phase error"
};
static_assert(sizeof(CommandStatusWrapper) == 13);

class MassStorageHandle {
public:
    MassStorageHandle(Device const& usb_device, USBInterface const& usb_interface, OwnPtr<Pipe> bulk_in, OwnPtr<Pipe> bulk_out)
    : m_usb_device(usb_device)
    , m_usb_interface(usb_interface)
    , m_bulk_in(move(bulk_in))
    , m_bulk_out(move(bulk_out))
    , m_tag(0)
    {
    }

    ErrorOr<u8> get_max_lun();

    template<class T>
    ErrorOr<CSWStatus> try_scsi_command(T scsi_command, u8 lun, Pipe::Direction dir, void *buf)
    {
        int retries;

        CommandBlockWrapper cbw {
	    .dCBWTag = m_tag++,
	    .dCBWDataTransferLength = scsi_command.data_len(),
	    .bmCBWFlags = static_cast<u8>((dir == Pipe::Direction::In ? 0x80 : 0)),
	    .bCBWLUN = lun,
            .bCBWCBLength = scsi_command.cbd_size(),
	    .CBWCB = {0}
	};
	memcpy(cbw.CBWCB, scsi_command.data(), scsi_command.cbd_size());
        CommandStatusWrapper csw;

        for (retries = 10; retries > 0; retries--) {
	    auto transfer_size_or_err = m_bulk_out->bulk_transfer(sizeof(CommandBlockWrapper), &cbw);
	    if (transfer_size_or_err.is_error())
	        continue;
	    if (scsi_command.data_len() > 0) {
	        if (dir == Pipe::Direction::In) {
	            transfer_size_or_err = m_bulk_in->bulk_transfer(scsi_command.data_len(), buf);
	            if (transfer_size_or_err.is_error())
	                continue;
	        } else if (dir == Pipe::Direction::Out) {
	            transfer_size_or_err = m_bulk_out->bulk_transfer(scsi_command.data_len(), buf);
	            if (transfer_size_or_err.is_error())
	                continue;
		}
	    }
            transfer_size_or_err = m_bulk_in->bulk_transfer(sizeof(CommandBlockWrapper), &csw);

	    break;
	}

	if (retries == 0)
            return EPROTO;

	return csw.bCSWStatus;
    }

private:
    Device const& m_usb_device;
    USBInterface const& m_usb_interface;
    OwnPtr<Pipe> m_bulk_in;
    OwnPtr<Pipe> m_bulk_out;
    u32 m_tag;
};

}
