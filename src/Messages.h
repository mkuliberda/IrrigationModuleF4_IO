#pragma once
#include <string>
#include <unordered_map>
#include <array>


enum class ExternalObject_t {
	unspecified,
	ntp_server,
	raspberry_pi,
	google_home,
	my_phone,
	broadcast,
	max = 255
};

enum class InternalObject_t {
	unspecified,
	sector,
	plant,
	watertank,
	system,
	all,
	max = 255
};

enum class MsgType {
	unknown,
	command,
	query,
	info,
	max = 1023
};

enum class MsgValueType {
	none,
	irrigation_start,
	irrigation_stop,
	get_status,
	get_time_and_date,
	max = 1023
};

inline std::unordered_map<InternalObject_t, std::string> internal_entities{  // NOLINT(clang-diagnostic-exit-time-destructors)
	{InternalObject_t::plant, "PLA"},
	{InternalObject_t::sector, "SEC"},
	{InternalObject_t::watertank, "WTK"},
	{InternalObject_t::system, "SYS"},
	{InternalObject_t::all, "ALL"}
};

inline std::unordered_map<ExternalObject_t, std::string> esp01s_external_addresses{  // NOLINT(clang-diagnostic-exit-time-destructors)
	{ExternalObject_t::ntp_server, "NTP"},
	{ExternalObject_t::raspberry_pi, "RPI"},
	{ExternalObject_t::google_home, "GOO"},
	{ExternalObject_t::broadcast, "ALL"},
};

inline std::unordered_map<ExternalObject_t, std::string> mobile_numbers{  // NOLINT(clang-diagnostic-exit-time-destructors)
	{ExternalObject_t::my_phone, "506696574"},
};

struct ExternalObject {

	ExternalObject_t object;
	uint16_t id;
};

struct InternalObject {

	InternalObject_t object;
	uint16_t id;
};

struct MessagePayload
{
	MsgType msg_type;
	MsgValueType msg_value;
};

struct InternalMessage
{
	InternalObject recipient;
	InternalObject sender;
	MsgType msg_type;
	MsgValueType msg_val;
	uint32_t val_u32;
};

struct IncomingMessage
{
	ExternalObject sender;
	InternalObject recipient;
	MsgValueType type;
	uint16_t msg_value1;
	uint16_t msg_value2;
	uint16_t msg_value3;
	uint16_t msg_value4;
	uint16_t msg_value5;
	uint16_t msg_value6;
	uint16_t msg_value7;
	uint16_t msg_value8;
	uint8_t end;
};

struct OutgoingMessage
{
	ExternalObject recipient;
	InternalObject sender;
	std::array<MessagePayload, 8> payload;
};
