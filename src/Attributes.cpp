// Copyright (c) embedded ocean GmbH
#include "Attributes.hpp"

#include <xentara/utils/core/Uuid.hpp>
#include <xentara/data/DataType.hpp>

#include <string_view>

namespace xentara::plugins::templateDriver::attributes
{

using namespace std::literals;

const model::Attribute kError { model::Attribute::kError, model::Attribute::Access::ReadOnly, data::DataType::kErrorCode };

const model::Attribute kWriteError { model::Attribute::kWriteError, model::Attribute::Access::ReadOnly, data::DataType::kErrorCode };

/// @todo assign a unique UUID
const model::Attribute kConnectionTime { "deadbeef-dead-beef-dead-beefdeadbeef"_uuid, "connectionTime"sv, model::Attribute::Access::ReadOnly, data::DataType::kTimeStamp };

/// @todo assign a unique UUID
const model::Attribute kDeviceError { "deadbeef-dead-beef-dead-beefdeadbeef"_uuid, "error"sv, model::Attribute::Access::ReadOnly, data::DataType::kErrorCode };

} // namespace xentara::plugins::templateDriver::attributes
