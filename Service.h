#pragma once

class Client;

class CService
{
public:
	CService(boost::asio::io_context *ioc);
	virtual ~CService();

	bool Start();
	/**
	* 定时器回调，间隔1分钟
	*/
	void on_timeout(boost::shared_ptr<boost::asio::deadline_timer> timer, const boost::system::error_code& error);
	bool read_data_from_db();			//从数据库读取数据
	void broadcast(const boost::property_tree::ptree& ptree);	//全服广播
	void set_user(int64_t uid, boost::weak_ptr<Client> client);
	void del_user(int64_t uid);
	bool exist_user(int64_t uid);
	boost::weak_ptr<Client> find_user(int64_t uid);

public:
	boost::asio::io_context *ioc;	//io上下文（线程安全）
	boost::signals2::signal<void(const boost::property_tree::ptree&)> sig_broadcast;	//全服广播（线程安全）
	std::map<std::string, CDbPool*> dbpools;	//线程池列表（线程安全）

private:
	std::map<int64_t, boost::weak_ptr<Client> > users;	//在线用户列表
	boost::shared_mutex mutex_users;
};

extern CService *g_service;