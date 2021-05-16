#pragma once
#include <string>
#include <utility>
#include <vector>
#include <sstream>

struct Footer;
struct Data;
struct DataItem;
struct Header;

struct Encoder
{
	virtual ~Encoder() = default;

	virtual void visit(const Header& p) = 0;
	virtual void visit(const DataItem& p) = 0;
	virtual void visit(const Data& p) = 0;
	virtual void visit(const Footer& p) = 0;

	virtual std::string str() = 0;
};

struct Element
{
	virtual ~Element() = default;
	virtual void accept(Encoder& v) const = 0;
};

struct TextMessage : Element
{
	std::string part1;
	std::string part2;

	explicit TextMessage(std::string&& _part1, std::string&& _part2)
		: part1(std::move(_part1)), part2(std::move(_part2))
	{
	}

	explicit TextMessage(const std::string& _part1, const std::string& _part2)
		: part1(_part1), part2(_part2)
	{
	}
};

struct Header : TextMessage
{
	explicit Header(std::string&& _part1, std::string&& _part2)
		: TextMessage(std::move(_part1), std::move(_part2))
	{
	}

	explicit Header(const std::string& _part1, const std::string& _part2)
		: TextMessage(_part1, _part2)
	{
	}


	void accept(Encoder& v) const override
	{
		v.visit(*this);
	}
};

struct DataItem : TextMessage
{
	explicit DataItem(std::string&& _part1, std::string&& _part2)
		: TextMessage(std::move(_part1), std::move(_part2))
	{
	}

	explicit DataItem(const std::string& _part1, const std::string& _part2)
		: TextMessage(_part1, _part2)
	{
	}
	


	void accept(Encoder& v) const override
	{
		v.visit(*this);
	}
};

struct Footer : TextMessage
{
	explicit Footer(std::string&& _part1, std::string&& _part2)
		: TextMessage(std::move(_part1), std::move(_part2))
	{
	}

	explicit Footer(const std::string& _part1, const std::string& _part2)
		: TextMessage(_part1, _part2)
	{
	}

	void accept(Encoder& v) const override
	{
		v.visit(*this);
	}
};

struct Data : std::vector<DataItem>, Element
{
	Data(const std::initializer_list<value_type>& _init_list)
		: std::vector<DataItem>(_init_list)
	{
	}

	void accept(Encoder& v) const override
	{
		v.visit(*this);
	}
};

struct JsonPubEncoder :Encoder {

	void visit(const Header& p) override {
		oss << "$PUB{\"" << p.part1 << ">" << p.part2 << "\":";
	}

	void visit(const DataItem& li) override {
		oss << "{\"" << li.part1 << "\":\"" << li.part2 << "\"},";
	}

	void visit(const Footer& li) override {
		oss << li.part1 << "}\n" << li.part2;
	}

	void visit(const Data& l) override {
		for (const auto& item : l) {
			item.accept(*this);
		}
		oss.seekp(-1, std::ios_base::end);
	}

	std::string str() override {
		std::string str_{ oss.str() };
		oss.str(""); oss.clear();
		return str_;
	}

private:
	std::ostringstream oss;
};

struct JsonMsgEncoder :Encoder {

	void visit(const Header& p) override {
		oss << "$MSG{\"" << p.part1 << ">" << p.part2 << "\":";
	}

	void visit(const DataItem& li) override {
		oss << "{\"" << li.part1 << "\":\"" << li.part2 << "\"}";
	}

	void visit(const Footer& li) override {
		oss << li.part1 << "}\n" << li.part2;
	}

	void visit(const Data& l) override {
	}

	std::string str() override {
		std::string str_{ oss.str() };
		oss.str(""); oss.clear();
		return str_;
	}

private:
	std::ostringstream oss;
};


struct JsonReqEncoder :Encoder {

	void visit(const Header& p) override {
		oss << "$GET{\"" << p.part1 << ">" << p.part2 << "\":";
	}

	void visit(const DataItem& li) override {
		oss << "{\"" << li.part1 << "\":\"" << li.part2 << "\"}";
	}

	void visit(const Footer& li) override {
		oss << li.part1 << "}\n" << li.part2;
	}

	void visit(const Data& l) override {
	}

	std::string str() override {
		std::string str_{ oss.str() };
		oss.str(""); oss.clear();
		return str_;
	}

private:
	std::ostringstream oss;
};

struct SmsMsgEncoder :Encoder {

	void visit(const Header& p) override {
		oss << "[" << p.part1 << " ";
	}

	void visit(const DataItem& li) override {
		oss << li.part1 << ": " << li.part2;
	}

	void visit(const Footer& li) override {
		oss << li.part1 << "]\n" << li.part2;
	}

	void visit(const Data& l) override {
	}

	std::string str() override {
		std::string str_{ oss.str() };
		oss.str(""); oss.clear();
		return str_;
	}

private:
	std::ostringstream oss;
};

