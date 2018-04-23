#pragma once

class Client : public boost::enable_shared_from_this<Client>
{
public:
	Client(tcp::socket socket);
	virtual ~Client();

	void run();
	static void broadcast(boost::weak_ptr<Client> ptr, const boost::property_tree::ptree& ptree);
	void on_accept(boost::system::error_code ec);
	void async_write(const boost::property_tree::ptree& ptree);
	void on_write(const boost::system::error_code& ec, std::size_t bytes_transferred);
	void async_read();
	void on_read(const boost::system::error_code& ec, std::size_t bytes_transferred);
	void on_msg(std::string cmd, const boost::property_tree::ptree& ptree);
	void on_heart(std::string cmd, const boost::property_tree::ptree& ptree);
	void on_login(std::string cmd, const boost::property_tree::ptree& ptree);

private:
	websocket::stream<tcp::socket> ws_;
	std::string remote_addr;
	boost::asio::strand<boost::asio::io_context::executor_type> strand_;
	boost::beast::multi_buffer read_buffer;
	std::queue<std::string> write_buffer;
	time_t last_recv_time;
	bool writing;
	uint64_t userid;
	boost::signals2::connection cn_sig_broadcast;
};

typedef boost::shared_ptr<Client> CLIENT_PTR;