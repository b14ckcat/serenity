/*
 * Copyright (c) 2022, blackcat <b14ckcat@protonmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <Kernel/Storage/USB/SCSI.h>

namespace Kernel {

constexpr CommandDescriptorBlock6 CDB_SCSI_INQUIRY = {
    .opcode = SCSI_INQUIRY,
    .misc = 0x00,
    .logical_block_addr = 0x00,
    .len = 36,
    .control = 0x00
};

}
