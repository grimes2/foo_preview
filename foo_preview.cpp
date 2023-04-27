#define _WIN32_WINNT _WIN32_WINNT_WIN7
#define WINVER _WIN32_WINNT_WIN7

#include <random>
#include <foobar2000/SDK/foobar2000.h>

static constexpr const char* component_name = "Preview";

DECLARE_COMPONENT_VERSION(
	component_name,
	"1.15",
	"grimes\n\n"
	"Build: " __TIME__ ", " __DATE__
);

VALIDATE_COMPONENT_FILENAME("foo_preview.dll");

#define ID_TIMER3 1012
#define ID_TIMER4 1013

UINT_PTR ptr3 = 0;
UINT_PTR ptr4 = 0;
pfc::string8 preview_time;
pfc::string8 total_time;
pfc::string8 preview_start;
pfc::string8 preview_start_percent;
pfc::string8 bypass_track_length;
double preview_start_percent2;
double preview_start2;
double total_time2;
double pause_time;
double pause_preview_time;
double pause_remaining;
double pause_remaining2;
double preview_time2;
double bypass_track_length2;
bool menu_preview_enabled = false;
bool random_enabled;

// {90073616-61A0-473D-A172-703924FEB0A1}
static const GUID guid_cfg_branch =
{ 0x90073616, 0x61a0, 0x473d, { 0xa1, 0x72, 0x70, 0x39, 0x24, 0xfe, 0xb0, 0xa1 } };
static advconfig_branch_factory cfg_branch("Preview", guid_cfg_branch, advconfig_entry::guid_branch_playback, 0);

// {43009833-D765-4515-9C59-7EED782B622B}
static const GUID guid_cfg_preview =
{ 0x43009833, 0xd765, 0x4515, { 0x9c, 0x59, 0x7e, 0xed, 0x78, 0x2b, 0x62, 0x2b } };
advconfig_string_factory cfg_preview("Preview length (s)", guid_cfg_preview, guid_cfg_branch, 0, "5");

// {547E2382-09CB-4EC3-B325-C0FFBFF3BDA6}
static const GUID guid_cfg_previewstart =
{ 0x547e2382, 0x9cb, 0x4ec3, { 0xb3, 0x25, 0xc0, 0xff, 0xbf, 0xf3, 0xbd, 0xa6 } };
advconfig_string_factory cfg_previewstart("Start time (s)", guid_cfg_previewstart, guid_cfg_branch, 0, "0");

// {7C8B7E19-5BA9-4391-9299-20CCC620F4E7}
static const GUID guid_cfg_percent_enabled =
{ 0x7c8b7e19, 0x5ba9, 0x4391, { 0x92, 0x99, 0x20, 0xcc, 0xc6, 0x20, 0xf4, 0xe7 } };
advconfig_checkbox_factory cfg_percent_enabled("Start time in %", guid_cfg_percent_enabled, guid_cfg_branch, 0, false);

// {E0B5AA2A-189E-4F1C-B895-6720B22FA4EA}
static const GUID guid_cfg_random_enabled =
{ 0xe0b5aa2a, 0x189e, 0x4f1c, { 0xb8, 0x95, 0x67, 0x20, 0xb2, 0x2f, 0xa4, 0xea } };
advconfig_checkbox_factory cfg_random_enabled("Random start time", guid_cfg_random_enabled, guid_cfg_branch, 0, false);

// {1D5D5C64-18E6-4FF5-B5DE-50CEDA4E975D}
static const GUID guid_cfg_previewstartpercent =
{ 0x1d5d5c64, 0x18e6, 0x4ff5, { 0xb5, 0xde, 0x50, 0xce, 0xda, 0x4e, 0x97, 0x5d } };
advconfig_string_factory cfg_previewstartpercent("Start time (%)", guid_cfg_previewstartpercent, guid_cfg_branch, 0, "50");

// {91876C5A-7200-4FCC-BAAE-1B77F1D48881}
static const GUID guid_cfg_bypass_track_length =
{ 0x91876c5a, 0x7200, 0x4fcc, { 0xba, 0xae, 0x1b, 0x77, 0xf1, 0xd4, 0x88, 0x81 } };
advconfig_string_factory cfg_bypass_track_length("Bypass track length (s)", guid_cfg_bypass_track_length, guid_cfg_branch, 0, "5");

VOID CALLBACK PreviewTimer(
	HWND,        // handle to window for timer messages
	UINT,     // WM_TIMER message
	UINT idEvent1,     // timer identifier
	DWORD)     // current system time
{
	if (menu_preview_enabled)
	{
		KillTimer(NULL, idEvent1);
		static_api_ptr_t<playback_control>()->start(playback_control::track_command_next, false);
	}
	else
	{
		KillTimer(NULL, idEvent1);
	}
}

VOID CALLBACK PreviewTimer2(
	HWND,        // handle to window for timer messages
	UINT,     // WM_TIMER message
	UINT idEvent2,     // timer identifier
	DWORD)     // current system time
{
	if (menu_preview_enabled)
	{
		static_api_ptr_t<playback_control>()->playback_seek(preview_start2);
		KillTimer(NULL, idEvent2);
	}
	else
	{
		KillTimer(NULL, idEvent2);
	}
}

class mainmenu_commands_preview : public mainmenu_commands
{

public:

	// Return the number of commands we provide.
	virtual t_uint32 get_command_count()
	{
		return 1;
	}

	// All commands are identified by a GUID.
	virtual GUID get_command(t_uint32 p_index)
	{

		// {1F2F7256-0C85-4507-BC76-7BEDA59C7556}
		static const GUID guid_main_preview =
		{ 0x1f2f7256, 0xc85, 0x4507, { 0xbc, 0x76, 0x7b, 0xed, 0xa5, 0x9c, 0x75, 0x56 } };
		if (p_index == 0)
			return guid_main_preview;
		return pfc::guid_null;
	}

