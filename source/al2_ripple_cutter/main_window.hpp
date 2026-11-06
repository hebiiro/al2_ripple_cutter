#pragma once

namespace apn::ripple_cutter
{
	//
	// このクラスはメインウィンドウです。
	//
	inline struct main_window_t : my::layout::Window<config_dialog_base_t>
	{
		//
		// コンフィグでコントロールを更新します。
		//
		virtual BOOL on_to_ui() override
		{
			MY_TRACE_FUNC("");

			return TRUE;
		}

		//
		// コントロールでコンフィグを更新します。
		//
		virtual BOOL on_from_ui() override
		{
			MY_TRACE_FUNC("");

			return TRUE;
		}

		//
		// WM_INITDIALOGを処理します。
		//
		virtual void on_init_dialog() override
		{
			MY_TRACE_FUNC("");
		}

		//
		// WM_COMMANDを処理します。
		//
		virtual void on_command(UINT code, UINT control_id, HWND control) override
		{
			switch (control_id)
			{
			// ボタン
			case idc_cut_from_start_to_cursor: app->cut_from_start_to_cursor(); break;
			case idc_cut_from_cursor_to_end: app->cut_from_cursor_to_end(); break;
			case idc_cut_timeline_with_range: app->cut_timeline_with_range(); break;
			case idc_cut_timeline_with_object: app->cut_timeline_with_object(); break;
			}
		}
	} main_window;
}
