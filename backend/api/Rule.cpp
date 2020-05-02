#include "Rule.h"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <vector>

#include "api/rule_conditions.pb.h"
#include <google/protobuf/wrappers.pb.h>

#include "DeviceRegistry.h"
#include "IRuleSerialize.h"
#include "../database/HeldTransaction.h"

#include "../events/Events.h"
#include "../utility/Logger.h"

void RuleConditions::RuleCondition::Parse(
    const IRuleConditionSerialize&, const Registry&, const RuleConditionsRow& result,
	const UserHeldTransaction&)
{
    m_id = result.conditionId;
    m_type = result.conditionType;
    // Default is empty, child classes implement behavior
}

void RuleConditions::RuleCondition::Parse(const Registry&, const nlohmann::json& json)
{
    if (json.count("id"))
    {
        m_id = json.at("id");
    }
    else
    {
        m_id = 0;
    }
    m_type = json.at("type");
}

void RuleConditions::RuleCondition::Deserialize(const Registry&, const messages::RuleCondition& msg) {
	m_id = msg.id();
	m_type = msg.type();
}

nlohmann::json RuleConditions::RuleCondition::ToJson() const
{
    return {{"id", m_id}, {"type", m_type}};
}

messages::RuleCondition RuleConditions::RuleCondition::Serialize(bool includeChildren) const
{
	messages::RuleCondition msg;
	msg.set_id(m_id);
	msg.set_type(m_type);
	return msg;
}

RuleConditions::Ptr RuleConditions::RuleCondition::Clone(const Registry& registry) const
{
    Ptr result = registry.GetCondition(m_type);
    result->m_id = m_id;
    return result;
}

RuleConditions::Ptr RuleConditions::RuleCondition::Create(const Registry& registry) const
{
    return registry.GetCondition(m_type);
}

void RuleConditions::RuleConstantCondition::Parse(
    const IRuleConditionSerialize&, const Registry&, const RuleConditionsRow& result,
	const UserHeldTransaction&)
{
    // condition_id
    m_id = result.conditionId;
    // condition_type
    m_type = result.conditionType;
	google::protobuf::Any any;
	google::protobuf::BoolValue val;
	if (result.conditionData.is_null() || !any.ParseFromArray(result.conditionData.blob, result.conditionData.len)
		|| !any.UnpackTo(&val)) {
		throw std::invalid_argument("RuleConditions::RuleConstantCondition::Parse(db)");
	}
	else
	{
		m_state = val.value();
	}
}

void RuleConditions::RuleConstantCondition::Parse(const Registry& registry, const nlohmann::json& json)
{
    RuleCondition::Parse(registry, json);
    m_state = json.at("state");
}
void RuleConditions::RuleConstantCondition::Deserialize(const Registry& registry, const messages::RuleCondition& msg)
{
	RuleCondition::Deserialize(registry, msg);
	google::protobuf::BoolValue val;
	if (msg.data().UnpackTo(&val)) {
		m_state = val.value();
	}
	else
	{
		throw std::invalid_argument("RuleConditions::RuleConstantCondition::Deserialize");
	}
}

nlohmann::json RuleConditions::RuleConstantCondition::ToJson() const
{
    nlohmann::json value = RuleCondition::ToJson();
    value["state"] = m_state;
    return value;
}
messages::RuleCondition RuleConditions::RuleConstantCondition::Serialize(bool includeChildren) const {
	messages::RuleCondition msg = RuleCondition::Serialize(includeChildren);
	google::protobuf::BoolValue value;
	value.set_value(m_state);
	msg.mutable_data()->PackFrom(value);
	return msg;
}


RuleConditions::Ptr RuleConditions::RuleConstantCondition::Clone(const Registry& registry) const
{
    Ptr result(RuleCondition::Clone(registry));
    RuleConstantCondition* castRes = dynamic_cast<RuleConstantCondition*>(result.get());
    if (castRes == nullptr)
    {
        Res::Logger().Error("Failed to copy RuleConstantCondition! Cast returned nullptr.");
        throw std::bad_cast();
    }
    else
    {
        castRes->m_state = m_state;
        return result;
    }
}

void RuleConditions::RuleCompareCondition::Parse(
    const IRuleConditionSerialize& condSer, const Registry& registry, const RuleConditionsRow& result,
	const UserHeldTransaction& transaction)
{
    // condition_id
    m_id = result.conditionId;
    // condition_type
    m_type = result.conditionType;

	google::protobuf::Any any;
	messages::RuleCompareConditionData data;
	if (result.conditionData.is_null() || !any.ParseFromArray(result.conditionData.blob, result.conditionData.len) 
		|| !any.UnpackTo(&data)) {
		throw std::invalid_argument("RuleCompareCondition::Parse(db)");
	}
	int cmp = data.compare();
	if (cmp > static_cast<int>(Operator::NOT_EQUAL) || cmp < 0)
	{
		throw std::out_of_range("RuleCompareCondition::Operator out of range in Parse(db)");
	}
	m_compare = static_cast<Operator>(cmp);
	if (data.has_left_condition()) {
		m_left = registry.DeserializeCondition(data.left_condition());
	}
	else if (data.left_oneof_case() == data.kLeftId) {
		m_left = condSer.GetRuleCondition(data.left_id(), transaction);
	}
	else {
		m_left = nullptr;
	}
	if (data.has_right_condition()) {
		m_right = registry.DeserializeCondition(data.right_condition());
	}
	else if (data.right_oneof_case() == data.kRightId) {
		m_right = condSer.GetRuleCondition(data.right_id(), transaction);
	}
	else {
		m_right = nullptr;
	}
	m_compare = static_cast<Operator>(data.compare());
}

