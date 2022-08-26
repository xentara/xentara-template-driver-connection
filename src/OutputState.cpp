// Copyright (c) embedded ocean GmbH
#include "OutputState.hpp"

#include "Attributes.hpp"

#include <xentara/memory/WriteSentinel.hpp>

#include <string_view>

namespace xentara::plugins::templateDriver
{

using namespace std::literals;

auto OutputState::resolveAttribute(std::u16string_view name) -> const model::Attribute *
{
	// Check all the attributes we support
	return model::Attribute::resolve(name,
		model::Attribute::kWriteTime,
		attributes::kWriteError);
}

auto OutputState::resolveEvent(std::u16string_view name, std::shared_ptr<void> parent) -> std::shared_ptr<process::Event>
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

auto OutputState::readHandle(const model::Attribute &attribute) const noexcept -> std::optional<data::ReadHandle>
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

	return std::nullopt;
}

auto OutputState::realize() -> void
{
	// Create the data block
	_dataBlock.create(memory::memoryResources::data());
}

auto OutputState::update(std::chrono::system_clock::time_point timeStamp, std::error_code error) -> void
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

} // namespace xentara::plugins::templateDriver