	// Set p_out to the name of the n-th command.
	// This name is used to identify the command and determines
	// the default position of the command in the menu.
	virtual void get_name(t_uint32 p_index, pfc::string_base& p_out)
	{
		if (p_index == 0)
			p_out = "Preview";
	}

	// Set p_out to the description for the n-th command.
	virtual bool get_description(t_uint32 p_index, pfc::string_base& p_out)
	{
		if (p_index == 0)
			p_out = "Plays first seconds of consecutive tracks.";
		else
			return false;
		return true;
	}

	// Every set of commands needs to declare which group it belongs to.
	virtual GUID get_parent()
	{
		return mainmenu_groups::playback;
	}

	// Execute n-th command.
	// p_callback is reserved for future use.
	virtual void execute(t_uint32 p_index, service_ptr_t<service_base> p_callback)
	{
		if (p_index == 0)
		{
			menu_preview_enabled = !menu_preview_enabled;
			if (menu_preview_enabled)
			{
				cfg_preview.get(preview_time);
				preview_time2 = atoi(preview_time);
				if (preview_time2 > 30)
				{
					FB2K_console_formatter() << "[Warning] Preview length: " << preview_time2 << "s";
				}
				else
				{
					FB2K_console_formatter() << "Preview length: " << preview_time2 << "s";
				}
				static_api_ptr_t<playback_control>()->start(playback_control::track_command_play, false);
			}
		}
	}

	// The standard version of this command does not support checked or disabled
	// commands, so we use our own version.
	virtual bool get_display(t_uint32 p_index, pfc::string_base& p_text, t_uint32& p_flags)
	{
		if (p_index == 0) {
			if (menu_preview_enabled)
			{
				p_flags |= flag_checked;
			}
			else {
				p_flags = 0;
			}
			get_name(p_index, p_text);
		}
		return true;
	}
	virtual t_uint32 get_sort_priority()
	{
		return 0x80000000;
	}
	bool is_checked(t_uint32 p_index)
	{
		if (p_index == 0)
			return menu_preview_enabled;
	}
};

static mainmenu_commands_factory_t<mainmenu_commands_preview> g_mainmenu_commands_preview;

class play_callback_preview : public play_callback_static
{
public:
	unsigned get_flags() { return flag_on_playback_stop | flag_on_playback_pause | flag_on_playback_new_track; }
	virtual void on_playback_seek(double) {}
	virtual void on_playback_new_track(metadb_handle_ptr p_track)
	{
		if (menu_preview_enabled)
		{
			cfg_preview.get(preview_time);
			preview_time2 = atoi(preview_time);
			cfg_bypass_track_length.get(bypass_track_length);
			bypass_track_length2 = atoi(bypass_track_length);
			KillTimer(NULL, ptr3);
			titleformat_object::ptr titleformat;
			titleformat_compiler::get()->compile_safe_ex(titleformat, "%length_seconds%", "<ERROR>");
			p_track->format_title(nullptr, total_time, titleformat, nullptr);
			total_time2 = atoi(total_time);
			if (bypass_track_length2 < total_time2) {
				if (cfg_percent_enabled)
				{
					cfg_previewstartpercent.get(preview_start_percent);
					preview_start_percent2 = atoi(preview_start_percent);
					preview_start2 = total_time2 * preview_start_percent2 / 100;
				}
				else if (cfg_random_enabled)
				{
					std::random_device rd; // obtain a random number from hardware
					std::mt19937 gen(rd()); // seed the generator
					std::uniform_int_distribution<> distr(0, (int)total_time2 - (int)preview_time2); // define the range
					preview_start2 = distr(gen);
					FB2K_console_formatter() << "Random start: " << preview_start2 << "s";
				}
				else
				{
					cfg_previewstart.get(preview_start);
					preview_start2 = atoi(preview_start);
					if (preview_start2 > total_time2 - preview_time2) {
						preview_start2 = total_time2 - preview_time2;
					}
				}
				ptr4 = SetTimer(NULL, ID_TIMER4, 0, (TIMERPROC)PreviewTimer2);
				ptr3 = SetTimer(NULL, ID_TIMER3, (UINT)preview_time2 * 1000, (TIMERPROC)PreviewTimer);
			}
		}
	}
	virtual void on_playback_stop(play_control::t_stop_reason p_reason)
	{
		if (p_reason == 0)
		{
			KillTimer(NULL, ptr3);
			KillTimer(NULL, ptr4);
			menu_preview_enabled = false;
		}
	}
	virtual void on_playback_pause(bool paused) {
		if (menu_preview_enabled) {
			if (bypass_track_length2 < total_time2) {

				if (paused) {
					cfg_preview.get(preview_time);
					pause_preview_time = atoi(preview_time);
					pause_time = static_api_ptr_t<playback_control>()->playback_get_position();
					pause_remaining = pause_preview_time + preview_start2 - pause_time;
					pause_remaining2 = pause_remaining * 1000;
					KillTimer(NULL, ptr3);
				}
				else {
					KillTimer(NULL, ptr3);
					ptr3 = SetTimer(NULL, ID_TIMER3, (UINT)pause_remaining2, (TIMERPROC)PreviewTimer);
				}
			}
		}
	}
	virtual void on_playback_starting(play_control::t_track_command, bool) {}
	virtual void on_playback_edited(metadb_handle_ptr) {}
	virtual void on_playback_dynamic_info(const file_info&) {}
	virtual void on_playback_dynamic_info_track(const file_info&) {}
	virtual void on_playback_time(double) {}
	virtual void on_volume_change(float) {}
};
static play_callback_static_factory_t<play_callback_preview> g_play_callback_preview;