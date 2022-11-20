#pragma once

namespace
{
	struct logger
	{
		logger(const char *)
		{	}

		template <typename T>
		const logger &operator %(T) const
		{	return *this;	}
	};
}

#define LOG(x) (logger(x))
#define LOGE(x) (logger(x))
#define A(x) (x)
