// main.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "listener.h"
#include "Base.h"
#include "Service.h"

using boost::asio::ip::tcp;

boost::asio::io_context *g_ioc = nullptr;
unsigned short g_port = 8888;	//侦听端口
boost::log::trivial::severity_level g_log_level = boost::log::trivial::trace;

void init_log()
{
	boost::log::add_console_log(std::cout, boost::log::keywords::format = (
		boost::log::expressions::stream
		<< boost::log::expressions::format_date_time< boost::posix_time::ptime >("TimeStamp", "%H:%M:%S.%f")
		<< " <" << boost::log::trivial::severity
		<< "> " << boost::log::expressions::smessage)
	);
	std::string log_file = "log/websocket-service_%Y%m%d_%N.log";
	AddPath(log_file);
	boost::log::add_file_log
	(
		boost::log::keywords::file_name = log_file,
		boost::log::keywords::rotation_size = 50 * 1024 * 1024,
		boost::log::keywords::time_based_rotation = boost::log::sinks::file::rotation_at_time_point(0, 0, 0),
		boost::log::keywords::auto_flush = true,
		boost::log::keywords::open_mode = std::ios::app,
		boost::log::keywords::format = (
			boost::log::expressions::stream
			<< boost::log::expressions::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
			<< "|" << boost::log::expressions::attr<boost::log::attributes::current_thread_id::value_type>("ThreadID")
			<< "|" << boost::log::trivial::severity
			<< "|" << boost::log::expressions::smessage
			)
	);
	boost::log::core::get()->set_filter
	(
		boost::log::trivial::severity >= boost::log::trivial::trace
	);
	boost::log::add_common_attributes();
}

void set_log_level(boost::log::trivial::severity_level log_level)
{
	boost::log::core::get()->set_filter(boost::log::trivial::severity >= log_level);
}

void load_config()
{
	boost::property_tree::ptree ptree;
	try {
		std::string config_file = "config.xml";
		AddPath(config_file);
		boost::property_tree::xml_parser::read_xml(config_file, ptree);
		g_log_level = (boost::log::trivial::severity_level)ptree.get("config.log_level", 0);
		if (g_log_level < 0 || g_log_level > 5)
			g_log_level = boost::log::trivial::trace;
		BOOST_LOG_TRIVIAL(info) << "log_level: " << (int)g_log_level;
		g_port = ptree.get<unsigned short>("config.port");

		for (auto& ptree_db : ptree.get_child("config")) {
			if (ptree_db.first != "db")
				continue;

			boost::property_tree::ptree db = ptree_db.second;
			auto attributes = db.get_child("<xmlattr>");
			std::string name = attributes.get<std::string>("name");
			std::string host = attributes.get<std::string>("ip");
			unsigned int port = attributes.get<unsigned int>("port");
			std::string user = attributes.get<std::string>("user");
			std::string passwd = attributes.get<std::string>("pass");
			int threads = attributes.get<int>("threads");
			auto pool = new CDbPool(host, user, passwd, name, port, threads, "utf8");

			g_service->dbpools[name] = pool;
		}
	}
	catch (const std::exception& e) {
		BOOST_LOG_TRIVIAL(fatal) << u8"创建数据库连接池失败！" << e.what();
		std::abort();
	}
}

int main()
{
	init_log();
	BOOST_LOG_TRIVIAL(info) << "--------------------------------------------------";
	BOOST_LOG_TRIVIAL(info) << "compiled at " << __DATE__ << " " << __TIME__;

	boost::asio::io_context ioc;
	boost::asio::io_context::work work(ioc);
	g_ioc = &ioc;
	CService service(&ioc);

	load_config();
	set_log_level(g_log_level);

	boost::make_shared<listener>(ioc, tcp::endpoint{ tcp::v4(),g_port })->run();
	BOOST_LOG_TRIVIAL(info) << "listening on 0.0.0.0:" << g_port << " ...";

	boost::thread_group tg;
	for (int i = 0; i < 4; i++) {
		tg.add_thread(new boost::thread([&ioc] {
			srand((uint32_t)time(NULL));
			ioc.run();
		}));
	}

	if (!service.Start())
		return -1;

	tg.join_all();

	return 0;
}