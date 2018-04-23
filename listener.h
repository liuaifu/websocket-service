#pragma once

class listener : public boost::enable_shared_from_this<listener>
{
public:
	listener(boost::asio::io_context& ioc, tcp::endpoint endpoint);
	virtual ~listener();

	void run();
	void do_accept();
	void on_accept(const boost::system::error_code& ec);

private:
	tcp::acceptor acceptor_;
	tcp::socket socket_;
};

