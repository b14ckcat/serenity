/*
 * Copyright (c) 2018-2020, Andreas Kling <kling@serenityos.org>
 * Copyright (c) 2021, kleines Filmröllchen <filmroellchen@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/MemoryStream.h>
#include <AK/OwnPtr.h>
#include <AK/RefPtr.h>
#include <AK/Span.h>
#include <AK/Stream.h>
#include <AK/String.h>
#include <AK/StringView.h>
#include <AK/WeakPtr.h>
#include <LibAudio/Buffer.h>
#include <LibAudio/Loader.h>
#include <LibCore/File.h>
#include <LibCore/FileStream.h>

namespace Audio {

// constants for handling the WAV header data
static constexpr unsigned const WAVE_FORMAT_PCM = 0x0001;        // PCM
static constexpr unsigned const WAVE_FORMAT_IEEE_FLOAT = 0x0003; // IEEE float
static constexpr unsigned const WAVE_FORMAT_ALAW = 0x0006;       // 8-bit ITU-T G.711 A-law
static constexpr unsigned const WAVE_FORMAT_MULAW = 0x0007;      // 8-bit ITU-T G.711 µ-law
static constexpr unsigned const WAVE_FORMAT_EXTENSIBLE = 0xFFFE; // Determined by SubFormat

// Parses a WAV file and produces an Audio::LegacyBuffer.
class WavLoaderPlugin : public LoaderPlugin {
public:
    explicit WavLoaderPlugin(StringView path);
    explicit WavLoaderPlugin(Bytes const& buffer);

    virtual MaybeLoaderError initialize() override;

    virtual LoaderSamples get_more_samples(size_t max_samples_to_read_from_input = 128 * KiB) override;

    virtual MaybeLoaderError reset() override { return seek(0); }

    // sample_index 0 is the start of the raw audio sample data
    // within the file/stream.
    virtual MaybeLoaderError seek(int sample_index) override;

    virtual int loaded_samples() override { return static_cast<int>(m_loaded_samples); }
    virtual int total_samples() override { return static_cast<int>(m_total_samples); }
    virtual u32 sample_rate() override { return m_sample_rate; }
    virtual u16 num_channels() override { return m_num_channels; }
    virtual String format_name() override { return "RIFF WAVE (.wav)"; }
    virtual PcmSampleFormat pcm_format() override { return m_sample_format; }
    virtual RefPtr<Core::File> file() override { return m_file; }

private:
    MaybeLoaderError parse_header();

    RefPtr<Core::File> m_file;
    OwnPtr<AK::InputStream> m_stream;
    AK::InputMemoryStream* m_memory_stream;
    Optional<LoaderError> m_error {};

    u32 m_sample_rate { 0 };
    u16 m_num_channels { 0 };
    PcmSampleFormat m_sample_format;
    size_t m_byte_offset_of_data_samples { 0 };

    size_t m_loaded_samples { 0 };
    size_t m_total_samples { 0 };
};

}
