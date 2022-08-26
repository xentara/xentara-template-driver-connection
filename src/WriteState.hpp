// Copyright (c) embedded ocean GmbH
#pragma once

#include "Attributes.hpp"

#include <xentara/data/ReadHandle.hpp>
#include <xentara/memory/memoryResources.hpp>
#include <xentara/memory/ObjectBlock.hpp>
#include <xentara/process/Event.hpp>

#include <chrono>
#include <concepts>
#include <optional>
#include <memory>

// TODO: rename namespace
namespace xentara::plugins::templateDriver
{

// State information for a write operation.
class WriteState final
{
public:
	// Resolves an attribute that belong to this state.
	auto resolveAttribute(std::u16string_view name) -> const model::Attribute *;

	// Resolves an event.
	// Note: This function uses the aliasing constructor of std::shared_ptr, which will cause the returned pointer to the control block of the parent.
	// This is why the parent pointer is passed along.
	auto resolveEvent(std::u16string_view name, std::shared_ptr<void> parent) -> std::shared_ptr<process::Event>;

	// Createas a read-handle for an attribute that belong to this state.
	// This function returns std::nullopt if the attribute is unknown
	auto readHandle(const model::Attribute &attribute) const noexcept -> std::optional<data::ReadHandle>;

	// Realizes the state
	auto realize() -> void;

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

	// A Xentara event that is fired when the value was successfully written
	process::Event _writtenEvent { io::Direction::Output };
	// A Xentara event that is fired when a write error occurred
	process::Event _writeErrorEvent { io::Direction::Output };

	// The data block that contains the state
	memory::ObjectBlock<memory::memoryResources::Data, State> _dataBlock;
};

} // namespace xentara::plugins::templateDriver