// Copyright (c) embedded ocean GmbH
#pragma once

#include "TemplateIoComponent.hpp"
#include "TemplateOutput.hpp"
#include "TemplateInput.hpp"

#include <xentara/utils/core/Uuid.hpp>
#include <xentara/io/Driver.hpp>

#include <string_view>

// TODO: rename namespace
namespace xentara::plugins::templateDriver
{

using namespace std::literals;

// The driver class
class Driver final : public io::Driver
{
public:
	auto name() const -> std::u16string_view final
	{
		// TODO: change class name
		return u"TemplateDriver"sv;
	}

	auto uuid() const -> utils::core::Uuid final
	{
		// TODO: assign a unique UUID
		return "aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa"_uuid;
	}

	auto registerObjects(Registry &registry) -> void final
	{
		// TODO: register any additional element classes
		registry <<
			TemplateIoComponent::Class::instance() <<
			TemplateOutput::Class::instance() <<
			TemplateInput::Class::instance();
	}

	auto createEnvironment() -> std::unique_ptr<io::Driver::Environment> final;

private:
	// The driver runtime environment
	class Environment;
};

} // namespace xentara::plugins::templateDriver