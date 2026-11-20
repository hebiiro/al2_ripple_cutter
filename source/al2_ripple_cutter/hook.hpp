#pragma once

namespace apn::ripple_cutter
{
	//
	// このクラスはフック処理を担当します。
	//
	inline struct hook_t : my::Window
	{
		//
		// フックの初期化処理を実行します。
		//
		BOOL init()
		{
			MY_TRACE_FUNC("");

			// ホストウィンドウをサブクラス化します。
			if (!__super::subclass(hive.aviutl2_window)) return FALSE;

			// APIフックを開始します。
			{
				my::hook::detours detours;
				my::hook::attach(TrackPopupMenu);
			}

			return TRUE;
		}

		//
		// フックの後始末処理を実行します。
		//
		BOOL exit()
		{
			MY_TRACE_FUNC("");

			// APIフックを終了します。
			{
				my::hook::detours detours;
				my::hook::detach(TrackPopupMenu);
			}

			// ホストウィンドウをのサブクラス化を解除します。
			__super::unsubclass();

			return TRUE;
		}

		//
		// 編集情報です。
		//
		EDIT_INFO edit_info = {};

		//
		// このクラスは::TrackPopupMenu()をフックします。
		//
		struct {
			inline static BOOL WINAPI hook_proc(HMENU menu,
				UINT flags, int x, int y, int reserved, HWND hwnd, LPCRECT rc)
			{
				MY_TRACE_FUNC("{/hex}, {/hex}, {/}, {/}, {/hex}", menu, flags, x, y, hwnd);

				auto result = orig_proc(menu, flags, x, y, reserved, hwnd, rc);
				MY_TRACE_HEX(result);

				// フラグがセットされていない場合は何もしません。
				if (!hive.flag_ignore_select_range) return result;

				// タイムラインで「切り取り」が実行された場合は
				if (hwnd == hive.aviutl2_window && result == 40000)
				{
					// 「メニュー➡編集➡オブジェクトを切り取り」を実行します。
					::SendMessage(hwnd, WM_COMMAND, 40234, 0);

					// コマンドを実行した後なので、
					// ホストアプリが何もしないように0を返します。
					return 0;
				}

				return result;
			}
			inline static decltype(&hook_proc) orig_proc = ::TrackPopupMenu;
		} TrackPopupMenu;

		//
		// ホストウィンドウのウィンドウプロシージャです。
		//
		virtual LRESULT on_wnd_proc(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param) override
		{
			switch (message)
			{
			case WM_COMMAND:
				{
					// フラグがセットされていない場合は何もしません。
					if (!hive.flag_ignore_select_range) break;

					// 「メニュー➡編集➡オブジェクトを切り取り」の場合は
					if (hive.flag_ignore_select_range && w_param == 40234)
					{
						// shiftキーが押されている場合は何もしません。
						if (::GetKeyState(VK_SHIFT) < 0) break;

						// 編集セクションを開始します。
						editor.handle->call_edit_section([](EDIT_SECTION* edit)
						{
							// 編集情報を取得しておきます。
							hook.edit_info = *edit->info;

							// 一時的にフレーム範囲選択を解除します。
							edit->set_select_range(-1, -1);
						});

						// スコープ終了時に実行します。
						my::scope_exit scope_exit([]()
						{
							// フレーム範囲選択が有効の場合は
							if (hook.edit_info.select_range_start != -1 &&
								hook.edit_info.select_range_end != -1)
							{
								// 編集セクションを開始します。
								editor.handle->call_edit_section([](EDIT_SECTION* edit)
								{
									// フレーム範囲選択を復元します。
									edit->set_select_range(
										hook.edit_info.select_range_start,
										hook.edit_info.select_range_end);
								});
							}
						});

						// デフォルト処理を実行します。
						return __super::on_wnd_proc(hwnd, message, w_param, l_param);
					}

					break;
				}
			}

			return __super::on_wnd_proc(hwnd, message, w_param, l_param);
		}
	} hook;
}
