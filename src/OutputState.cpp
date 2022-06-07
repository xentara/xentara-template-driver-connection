// Copyright (c) embedded ocean GmbH
#include "OutputState.hpp"

#include "Attributes.hpp"

#include <xentara/memory/WriteSentinel.hpp>

namespace xentara::plugins::templateDriver
{

template <std::regular DataType>
auto OutputState<DataType>::resolveAttribute(std::u16string_view name) -> const model::Attribute *
{
	// Check all the attributes we support
	return model::Attribute::resolve(name,
		model::Attribute::kUpdateTime,
		kValueAttribute,
		model::Attribute::kChangeTime,
		model::Attribute::kQuality,
		attributes::kError);
}

template <std::regular DataType>
auto OutputState<DataType>::resolveEvent(std::u16string_view name, std::shared_ptr<void> parent) -> std::shared_ptr<process::Event>
{
	// Check all the events we support
	if (name == model::Attribute::kValue)
	{
		return std::shared_ptr<process::Event>(parent, &_valueChangedEvent);
	}
	else if (name == model::Attribute::kQuality)
	{
		return std::shared_ptr<process::Event>(parent, &_qualityChangedEvent);
	}
	else if (name == process::Event::kChanged)
	{
		return std::shared_ptr<process::Event>(parent, &_changedEvent);
	}

	// The event name is not known
	return nullptr;
}

template <std::regular DataType>
auto OutputState<DataType>::readHandle(const model::Attribute &attribute) const noexcept -> std::optional<data::ReadHandle>
{
	// Try reach readable attribute
	if (attribute == model::Attribute::kUpdateTime)
	{
		return _dataBlock.member(&State::_updateTime);
	}
	else if (attribute == kValueAttribute)
	{
		return _dataBlock.member(&State::_value);
	}
	else if (attribute == model::Attribute::kChangeTime)
	{
		return _dataBlock.member(&State::_changeTime);
	}
	else if (attribute == model::Attribute::kQuality)
	{
		return _dataBlock.member(&State::_quality);
	}
	else if (attribute == attributes::kError)
	{
		return _dataBlock.member(&State::_error);
	}

	return data::ReadHandle::Error::Unknown;
}

template <std::regular DataType>
auto OutputState<DataType>::valueWriteHandle(std::shared_ptr<void> parent) noexcept -> data::WriteHandle
{
	return { std::in_place_type<double>, &OutputState<DataType>::schedule, std::shared_ptr<process::Event>(parent, this) };
}

template <std::regular DataType>
auto OutputState<DataType>::prepare() -> void
{
	// Create the data block
	_dataBlock.create(memory::memoryResources::data());
}

template <std::regular DataType>
auto OutputState<DataType>::update(std::chrono::system_clock::time_point time, std::error_code error) -> void
{
	// Make a write sentinel
	memory::WriteSentinel sentinel { _dataBlock };
	auto &state = *sentinel;

	// Update the state
	state._writeTime = context.scheduledTime();
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