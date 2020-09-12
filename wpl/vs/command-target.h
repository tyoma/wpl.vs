//	Copyright (c) 2011-2020 by Artem A. Gevorkyan (gevorkyan.org)
//
//	Permission is hereby granted, free of charge, to any person obtaining a copy
//	of this software and associated documentation files (the "Software"), to deal
//	in the Software without restriction, including without limitation the rights
//	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//	copies of the Software, and to permit persons to whom the Software is
//	furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//	THE SOFTWARE.

#pragma once

#include <functional>
#include <string>

namespace wpl
{
	namespace vs
	{
		struct command_target
		{
			enum state_flags { supported = 0x01, enabled = 0x02, visible = 0x04, checked = 0x08 };

			typedef std::function<void (unsigned item)> execute_fn;
			typedef std::function<bool (unsigned item, unsigned /*state_flags*/ &state)> query_state_fn;
			typedef std::function<bool (unsigned item, std::wstring &name)> get_name_fn;

			virtual void add_command(int id, const execute_fn &execute, bool is_group = false,
				const query_state_fn &query_state = query_state_fn(), const get_name_fn &get_name = get_name_fn()) = 0;
		};
	}
}
