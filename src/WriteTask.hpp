// Copyright (c) embedded ocean GmbH
#pragma once

#include <xentara/process/Task.hpp>
#include <xentara/process/ExecutionContext.hpp>

#include <chrono>
#include <functional>

// TODO: rename namespace
namespace xentara::plugins::templateDriver
{

// A concept for objects that can be used as targets for WriteTask
template <typename Target>
concept WriteTaskTarget = requires(
	Target &target, const process::ExecutionContext &context, std::chrono::system_clock::time_point timeStamp)
{
	{ target.requestConnect(timeStamp) };
	{ target.requestDisconnect(timeStamp) };
	{ target.performWriteTask(context) };
};

// This class providing callbacks for the Xentara scheduler for the "read" task of I/O points
template <WriteTaskTarget Target>
class WriteTask final : public process::Task
{
public:
	// This constuctor attached the task to its target
	WriteTask(std::reference_wrapper<Target> target) : _target(target)
	{
	}

	auto stages() const -> Stages final
	{
		return Stage::PreOperational | Stage::Operational | Stage::PostOperational;
	}

	auto preparePreOperational(const process::ExecutionContext &context) -> Status final;

	auto preOperational(const process::ExecutionContext &context) -> Status final;

	auto operational(const process::ExecutionContext &context) -> void final;

	auto preparePostOperational(const process::ExecutionContext &context) -> Status final;

private:
	// A reference to the target element
	std::reference_wrapper<Target> _target;
};

template <WriteTaskTarget Target>
auto WriteTask<Target>::preparePreOperational(const process::ExecutionContext &context) -> Status
{
	// Request a connection
	_target.get().requestConnect(context.scheduledTime());

	return Status::Ready;
}

template <WriteTaskTarget Target>
auto WriteTask<Target>::preOperational(const process::ExecutionContext &context) -> Status
{
	// We just do the same thing as in the operational stage
	operational(context);

	return Status::Ready;
}

template <WriteTaskTarget Target>
auto WriteTask<Target>::operational(const process::ExecutionContext &context) -> void
{
	// read the value
	_target.get().performWriteTask(context);
}

template <WriteTaskTarget Target>
auto WriteTask<Target>::preparePostOperational(const process::ExecutionContext &context) -> Status
{
	// Request a disconnect
	_target.get().requestDisconnect(context.scheduledTime());

	return Status::Completed;
}

} // namespace xentara::plugins::templateDriver