// Copyright (c) embedded ocean GmbH
#include "TemplateOutput.hpp"

#include "Attributes.hpp"

#include <xentara/data/DataType.hpp>
#include <xentara/data/ReadHandle.hpp>
#include <xentara/data/WriteHandle.hpp>
#include <xentara/memory/WriteSentinel.hpp>
#include <xentara/model/Attribute.hpp>
#include <xentara/process/ExecutionContext.hpp>
#include <xentara/utils/io/FileOutputStream.hpp>
#include <xentara/utils/json/decoder/Object.hpp>
#include <xentara/utils/json/decoder/Errors.hpp>
#include <xentara/utils/eh/currentErrorCode.hpp>

#include <filesystem>

namespace xentara::plugins::templateDriver
{
	
using namespace std::literals;

TemplateOutput::Class TemplateOutput::Class::_instance;

using namespace std::literals;

const model::Attribute TemplateOutput::kValueAttribute { model::Attribute::kValue, model::Attribute::Access::ReadWrite, data::DataType::kFloatingPoint };

auto TemplateOutput::loadConfig(const ConfigIntializer &initializer,
		utils::json::decoder::Object &jsonObject,
		config::Resolver &resolver,
		const FallbackConfigHandler &fallbackHandler) -> void
{
	// Get a reference that allows us to modify our own config attributes
    auto &&configAttributes = initializer[Class::instance().configHandle()];

	// Go through all the members of the JSON object that represents this object
	for (auto && [name, value] : jsonObject)
    {
		// TODO: load configuration parameters
		if (name == u8"TODO"sv)
		{
			// TODO: parse the value correctly
			auto todo = value.asNumber<std::uint64_t>();

			// TODO: check that the value is valid
			if (!"TODO")
			{
				// TODO: use an error message that tells the user exactly what is wrong
				utils::json::decoder::throwWithLocation(value, std::runtime_error("TODO is wrong with TODO parameter of template I/O component"));
			}

			// TODO: set the appropriate member variables, and update configAttributes accordingly (if necessary) 
		}
		else
		{
			// Pass any unknown parameters on to the fallback handler, which will load the built-in parameters ("id", "uuid", and "children"),
			// and throw an exception if the key is unknown
            fallbackHandler(name, value);
		}
    }

	// TODO: perform consistency and completeness checks
	if (!"TODO")
	{
		// TODO: use an error message that tells the user exactly what is wrong
		utils::json::decoder::throwWithLocation(jsonObject, std::runtime_error("TODO is wrong with template I/O component"));
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
		// TODO: read the value
		double value = {};

		// TODO: if the read function does not throw errors, but uses return types or internal handle state,
		// throw an std::system_error here on failure, or call handleReadError() directly.

		// The read was successful
		_readState.update(timeStamp, value);
	}
	catch (const std::exception &exception)
	{
		// Get the error from the current exception using this special utility function
		const auto error = utils::eh::currentErrorCode();
		// Update the state
		handleReadError(timeStamp, error);
	}
}

auto TemplateOutput::handleReadError(std::chrono::system_clock::time_point timeStamp, std::error_code error)
	-> void
{
	// Update our own state
	_readState.update(timeStamp, error);
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
	auto pendingValue = _writeState.fetchScheduledValue();
	// If there was no pending value, just bail
	if (!pendingValue)
	{
		return;
	}

	try
	{
		// TODO: write the value

		// TODO: if the write function does not throw errors, but uses return types or internal handle state,
		// throw an std::system_error here on failure, or call updateWriteState() directly.

		// The write was successful
		_writeState.update(timeStamp);
	}
	catch (const std::exception &exception)
	{
		// Get the error from the current exception using this special utility function
		const auto error = utils::eh::currentErrorCode();
		// Update the state
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

auto TemplateOutput::resolveAttribute(std::u16string_view name) -> const model::Attribute *
{
	// Check all the attributes we support directly
	// TODO: add any additional attributes this class supports, including attributes inherited from the I/O component
	if (auto attribute = model::Attribute::resolve(name,
		kValueAttribute))
	{
		return attribute;
	}

	// Check the input state attributes
	if (auto attribute = _readState.resolveAttribute(name))
	{
		return attribute;
	}
	// Check the output state attributes
	if (auto attribute = _writeState.resolveAttribute(name))
	{
		return attribute;
	}

	return nullptr;
}

auto TemplateOutput::resolveTask(std::u16string_view name) -> std::shared_ptr<process::Task>
{
	// TODO: add any additional tasks this class supports
	if (name == u"read"sv)
	{
		return std::shared_ptr<process::Task>(sharedFromThis(), &_readTask);
	}
	else if (name == u"write"sv)
	{
		return std::shared_ptr<process::Task>(sharedFromThis(), &_writeTask);
	}

	return nullptr;
}

auto TemplateOutput::resolveEvent(std::u16string_view name) -> std::shared_ptr<process::Event>
{
	// TODO: add any events this class supports directly

	// Check the input state events
	if (auto event = _readState.resolveEvent(name, sharedFromThis()))
	{
		return event;
	}
	// Check the output state events
	if (auto event = _writeState.resolveEvent(name, sharedFromThis()))
	{
		return event;
	}

	return nullptr;
}

auto TemplateOutput::readHandle(const model::Attribute &attribute) const noexcept -> data::ReadHandle
{
	// TODO: add any additional attributes this class supports
	if (attribute == kValueAttribute)
	{
		return _readState.valueReadHandle();
	}
	
	// Check the input state attributes
	if (auto handle = _readState.readHandle(attribute))
	{
		return *handle;
	}
	// Check the output state attributes
	if (auto handle = _writeState.readHandle(attribute))
	{
		return *handle;
	}

	// TODO: add any attributes inherited from the I/O component

	return data::ReadHandle::Error::Unknown;
}

auto TemplateOutput::writeHandle(const model::Attribute &attribute) noexcept -> data::WriteHandle
{
	// There is only one writable attribute
	if (attribute == kValueAttribute)
	{
		return _writeState.valueWriteHandle(sharedFromThis());
	}

	return data::WriteHandle::Error::Unknown;
}

auto TemplateOutput::prepare() -> void
{
	// Prepare the state objects
	_readState.prepare();
	_writeState.prepare();
}

auto TemplateOutput::ioComponentStateChanged(std::chrono::system_clock::time_point timeStamp, std::error_code error) -> void
{
	// Update the read state. We do not notify the I/O component, because that is who this message comes from in the first place.
	// Note: the write state is not updated, because the write state simply contains the last write error, which is unaffected
	// by I/O component errors.
	_readState.update(timeStamp, error);
}

} // namespace xentara::plugins::templateDriver