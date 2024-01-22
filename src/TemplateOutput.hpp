// Copyright (c) embedded ocean GmbH
#pragma once

#include "TemplateIoComponent.hpp"
#include "ReadState.hpp"
#include "WriteState.hpp"
#include "ReadTask.hpp"
#include "SingleValueQueue.hpp"
#include "WriteTask.hpp"

#include <xentara/process/Task.hpp>
#include <xentara/skill/DataPoint.hpp>
#include <xentara/skill/EnableSharedFromThis.hpp>

#include <functional>
#include <string_view>

namespace xentara::plugins::templateDriver
{

using namespace std::literals;

/// @brief A class representing a specific type of output.
/// @todo rename this class to something more descriptive
class TemplateOutput final : public skill::DataPoint, public TemplateIoComponent::ErrorSink, public skill::EnableSharedFromThis<TemplateOutput>
{
public:
	/// @brief The class object containing meta-information about this element type
	/// @todo change class name
	/// @todo assign a unique UUID
	/// @todo change display name
	using Class =
		ConcreteClass<"TemplateOutput", "deadbeef-dead-beef-dead-beefdeadbeef"_uuid, "template driver output">;

	/// @brief This constructor attaches the output to its I/O component
	TemplateOutput(std::reference_wrapper<TemplateIoComponent> ioComponent) :
		_ioComponent(ioComponent)
	{
		ioComponent.get().addErrorSink(*this);
	}
	
	/// @name Virtual Overrides for skill::DataPoint
	/// @{

	auto dataType() const -> const data::DataType & final;

	auto directions() const -> io::Directions final;

	auto forEachAttribute(const model::ForEachAttributeFunction &function) const -> bool final;
	
	auto forEachEvent(const model::ForEachEventFunction &function) -> bool final;

	auto forEachTask(const model::ForEachTaskFunction &function) -> bool final;

	auto makeReadHandle(const model::Attribute &attribute) const noexcept -> std::optional<data::ReadHandle> final;

	auto makeWriteHandle(const model::Attribute &attribute) noexcept -> std::optional<data::WriteHandle> final;

	auto realize() -> void final;

	/// @}

	/// @name Virtual Overrides for TemplateIoComponent::ErrorSink
	/// @{

	auto ioComponentStateChanged(std::chrono::system_clock::time_point timeStamp, std::error_code error) -> void final;

	/// @}

	/// @brief A Xentara attribute containing the current value.
	/// @note This is a member of this class rather than of the attributes namespace, because the access flags
	/// and type may differ from class to class
	static const model::Attribute kValueAttribute;

protected:
	/// @name Virtual Overrides for skill::DataPoint
	/// @{

	auto load(utils::json::decoder::Object &jsonObject,
		config::Resolver &resolver,
		const config::FallbackHandler &fallbackHandler) -> void final;

	/// @}

private:
	// The tasks need access to out private member functions
	friend class ReadTask<TemplateOutput>;
	friend class WriteTask<TemplateOutput>;

	/// @brief This function is forwarded to the I/O component.
	auto requestConnect(std::chrono::system_clock::time_point timeStamp) noexcept -> void
	{
		_ioComponent.get().requestConnect(timeStamp);
	}

	/// @brief This function is forwarded to the I/O component.
	auto requestDisconnect(std::chrono::system_clock::time_point timeStamp) noexcept -> void
	{
		_ioComponent.get().requestDisconnect(timeStamp);
	}

	/// @brief This function is called by the "read" task.
	///
	/// This function attempts to read the value if the I/O component is up.
	auto performReadTask(const process::ExecutionContext &context) -> void;
	/// @brief Attempts to read the data from the I/O component and updates the state accordingly.
	auto read(std::chrono::system_clock::time_point timeStamp) -> void;
	/// @brief Handles a read error
	auto handleReadError(std::chrono::system_clock::time_point timeStamp, std::error_code error) -> void;

	/// @brief This function is called by the "write" task.
	///
	/// This function attempts to write the value if the I/O component is up.
	auto performWriteTask(const process::ExecutionContext &context) -> void;
	/// @brief Attempts to write any pending value to the I/O component and updates the state accordingly.
	auto write(std::chrono::system_clock::time_point timeStamp) -> void;	
	/// @brief Handles a write error
	auto handleWriteError(std::chrono::system_clock::time_point timeStamp, std::error_code error) -> void;

	/// @brief Schedules a value to be written.
	/// 
	/// This function is called by the value write handle.
	/// @todo use the correct value type
	auto scheduleOutputValue(double value) noexcept
	{
		_pendingOutputValue.enqueue(value);
	}

	/// @brief The I/O component this output belongs to
	/// @todo give this a more descriptive name, e.g. "_device"
	std::reference_wrapper<TemplateIoComponent> _ioComponent;

	/// @brief The read state
	/// @todo use the correct value type
	ReadState<double> _readState;
	/// @brief The write state
	WriteState _writeState;

	/// @brief The queue for the pending output value
	/// @todo use the correct value type
	SingleValueQueue<double> _pendingOutputValue;

	/// @brief The "read" task
	ReadTask<TemplateOutput> _readTask { *this };
	/// @brief The "write" task
	WriteTask<TemplateOutput> _writeTask { *this };
};

} // namespace xentara::plugins::templateDriver