// Copyright (c) embedded ocean GmbH
#include "TemplateOutput.hpp"

#include "Attributes.hpp"
#include "Tasks.hpp"

#include <xentara/config/Errors.hpp>
#include <xentara/data/DataType.hpp>
#include <xentara/data/ReadHandle.hpp>
#include <xentara/data/WriteHandle.hpp>
#include <xentara/model/Attribute.hpp>
#include <xentara/model/ForEachAttributeFunction.hpp>
#include <xentara/model/ForEachEventFunction.hpp>
#include <xentara/model/ForEachTaskFunction.hpp>
#include <xentara/process/ExecutionContext.hpp>
#include <xentara/utils/json/decoder/Object.hpp>
#include <xentara/utils/json/decoder/Errors.hpp>
#include <xentara/utils/eh/currentErrorCode.hpp>

namespace xentara::plugins::templateDriver
{
	
using namespace std::literals;

const model::Attribute TemplateOutput::kValueAttribute { model::Attribute::kValue, model::Attribute::Access::ReadWrite, data::DataType::kFloatingPoint };

auto TemplateOutput::load(utils::json::decoder::Object &jsonObject, config::Context &context) -> void
{
	// Go through all the members of the JSON object that represents this object
	for (auto && [name, value] : jsonObject)
    {
		/// @todo load configuration parameters
		if (name == "TODO"sv)
		{
			/// @todo parse the value correctly
			auto todo = value.asNumber<std::uint64_t>();

			/// @todo check that the value is valid
			if (!"TODO")
			{
				/// @todo use an error message that tells the user exactly what is wrong
				utils::json::decoder::throwWithLocation(value, std::runtime_error("TODO is wrong with TODO parameter of template output"));
			}

			/// @todo set the appropriate member variables
		}
		else
		{
            config::throwUnknownParameterError(name);
		}
    }

	/// @todo perform consistency and completeness checks
	if (!"TODO")
	{
		/// @todo use an error message that tells the user exactly what is wrong
		utils::json::decoder::throwWithLocation(jsonObject, std::runtime_error("TODO is wrong with template output"));
	}
}

auto TemplateOutput::performReadTask(const process::ExecutionContext &context) -> void
{
	// Only perform the read only if the I/O component is connected
	if (!_ioComponent.get().connected())
	{
		return;
	}

	// Read the data
	read(context.scheduledTime());
}

auto TemplateOutput::read(std::chrono::system_clock::time_point timeStamp) -> void
{
	try
	{
		/// @todo read the value
		double value = {};

		/// @todo if the read function does not throw errors, but uses return types or internal handle state,
		// throw an std::system_error here on failure, or call handleReadError() directly.

		// The read was successful
		_readState.update(timeStamp, value);
	}
	catch (const std::exception &)
	{
		// Get the error from the current exception using this special utility function
		const auto error = utils::eh::currentErrorCode();
		// Handle the error
		handleReadError(timeStamp, error);
	}
}

auto TemplateOutput::handleReadError(std::chrono::system_clock::time_point timeStamp, std::error_code error)
	-> void
{
	// Update our own state
	_readState.update(timeStamp, utils::eh::unexpected(error));
	// Notify the I/O component
	_ioComponent.get().handleError(timeStamp, error, this);
}

auto TemplateOutput::performWriteTask(const process::ExecutionContext &context) -> void
{
	// Only perform the read only if the I/O component is connected
	if (!_ioComponent.get().connected())
	{
		return;
	}

	// Write the data
	write(context.scheduledTime());
}

auto TemplateOutput::write(std::chrono::system_clock::time_point timeStamp) -> void
{
	// Get the value
	auto pendingValue = _pendingOutputValue.dequeue();
	// If there was no pending value, just bail
	if (!pendingValue)
	{
		return;
	}

	try
	{
		/// @todo write the value

		/// @todo if the write function does not throw errors, but uses return types or internal handle state,
		// throw an std::system_error here on failure, or call handleWriteError() directly.

		// The write was successful
		_writeState.update(timeStamp, std::error_code());
	}
	catch (const std::exception &)
	{
		// Get the error from the current exception using this special utility function
		const auto error = utils::eh::currentErrorCode();
		// Handle the error
		handleWriteError(timeStamp, error);
	}
}

auto TemplateOutput::handleWriteError(std::chrono::system_clock::time_point timeStamp, std::error_code error)
	-> void
{
	// Update our own state
	_writeState.update(timeStamp, error);
	// Notify the I/O component
	_ioComponent.get().handleError(timeStamp, error, this);
}

auto TemplateOutput::dataType() const -> const data::DataType &
{
	return kValueAttribute.dataType();
}

auto TemplateOutput::directions() const -> io::Directions
{
	return io::Direction::Input | io::Direction::Output;
}

auto TemplateOutput::forEachAttribute(const model::ForEachAttributeFunction &function) const -> bool
{
	return
		// Handle all the attributes we support directly
		function(kValueAttribute) ||

		// Handle the read state attributes
		_readState.forEachAttribute(function) ||
		// Handle the write state attributes
		_writeState.forEachAttribute(function);

	/// @todo handle any additional attributes this class supports, including attributes inherited from the I/O component
}

auto TemplateOutput::forEachEvent(const model::ForEachEventFunction &function) -> bool
{
	return
		// Handle the read state events
		_readState.forEachEvent(function, sharedFromThis()) ||
		// Handle the write state events
		_writeState.forEachEvent(function, sharedFromThis());

	/// @todo handle any additional events this class supports, including events inherited from the I/O component
}

auto TemplateOutput::forEachTask(const model::ForEachTaskFunction &function) -> bool
{
	// Handle all the tasks we support
	return
		function(tasks::kRead, sharedFromThis(&_readTask)) ||
		function(tasks::kWrite, sharedFromThis(&_writeTask));

	/// @todo handle any additional tasks this class supports
}

auto TemplateOutput::makeReadHandle(const model::Attribute &attribute) const noexcept -> std::optional<data::ReadHandle>
{
	// Handle the value attribute separately
	if (attribute == kValueAttribute)
	{
		return _readState.valueReadHandle();
	}
	
	// Handle the read state attributes
	if (auto handle = _readState.makeReadHandle(attribute))
	{
		return handle;
	}
	// Handle the write state attributes
	if (auto handle = _writeState.makeReadHandle(attribute))
	{
		return handle;
	}

	/// @todo handle any additional readable attributes this class supports, including attributes inherited from the I/O component

	return std::nullopt;
}

auto TemplateOutput::makeWriteHandle(const model::Attribute &attribute) noexcept -> std::optional<data::WriteHandle>
{
	// Handle the value attribute
	if (attribute == kValueAttribute)
	{
		// This magic code creates a write handle of type double that calls scheduleWrite() on this.
		/// @todo use the correct value type
		return data::WriteHandle { std::in_place_type<double>, &TemplateOutput::scheduleOutputValue, weakFromThis() };
	}

	/// @todo handle any additional writable attributes this class supports, including attributes inherited from the I/O component

	return std::nullopt;
}

auto TemplateOutput::realize() -> void
{
	// Realize the state objects
	_readState.realize();
	_writeState.realize();
}

auto TemplateOutput::ioComponentStateChanged(std::chrono::system_clock::time_point timeStamp, std::error_code error) -> void
{
	// We cannot reset the error to Ok because we don't have a value. So we use the special custom error code instead.
	auto effectiveError = error ? error : CustomError::NoData;

	// Update the read state. We do not notify the I/O component, because that is who this message comes from in the first place.
	// Note: the write state is not updated, because the write state simply contains the last write error, which is unaffected
	// by I/O component errors.
	_readState.update(timeStamp, utils::eh::unexpected(effectiveError));
}

} // namespace xentara::plugins::templateDriver