#pragma once

#include "command-target.h"

#include <docobj.h>
#include <vector>
#include <wpl/concepts.h>

namespace wpl
{
	namespace vs
	{
		class ole_command_target : public IOleCommandTarget, public command_target, noncopyable
		{
		public:
			ole_command_target(const GUID &group_id);

			virtual void add_command(int id, const execute_fn &execute, bool is_group = false,
				const query_state_fn &query_state = query_state_fn(),
				const get_name_fn &get_name = get_name_fn());

			STDMETHODIMP QueryStatus(const GUID *group, ULONG count, OLECMD commands[], OLECMDTEXT *command_text);
			STDMETHODIMP Exec(const GUID *group, DWORD command_, DWORD option, VARIANT *in, VARIANT *out);

		private:
			struct command
			{
				int id;
				execute_fn execute;
				bool is_group;
				query_state_fn query_state;
				get_name_fn get_name;
			};
			struct command_less;
			typedef std::vector<command> commands;

		private:
			static unsigned convert_state(unsigned /*command_target::state_flags*/ state);
			const command *get_command(int id, unsigned &item) const;

		private:
			const GUID _group_id;
			commands _commands;
			std::wstring _cache;
		};
	}
}
