#include "stdafx.h"
#include "listener.h"
#include "Client.h"

listener::listener(boost::asio::io_context& ioc, tcp::endpoint endpoint)
	: acceptor_(ioc)
	, socket_(ioc)
{
	boost::system::error_code ec;

	// Open the acceptor
	acceptor_.open(endpoint.protocol(), ec);
	if (ec)
	{
		BOOST_LOG_TRIVIAL(fatal) << "open acceptor fail! " << ec.message();
		return;
	}
	acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
	// Bind to the server address
	acceptor_.bind(endpoint, ec);
	if (ec)
	{
		BOOST_LOG_TRIVIAL(fatal) << "bind fail! " << ec.message();
		boost::this_thread::sleep(boost::posix_time::seconds(3));
		acceptor_.bind(endpoint, ec);
		if (ec)
		{
			BOOST_LOG_TRIVIAL(fatal) << "rebind fail! " << ec.message();
			std::abort();
			return;
		}
		BOOST_LOG_TRIVIAL(info) << "rebind success " << ec.message();
	}

	// Start listening for connections
	acceptor_.listen(boost::asio::socket_base::max_listen_connections, ec);
	if (ec)
	{
		BOOST_LOG_TRIVIAL(fatal) << "listen fail! " << ec.message();
		return;
	}
}

listener::~listener()
{
}

void listener::run()
{
	if (!acceptor_.is_open())
		return;
	do_accept();
}

void listener::do_accept()
{
	acceptor_.async_accept(
		socket_,
		std::bind(
			&listener::on_accept,
			shared_from_this(),
			std::placeholders::_1));
}

void listener::on_accept(const boost::system::error_code& ec)
{
	if (ec)
	{
		BOOST_LOG_TRIVIAL(error) << "accept fail! " << ec.message();
		return;
	}

	// Create the session and run it
	boost::make_shared<Client>(std::move(socket_))->run();

	// Accept another connection
	do_accept();
}