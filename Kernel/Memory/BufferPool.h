/*
 * Copyright (c) 2022, blackcat <b14ckcat@pm.me>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Noncopyable.h>
#include <AK/Vector.h>
#include <Kernel/Library/LockWeakPtr.h>
#include <Kernel/Memory/MemoryManager.h>
#include <Kernel/Locking/Spinlock.h>

namespace Kernel::Memory {

class BufferPool {
public:
    enum class Type {
        Regular,
        DMA
    };

    static ErrorOr<NonnullOwnPtr<BufferPool>> create(size_t count, size_t size, Type type=Type::Regular)
    {
	size_t num_pages = TRY(page_round_up(count * size));
        if (type == Type::Regular) {
            auto buffer_region = TRY(MM.allocate_kernel_region(num_pages, "Object pool"sv, Memory::Region::Access::ReadWrite));
	    return TRY(adopt_nonnull_own_or_enomem(new (nothrow) BufferPool(type, count, size, move(buffer_region))));
	} else if (type == Type::DMA) {
            auto buffer_region = TRY(MM.allocate_dma_buffer_pages(num_pages, "DMA object pool"sv, Memory::Region::Access::ReadWrite));
	    return TRY(adopt_nonnull_own_or_enomem(new (nothrow) BufferPool(type, count, size, move(buffer_region))));
	}

	return EINVAL;
    }

    ErrorOr<void> take(VirtualAddress &vaddr)
    {
        SpinlockLocker locker {m_lock};

        if (!m_free)
            return ENOMEM;

        size_t buffer_idx = m_free_indices[--m_free];
        vaddr.set(m_region->vaddr().get() + (m_size * buffer_idx));
	m_free--;

	return {};
    }

    ErrorOr<void> take(VirtualAddress &vaddr, PhysicalAddress &paddr)
    {
	dbgln("TAKE: {}", m_count);
        SpinlockLocker locker {m_lock};

        if (!m_free)
            return ENOMEM;

        size_t buffer_idx = m_free_indices[--m_free];
	size_t page_idx = (buffer_idx * m_size) / PAGE_SIZE;
        vaddr.set(m_region->vaddr().get() + (m_size * buffer_idx));
        paddr.set(m_region->physical_page(0)->paddr().get() + (m_size * buffer_idx));
	dbgln("BASE VADDR: {} BASE PADDR: {} PAGE IDX: {} OFFSET: {} IDX: {} VADDR: {} PADDR: {}", m_region->vaddr().get(), m_region->physical_page(0)->paddr().get(), page_idx, m_size * buffer_idx, buffer_idx, vaddr.get(), paddr.get());
	return {};
    }

    void release(void *addr)
    {
        release(VirtualAddress(addr));
    }

    void release(VirtualAddress addr)
    {
        SpinlockLocker locker {m_lock};
        if (m_free == m_count-1) {
            dbgln("Error: attempt to return address to full pool");
	    return;
	}
        auto idx = (addr - m_region->vaddr()).get() / m_size;
        m_free_indices[m_free++] = idx;
	dbgln("RELEASE: {}", m_count);
    }

    void print_pool_information()
    {
        dbgln("Pool: size {}, count {}, free {}, location {}", m_count, m_size, m_free, m_region->physical_page(0)->paddr());
    }

private:
    BufferPool(Type type, size_t count, size_t size, NonnullOwnPtr<Region> region)
    : m_type(type)
    , m_count(count)
    , m_size(size)
    , m_free(count)
    , m_region(move(region))
    , m_lock(LockRank::None)
    {
        m_free_indices.resize(count);
        for (size_t i = 0; i < m_count; i++) {
            m_free_indices[i] = i;
	}
    }

    const Type m_type;
    const size_t m_count;
    const size_t m_size;
    size_t m_free;
    NonnullOwnPtr<Region> m_region;
    Vector<size_t> m_free_indices;
    Spinlock m_lock;
};

}
