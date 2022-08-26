// Copyright (c) embedded ocean GmbH
#pragma once

#include <xentara/utils/threads/AtomicOptional.hpp>

// TODO: rename namespace
namespace xentara::plugins::templateDriver
{

// A thread-safe, lock-free queue that can hold a single value.
// This queues only allows enqueuing a single value. Enqueuing a second value will overwrite the first.
template <typename DataType>
class SingleValueQueue final
{
public:
	// Enqueues a value to be written by the write task next time it is called. This
	// function is called by write handles.
	auto enqueue(const DataType &value) noexcept -> void
	{
		_value.store(value, std::memory_order_release);
	}

	// Gets the last scheduled value, or std::nullopt if none was scheduled since the last call
	auto dequeue() noexcept -> std::optional<DataType>
	{
		return _value.exchange(std::nullopt, std::memory_order_acq_rel);
	}

private:
	// The queued value, or std::nullopt if the queue is empty.
	utils::threads::AtomicOptional<DataType> _value;

	// The value needs to be atomic, or blocking will occurr
	static_assert(decltype(_value)::is_always_lock_free);
};

} // namespace xentara::plugins::templateDriver