// Copyright (c) embedded ocean GmbH
#pragma once

#include "Attributes.hpp"

#include <xentara/data/ReadHandle.hpp>
#include <xentara/data/WriteHandle.hpp>
#include <xentara/memory/memoryResources.hpp>
#include <xentara/memory/ObjectBlock.hpp>
#include <xentara/process/Event.hpp>
#include <xentara/utils/threads/AtomicOptional.hpp>

#include <chrono>
#include <concepts>
#include <optional>
#include <memory>

// TODO: rename namespace
namespace xentara::plugins::templateDriver
{

// State information for an output.
template <std::regular DataType>
class OutputState final
{
public:
	// Resolves an attribute that belong to this state.
	// The value attribute is not resolved, as it may be shared with an output state.
	auto resolveAttribute(std::u16string_view name) -> const model::Attribute *;

	// Resolves an event.
	// Note: This function the aliasing constructor of std::shared_ptr, which will cause the returned pointer to the control block of the parent.
	// This is why the parent pointer is passed along.
	auto resolveEvent(std::u16string_view name, std::shared_ptr<void> parent) -> std::shared_ptr<process::Event>;

	// Createas a read-handle for an attribute that belong to this state.
	// This function returns std::nullopt if the attribute is unknown
	auto readHandle(const model::Attribute &attribute) const noexcept -> std::optional<data::ReadHandle>;

	// Createas a write-handle for the value attribute
	// Note: This function the aliasing constructor of std::shared_ptr, which will cause the returned pointer to the control block of the parent.
	// This is why the parent pointer is passed along.
	auto valueWriteHandle(std::shared_ptr<void> parent) noexcept -> data::WriteHandle;

	// Realizes the state
	auto realize() -> void;

	// Gets the last scheduled value, or std::nullopt if none was scheduled since the last call
	auto fetchScheduledValue() -> std::optional<DataType>
	{
		return _pendingValue.exchange(std::nullopt, std::memory_order_acq_rel);
	}

	// Updates the data and sends events
	auto update(std::chrono::system_clock::time_point timeStamp, std::error_code error) -> void;

private:
	// This structure is used to represent the state inside the memory block
	struct State final
	{
		// The last time the value was written (successfully or not)
		std::chrono::system_clock::time_point _writeTime { std::chrono::system_clock::time_point::min() };
		// The error code when writing the value, or 0 for none.
		// The error is initialized to 0, because it is not an error if the value was never written.
		attributes::ErrorCode _writeError { 0 };
	};

	// Schedules a value to be written by the write task next time it is called. This
	// function is called by value attribute write handles.
	auto schedule(const DataType &value) -> void
	{
		_pendingValue.store(value, std::memory_order_release);
	}

	// A Xentara event that is fired when the value was successfully written
	process::Event _writtenEvent { io::Direction::Output };
	// A Xentara event that is fired when a write error occurred
	process::Event _writeErrorEvent { io::Direction::Output };

	// The the last scheduled value, or std::nullopt if none is pending
	utils::threads::AtomicOptional<DataType> _pendingValue;
	// The value needs to be atomic, or blocking will occurr
	static_assert(decltype(_pendingValue)::is_always_lock_free);

	// The data block that contains the state
	memory::ObjectBlock<memory::memoryResources::Data, State> _dataBlock;
};

// TODO: add extern template statements for other supported types
extern template class OutputState<double>;

} // namespace xentara::plugins::templateDriver