#include "stdafx.h"
#include "Service.h"

CService *g_service = NULL;

CService::CService(boost::asio::io_context *ioc)
{
	this->ioc = ioc;
	g_service = this;
}

CService::~CService()
{
}

bool CService::Start()
{
	//设置定时器
	auto timer = boost::shared_ptr<boost::asio::deadline_timer>(new boost::asio::deadline_timer(*ioc));
	timer->expires_from_now(boost::posix_time::minutes(1));
	timer->async_wait(boost::bind(&CService::on_timeout, this, timer, _1));

	return true;
}

void CService::on_timeout(boost::shared_ptr<boost::asio::deadline_timer> timer, const boost::system::error_code& error)
{
	if (error == boost::asio::error::operation_aborted)
		return;

	auto t = boost::posix_time::second_clock::local_time();
	if (t.time_of_day().hours() == 0 && t.time_of_day().minutes() == 0) {
		//每天0点0分定时任务
		//...
	}

	timer->expires_from_now(boost::posix_time::minutes(1));
	timer->async_wait(boost::bind(&CService::on_timeout, this, timer, _1));
}

bool CService::read_data_from_db()
{
	FunctionTimer ft(__FUNCTION__);	//函数计时

	std::string dbname = "db1";
	auto it = dbpools.find(dbname);
	if (it == dbpools.end()) {
		BOOST_LOG_TRIVIAL(error) << __FUNCTION__ << ": dbname error, " << dbname;
		return false;
	}

	auto cn = it->second->GetDbConnection();
	if (cn == NULL) {
		BOOST_LOG_TRIVIAL(error) << __FUNCTION__ << ": get db connection fail";
		return false;
	}

	BOOST_SCOPE_EXIT_ALL(&) {
		it->second->FreeDbConnection(cn);
	};

	boost::format fmt = boost::format("SELECT 1,2 FROM table1");
	std::string sql = fmt.str();
	bool ret = cn->Query(sql.c_str());
	if (!ret) {
		BOOST_LOG_TRIVIAL(error) << __FUNCTION__ << ": " << cn->GetLastError() << ", " << sql;
		return false;
	}
	auto result = cn->GetResult();
	if (result.size() == 0) {
		BOOST_LOG_TRIVIAL(warning) << __FUNCTION__ << ": " << fmt.str() << ", no result";
		return false;
	}

	try {
		for (size_t i = 0; i < result.size(); i++)
		{
			auto col1 = boost::lexical_cast<int> (result[i][0]);
			auto col2 = boost::lexical_cast<int> (result[i][1]);
			//...
		}
	}
	catch (const boost::bad_lexical_cast& e) {
		BOOST_LOG_TRIVIAL(warning) << __FUNCTION__ << ": " << e.what();
		return false;
	}

	return true;
}

void CService::broadcast(const boost::property_tree::ptree& ptree)
{
	sig_broadcast(ptree);
}

void CService::set_user(int64_t uid, boost::weak_ptr<Client> client)
{
	boost::unique_lock<boost::shared_mutex> lock(mutex_users);
	users[uid] = client;
}

void CService::del_user(int64_t uid)
{
	boost::unique_lock<boost::shared_mutex> lock(mutex_users);
	users.erase(uid);
}

bool CService::exist_user(int64_t uid)
{
	boost::shared_lock<boost::shared_mutex> lock(mutex_users);
	return users.count(uid) == 1;
}

boost::weak_ptr<Client> CService::find_user(int64_t uid)
{
	boost::shared_lock<boost::shared_mutex> lock(mutex_users);
	if (users.count(uid) == 1)
		return users[uid];

	return boost::weak_ptr<Client>();
}