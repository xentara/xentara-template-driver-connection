// Copyright (c) embedded ocean GmbH
#include "OutputState.hpp"

#include "Attributes.hpp"

#include <xentara/memory/WriteSentinel.hpp>

#include <string_view>

namespace xentara::plugins::templateDriver
{

using namespace std::literals;

template <std::regular DataType>
auto OutputState<DataType>::resolveAttribute(std::u16string_view name) -> const model::Attribute *
{
	// Check all the attributes we support
	return model::Attribute::resolve(name,
		model::Attribute::kWriteTime,
		attributes::kWriteError);
}

template <std::regular DataType>
auto OutputState<DataType>::resolveEvent(std::u16string_view name, std::shared_ptr<void> parent) -> std::shared_ptr<process::Event>
{
	// Check all the events we support
	if (name == u"written"sv)
	{
		return std::shared_ptr<process::Event>(parent, &_writtenEvent);
	}
	else if (name == u"writeError"sv)
	{
		return std::shared_ptr<process::Event>(parent, &_writeErrorEvent);
	}

	// The event name is not known
	return nullptr;
}

template <std::regular DataType>
auto OutputState<DataType>::readHandle(const model::Attribute &attribute) const noexcept -> std::optional<data::ReadHandle>
{
	// Try reach readable attribute
	if (attribute == model::Attribute::kWriteTime)
	{
		return _dataBlock.member(&State::_writeTime);
	}
	else if (attribute == attributes::kWriteError)
	{
		return _dataBlock.member(&State::_writeError);
	}

	return data::ReadHandle::Error::Unknown;
}

template <std::regular DataType>
auto OutputState<DataType>::valueWriteHandle(std::shared_ptr<void> parent) noexcept -> data::WriteHandle
{
	return { std::in_place_type<DataType>, &OutputState<DataType>::schedule, std::shared_ptr<OutputState<DataType>>(parent, this) };
}

template <std::regular DataType>
auto OutputState<DataType>::realize() -> void
{
	// Create the data block
	_dataBlock.create(memory::memoryResources::data());
}

template <std::regular DataType>
auto OutputState<DataType>::update(std::chrono::system_clock::time_point timeStamp, std::error_code error) -> void
{
	// Make a write sentinel
	memory::WriteSentinel sentinel { _dataBlock };
	auto &state = *sentinel;

	// Update the state
	state._writeTime = timeStamp;
	state._writeError = attributes::errorCode(error);
	// Commit the data before sending the event
	sentinel.commit();

	// Fire the correct event
	if (!error)
	{
		_writtenEvent.fire();
	}
	else
	{
		_writeErrorEvent.fire();
	}
}

// TODO: add template instantiations for other supported types
template class OutputState<double>;

} // namespace xentara::plugins::templateDriver