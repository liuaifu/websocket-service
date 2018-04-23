#pragma once

class FunctionTimer : public boost::timer::cpu_timer
{
public:
	FunctionTimer(std::string name, int threshold_value = 100) {
		this->name = name;
		this->threshold_value = threshold_value;
	}

	~FunctionTimer() {
		int64_t elapsed_time = elapsed().wall / 1000000;
		if (elapsed_time >= threshold_value)
			BOOST_LOG_TRIVIAL(warning) << name << u8"耗时" << elapsed_time << "ms";
		else if(elapsed_time > 10)
			BOOST_LOG_TRIVIAL(trace) << name << u8"耗时" << elapsed_time << "ms";
	}

private:
	std::string name;
	int threshold_value;
};