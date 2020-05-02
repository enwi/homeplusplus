#pragma once

#include <json.hpp>

// Wraps a nlohmann::json object because there are problems with conversion when using it as mocked functions'
// parameters
class JsonWrapper
{
public:
    explicit JsonWrapper(const nlohmann::json& json) : json(json) {}
    nlohmann::json& get() { return json; }
    const nlohmann::json& get() const { return json; }
	// Equality to simplify matching
    bool operator==(const JsonWrapper& rhs) const { return json == rhs.get(); }

private:
    nlohmann::json json;
};