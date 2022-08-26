// Copyright (c) embedded ocean GmbH
#pragma once

#include "TemplateIoComponent.hpp"
#include "ReadState.hpp"
#include "WriteState.hpp"
#include "ReadTask.hpp"
#include "SingleValueQueue.hpp"
#include "WriteTask.hpp"

#include <xentara/io/Io.hpp>
#include <xentara/io/IoClass.hpp>
#include <xentara/plugin/EnableSharedFromThis.hpp>
#include <xentara/process/Task.hpp>

#include <functional>
#include <string_view>

// TODO: rename namespace
namespace xentara::plugins::templateDriver
{

using namespace std::literals;

// A class representing a specific type of output.
// TODO: rename this class to something more descriptive
class TemplateOutput final : public io::Io, public TemplateIoComponent::ErrorSink, public plugin::EnableSharedFromThis<TemplateOutput>
{
private:
	// A structure used to store the class specific attributes within an element's configuration
	struct Config final
	{
		// TODO: Add custom config attributes
	};
	
public:
	// The class object containing meta-information about this element type
	class Class final : public io::IoClass
	{
	public:
		// Gets the global object
		static auto instance() -> Class&
		{
			return _instance;
		}

	    // Returns the array handle for the class specific attributes within an element's configuration
	    auto configHandle() const -> const auto &
        {
            return _configHandle;
        }

		auto name() const -> std::u16string_view final
		{
			// TODO: change class name
			return u"TemplateOutput"sv;
		}
	
		auto uuid() const -> utils::core::Uuid final
		{
			// TODO: assign a unique UUID
			return "dddddddd-dddd-dddd-dddd-dddddddddddd"_uuid;
		}

	private:
	    // The array handle for the class specific attributes within an element's configuration
		memory::Array::ObjectHandle<Config> _configHandle { config().appendObject<Config>() };

		// The global object that represents the class
		static Class _instance;
	};

	// This constructor attaches the output to its I/O component
	TemplateOutput(std::reference_wrapper<TemplateIoComponent> ioComponent) :
		_ioComponent(ioComponent)
	{
		ioComponent.get().addErrorSink(*this);
	}

	auto dataType() const -> const data::DataType &;

	auto directions() const -> io::Directions;

	auto resolveAttribute(std::u16string_view name) -> const model::Attribute * final;
	
	auto resolveTask(std::u16string_view name) -> std::shared_ptr<process::Task> final;

	auto resolveEvent(std::u16string_view name) -> std::shared_ptr<process::Event> final;

	auto readHandle(const model::Attribute &attribute) const noexcept -> data::ReadHandle final;

	auto writeHandle(const model::Attribute &attribute) noexcept -> data::WriteHandle final;

	auto realize() -> void final;

	auto ioComponentStateChanged(std::chrono::system_clock::time_point timeStamp, std::error_code error) -> void final;

	// A Xentara attribute containing the current value. This is a member of this class rather than
	// of the attributes namespace, because the access flags and type may differ from class to class
	static const model::Attribute kValueAttribute;

protected:
	auto loadConfig(const ConfigIntializer &initializer,
		utils::json::decoder::Object &jsonObject,
		config::Resolver &resolver,
		const FallbackConfigHandler &fallbackHandler) -> void final;

private:
	// The tasks need access to out private member functions
	friend class ReadTask<TemplateOutput>;
	friend class WriteTask<TemplateOutput>;

	// This function is forwarded to the I/O component.
	auto requestConnect(std::chrono::system_clock::time_point timeStamp) noexcept -> void
	{
		_ioComponent.get().requestConnect(timeStamp);
	}

	// This function is forwarded to the I/O component.
	auto requestDisconnect(std::chrono::system_clock::time_point timeStamp) noexcept -> void
	{
		_ioComponent.get().requestDisconnect(timeStamp);
	}

	// This function is called by the "read" task. It attempts to read the value if the I/O component is up.
	auto performReadTask(const process::ExecutionContext &context) -> void;
	// Attempts to read the data from the I/O component and updates the state accordingly.
	auto read(std::chrono::system_clock::time_point timeStamp) -> void;
	// Handles a read error
	auto handleReadError(std::chrono::system_clock::time_point timeStamp, std::error_code error) -> void;

	// This function is called by the "write" task. It attempts to write the value if the I/O component is up.
	auto performWriteTask(const process::ExecutionContext &context) -> void;
	// Attempts to write any pending value to the I/O component and updates the state accordingly.
	auto write(std::chrono::system_clock::time_point timeStamp) -> void;	
	// Handles a write error
	auto handleWriteError(std::chrono::system_clock::time_point timeStamp, std::error_code error) -> void;

	// Schedules a value to be written. This function is called by the value write handle.
	// TODO: use the correct value type
	auto scheduleOutputValue(double value) noexcept
	{
		_pendingOutputValue.enqueue(value);
	}

	// The I/O component this output belongs to
	// TODO: give this a more descriptive name, e.g. "_device"
	std::reference_wrapper<TemplateIoComponent> _ioComponent;

	// The read state
	// TODO: use the correct value type
	ReadState<double> _readState;
	// The write state
	// TODO: use the correct value type
	WriteState _writeState;

	// The queue for the pending output value
	// TODO: use the correct value type
	SingleValueQueue<double> _pendingOutputValue;

	// The "read" task
	ReadTask<TemplateOutput> _readTask { *this };
	// The "write" task
	WriteTask<TemplateOutput> _writeTask { *this };
};

} // namespace xentara::plugins::templateDriver