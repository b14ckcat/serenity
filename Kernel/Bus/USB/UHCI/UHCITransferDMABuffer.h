/*
 * Copyright (c) 2022, blackcat <b14ckcat@protonmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Types.h>

static constexpr u8 UHCI_ISOCHRONOUS_TRANSFER_SIZE = 64;
static constexpr u8 UHCI_MAX_QUEUED_TRANSFER_SIZE = 64;

struct UHCITransferDMABuffer
{
    UHCITransferDMABuffer(u32 p_addr) { (void)p_addr; }

    u8 buf[UHCI_MAX_QUEUED_TRANSFER_SIZE];
};
static_assert(AssertSize<UHCITransferDMABuffer, UHCI_MAX_QUEUED_TRANSFER_SIZE>());
