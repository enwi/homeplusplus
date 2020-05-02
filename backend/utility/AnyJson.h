#pragma once

#include <json.hpp>
#include <google/protobuf/any.pb.h>
#include <google/protobuf/wrappers.pb.h>

template <typename WrapperType>
nlohmann::json UnpackAny(const google::protobuf::Any& any)
{
	if (any.Is<WrapperType>())
	{
		WrapperType val;
		if (any.UnpackTo(&val))
		{
			return val.value();
		}
	}
	throw std::runtime_error("UnpackAny failed");
}

template <typename T1, typename T2, typename... WrapperTypes>
nlohmann::json UnpackAny(const google::protobuf::Any& any)
{
	// Recursive implementation
	if (any.Is<T1>())
	{
		T1 val;
		if (!any.UnpackTo(&val))
		{
			throw std::runtime_error("UnpackAny failed");
		}
		return val.value();
	}
	return UnpackAny<T2, WrapperTypes...>(any);
}

inline nlohmann::json UnpackAny(const google::protobuf::Any& any)
{
	return UnpackAny<google::protobuf::BoolValue, google::protobuf::Int32Value, google::protobuf::Int64Value,
		google::protobuf::UInt32Value, google::protobuf::UInt64Value, google::protobuf::DoubleValue,
		google::protobuf::FloatValue, google::protobuf::StringValue>(any);
}

// json must not be null, Any cannot express null type
inline google::protobuf::Any JsonToAny(const nlohmann::json& json)
{
	google::protobuf::Any result;
	switch (json.type())
	{
	case nlohmann::json::value_t::boolean:
	{
		google::protobuf::BoolValue v;
		v.set_value(json);
		result.PackFrom(v);
		break;
	}
	case nlohmann::json::value_t::number_float:
	{
		google::protobuf::DoubleValue v;
		v.set_value(json);
		result.PackFrom(v);
		break;
	}
	case nlohmann::json::value_t::number_integer:
	{
		google::protobuf::Int64Value v;
		v.set_value(json);
		result.PackFrom(v);
		break;
	}
	case nlohmann::json::value_t::number_unsigned:
	{
		google::protobuf::UInt64Value v;
		v.set_value(json);
		result.PackFrom(v);
		break;
	}
	case nlohmann::json::value_t::string:
	{
		google::protobuf::StringValue v;
		v.set_value(json);
		result.PackFrom(v);
		break;
	}
	case nlohmann::json::value_t::null:
		throw std::logic_error("JsonToAny: json is null");
	default:
		throw std::logic_error("JsonToAny: unsupported type");
	}
	return result;
}