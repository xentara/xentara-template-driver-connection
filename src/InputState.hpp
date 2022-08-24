// Copyright (c) embedded ocean GmbH
#pragma once

#include "Attributes.hpp"

#include <xentara/data/Quality.hpp>
#include <xentara/data/ReadHandle.hpp>
#include <xentara/memory/memoryResources.hpp>
#include <xentara/memory/ObjectBlock.hpp>
#include <xentara/process/Event.hpp>
#include <xentara/utils/eh/Failable.hpp>

#include <chrono>
#include <concepts>
#include <optional>
#include <memory>

// TODO: rename namespace
namespace xentara::plugins::templateDriver
{

// State information for an input.
template <std::regular DataType>
class InputState final
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
	// The value attribute is not handled, it must be gotten separately using valueReadHandle().
	// This function returns std::nullopt if the attribute is unknown (including the value attribute)
	auto readHandle(const model::Attribute &attribute) const noexcept -> std::optional<data::ReadHandle>;

	// Createas a read-handle for the value attribute
	auto valueReadHandle() const noexcept -> data::ReadHandle;

	// Realizes the state
	auto realize() -> void;

	// Updates the data and sends events
	// Note: utils::eh::Failable is a variant that can hold either a value or an std::error_code
	auto update(std::chrono::system_clock::time_point timeStamp, const utils::eh::Failable<DataType> &valueOrError) -> void;

private:
	// This structure is used to represent the state inside the memory block
	struct State final
	{
		// The update time stamp
		std::chrono::system_clock::time_point _updateTime { std::chrono::system_clock::time_point::min() };
		// The current value
		DataType _value {};
		// The change time stamp
		std::chrono::system_clock::time_point _changeTime { std::chrono::system_clock::time_point::min() };
		// The quality of the value
		data::Quality _quality { data::Quality::Bad };
		// The error code when reading the value, or 0 for none.
		attributes::ErrorCode _error { attributes::errorCode(CustomError::NotConnected) };
	};

	// A Xentara event that is fired when the value changes
	process::Event _valueChangedEvent { model::Attribute::kValue };
	// A Xentara event that is fired when the quality changes
	process::Event _qualityChangedEvent { model::Attribute::kQuality };

	// A summary event that is fired when anything changes
	process::Event _changedEvent { io::Direction::Input };

	// The data block that contains the state
	memory::ObjectBlock<memory::memoryResources::Data, State> _dataBlock;
};

// TODO: add extern template statements for other supported types
extern template class InputState<double>;

} // namespace xentara::plugins::templateDriver