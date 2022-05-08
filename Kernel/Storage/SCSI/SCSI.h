/*
 * Copyright (c) 2022, blackcat <b14ckcat@protonmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

namespace Kernel {

enum SCSI_COMMANDS {
    SCSI_TEST_UNIT_READY = 0x00,
    SCSI_REWIND,
    SCSI_REQUEST_SENSE = 0x03,
    SCSI_FORMAT,
    SCSI_READ_BLOCK_LIMITS,
    SCSI_REASSIGN_BLOCKS = 0x07,
    SCSI_INITIALIZE_ELEMENT_STATUS = 0x07,
    SCSI_READ_6,
    SCSI_WRITE_6 = 0x0A,
    SCSI_SEEK_6,
    SCSI_READ_REVERSE_6 = 0x0F,
    SCSI_WRITE_FILEMARKS_6,
    SCSI_SPACE_6,
    SCSI_INQUIRY,
    SCSI_VERIFY_6,
    SCSI_RECOVER_BUFFERED_DATA,
    SCSI_MODE_SELECT_6,
    SCSI_RESERVE_6,
    SCSI_RELEASE_6,
    SCSI_COPY,
    SCSI_ERASE_6,
    SCSI_MODE_SENSE_6,
    SCSI_START_STOP_UNIT,
    SCSI_LOAD_UNLOAD,
    SCSI_RECEIVE_DIAGNOSTIC_RESULTS,
    SCSI_SEND_DIAGNOSTIC,
    SCSI_PREVENT_ALLOW_MEDIUM_REMOVAL,
    SCSI_READ_FORMAT_CAPACITIES,
    SCSI_READ_CAPACITY_10,
    SCSI_READ_10,
    SCSI_READ_GENERATION,
    SCSI_WRITE_10,
    SCSI_SEEK_10,
    SCSI_LOCATE_10,
    SCSI_ERASE_10,
    SCSI_READ_UPDATED_BLOCK,
    SCSI_WRITE_AND_VERIFY_10,
    SCSI_VERIFY_10,
    SCSI_SET_LIMITS_10,
    SCSI_PRE_FETCH_10,
    SCSI_READ_POSITION,
    SCSI_SYNCHRONIZE_CACHE_10,
    SCSI_LOCK_UNLOCK_CACHE_10,
    SCSI_READ_DEFECT_DATA_10,
    SCSI_INITIALIZE_ELEMENT_STATUS_WITH_RANGE,
    SCSI_MEDIUM_SCAN,
    SCSI_COMPARE,
    SCSI_COPY_AND_VERIFY,
    SCSI_WRITE_BUFFER,
    SCSI_READ_BUFFER,
    SCSI_UPDATE_BLOCK,
    SCSI_READ_LONG_10,
    SCSI_WRITE_LONG_10,
    SCSI_CHANGE_DEFINITION,
    SCSI_WRITE_SAME_10,
    SCSI_UNMAP,
    SCSI_READ_TOC_PMA_ATIP,
    SCSI_REPORT_DENSITY_SUPPORT,
    SCSI_PLAY_AUDIO_10,
    SCSI_GET_CONFIGURATION,
    SCSI_PLAY_AUDIO_MSF,
    SCSI_SANITIZE,
    SCSI_GET_EVENT_STATUS_NOTIFICATION,
    SCSI_PAUSE_RESUME,
    SCSI_LOG_SELECT,
    SCSI_LOG_SENSE,
    SCSI_XDWRITE_10,
    SCSI_XPWRITE_10,
    SCSI_READ_DISC_INFORMATION,
    SCSI_XDREAD_10,
    SCSI_XDWRITEREAD_10,
    SCSI_SEND_OPC_INFORMATION,
    SCSI_MODE_SELECT_10,
    SCSI_RESERVE_10,
    SCSI_RELEASE_10,
    SCSI_REPAIR_TRACK,
    SCSI_MODE_SENSE_10,
    SCSI_CLOSE_TRACK_SESSION,
    SCSI_READ_BUFFER_CAPACITY,
    SCSI_SEND_CUE_SHEET,
    SCSI_PERSISTENT_RESERVE_IN,
    SCSI_PERSISTENT_RESERVE_OUT,
    SCSI_EXTENDED_CDB,
    SCSI_VARIABLE_LENGTH_CDB,
    SCSI_XDWRITE_EXTENDED_16,
    SCSI_WRITE_FILEMARKS_16,
    SCSI_READ_REVERSE_16,
    SCSI_THIRD_PARTY_COPY_OUT_COMMANDS,
    SCSI_THIRD_PARTY_COPY_IN_COMMANDS,
    SCSI_ATA_PASS_THROUGH_16,
    SCSI_ACCESS_CONTROL_IN,
    SCSI_ACCESS_CONTROL_OUT,
    SCSI_READ_16,
    SCSI_COMPARE_AND_WRITE,
    SCSI_WRITE_16,
    SCSI_ORWRITE,
    SCSI_READ_ATTRIBUTE,
    SCSI_WRITE_ATTRIBUTE,
    SCSI_WRITE_AND_VERIFY_16,
    SCSI_VERIFY_16,
    SCSI_PRE_FETCH_16,
    SCSI_SYNCHRONIZE_CACHE_16,
    SCSI_SPACE_16,
    SCSI_LOCK_UNLOCK_CACHE_16,
    SCSI_LOCATE_16,
    SCSI_WRITE_SAME_16,
    SCSI_ERASE_16,
    SCSI_SERVICE_ACTION_BIDIRECTIONAL,
    SCSI_SERVICE_ACTION_IN_16,
    SCSI_SERVICE_ACTION_OUT_16,
    SCSI_REPORT_LUNS,
    SCSI_ATA_PASS_THROUGH_12,
    SCSI_SECURITY_PROTOCOL_IN,
    SCSI_MAINTENANCE_IN,
    SCSI_MAINTENANCE_OUT,
    SCSI_REPORT_KEY,
    SCSI_MOVE_MEDIUM,
    SCSI_PLAY_AUDIO_12,
    SCSI_EXCHANGE_MEDIUM,
    SCSI_MOVE_MEDIUM_ATTACHED,
    SCSI_READ_12,
    SCSI_SERVICE_ACTION_OUT_12,
    SCSI_WRITE_12,
    SCSI_SERVICE_ACTION_IN_12,
    SCSI_ERASE_12,
    SCSI_READ_DVD_STRUCTURE,
    SCSI_WRITE_AND_VERIFY_12,
    SCSI_VERIFY_12,
    SCSI_SEARCH_DATA_HIGH_12,
    SCSI_SEARCH_DATA_EQUAL_12,
    SCSI_SEARCH_DATA_LOW_12,
    SCSI_SET_LIMITS_12,
    SCSI_READ_ELEMENT_STATUS_ATTACHED,
    SCSI_SECURITY_PROTOCOL_OUT,
    SCSI_SEND_VOLUME_TAG,
    SCSI_READ_DEFECT_DATA_12,
    SCSI_READ_ELEMENT_STATUS,
    SCSI_READ_CD_MSF,
    SCSI_REDUNDANCY_GROUP_IN,
    SCSI_REDUNDANCY_GROUP_OUT,
    SCSI_SPARE_IN,
    SCSI_SPARE_OUT,
    SCSI_VOLUME_SET_IN,
    SCSI_VOLUME_SET_OUT
};

struct [[gnu::packed]] CommandDescriptorBlock6 {
    u8 opcode;
    u8 misc; // Only 3 highest bits are used
    u16 logical_block_addr;
    u8 len; // Transfer, paramater list, or allocation length
    u8 control;
};
static_assert(sizeof(CommandDescriptorBlock6) == 6);

struct [[gnu::packed]] CommandDescriptorBlock10 {
    u8 opcode;
    u8 misc_and_service; // Highest 3 bits for misc, lower 5 for service action
    u32 logical_block_addr;
    u8 misc_continued; // More miscellaneous data
    u16 len; // Transfer, paramater list, or allocation length
    u8 control;
};
static_assert(sizeof(CommandDescriptorBlock10) == 10);

struct [[gnu::packed]] CommandDescriptorBlock12 {
    u8 opcode;
    u8 misc_and_service; // Highest 3 bits for misc, lower 5 for service action
    u32 logical_block_addr;
    u32 len; // Transfer, paramater list, or allocation length
    u8 misc_continued; // More miscellaneous data
    u8 control;
};
static_assert(sizeof(CommandDescriptorBlock12) == 12);

struct [[gnu::packed]] CommandDescriptorBlock16 {
    u8 opcode;
    u8 misc;
    u64 logical_block_addr;
    u32 len; // Transfer, paramater list, or allocation length
    u8 misc_continued; // More miscellaneous data
    u8 control;
};
static_assert(sizeof(CommandDescriptorBlock16) == 16);

}
