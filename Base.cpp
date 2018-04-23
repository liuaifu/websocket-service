#include "stdafx.h"
#include "Base.h"
#include <boost/dll.hpp>
#include <boost/locale.hpp>

std::string AddPath(std::string& filename)
{
	auto path = boost::dll::program_location().parent_path();
	filename = path.append(filename).string();
	return filename;
}

std::string GbToUtf8(std::string str)
{
	std::string str_utf8;

	try {
		auto str_unicode = boost::locale::conv::to_utf<wchar_t>(str, "GB18030");
		str_utf8 = boost::locale::conv::from_utf(str_unicode, "UTF-8");
	}
	catch (const std::exception&) {
		str_utf8 = str;
	}

	return str_utf8;
}

std::string Utf8ToGb(std::string str)
{
	std::string str_gbk;

	try {
		str_gbk = boost::locale::conv::from_utf(str, "GBK");
	}
	catch (const std::exception&) {
		str_gbk = str;
	}

	return str_gbk;
}