void RuleConditions::RuleCompareCondition::Parse(const Registry& registry, const nlohmann::json& json)
{
    RuleCondition::Parse(registry, json);
    int cmp = json.at("compare");
    if (cmp > static_cast<int>(Operator::NOT_EQUAL) || cmp < 0)
    {
        throw std::out_of_range("RuleCompareCondition::Operator out of range in Parse(json)");
    }
    m_compare = static_cast<Operator>(cmp);
    auto leftIt = json.find("left");
    if (leftIt != json.end() && leftIt->is_object())
    {
        m_left = registry.ParseCondition(*leftIt);
	}
	else {
		m_left = nullptr;
	}
    auto rightIt = json.find("right");
    if (rightIt != json.end() && rightIt->is_object())
    {
        m_right = registry.ParseCondition(*rightIt);
	}
	else {
		m_right = nullptr;
	}
}
void RuleConditions::RuleCompareCondition::Deserialize(const Registry& registry, const messages::RuleCondition& msg) {
	RuleCondition::Deserialize(registry, msg);
	messages::RuleCompareConditionData data;
	if (!msg.data().UnpackTo(&data)) {
		throw std::invalid_argument("RuleCompareCondition::Deserialize");
	}
	int cmp = data.compare();
	if (cmp > static_cast<int>(Operator::NOT_EQUAL) || cmp < 0)
	{
		throw std::out_of_range("RuleCompareCondition::Operator out of range in Deserialize");
	}
	m_compare = static_cast<Operator>(cmp);
	if (data.has_left_condition()) {
		m_left = registry.DeserializeCondition(data.left_condition());
	}
	else if (data.left_oneof_case() == data.kLeftId) {
		// TODO: Maybe pass ConditionSerialize?
		throw std::invalid_argument("RuleCompareCondition::Deserialize has to be called with included children");
	}
	if (data.has_right_condition()) {
		m_right = registry.DeserializeCondition(data.right_condition());
	}
	else if (data.right_oneof_case() == data.kRightId) {
		throw std::invalid_argument("RuleCompareCondition::Deserialize has to be called with included children");
	}
	m_compare = static_cast<Operator>(data.compare());
}

std::vector<const RuleConditions::RuleCondition*> RuleConditions::RuleCompareCondition::GetChilds() const
{
    std::vector<const RuleCondition*> result;
    result.reserve(2);
    if (m_right)
    {
        result.push_back(m_right.get());
    }
    if (m_left)
    {
        result.push_back(m_left.get());
    }
    return result;
}

std::vector<RuleConditions::RuleCondition*> RuleConditions::RuleCompareCondition::GetChilds()
{
    std::vector<RuleCondition*> result;
    result.reserve(2);
	if (m_left)
	{
		result.push_back(m_left.get());
	}
    if (m_right)
    {
        result.push_back(m_right.get());
    }
    return result;
}


bool RuleConditions::RuleCompareCondition::IsSatisfied() const
{
    // Check if condition is not empty
    if (!m_left && !m_right)
    {
        return false;
    }
    // Return false when only one is set, unless the compare is OR or NOR
    if ((!m_left || !m_right) && m_compare != Operator::OR && m_compare != Operator::NOR)
    {
        return false;
    }
    switch (m_compare)
    {
    case Operator::AND:
        return m_left->IsSatisfied() && m_right->IsSatisfied();
        break;
    case Operator::OR:
        // If only one is given, act as unary operator
        if (!m_left)
        {
            return m_right->IsSatisfied();
        }
        else if (!m_right)
        {
            return m_left->IsSatisfied();
        }
        else
        {
            return m_left->IsSatisfied() || m_right->IsSatisfied();
        }
        break;
    case Operator::NAND:
        return !(m_left->IsSatisfied() && m_right->IsSatisfied());
        break;
    case Operator::NOR:
        // If only one is given, act as unary operator
        if (!m_left)
        {
            return !m_right->IsSatisfied();
        }
        else if (!m_right)
        {
            return !m_left->IsSatisfied();
        }
        else
        {
            return !(m_left->IsSatisfied() || m_right->IsSatisfied());
        }
        break;
    case Operator::EQUAL:
        // Also known as XNOR
        return m_left->IsSatisfied() == m_right->IsSatisfied();
        break;
    case Operator::NOT_EQUAL:
        // Also known as XOR
        return m_left->IsSatisfied() != m_right->IsSatisfied();
        break;
    default:
        return false;
        break;
    }
}

nlohmann::json RuleConditions::RuleCompareCondition::ToJson() const
{
    nlohmann::json value = RuleCondition::ToJson();
    if (m_left)
    {
        value["left"] = m_left->ToJson();
    }
    if (m_right)
    {
        value["right"] = m_right->ToJson();
    }
    value["compare"] = static_cast<int>(m_compare);
    return value;
}

messages::RuleCondition RuleConditions::RuleCompareCondition::Serialize(bool includeChildren) const {
	messages::RuleCondition msg = RuleCondition::Serialize(includeChildren);
	messages::RuleCompareConditionData data;
	if (includeChildren) {
		if (m_left) {
			*data.mutable_left_condition() = m_left->Serialize(includeChildren);
		}
		if (m_right) {
			*data.mutable_right_condition() = m_right->Serialize(includeChildren);
		}
	}
	else {
		if (m_left) {
			data.set_left_id(m_left->GetId());
		}
		if (m_right) {
			data.set_right_id(m_right->GetId());
		}
	}
	data.set_compare(static_cast<messages::RuleCompareConditionData::Operator>(m_compare));
	msg.mutable_data()->PackFrom(data);
	return msg;
}

