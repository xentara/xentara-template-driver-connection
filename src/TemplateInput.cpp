// Copyright (c) embedded ocean GmbH
#include "TemplateInput.hpp"

#include "Attributes.hpp"

#include <xentara/data/DataType.hpp>
#include <xentara/data/ReadHandle.hpp>
#include <xentara/memory/WriteSentinel.hpp>
#include <xentara/model/Attribute.hpp>
#include <xentara/process/ExecutionContext.hpp>
#include <xentara/utils/io/FileInputStream.hpp>
#include <xentara/utils/json/decoder/Object.hpp>
#include <xentara/utils/json/decoder/Errors.hpp>
#include <xentara/utils/eh/currentErrorCode.hpp>

#include <filesystem>
#include <charconv>

namespace xentara::plugins::templateDriver
{
	
using namespace std::literals;

TemplateInput::Class TemplateInput::Class::_instance;

using namespace std::literals;

const model::Attribute TemplateInput::kValueAttribute { model::Attribute::kValue, model::Attribute::Access::ReadOnly, data::DataType::kFloatingPoint };

auto TemplateInput::loadConfig(const ConfigIntializer &initializer,
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

auto TemplateInput::performReadTask(const process::ExecutionContext &context) -> void
{
	// Only perform the reconnect if we are supposed to be connected in the first place
	if (!_ioComponent.get().connected())
	{
		return;
	}

	// Read the data
	read(context.scheduledTime());
}

auto TemplateInput::read(std::chrono::system_clock::time_point timeStamp) -> void
{
	try
	{
		// TODO: read the value
		double value = {};

		// TODO: if the read function does not throw errors, but uses return types or internal handle state,
		// throw an std::system_error here on failure, or call handleReadError() directly.

		// The read was successful
		_state.update(timeStamp, value);
	}
	catch (const std::exception &exception)
	{
		// Get the error from the current exception using this special utility function
		const auto error = utils::eh::currentErrorCode();
		// Update the state
		handleReadError(timeStamp, error);
	}
}

auto TemplateInput::handleReadError(std::chrono::system_clock::time_point timeStamp, std::error_code error)
	-> void
{
	// Update our own state
	_state.update(timeStamp,error);
	// Notify the I/O component
	_ioComponent.get().handleError(timeStamp, error, this);
}

auto TemplateInput::dataType() const -> const data::DataType &
{
	return kValueAttribute.dataType();
}

auto TemplateInput::directions() const -> io::Directions
{
	return io::Direction::Input;
}

auto TemplateInput::resolveAttribute(std::u16string_view name) -> const model::Attribute *
{
	// Check all the attributes we support directly
	// TODO: add any additional attributes this class supports, including attributes inherited from the I/O component
	if (auto attribute = model::Attribute::resolve(name,
		kValueAttribute))
	{
		return attribute;
	}

	// Check the input state attributes
	if (auto attribute = _state.resolveAttribute(name))
	{
		return attribute;
	}

	return nullptr;
}

auto TemplateInput::resolveTask(std::u16string_view name) -> std::shared_ptr<process::Task>
{
	// TODO: add any additional tasks this class supports
	if (name == u"read"sv)
	{
		return std::shared_ptr<process::Task>(sharedFromThis(), &_readTask);
	}

	return nullptr;
}

auto TemplateInput::resolveEvent(std::u16string_view name) -> std::shared_ptr<process::Event>
{
	// TODO: add any events this class supports directly

	// Check the input state events
	if (auto event = _state.resolveEvent(name, sharedFromThis()))
	{
		return event;
	}

	return nullptr;
}

auto TemplateInput::readHandle(const model::Attribute &attribute) const noexcept -> data::ReadHandle
{
	// TODO: add any additional attributes this class supports
	if (attribute == kValueAttribute)
	{
		return _state.valueReadHandle();
	}
	
	// Check the input state attributes
	if (auto handle = _state.readHandle(attribute))
	{
		return *handle;
	}

	// TODO: add any attributes inherited from the I/O component

	return data::ReadHandle::Error::Unknown;
}

auto TemplateInput::prepare() -> void
{
	// Prepare the state object
	_state.prepare();
}

auto TemplateInput::ioComponentStateChanged(std::chrono::system_clock::time_point timeStamp, std::error_code error) -> void
{
	// Update the state. We do not notify the I/O component, because that is who this message comes from in the first place.
	_state.update(timeStamp, error);
}

} // namespace xentara::plugins::templateDriver