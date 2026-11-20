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

		//
		// ウィンドウプロシージャです。
		//
		virtual LRESULT on_wnd_proc(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param) override
		{
			switch (message)
			{
			case WM_CONTEXTMENU:
				{
					// ポップアップメニューの項目IDです。
					constexpr uint32_t k_flag_ignore_select_range = 1000;

					// ポップアップメニューを作成します。
					my::menu::unique_ptr menu(::CreatePopupMenu());

					// ポップアップメニューに項目を追加します。
					::AppendMenu(menu.get(), MF_STRING, k_flag_ignore_select_range, _T("フレーム範囲選択を無視"));

					// 変数の状態に合わせて、ポップアップメニューの項目にチェックマークを入れます。
					if (hive.flag_ignore_select_range) ::CheckMenuItem(menu.get(), k_flag_ignore_select_range, MF_CHECKED);

					// ポップアップメニューを表示します。
					auto point = my::get_cursor_pos();
					auto id = ::TrackPopupMenuEx(menu.get(),
						TPM_NONOTIFY | TPM_RETURNCMD, point.x, point.y, hwnd, nullptr);

					switch (id)
					{
					case k_flag_ignore_select_range:
						{
							// フレーム範囲選択を無視するフラグを反転します。
							hive.flag_ignore_select_range = !hive.flag_ignore_select_range;

							break;
						}
					}

					return 0;
				}
			}

			return __super::on_wnd_proc(hwnd, message, w_param, l_param);
		}
	} main_window;
}