RuleConditions::Ptr RuleConditions::RuleCompareCondition::Clone(const Registry& registry) const
{
    Ptr result(RuleCondition::Clone(registry));
    RuleCompareCondition* castRes = dynamic_cast<RuleCompareCondition*>(result.get());
    if (castRes == nullptr)
    {
        Res::Logger().Error("Failed to copy RuleCompareCondition! Cast returned nullptr.");
        throw std::bad_cast();
    }
    else
    {
        castRes->m_compare = m_compare;
        castRes->m_type = m_type;
        if (m_left)
        {
            castRes->m_left = m_left->Clone(registry);
        }
        else
        {
            castRes->m_left = nullptr;
        }
        if (m_right)
        {
            castRes->m_right = m_right->Clone(registry);
        }
        else
        {
            castRes->m_right = nullptr;
        }
        return result;
    }
}

bool RuleConditions::RuleCompareCondition::IsSatisfiedAfterEvent(const EventBase& e) const
{
    // Check if condition is not empty
    if (!m_left && !m_right)
    {
        return false;
    }
    // Return false when only one is set, unless the compare is OR or NOR
    if ((!m_left || !m_right) && m_compare != Operator::OR && m_compare != Operator::NOR)
    {
        return false;
    }
    switch (m_compare)
    {
    case Operator::AND:
        return (m_left->ShouldExecuteOn(e.GetType()) ? m_left->IsSatisfiedAfterEvent(e) : m_left->IsSatisfied())
            && (m_right->ShouldExecuteOn(e.GetType()) ? m_right->IsSatisfiedAfterEvent(e) : m_right->IsSatisfied());
    case Operator::OR:
        // If only one is given, act as unary operator
        if (!m_left && m_right->ShouldExecuteOn(e.GetType()))
        {
            return m_right->ShouldExecuteOn(e.GetType()) ? m_right->IsSatisfiedAfterEvent(e) : m_right->IsSatisfied();
        }
        else if (!m_right)
        {
            return m_left->ShouldExecuteOn(e.GetType()) ? m_left->IsSatisfiedAfterEvent(e) : m_left->IsSatisfied();
        }
        else
        {
            return (m_left->ShouldExecuteOn(e.GetType()) ? m_left->IsSatisfiedAfterEvent(e) : m_left->IsSatisfied())
                || (m_right->ShouldExecuteOn(e.GetType()) ? m_right->IsSatisfiedAfterEvent(e) : m_right->IsSatisfied());
        }
        break;
    case Operator::NAND:
        return !((m_left->ShouldExecuteOn(e.GetType()) ? m_left->IsSatisfiedAfterEvent(e) : m_left->IsSatisfied())
            && (m_right->ShouldExecuteOn(e.GetType()) ? m_right->IsSatisfiedAfterEvent(e) : m_right->IsSatisfied()));
    case Operator::NOR:
        // If only one is given, act as unary operator
        if (!m_left)
        {
            return !(
                m_right->ShouldExecuteOn(e.GetType()) ? m_right->IsSatisfiedAfterEvent(e) : m_right->IsSatisfied());
        }
        else if (!m_right)
        {
            return !(m_left->ShouldExecuteOn(e.GetType()) ? m_left->IsSatisfiedAfterEvent(e) : m_left->IsSatisfied());
        }
        else
        {
            return !((m_left->ShouldExecuteOn(e.GetType()) ? m_left->IsSatisfiedAfterEvent(e) : m_left->IsSatisfied())
                || (m_right->ShouldExecuteOn(e.GetType()) ? m_right->IsSatisfiedAfterEvent(e)
                                                          : m_right->IsSatisfied()));
        }
        break;
    case Operator::EQUAL:
        // Also known as XNOR
        return (m_left->ShouldExecuteOn(e.GetType()) ? m_left->IsSatisfiedAfterEvent(e) : m_left->IsSatisfied())
            == (m_right->ShouldExecuteOn(e.GetType()) ? m_right->IsSatisfiedAfterEvent(e) : m_right->IsSatisfied());
    case Operator::NOT_EQUAL:
        // Also known as XOR
        return (m_left->ShouldExecuteOn(e.GetType()) ? m_left->IsSatisfiedAfterEvent(e) : m_left->IsSatisfied())
            != (m_right->ShouldExecuteOn(e.GetType()) ? m_right->IsSatisfiedAfterEvent(e) : m_right->IsSatisfied());
    default:
        return false;
    }
}

void RuleConditions::RuleTimeCondition::Parse(
    const IRuleConditionSerialize&, const Registry&, const RuleConditionsRow& result, const UserHeldTransaction&)
{
    // condition_id
    m_id = result.conditionId;
    // condition_type
    m_type = result.conditionType;

	google::protobuf::Any any;
	messages::RuleTimeConditionData data;
	if (result.conditionData.is_null() || !any.ParseFromArray(result.conditionData.blob, result.conditionData.len)
		|| !any.UnpackTo(&data)) {
		throw std::invalid_argument("RuleTimeCondition::Deserialize");
	}
	m_time1 = static_cast<time_t>(data.time1());
	m_time2 = static_cast<time_t>(data.time2());
	int cmp = data.compare();
	if (cmp > static_cast<int>(Operator::inRange) || cmp < 0)
	{
		throw std::out_of_range("RuleTimeCondition::Operator out of range in Deserialize");
	}
	m_compare = static_cast<Operator>(cmp);
	int type = data.time_type();
	if (type > static_cast<int>(Type::absolute) || type < 0)
	{
		throw std::out_of_range("RuleTimeCondition::Type out of range in Deserialize");
	}
	m_timeType = static_cast<Type>(type);
}

