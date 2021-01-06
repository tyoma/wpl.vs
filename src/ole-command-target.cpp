//	Copyright (c) 2011-2021 by Artem A. Gevorkyan (gevorkyan.org)
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

#include <wpl/vs/ole-command-target.h>

#include <algorithm>

using namespace std;

namespace wpl
{
	namespace vs
	{
		struct ole_command_target::command_less
		{
			bool operator ()(const command &lhs, const command &rhs) const
			{	return lhs.id < rhs.id;	}

			bool operator ()(int lhs, const command &rhs) const
			{	return lhs < rhs.id;	}

			bool operator ()(const command &lhs, int rhs) const
			{	return lhs.id < rhs;	}
		};


		ole_command_target::ole_command_target(const GUID &group_id)
			: _group_id(group_id)
		{	}

		void ole_command_target::add_command(int id, const execute_fn &execute, bool is_group,
			const query_state_fn &query_state, const get_name_fn &get_name)
		{
			const auto at = upper_bound(_commands.begin(), _commands.end(), id, command_less());
			const command c = { id, execute, is_group, query_state, get_name };

			_commands.insert(at, c);
		}

		STDMETHODIMP ole_command_target::QueryStatus(const GUID *group, ULONG count, OLECMD commands[],
			OLECMDTEXT *command_text)
		try
		{
			if (!group || *group != _group_id)
				return OLECMDERR_E_UNKNOWNGROUP;
			while (count--)
			{
				unsigned item;
				OLECMD &cmd = commands[count];

				cmd.cmdf = 0;
				if (const auto c = get_command(cmd.cmdID, item))
				{
					unsigned state;

					if (!c->query_state)
					{
						state = supported;
					}
					else if (!c->query_state(item, state))
					{
						if (item)
							return OLECMDERR_E_NOTSUPPORTED;
						state = supported;
					}
					cmd.cmdf = convert_state(state);
					if (command_text)
					{
						if (c->get_name && c->get_name(item, _cache))
						{
							wcsncpy(command_text->rgwz, _cache.c_str(), command_text->cwBuf - 1);
							command_text->rgwz[command_text->cwBuf - 1] = L'\0';
							command_text->cwActual = static_cast<ULONG>(_cache.size() + 1);
						}
						else
						{
							command_text->cwActual = 0;
						}
					}
				}
				else
				{
					return OLECMDERR_E_NOTSUPPORTED;
				}
			}
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}

		STDMETHODIMP ole_command_target::Exec(const GUID *group, DWORD command_id, DWORD, VARIANT *, VARIANT *)
		try
		{
				if (!group || *group != _group_id)
					return OLECMDERR_E_UNKNOWNGROUP;

			unsigned item;
			const auto c = get_command(command_id, item);

			return c ? c->execute(item), S_OK : OLECMDERR_E_NOTSUPPORTED;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}

		unsigned ole_command_target::convert_state(unsigned state)
		{
			return (state & command_target::supported ? OLECMDF_SUPPORTED : 0)
				| (state & command_target::enabled ? OLECMDF_ENABLED : 0)
				| (state & command_target::visible ? 0 : OLECMDF_INVISIBLE | OLECMDF_DEFHIDEONCTXTMENU)
				| (state & command_target::checked ? OLECMDF_LATCHED : 0);
		}

		inline const ole_command_target::command *ole_command_target::get_command(int id, unsigned &item) const
		{
			auto i = std::upper_bound(_commands.begin(), _commands.end(), id, command_less());

			return i != _commands.begin() && ((--i)->is_group || i->id == id)
				? item = id - i->id, &*i
				: nullptr;
		}
	}
}
