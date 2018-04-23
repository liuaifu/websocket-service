#include "stdafx.h"
#include "Service.h"
#include "Client.h"

#define log_trace BOOST_LOG_TRIVIAL(trace) << remote_addr << "|"
#define log_debug BOOST_LOG_TRIVIAL(debug) << remote_addr << "|"
#define log_info BOOST_LOG_TRIVIAL(info) << remote_addr << "|"
#define log_warning BOOST_LOG_TRIVIAL(warning) << remote_addr << "|"
#define log_error BOOST_LOG_TRIVIAL(error) << remote_addr << "|"
#define log_fatal BOOST_LOG_TRIVIAL(fatal) << remote_addr << "|"

Client::Client(tcp::socket socket)
	: ws_(std::move(socket)), strand_(ws_.get_executor())
{
	try {
		remote_addr = ws_.next_layer().remote_endpoint().address().to_string() + ":" + std::to_string(ws_.next_layer().remote_endpoint().port());
	}
	catch (const std::exception&) {
	}

	log_debug << "Client()";
	last_recv_time = time(NULL);
	writing = false;
	userid = 0;
}

Client::~Client()
{
	log_debug << "~Client()";

	cn_sig_broadcast.disconnect();
	g_service->del_user(userid);

	boost::system::error_code ec;
	ws_.next_layer().close(ec);
}

void Client::run()
{
	boost::beast::websocket::permessage_deflate opt;
	opt.client_enable = true; // for clients
	opt.server_enable = true; // for servers
	ws_.set_option(opt);

	// Accept the websocket handshake
	ws_.async_accept(
		boost::asio::bind_executor(
			strand_,
			std::bind(
				&Client::on_accept,
				shared_from_this(),
				std::placeholders::_1)));
}

void Client::broadcast(boost::weak_ptr<Client> ptr, const boost::property_tree::ptree& ptree)
{
	auto client = ptr.lock();
	if (!client)
		return;	//client已销毁

	client->async_write(ptree);
}

void Client::on_accept(boost::system::error_code ec)
{
	if (ec) {
		log_warning << __FUNCTION__ << u8"：" << ec.message();
		if (ws_.next_layer().is_open())
			ws_.next_layer().cancel();
		return;
	}

	async_read();
}

void Client::async_write(const boost::property_tree::ptree& ptree)
{
	if (!ws_.is_open())
		return;

	std::stringstream ss;
	boost::property_tree::write_json(ss, ptree);
	std::string msg = ss.str();

	log_trace << __FUNCTION__ << u8"：" << msg;

	ws_.text(true);

	if (!writing) {
		writing = true;
		ws_.async_write(boost::asio::buffer(msg),
			boost::asio::bind_executor(
				strand_,
				boost::bind(&Client::on_write, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2)
			)
		);
	}
	else {
		//排队发送
		write_buffer.push(msg);
	}
}

void Client::on_write(const boost::system::error_code& ec, std::size_t bytes_transferred)
{
	if (ec) {
		if (ec == boost::asio::error::operation_aborted)
			return;

		log_debug << __FUNCTION__ << u8"：" << ec.message();
		if (ws_.next_layer().is_open()) {
			boost::system::error_code ec;
			ws_.next_layer().cancel(ec);
		}

		return;
	}

	if (write_buffer.size() > 0) {
		auto msg = write_buffer.front();
		write_buffer.pop();
		ws_.async_write(boost::asio::buffer(msg),
			boost::asio::bind_executor(
				strand_,
				boost::bind(&Client::on_write, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2)
			)
		);
	}
	else
		writing = false;
}

void Client::async_read()
{
	ws_.async_read(
		read_buffer,
		boost::asio::bind_executor(
			strand_,
			boost::bind(&Client::on_read, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2)));
}

void Client::on_read(const boost::system::error_code& ec, std::size_t bytes_transferred)
{
	if (ec) {
		log_debug << __FUNCTION__ << u8"：" << ec.message();
		if (ec == boost::asio::error::operation_aborted)
			return;

		if (ws_.next_layer().is_open()) {
			boost::system::error_code ec;
			ws_.next_layer().cancel(ec);
		}
		return;
	}

	last_recv_time = time(NULL);

	std::string data = boost::beast::buffers_to_string(read_buffer.data());
	log_trace << __FUNCTION__ << u8"：" << data;
	size_t size = boost::asio::buffer_size(read_buffer.data());
	read_buffer.consume(size);

	boost::property_tree::ptree ptree;
	std::string cmd;
	try
	{
		std::stringstream ss(data);
		boost::property_tree::json_parser::read_json(ss, ptree);
		cmd = ptree.get<std::string>("command");
	}
	catch (const std::exception& e)
	{
		log_warning << __FUNCTION__ << u8"：解析请求失败，" << e.what() << u8"，" << data;
		async_read();
		return;
	}

	on_msg(cmd, ptree);
	async_read();
}

void Client::on_msg(std::string cmd, const boost::property_tree::ptree& ptree)
{
	if (cmd == "heart")
		on_heart(cmd, ptree);
	else if (cmd == "guest_login")
		on_login(cmd, ptree);
	else
		log_warning << "unhandled command: " << cmd;
}

void Client::on_heart(std::string cmd, const boost::property_tree::ptree& ptree)
{
	log_trace << __FUNCTION__;

	async_write(ptree);
}

void Client::on_login(std::string cmd, const boost::property_tree::ptree& ptree)
{
	FunctionTimer ft(__FUNCTION__);	//函数计时

	std::string username;
	std::string password;
	try {
		username = ptree.get<std::string>("username");
		password = ptree.get<std::string>("password");
	}
	catch (const std::exception&) {
		//参数错误
	}

	//校验校验
	//...

	//发送结果
	boost::property_tree::ptree ack;
	ack.put("command", cmd);
	ack.put("result", 0);
	//其它数据...
	async_write(ptree);

	//登录成功后绑定userid和弱指针
	//保存这么个map的作用是以便对个别或一批用户操作，比如用户在网站购买道具后网站通知游戏服务，游戏服务查询用户并推送消息
	userid = 123;
	g_service->set_user(userid, boost::weak_ptr<Client>(shared_from_this()));
	cn_sig_broadcast = g_service->sig_broadcast.connect(boost::bind(&Client::broadcast, boost::weak_ptr<Client>(shared_from_this()), _1));
}