void RuleConditions::RuleTimeCondition::Parse(const Registry& registry, const nlohmann::json& json)
{
    RuleCondition::Parse(registry, json);
    m_time1 = static_cast<time_t>(json.at("time1"));
    m_time2 = static_cast<time_t>(json.at("time2"));
    int cmp = json.at("compare");
    if (cmp > static_cast<int>(Operator::inRange) || cmp < 0)
    {
        throw std::out_of_range("RuleTimeCondition::Operator out of range in Parse(json)");
    }
    m_compare = static_cast<Operator>(cmp);
    int type = json.at("timeType");
    if (type > static_cast<int>(Type::absolute) || type < 0)
    {
        throw std::out_of_range("RuleTimeCondition::Type out of range in Parse(json)");
    }
    m_timeType = static_cast<Type>(type);
}

void RuleConditions::RuleTimeCondition::Deserialize(const Registry& registry, const messages::RuleCondition& msg) {
	RuleCondition::Deserialize(registry, msg);
	messages::RuleTimeConditionData data;
	if (!msg.data().UnpackTo(&data)) {
		throw std::invalid_argument("RuleTimeCondition::Deserialize");
	}
	m_time1 = static_cast<time_t>(data.time1());
	m_time2 = static_cast<time_t>(data.time2());
	int cmp = data.compare();
	if (cmp > static_cast<int>(Operator::inRange) || cmp < 0)
	{
		throw std::out_of_range("RuleTimeCondition::Operator out of range in Deserialize");
	}
	m_compare = static_cast<Operator>(cmp);
	int type = data.time_type();
	if (type > static_cast<int>(Type::absolute) || type < 0)
	{
		throw std::out_of_range("RuleTimeCondition::Type out of range in Deserialize");
	}
	m_timeType = static_cast<Type>(type);
	
}

