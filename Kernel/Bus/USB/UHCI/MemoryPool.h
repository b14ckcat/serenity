/*
 * Copyright (c) 2021, Jesse Buhagiar <jooster669@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/NonnullOwnPtr.h>
#include <AK/OwnPtr.h>
#include <AK/Stack.h>
#include <Kernel/Locking/Spinlock.h>
#include <Kernel/Memory/MemoryManager.h>
#include <Kernel/Memory/Region.h>
#include <Kernel/StdLib.h>

namespace Kernel::USB {

// This pool is bound by PAGE_SIZE / sizeof(T). The underlying allocation for the pointers
// is AK::Stack. As such, we never dynamically allocate any memory past the amount
// that can fit in a single page.
template<typename T>
class MemoryPool {
    AK_MAKE_NONCOPYABLE(MemoryPool);
    AK_MAKE_NONMOVABLE(MemoryPool);

    // Ensure that we can't get into a situation where we'll write past the page
    // and blow up
    static_assert(sizeof(T) <= PAGE_SIZE);

public:
    static ErrorOr<NonnullOwnPtr<MemoryPool<T>>> try_create(StringView name)
    {
        auto memory_page = TRY(MM.allocate_kernel_region(PAGE_SIZE, "Memory Pool"sv, Memory::Region::Access::ReadWrite));
        return adopt_nonnull_own_or_enomem(new (nothrow) MemoryPool(move(memory_page), name));
    }

    ~MemoryPool() = default;

    [[nodiscard]] T* try_take_free_block()
    {
        SpinlockLocker locker(m_pool_lock);

        // We're out of memory!
        if (m_free_block_stack.is_empty())
            return nullptr;

        dbgln_if(UHCI_VERBOSE_DEBUG, "Got a free memory block @ {} from pool {}", m_free_block_stack.top(), m_pool_name);
        T* block = m_free_block_stack.top();
        m_free_block_stack.pop();

        return block;
    }

    void release_to_pool(T* ptr)
    {
        SpinlockLocker locker(m_pool_lock);

        dbgln_if(UHCI_VERBOSE_DEBUG, "Returning block @ {} to pool {}", ptr, m_pool_name);
        if (!m_free_block_stack.push(ptr))
            dbgln("Failed to return block to pool {}. Stack overflow!", m_pool_name);
    }

    void print_pool_information() const
    {
        dbgln("Pool {} allocated @ {}", m_pool_name, m_pool_region->physical_page(0)->paddr());
    }

private:
    MemoryPool(NonnullOwnPtr<Memory::Region> pool_memory_block, StringView name)
        : m_pool_name(name)
        , m_pool_region(move(pool_memory_block))
        , m_pool_lock(LockRank::None)
    {
        // Go through the number of blocks to create in the pool, and create a virtual/physical address mapping
        for (size_t i = 0; i < PAGE_SIZE / sizeof(T); i++) {
            auto* placement_address = reinterpret_cast<void*>(m_pool_region->vaddr().get() + (i * sizeof(T)));
            auto physical_address = static_cast<u32>(m_pool_region->physical_page(0)->paddr().get() + (i * sizeof(T)));
            auto* object = new (placement_address) T(physical_address);
            m_free_block_stack.push(object); // Push the block's pointer onto the free list
        }
    }

    StringView m_pool_name;                                   // Name of this pool
    NonnullOwnPtr<Memory::Region> m_pool_region;              // Memory region where blocks actually reside
    Stack<T*, PAGE_SIZE / sizeof(T)> m_free_block_stack; // Stack of currently free block pointers
    Spinlock m_pool_lock;
};

}
