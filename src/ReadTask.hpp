// Copyright (c) embedded ocean GmbH
#pragma once

#include <xentara/process/Task.hpp>
#include <xentara/process/ExecutionContext.hpp>

#include <chrono>
#include <functional>

// TODO: rename namespace
namespace xentara::plugins::templateDriver
{

// A concept for objects that can be used as targets for ReadTask
template <typename Target>
concept ReadTaskTarget = requires(
	Target &target, const process::ExecutionContext &context, std::chrono::system_clock::time_point timeStamp)
{
	{ target.requestConnect(timeStamp) };
	{ target.requestDisconnect(timeStamp) };
	{ target.performReadTask(context) };
};

// This class providing callbacks for the Xentara scheduler for the "read" task of I/O points
template <ReadTaskTarget Target>
class ReadTask final : public process::Task
{
public:
	// This constuctor attached the task to its target
	ReadTask(std::reference_wrapper<Target> target) : _target(target)
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

template <ReadTaskTarget Target>
auto ReadTask<Target>::preparePreOperational(const process::ExecutionContext &context) -> Status
{
	// Request a connection
	_target.get().requestConnect(context.scheduledTime());

	// Read the value once to initialize it
	operational(context);

	// We are done now. Even if we couldn't read the value, we proceed to the next stage,
	// because attempting again is unlikely to succeed any better.
	return Status::Ready;
}

template <ReadTaskTarget Target>
auto ReadTask<Target>::preOperational(const process::ExecutionContext &context) -> Status
{
	// We just do the same thing as in the operational stage
	operational(context);

	return Status::Ready;
}

template <ReadTaskTarget Target>
auto ReadTask<Target>::operational(const process::ExecutionContext &context) -> void
{
	// read the value
	_target.get().performReadTask(context);
}

template <ReadTaskTarget Target>
auto ReadTask<Target>::preparePostOperational(const process::ExecutionContext &context) -> Status
{
	// Request a disconnect
	_target.get().requestDisconnect(context.scheduledTime());

	return Status::Completed;
}

} // namespace xentara::plugins::templateDriver