bool RuleConditions::RuleTimeCondition::IsSatisfied() const
{
    const auto cmpFun = [](time_t cmpTime, time_t t1, time_t t2, Operator compare) -> bool {
        switch (compare)
        {
        case Operator::equals:
            return cmpTime >= t1 - t2 && cmpTime <= t1 + t2;
            break;
        case Operator::notEquals:
            // Time 2 specifies half the length of the false period
            return cmpTime < t1 - t2 || cmpTime > t1 + t2;
            break;
        case Operator::greater:
            return cmpTime > t1;
            break;
        case Operator::less:
            return cmpTime < t1;
            break;
        case Operator::inRange:
            if (t1 >= t2)
            {
                return cmpTime >= t1 && cmpTime <= t2;
            }
            else
            {
                // Wrap around to the next day
                return cmpTime <= t1 && cmpTime >= t2;
            }
        default:
            // Should not happen
            return false;
            break;
        }
    };
    // Only for greater / less than
    const auto cmpHoursMinsSecs = [](int hour1, int min1, int sec1, int hour2, int min2, int sec2,
                                      const std::function<bool(int, int)>& cmp) -> bool {
        // Compare each value, if they are equal, go one deeper
        if (cmp(hour1, hour2))
        {
            return true;
        }
        else if (hour1 == hour2)
        {
            if (cmp(min1, min2))
            {
                return true;
            }
            else if (min1 == min2)
            {
                if (cmp(sec1, sec2))
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    };
    std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm tm = *std::localtime(&time);
    switch (m_timeType)
    {
    case Type::hourMinSec:
    {
        const int hour1 = static_cast<int>(m_time1 / 3600);
        const int min1 = static_cast<int>((m_time1 % 3600) / 60);
        const int sec1 = static_cast<int>(m_time1 % 60);

        const int hour2 = static_cast<int>(m_time2 / 3600);
        const int min2 = static_cast<int>((m_time2 % 3600) / 60);
        const int sec2 = static_cast<int>(m_time2 % 60);

        // Can't simply use compare function :(
        switch (m_compare)
        {
        case Operator::equals:
            return !(cmpHoursMinsSecs(tm.tm_hour, tm.tm_min, tm.tm_sec, hour1 - hour2, min1 - min2, sec1 - sec2,
                         std::function<bool(int, int)>(std::less<int>()))
                || cmpHoursMinsSecs(tm.tm_hour, tm.tm_min, tm.tm_sec, hour1 + hour2, min1 + min2, sec1 + sec2,
                       std::function<bool(int, int)>(std::greater<int>())));
            break;
        case Operator::notEquals:
            // Time 2 specifies half the length of the false period
            return cmpHoursMinsSecs(tm.tm_hour, tm.tm_min, tm.tm_sec, hour1 - hour2, min1 - min2, sec1 - sec2,
                       std::function<bool(int, int)>(std::less<int>()))
                || cmpHoursMinsSecs(tm.tm_hour, tm.tm_min, tm.tm_sec, hour1 + hour2, min1 + min2, sec1 + sec2,
                       std::function<bool(int, int)>(std::greater<int>()));
            break;
        case Operator::greater:
            return cmpHoursMinsSecs(tm.tm_hour, tm.tm_min, tm.tm_sec, hour1, min1, sec1,
                std::function<bool(int, int)>(std::greater<int>()));
            break;
        case Operator::less:
            return cmpHoursMinsSecs(
                tm.tm_hour, tm.tm_min, tm.tm_sec, hour1, min1, sec1, std::function<bool(int, int)>(std::less<int>()));
            break;
        case Operator::inRange:
            // t1 >= t2
            if (!cmpHoursMinsSecs(
                    hour1, min1, sec1, hour2, min2, sec2, std::function<bool(int, int)>(std::less<int>())))
            {
                // time >= t1 && time <= t2
                return !cmpHoursMinsSecs(tm.tm_hour, tm.tm_min, tm.tm_sec, hour1, min1, sec1,
                           std::function<bool(int, int)>(std::less<int>()))
                    && !cmpHoursMinsSecs(tm.tm_hour, tm.tm_min, tm.tm_sec, hour2, min2, sec2,
                           std::function<bool(int, int)>(std::greater<int>()));
            }
            else
            {
                // Wrap around to the next day
                // time <= t1 && time >= t2 (between t2 from first day and t1 on second day)
                return !cmpHoursMinsSecs(tm.tm_hour, tm.tm_min, tm.tm_sec, hour1, min1, sec1,
                           std::function<bool(int, int)>(std::greater<int>()))
                    && !cmpHoursMinsSecs(tm.tm_hour, tm.tm_min, tm.tm_sec, hour2, min2, sec2,
                           std::function<bool(int, int)>(std::less<int>()));
            }
        default:
            // Should not happen
            return false;
            break;
        }
        break;
    }
    case Type::hour:
        return cmpFun(tm.tm_hour, m_time1, m_time2, m_compare);
        break;
    case Type::dayWeek:
        return cmpFun(tm.tm_wday, m_time1, m_time2, m_compare);
        break;
    case Type::dayMonth:
        return cmpFun(tm.tm_mday, m_time1, m_time2, m_compare);
        break;
    case Type::dayYear:
        return cmpFun(tm.tm_yday, m_time1, m_time2, m_compare);
        break;
    case Type::month:
        return cmpFun(tm.tm_mon, m_time1, m_time2, m_compare);
        break;
    case Type::year:
        return cmpFun(tm.tm_year, m_time1, m_time2, m_compare);
        break;
    case Type::absolute:
        return cmpFun(time, m_time1, m_time2, m_compare);
        break;
    default:
        // Should not happen
        return false;
    }
}

nlohmann::json RuleConditions::RuleTimeCondition::ToJson() const
{
    nlohmann::json value = RuleCondition::ToJson();
    value["time1"] = static_cast<int64_t>(m_time1);
    value["time2"] = static_cast<int64_t>(m_time2);
    value["compare"] = static_cast<int>(m_compare);
    value["timeType"] = static_cast<int>(m_timeType);
    return value;
}

messages::RuleCondition RuleConditions::RuleTimeCondition::Serialize(bool includeChildren) const {
	messages::RuleCondition msg = RuleCondition::Serialize(includeChildren);
	messages::RuleTimeConditionData data;
	data.set_time1(m_time1);
	data.set_time2(m_time2);
	data.set_compare(static_cast<messages::RuleTimeConditionData::Operator>(m_compare));
	data.set_time_type(static_cast<messages::RuleTimeConditionData::Type>(m_timeType));
	msg.mutable_data()->PackFrom(data);
	return msg;
}

RuleConditions::Ptr RuleConditions::RuleTimeCondition::Clone(const Registry& registry) const
{
    Ptr result(RuleCondition::Clone(registry));
    RuleTimeCondition* castRes = dynamic_cast<RuleTimeCondition*>(result.get());
    if (castRes == nullptr)
    {
        Res::Logger().Error("Failed to copy RuleCompareCondition! Cast returned nullptr.");
        return nullptr;
    }
    else
    {
        castRes->m_time1 = m_time1;
        castRes->m_time2 = m_time2;
        castRes->m_compare = m_compare;
        castRes->m_timeType = m_timeType;
        return result;
    }
}

std::chrono::system_clock::time_point RuleConditions::RuleTimeCondition::GetNextExecutionTime() const
{
    using std::chrono::system_clock;
    system_clock::time_point ret_time_point;
    system_clock::time_point time_now = system_clock::now();
    std::time_t time = system_clock::to_time_t(time_now);
    std::tm tm = *std::localtime(&time);
    switch (m_timeType)
    {
    case Type::hourMinSec:
    {
        const int hour1 = static_cast<int>(m_time1 / 3600);
        const int min1 = static_cast<int>((m_time1 % 3600) / 60);
        const int sec1 = static_cast<int>(m_time1 % 60);

        if (m_compare != Operator::less)
        {
            tm.tm_hour = hour1;
            tm.tm_min = min1;
            tm.tm_sec = sec1;
            std::time_t result = std::mktime(&tm);
            ret_time_point = system_clock::from_time_t(result);
            if (ret_time_point < time_now)
            {
                ret_time_point += std::chrono::hours(24);
            }
        }
        else
        {
            tm.tm_hour = 0;
            tm.tm_min = 0;
            tm.tm_sec = 0;
            std::time_t result = std::mktime(&tm);
            ret_time_point = system_clock::from_time_t(result) + std::chrono::hours(24);
        }
        break;
    }
    case Type::hour:
        if (m_compare != Operator::less)
        {
            tm.tm_hour = m_time1;
            tm.tm_min = 0;
            tm.tm_sec = 0;
            std::time_t result = std::mktime(&tm);
            ret_time_point = system_clock::from_time_t(result);
            if (ret_time_point < time_now)
            {
                ret_time_point += std::chrono::hours(24);
            }
        }
        else
        {
            tm.tm_hour = 0;
            tm.tm_min = 0;
            tm.tm_sec = 0;
            std::time_t result = std::mktime(&tm);
            ret_time_point = system_clock::from_time_t(result) + std::chrono::hours(24);
        }
        break;
    case Type::dayWeek:
        if (m_compare != Operator::less)
        {
            // Cannot use tm_wday, because it is ignored in mktime
            tm.tm_mday -= tm.tm_wday - m_time1;
            tm.tm_hour = 0;
            tm.tm_min = 0;
            tm.tm_sec = 0;
            std::time_t result = std::mktime(&tm);
            ret_time_point = system_clock::from_time_t(result);
            if (ret_time_point < time_now)
            {
                ret_time_point += std::chrono::hours(24 * 7);
            }
        }
        else
        {
            tm.tm_mday -= tm.tm_wday;
            tm.tm_hour = 0;
            tm.tm_min = 0;
            tm.tm_sec = 0;
            std::time_t result = std::mktime(&tm);
            ret_time_point = system_clock::from_time_t(result) + std::chrono::hours(24 * 7);
        }
        break;
    case Type::dayMonth:
        if (m_compare != Operator::less)
        {
            tm.tm_mday = m_time1;
            tm.tm_hour = 0;
            tm.tm_min = 0;
            tm.tm_sec = 0;
            std::time_t result = std::mktime(&tm);
            ret_time_point = system_clock::from_time_t(result);
            if (ret_time_point < time_now)
            {
                tm.tm_mon += 1;
                ret_time_point = system_clock::from_time_t(std::mktime(&tm));
            }
        }
        else
        {
            tm.tm_mon += 1;
            tm.tm_mday = 1;
            tm.tm_hour = 0;
            tm.tm_min = 0;
            tm.tm_sec = 0;
            std::time_t result = std::mktime(&tm);
            ret_time_point = system_clock::from_time_t(result);
        }
        break;
    case Type::dayYear:
        if (m_compare != Operator::less)
        {
            tm.tm_mday = m_time1 + 1;
            tm.tm_mon = 0;
            tm.tm_hour = 0;
            tm.tm_min = 0;
            tm.tm_sec = 0;
            ret_time_point = system_clock::from_time_t(std::mktime(&tm));
            if (ret_time_point < time_now)
            {
                ++tm.tm_year;
                ret_time_point = system_clock::from_time_t(std::mktime(&tm));
            }
        }
        else
        {
            tm.tm_year += 1;
            tm.tm_mday = 0;
            tm.tm_mon = 0;
            tm.tm_hour = 0;
            tm.tm_min = 0;
            tm.tm_sec = 0;
            ret_time_point = system_clock::from_time_t(std::mktime(&tm));
        }
        break;
    case Type::month:
        if (m_compare != Operator::less)
        {
            tm.tm_mon = m_time1;
            tm.tm_mday = 1;
            tm.tm_hour = 0;
            tm.tm_min = 0;
            tm.tm_sec = 0;
            ret_time_point = system_clock::from_time_t(std::mktime(&tm));
            if (ret_time_point < time_now)
            {
                ++tm.tm_year;
                ret_time_point = system_clock::from_time_t(std::mktime(&tm));
            }
        }
        else
        {
            tm.tm_year += 1;
            tm.tm_mon = 0;
            tm.tm_mday = 1;
            tm.tm_hour = 0;
            tm.tm_min = 0;
            tm.tm_sec = 0;
            ret_time_point = system_clock::from_time_t(std::mktime(&tm));
        }
        break;
    case Type::year:
        if (m_compare != Operator::less)
        {
            tm.tm_year = m_time1;
            tm.tm_mon = 0;
            tm.tm_mday = 1;
            tm.tm_hour = 0;
            tm.tm_min = 0;
            tm.tm_sec = 0;
            ret_time_point = system_clock::from_time_t(std::mktime(&tm));
            if (ret_time_point < time_now)
            {
                ret_time_point = system_clock::time_point(std::chrono::seconds(0));
            }
        }
        else
        {
            ret_time_point = system_clock::time_point(std::chrono::seconds(0));
        }
        break;
    case Type::absolute:
        if (m_compare != Operator::less)
        {
            ret_time_point = system_clock::from_time_t(static_cast<std::time_t>(m_time1));
            if (ret_time_point < time_now)
            {
                ret_time_point = system_clock::time_point(std::chrono::seconds(0));
            }
        }
        else
        {
            ret_time_point = system_clock::time_point(std::chrono::seconds(0));
        }
        break;
    default:
        ret_time_point = system_clock::time_point(std::chrono::seconds(0));
        break;
    }
    return ret_time_point;
}

void RuleConditions::RuleDeviceCondition::Parse(
    const IRuleConditionSerialize&, const Registry&, const RuleConditionsRow& result, const UserHeldTransaction&)
{
    // condition_id
    m_id = result.conditionId;
    // condition_type
    m_type = result.conditionType;

    // TODO: Set nodeSerialize differently
    // m_nodeSer = ???;
	google::protobuf::Any any;
	messages::RuleDeviceConditionData data;
	if (result.conditionData.is_null() || !any.ParseFromArray(result.conditionData.blob, result.conditionData.len)
		|| !any.UnpackTo(&data)) {
		throw std::invalid_argument("RuleDeviceCondition::Deserialize");
	}
	// TODO: Support other types
	google::protobuf::Int32Value value;
	data.val1().UnpackTo(&value);
	m_val1 = value.value();
	data.val2().UnpackTo(&value);
	m_val2 = value.value();
	int cmp = data.compare();
	if (cmp > static_cast<int>(Operator::IN_RANGE) || cmp < 0)
	{
		throw std::out_of_range("RuleDeviceCondition::Operator out of range in Deserialize");
	}
	m_property = data.property();
}

void RuleConditions::RuleDeviceCondition::Parse(const Registry& registry, const nlohmann::json& json)
{
    RuleCondition::Parse(registry, json);
    m_val1 = json.at("value1");
    m_val2 = json.at("value2");
    int cmp = json.at("compare");
    if (cmp > static_cast<int>(Operator::IN_RANGE) || cmp < 0)
    {
        throw std::out_of_range("RuleDeviceCondition::Operator out of range in Parse(json)");
    }
    m_compare = static_cast<Operator>(cmp);
    m_deviceId = DeviceId(json.at("deviceId").get<uint64_t>());
    m_property = json.at("property").get<std::string>();
}

void RuleConditions::RuleDeviceCondition::Deserialize(const Registry& registry, const messages::RuleCondition& msg) {
	RuleCondition::Deserialize(registry, msg);
	messages::RuleDeviceConditionData data;
	if (!msg.data().UnpackTo(&data) || !data.IsInitialized()) {
		throw std::invalid_argument("RuleDeviceCondition::Deserialize");
	}
	// TODO: Support other types
	google::protobuf::Int32Value value;
	data.val1().UnpackTo(&value);
	m_val1 = value.value();
	data.val2().UnpackTo(&value);
	m_val2 = value.value();
	int cmp = data.compare();
	if (cmp > static_cast<int>(Operator::IN_RANGE) || cmp < 0)
	{
		throw std::out_of_range("RuleDeviceCondition::Operator out of range in Deserialize");
	}
	m_property = data.property();
}

bool RuleConditions::RuleDeviceCondition::IsSatisfied() const
{
    if (m_deviceReg == nullptr || /*m_deviceId < 0 ||*/ m_property.empty())
    {
        Res::Logger().Warning(
            "RuleDeviceCondition with invalid configuration: DeviceRegistry null or negative device/sensorId!");
        return false;
    }

    absl::optional<Device> device = m_deviceReg->GetStorage().GetDevice(m_deviceId, UserId::Dummy());

    if (!device)
    {
        Res::Logger().Warning("RuleDeviceCondition: Device not found!");
        // Res::Logger().Debug("DeviceId: " + std::to_string(m_deviceId));
        return false;
    }
    nlohmann::json property = device->GetProperty(m_property);
    // if (it == sensors.end())
    //{
    //    Res::Logger().Warning("RuleDeviceCondition: Device does not contain sensor!");
    //    Res::Logger().Debug("DeviceId: " + std::to_string(m_deviceId) + ", sensorId: " + std::to_string(m_property));
    //    return false;
    //}
    try
    {
        // Check if the sensor value is in the affected range
        return Compare(property);
    }
    catch (const std::exception& e)
    {
        Res::Logger().Warning(
            "RuleDeviceCondition: Failed to convert sensor value to int: " + property.dump() + ";" + e.what());
        return false;
    }
}

nlohmann::json RuleConditions::RuleDeviceCondition::ToJson() const
{
    nlohmann::json value = RuleCondition::ToJson();
    value["value1"] = m_val1;
    value["value2"] = m_val2;
    value["compare"] = static_cast<int>(m_compare);
    value["deviceId"] = m_deviceId.GetValue();
    value["property"] = m_property;
    return value;
}

messages::RuleCondition RuleConditions::RuleDeviceCondition::Serialize(bool includeChildren) const {
	messages::RuleCondition msg = RuleCondition::Serialize(includeChildren);
	messages::RuleDeviceConditionData data;
	google::protobuf::Int32Value value;
	value.set_value(m_val1);
	data.mutable_val1()->PackFrom(value);
	value.set_value(m_val2);
	data.mutable_val2()->PackFrom(value);
	data.set_compare(static_cast<messages::RuleDeviceConditionData::Operator>(m_compare));
	data.set_property(m_property);
	msg.mutable_data()->UnpackTo(&data);
	return msg;
}

RuleConditions::Ptr RuleConditions::RuleDeviceCondition::Clone(const Registry& registry) const
{
    Ptr result(RuleCondition::Clone(registry));
    RuleDeviceCondition* castRes = dynamic_cast<RuleDeviceCondition*>(result.get());
    if (castRes == nullptr)
    {
        Res::Logger().Error("Failed to copy RuleCompareCondition! Cast returned nullptr.");
        return nullptr;
    }
    else
    {
        castRes->m_deviceReg = m_deviceReg;
        castRes->m_deviceId = m_deviceId;
        castRes->m_property = m_property;
        castRes->m_val1 = m_val1;
        castRes->m_val2 = m_val2;
        castRes->m_compare = m_compare;
        return result;
    }
}

bool RuleConditions::RuleDeviceCondition::ShouldExecuteOn(EventType type) const
{
    return type == EventTypes::devicePropertyChange;
}

bool RuleConditions::RuleDeviceCondition::IsSatisfiedAfterEvent(const EventBase& e) const
{
    if (e.GetType() != EventTypes::devicePropertyChange)
    {
        throw std::logic_error(
            "RuleDeviceCondition got wrong EventType: " + std::to_string(static_cast<int>(e.GetType())));
    }
    const auto& casted = EventCast<Events::DevicePropertyChangeEvent>(e);
    if (casted.GetChanged().GetId() == m_deviceId && casted.GetChangedFields() == m_property)
    {
        // This Event affects the rule
        nlohmann::json value = casted.GetChanged().GetProperty(m_property);
        try
        {
            // Check if the sensor value is in the affected range
            return Compare(value);
        }
        catch (const std::exception& e)
        {
            Res::Logger().Warning(
                "RuleDeviceCondition: Failed to convert property value to int: " + value.dump() + ";" + e.what());
            return false;
        }
    }
    return false;
}

bool RuleConditions::RuleDeviceCondition::Compare(int compareValue) const
{
    switch (m_compare)
    {
    case Operator::EQUALS:
        return compareValue >= m_val1 - m_val2 && compareValue <= m_val1 + m_val2;
        break;
    case Operator::NOT_EQUALS:
        // Value 2 specifies half the length of the false range
        return !(compareValue >= m_val1 - m_val2 && compareValue <= m_val1 + m_val2);
        break;
    case Operator::GREATER:
        return compareValue > m_val1;
        break;
    case Operator::LESS:
        return compareValue < m_val1;
        break;
    case Operator::IN_RANGE:
        if (m_val1 <= m_val2)
        {
            return compareValue >= m_val1 && compareValue <= m_val2;
        }
        else
        {
            // Wrap around to the next value
            return compareValue <= m_val2 || compareValue >= m_val1;
        }
    default:
        // Should not happen
        return false;
        break;
    }
}

void RuleConditions::Registry::RegisterDefaultConditions()
{
    if (!m_factories.empty())
    {
        Res::Logger().Error("RuleConditions::RegisterDefaultConditions() was called after conditions were added!");
    }
    else
    {
        Register({[](uint64_t id) { return Ptr(new RuleConstantCondition(id)); }, "constant"}, 0);
        Register({[](uint64_t id) { return Ptr(new RuleCompareCondition(id)); }, "compare"}, 1);
        Register({[](uint64_t id) { return Ptr(new RuleTimeCondition(id)); }, "time"}, 2);
        Register({[](uint64_t id) { return Ptr(new RuleDeviceCondition(id)); }, "sensor"}, 3);
    }
}

RuleConditions::Ptr RuleConditions::Registry::GetCondition(uint64_t type) const
{
    if (m_factories.size() > type && m_factories[type] != nullptr)
    {
        return m_factories[type].function(type);
    }
    else
    {
        Res::Logger().Warning("Unknown rule condition type: " + std::to_string(type));
        throw std::out_of_range("RuleConditions::GetCondition: Unknown rule condition type: " + std::to_string(type));
    }
}

RuleConditions::Ptr RuleConditions::Registry::ParseCondition(
    const IRuleConditionSerialize& condSer, const RuleCondition::RuleConditionsRow& result, const UserHeldTransaction& transaction) const
{
    int64_t type = result.conditionType;
    Ptr instance = GetCondition(type);
    instance->Parse(condSer, *this, result, transaction);
    return instance;
}

RuleConditions::Ptr RuleConditions::Registry::ParseCondition(const nlohmann::json& json) const
{
    const uint64_t type = json.at("type");
    Ptr result = GetCondition(type);
    result->Parse(*this, json);
    return result;
}

RuleConditions::Ptr RuleConditions::Registry::DeserializeCondition(const messages::RuleCondition& msg) const {
	const uint64_t type = msg.type();
	Ptr result = GetCondition(type);
	result->Deserialize(*this, msg);
	return result;
}

const std::vector<RuleConditions::RuleConditionInfo>& RuleConditions::Registry::GetRegistered() const
{
    return m_factories;
}

nlohmann::json Rule::ToJson() const
{
    nlohmann::json value;
    value["id"] = m_id;
    value["name"] = m_name;
    value["icon"] = m_icon;
    value["color"] = m_color;
    if (!m_condition)
    {
        value["condition"] = nlohmann::json();
    }
    else
    {
        value["condition"] = m_condition->ToJson();
    }
    value["effect"] = m_effect.ToJson();
    value["enabled"] = m_enabled;
    return value;
}

messages::Rule Rule::Serialize() const {
	messages::Rule msg;
	msg.set_id(m_id);
	msg.set_name(m_name);
	msg.set_icon(m_icon);
	msg.set_color(m_color);
	if (m_condition) {
		*msg.mutable_condition() = m_condition->Serialize(true);
	}
	*msg.mutable_effect() = m_effect.Serialize();
	msg.set_enabled(m_enabled);
	return msg;
}

Rule Rule::Parse(const nlohmann::json& json)
{
    Rule rule;
    if (json.count("id"))
    {
        rule.m_id = json.at("id");
    }
    else
    {
        rule.m_id = 0;
    }
    rule.m_name = json.at("name").get<std::string>();
    rule.m_icon = json.at("icon").get<std::string>();
    rule.m_color = json.at("color");
    rule.m_condition = Res::ConditionRegistry().ParseCondition(json.at("condition"));
    rule.m_effect = Action::Parse(json.at("effect"), Res::ActionRegistry());
    rule.m_enabled = json.at("enabled");
    return rule;
}

Rule Rule::Deserialize(const messages::Rule& msg, const RuleConditions::Registry& condReg, 
	const SubActionRegistry& actionReg) {
	Rule rule;
	rule.m_id = msg.id();
	rule.m_name = msg.name();
	rule.m_icon = msg.icon();
	rule.m_color = msg.color();
	rule.m_condition = condReg.DeserializeCondition(msg.condition());
	rule.m_effect = Action::Deserialize(msg.effect(), actionReg);
	rule.m_enabled = msg.enabled();
	return rule;
}

bool Rule::operator==(const Rule& other) const
{
    return m_id == other.m_id && m_name == other.m_name && m_icon == other.m_icon && m_color == other.m_color
        && m_condition->GetId() == other.m_condition->GetId() && m_effect == other.m_effect
        && m_enabled == other.m_enabled;
}
