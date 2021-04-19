#include <wpl/vs/ole-command-target.h>

#include <vector>
#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace wpl
{
	namespace vs
	{
		namespace tests
		{
			namespace
			{
				extern const GUID c_guid1 = { 0x12345678, 0x000, 0x000, { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 } };
				extern const GUID c_guid2 = { 0x22345678, 0x000, 0x000, { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 } };
				extern const GUID c_guid3 = { 0x32345678, 0x000, 0x000, { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 } };

				class ole_command_target_final : public ole_command_target
				{
				public:
					ole_command_target_final(const GUID &group_id)
						: ole_command_target(group_id)
					{	}

				private:
					STDMETHODIMP QueryInterface(REFIID /*iid*/, void ** /*ppv*/) {	throw 0;	}
					STDMETHODIMP_(ULONG) AddRef() {	throw 0;	}
					STDMETHODIMP_(ULONG) Release() {	throw 0;	}
				};
			}

			begin_test_suite( OleCommandTargetTests )
				test( CommandTargetRejectsProcessingOfCommandsFromUnknownSets )
				{
					// INIT
					ole_command_target_final ct1(c_guid1);
					IOleCommandTarget &ict1 = ct1;
					ole_command_target_final ct2(c_guid2);
					IOleCommandTarget &ict2 = ct2;
					OLECMD dummy[1];

					// ACT / ASSERT
					assert_equal(OLECMDERR_E_UNKNOWNGROUP, ict1.QueryStatus(NULL, 1, dummy, NULL));
					assert_equal(OLECMDERR_E_UNKNOWNGROUP, ict1.QueryStatus(&c_guid2, 1, dummy, NULL));
					assert_equal(OLECMDERR_E_UNKNOWNGROUP, ict1.QueryStatus(&c_guid3, 1, dummy, NULL));
					assert_equal(OLECMDERR_E_UNKNOWNGROUP, ict2.QueryStatus(NULL, 1, dummy, NULL));
					assert_equal(OLECMDERR_E_UNKNOWNGROUP, ict2.QueryStatus(&c_guid1, 1, dummy, NULL));
					assert_equal(OLECMDERR_E_UNKNOWNGROUP, ict2.QueryStatus(&c_guid3, 1, dummy, NULL));

					assert_equal(OLECMDERR_E_UNKNOWNGROUP, ict1.Exec(NULL, 1, OLECMDEXECOPT_DODEFAULT, NULL, NULL));
					assert_equal(OLECMDERR_E_UNKNOWNGROUP, ict1.Exec(&c_guid2, 1, OLECMDEXECOPT_DODEFAULT, NULL, NULL));
					assert_equal(OLECMDERR_E_UNKNOWNGROUP, ict1.Exec(&c_guid3, 1, OLECMDEXECOPT_DODEFAULT, NULL, NULL));
					assert_equal(OLECMDERR_E_UNKNOWNGROUP, ict2.Exec(NULL, 1, OLECMDEXECOPT_DODEFAULT, NULL, NULL));
					assert_equal(OLECMDERR_E_UNKNOWNGROUP, ict2.Exec(&c_guid1, 1, OLECMDEXECOPT_DODEFAULT, NULL, NULL));
					assert_equal(OLECMDERR_E_UNKNOWNGROUP, ict2.Exec(&c_guid3, 1, OLECMDEXECOPT_DODEFAULT, NULL, NULL));
				}


				test( CommandTargetRejectsProcessingOfUnknownCommands )
				{
					// INIT
					ole_command_target_final ct1(c_guid1);
					IOleCommandTarget &ict1 = ct1;
					ole_command_target_final ct2(c_guid2);
					IOleCommandTarget &ict2 = ct2;
					OLECMD cmd1 = { 1, };
					OLECMD cmd2 = { 2, };

					// ACT / ASSERT
					assert_equal(OLECMDERR_E_NOTSUPPORTED, ict1.QueryStatus(&c_guid1, 1, &cmd1, NULL));
					assert_equal(OLECMDERR_E_NOTSUPPORTED, ict1.QueryStatus(&c_guid1, 1, &cmd2, NULL));
					assert_equal(OLECMDERR_E_NOTSUPPORTED, ict2.QueryStatus(&c_guid2, 1, &cmd1, NULL));
					assert_equal(OLECMDERR_E_NOTSUPPORTED, ict2.QueryStatus(&c_guid2, 1, &cmd2, NULL));

					assert_equal(OLECMDERR_E_NOTSUPPORTED, ict1.Exec(&c_guid1, 1, OLECMDEXECOPT_DODEFAULT, NULL, NULL));
					assert_equal(OLECMDERR_E_NOTSUPPORTED, ict1.Exec(&c_guid1, 2, OLECMDEXECOPT_DODEFAULT, NULL, NULL));
					assert_equal(OLECMDERR_E_NOTSUPPORTED, ict2.Exec(&c_guid2, 3, OLECMDEXECOPT_DODEFAULT, NULL, NULL));
					assert_equal(OLECMDERR_E_NOTSUPPORTED, ict2.Exec(&c_guid2, 4, OLECMDEXECOPT_DODEFAULT, NULL, NULL));
				}


				static int getSingleCommandState(IOleCommandTarget &target, const GUID &command_set, unsigned command)
				{
					OLECMD cmd = { command, 0 };

					assert_equal(S_OK, target.QueryStatus(&command_set, 1, &cmd, NULL));
					return cmd.cmdf;
				}

				test( CommandStateIsResponded )
				{
					// INIT
					unsigned state = 0u;
					ole_command_target_final oct(c_guid1);
					command_target &ct = oct;

					ct.add_command(11, [] (unsigned) {}, false, [&] (unsigned, unsigned &state_) {
						return state_ = state, true;
					});

					// ACT / ASSERT
					assert_equal(OLECMDF_DEFHIDEONCTXTMENU | OLECMDF_INVISIBLE,
						getSingleCommandState(oct, c_guid1, 11));

					// INIT
					state = command_target::supported;

					// ACT / ASSERT
					assert_equal(OLECMDF_SUPPORTED | OLECMDF_DEFHIDEONCTXTMENU | OLECMDF_INVISIBLE,
						getSingleCommandState(oct, c_guid1, 11));

					// INIT
					state = command_target::supported | command_target::enabled;

					// ACT / ASSERT
					assert_equal(OLECMDF_SUPPORTED | OLECMDF_ENABLED | OLECMDF_DEFHIDEONCTXTMENU | OLECMDF_INVISIBLE,
						getSingleCommandState(oct, c_guid1, 11));

					// INIT
					state = command_target::supported | command_target::visible;

					// ACT / ASSERT
					assert_equal(OLECMDF_SUPPORTED, getSingleCommandState(oct, c_guid1, 11));

					// INIT
					state = command_target::supported | command_target::visible | command_target::checked;

					// ACT / ASSERT
					assert_equal(OLECMDF_SUPPORTED | OLECMDF_LATCHED, getSingleCommandState(oct, c_guid1, 11));
				}


				test( CommandIsSelectedByID )
				{
					// INIT
					ole_command_target_final oct(c_guid1);

					oct.add_command(10, [] (unsigned) {}, false, [] (unsigned, unsigned &) {
						return false;
					});
					oct.add_command(19, [] (unsigned) {}, false, [] (unsigned, unsigned &) {
						return false;
					});
					oct.add_command(11, [] (unsigned) {}, false, [] (unsigned, unsigned &state_) {
						return state_ = command_target::supported | command_target::visible | command_target::enabled, true;
					});
					oct.add_command(13, [] (unsigned) {}, false, [] (unsigned, unsigned &state_) {
						return state_ = command_target::supported | command_target::visible | command_target::checked, true;
					});
					oct.add_command(5, [] (unsigned) {}, false, [] (unsigned, unsigned &state_) {
						return state_ = command_target::supported | command_target::visible, true;
					});

					// ACT / ASSERT
					assert_equal(OLECMDF_SUPPORTED, getSingleCommandState(oct, c_guid1, 5));
					assert_equal(OLECMDF_SUPPORTED | OLECMDF_DEFHIDEONCTXTMENU | OLECMDF_INVISIBLE, getSingleCommandState(oct, c_guid1, 10));
					assert_equal(OLECMDF_SUPPORTED | OLECMDF_ENABLED, getSingleCommandState(oct, c_guid1, 11));
					assert_equal(OLECMDF_SUPPORTED | OLECMDF_LATCHED, getSingleCommandState(oct, c_guid1, 13));
					assert_equal(OLECMDF_SUPPORTED | OLECMDF_DEFHIDEONCTXTMENU | OLECMDF_INVISIBLE, getSingleCommandState(oct, c_guid1, 19));
				}


				test( MissingCommandsAreNotSupported )
				{
					// INIT
					ole_command_target_final oct(c_guid2);
					OLECMD cmd = { 10, 10000 };

					oct.add_command(11, [] (unsigned) {}, false, [] (unsigned, unsigned &) {
						return true;
					});

					// ACT / ASSERT
					assert_equal(OLECMDERR_E_NOTSUPPORTED, oct.QueryStatus(&c_guid2, 1, &cmd, NULL));

					// ASSERT
					assert_equal(0u, cmd.cmdf);

					// INIT
					cmd.cmdID = 12;
					cmd.cmdf = 123;

					// ACT / ASSERT
					assert_equal(OLECMDERR_E_NOTSUPPORTED, oct.QueryStatus(&c_guid2, 1, &cmd, NULL));

					// ASSERT
					assert_equal(0u, cmd.cmdf);
				}


				test( GroupCommandIsSelectedByRange )
				{
					// INIT
					ole_command_target_final oct(c_guid1);
					OLECMD cmd = { 48, 10000 };

					oct.add_command(10, [] (unsigned) {}, false, [] (unsigned, unsigned &state) {
						return state = command_target::supported, true;
					});
					oct.add_command(31, [] (unsigned) {}, true, [] (unsigned item, unsigned &state) {
						return item < 17
							? state = command_target::supported | command_target::enabled | command_target::visible, true
							: false;
					});
					oct.add_command(130, [] (unsigned) {}, true, [] (unsigned item, unsigned &state) {
						return item < 2 ? state = command_target::supported | command_target::visible, true : false;
					});

					// ACT / ASSERT
					assert_equal(OLECMDF_SUPPORTED | OLECMDF_ENABLED, getSingleCommandState(oct, c_guid1, 31));
					assert_equal(OLECMDF_SUPPORTED | OLECMDF_ENABLED, getSingleCommandState(oct, c_guid1, 35));
					assert_equal(OLECMDF_SUPPORTED | OLECMDF_ENABLED, getSingleCommandState(oct, c_guid1, 47));
					assert_equal(OLECMDERR_E_NOTSUPPORTED, oct.QueryStatus(&c_guid1, 1, &cmd, NULL));

					// ASSERT
					assert_equal(0u, cmd.cmdf);

					// INIT
					cmd.cmdID = 132;

					// ACT / ASSERT
					assert_equal(OLECMDF_SUPPORTED, getSingleCommandState(oct, c_guid1, 130));
					assert_equal(OLECMDF_SUPPORTED, getSingleCommandState(oct, c_guid1, 131));
					assert_equal(OLECMDERR_E_NOTSUPPORTED, oct.QueryStatus(&c_guid1, 1, &cmd, NULL));
				}


				test( EmptyGroupIsRespondedAsOKButDisabledAndInvisible )
				{
					// INIT
					ole_command_target_final oct(c_guid1);

					oct.add_command(10, [] (unsigned) {}, false, [] (unsigned, unsigned &) {
						return true;
					});
					oct.add_command(31, [] (unsigned) {}, true, [] (unsigned, unsigned &state) {
						return state = command_target::supported | command_target::enabled | command_target::visible, false;
					});
					oct.add_command(130, [] (unsigned) {}, true, [] (unsigned item, unsigned &) {
						return item < 2;
					});

					// ACT / ASSERT
					assert_equal(OLECMDF_SUPPORTED | OLECMDF_INVISIBLE | OLECMDF_DEFHIDEONCTXTMENU, getSingleCommandState(oct, c_guid1, 31));
				}


				test( SeveralCommandsCanBeProcessed )
				{
					// INIT
					ole_command_target_final oct(c_guid1);
					OLECMD cmd[3] = { 0 };

					oct.add_command(10, [] (unsigned) {}, false, [] (unsigned, unsigned &state) {
						return state = command_target::supported, true;
					});
					oct.add_command(31, [] (unsigned) {}, true, [] (unsigned item, unsigned &state) {
						return item < 17
							? state = command_target::supported | command_target::enabled | command_target::visible, true
							: false;
					});
					oct.add_command(130, [] (unsigned) {}, true, [] (unsigned item, unsigned &state) {
						return item < 2 ? state = command_target::supported | command_target::visible, true : false;
					});

					cmd[0].cmdID = 10;
					cmd[1].cmdID = 33;

					// ACT / ASSERT
					assert_equal(S_OK, oct.QueryStatus(&c_guid1, 2, cmd, NULL));

					// ASSERT
					assert_equal(OLECMDF_SUPPORTED | OLECMDF_INVISIBLE | OLECMDF_DEFHIDEONCTXTMENU, (int)cmd[0].cmdf);
					assert_equal(OLECMDF_SUPPORTED | OLECMDF_ENABLED, (int)cmd[1].cmdf);

					// INIT
					cmd[0].cmdID = 34;
					cmd[1].cmdID = 10;
					cmd[2].cmdID = 131;

					// ACT / ASSERT
					assert_equal(S_OK, oct.QueryStatus(&c_guid1, 3, cmd, NULL));

					// ASSERT
					assert_equal(OLECMDF_SUPPORTED | OLECMDF_ENABLED, (int)cmd[0].cmdf);
					assert_equal(OLECMDF_SUPPORTED | OLECMDF_INVISIBLE | OLECMDF_DEFHIDEONCTXTMENU, (int)cmd[1].cmdf);
					assert_equal(OLECMDF_SUPPORTED, (int)cmd[2].cmdf);
				}


				test( ExceptionsFromCommandAreAbsorbed )
				{
					// INIT
					ole_command_target_final oct(c_guid1);
					OLECMD cmd = { 1, 0 };

					oct.add_command(1, [] (unsigned) { throw 0; }, true, [] (unsigned, unsigned &) -> bool { throw 0; });

					// ACT / ASSERT
					assert_equal(E_UNEXPECTED, oct.QueryStatus(&c_guid1, 1, &cmd, NULL));
					assert_equal(E_UNEXPECTED, oct.Exec(&c_guid1, 1, OLECMDEXECOPT_DODEFAULT, NULL, NULL));
				}


				test( CommandIsExecutedBasedOnID )
				{
					// INIT
					ole_command_target_final oct(c_guid1);
					vector<int> exec_log;

					oct.add_command(10, [&] (unsigned) { exec_log.push_back(10); });
					oct.add_command(31, [&] (unsigned) { exec_log.push_back(31); });
					oct.add_command(130, [&] (unsigned) { exec_log.push_back(130); });

					// ACT
					assert_equal(S_OK, oct.Exec(&c_guid1, 10, OLECMDEXECOPT_DODEFAULT, NULL, NULL));

					// ACT / ASSERT
					int reference1[] = { 10, };

					assert_equal(reference1, exec_log);

					// ACT
					assert_equal(S_OK, oct.Exec(&c_guid1, 130, OLECMDEXECOPT_DODEFAULT, NULL, NULL));
					assert_equal(S_OK, oct.Exec(&c_guid1, 31, OLECMDEXECOPT_DODEFAULT, NULL, NULL));

					// ACT / ASSERT
					int reference2[] = { 10, 130, 31, };

					assert_equal(reference2, exec_log);
				}


				test( MissingCommandIsReported )
				{
					// INIT
					ole_command_target_final oct(c_guid1);

					oct.add_command(33, [&] (unsigned) { assert_is_true(false); });

					// ACT / ASSERT
					assert_equal(OLECMDERR_E_NOTSUPPORTED, oct.Exec(&c_guid1, 32, OLECMDEXECOPT_DODEFAULT, NULL, NULL));
					assert_equal(OLECMDERR_E_NOTSUPPORTED, oct.Exec(&c_guid1, 34, OLECMDEXECOPT_DODEFAULT, NULL, NULL));
				}


				test( GroupCommandIsSelectedByRangeForExecution )
				{
					// INIT
					ole_command_target_final oct(c_guid1);
					vector< pair<int, unsigned> > exec_log;

					oct.add_command(10, [&] (unsigned item) { exec_log.push_back(make_pair(10, item)); });
					oct.add_command(31, [&] (unsigned item) { exec_log.push_back(make_pair(31, item)); }, true);
					oct.add_command(130, [&] (unsigned item) { exec_log.push_back(make_pair(130, item)); }, true);

					// ACT / ASSERT
					oct.Exec(&c_guid1, 31, OLECMDEXECOPT_DODEFAULT, NULL, NULL);

					// ASSERT
					pair<int, unsigned> reference1[] = { make_pair(31, 0u), };

					assert_equal(reference1, exec_log);

					// ACT / ASSERT
					oct.Exec(&c_guid1, 32, OLECMDEXECOPT_DODEFAULT, NULL, NULL);

					// ASSERT
					pair<int, unsigned> reference2[] = { make_pair(31, 0u), make_pair(31, 1u), };

					assert_equal(reference2, exec_log);

					// ACT / ASSERT
					oct.Exec(&c_guid1, 47, OLECMDEXECOPT_DODEFAULT, NULL, NULL);

					// ASSERT
					pair<int, unsigned> reference3[] = { make_pair(31, 0u), make_pair(31, 1u), make_pair(31, 16u), };

					assert_equal(reference3, exec_log);

					// ACT / ASSERT
					oct.Exec(&c_guid1, 131, OLECMDEXECOPT_DODEFAULT, NULL, NULL);

					// ASSERT
					pair<int, unsigned> reference4[] = { make_pair(31, 0u), make_pair(31, 1u), make_pair(31, 16u), make_pair(130, 1u), };

					assert_equal(reference4, exec_log);
				}


				test( GetingTheCaptionFillsTheBufferProvidedWithZeroTermination )
				{
					// INIT
					ole_command_target_final oct(c_guid1);
					const size_t buffer_size = 50;
					wchar_t buffer[sizeof(OLECMDTEXT) + (buffer_size - 1) * sizeof(wchar_t)] = { 0 };
					OLECMDTEXT *cmdtext = reinterpret_cast<OLECMDTEXT *>(buffer);
					OLECMD cmd = { };

					cmdtext->cmdtextf = OLECMDTEXTF_NAME;
					cmdtext->cwBuf = 20;

					oct.add_command(10, [] (unsigned) { }, false, [] (unsigned, unsigned &) { return true; },
						[] (unsigned, string &caption) { return caption = "a short name", true; });
					oct.add_command(31, [] (unsigned) { }, true, [] (unsigned, unsigned &) { return true; },
						[] (unsigned item, string &caption) -> bool {

						if (item == 0)
							caption = "Lorem ipsum dolor sit amet, consectetur adipiscing elit";
						else if (item == 1)
							caption = "medium-sized strings are good for your health";
						else
							caption.clear();
						return true;
					});

					// ACT
					cmd.cmdID = 10;
					oct.QueryStatus(&c_guid1, 1, &cmd, cmdtext);

					// ASSERT
					assert_equal(L"a short name", wstring(cmdtext->rgwz));
					assert_equal(wcslen(L"a short name") + 1, cmdtext->cwActual);

					// ACT
					cmd.cmdID = 31;
					oct.QueryStatus(&c_guid1, 1, &cmd, cmdtext);

					// ASSERT
					assert_equal(L"Lorem ipsum dolor s", wstring(cmdtext->rgwz));
					assert_equal(wcslen(L"Lorem ipsum dolor sit amet, consectetur adipiscing elit") + 1, cmdtext->cwActual);

					// ACT
					cmdtext->cwBuf = 22;
					oct.QueryStatus(&c_guid1, 1, &cmd, cmdtext);

					// ASSERT
					assert_equal(L"Lorem ipsum dolor sit", wstring(cmdtext->rgwz));
					assert_equal(wcslen(L"Lorem ipsum dolor sit amet, consectetur adipiscing elit") + 1, cmdtext->cwActual);

					// ACT
					cmdtext->cwBuf = 46;
					cmd.cmdID = 32;
					oct.QueryStatus(&c_guid1, 1, &cmd, cmdtext);

					// ASSERT
					assert_equal(L"medium-sized strings are good for your health", wstring(cmdtext->rgwz));
					assert_equal(wcslen(L"medium-sized strings are good for your health") + 1, cmdtext->cwActual);
				}


				test( OnlyTheTextForTheFirstCommandIsReturned )
				{
					// INIT
					ole_command_target_final oct(c_guid1);
					const size_t buffer_size = 50;
					wchar_t buffer[sizeof(OLECMDTEXT) + (buffer_size - 1) * sizeof(wchar_t)] = { 0 };
					OLECMDTEXT *cmdtext = reinterpret_cast<OLECMDTEXT *>(buffer);
					OLECMD cmd[] = { { 31, }, { 10, }, };

					cmdtext->cmdtextf = OLECMDTEXTF_NAME;

					oct.add_command(10, [] (unsigned) { }, false, [] (unsigned, unsigned &) { return true; },
						[] (unsigned, string &caption) { return caption = "a short name", true; });
					oct.add_command(31, [] (unsigned) { }, true, [] (unsigned, unsigned &) { return true; },
						[] (unsigned item, string &caption) -> bool {

						if (item == 0)
							caption = "Lorem ipsum dolor sit amet, consectetur adipiscing elit";
						else
							caption.clear();
						return true;
					});

					// ACT
					cmdtext->cwBuf = 23;
					oct.QueryStatus(&c_guid1, _countof(cmd), cmd, cmdtext);

					// ASSERT
					assert_equal(L"Lorem ipsum dolor sit ", wstring(cmdtext->rgwz));
				}


				test( MissingDynamicTextSetsNameStringToZero )
				{
					// INIT
					ole_command_target_final oct(c_guid1);
					const size_t buffer_size = 50;
					wchar_t buffer[sizeof(OLECMDTEXT) + (buffer_size - 1) * sizeof(wchar_t)] = { 0 };
					OLECMDTEXT *cmdtext = reinterpret_cast<OLECMDTEXT *>(buffer);
					OLECMD cmd[] = { { 31, }, { 10, }, };

					oct.add_command(10, [] (unsigned) { }, false, [] (unsigned, unsigned &) { return true; },
						[] (unsigned, string &) { return false; });
					oct.add_command(31, [] (unsigned) { }, true);

					cmdtext->cmdtextf = OLECMDTEXTF_NAME;

					// ACT
					cmdtext->cwBuf = 23;
					cmdtext->cwActual = 1542231;
					oct.QueryStatus(&c_guid1, 1, &cmd[0], cmdtext);

					// ASSERT
					assert_equal(0u, cmdtext->cwActual);

					// ACT
					cmdtext->cwBuf = 23;
					cmdtext->cwActual = 1542231;
					oct.QueryStatus(&c_guid1, 1, &cmd[1], cmdtext);

					// ASSERT
					assert_equal(0u, cmdtext->cwActual);
				}

			end_test_suite
		}
	}
}
