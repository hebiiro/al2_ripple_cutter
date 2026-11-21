#pragma once

namespace apn::ripple_cutter
{
	//
	// このクラスはフック処理を担当します。
	//
	inline struct hook_t : my::Window
	{
		//
		// 指定されたテキストのメニュー項目のインデックスを返します。
		//
		inline static int find_menu_item(HMENU menu, const std::wstring& lhs)
		{
			// メニュー項目の数を取得します。
			auto nb_menu_items = ::GetMenuItemCount(menu);

			// メニュー項目を走査します。
			for (auto i = 0; i < nb_menu_items; i++)
			{
				// メニュー項目のテキストを取得します。
				auto rhs = my::get_menu_item_text(menu, i, MF_BYPOSITION);

				// 指定されたテキストの場合は、そのインデックスを返します。
				if (lhs == rhs) return i;
			}

			// 見つからなかった場合は-1を返します。
			return -1;
		}

		//
		// 「メニュー➡編集➡オブジェクトを切り取り」のIDを返します。
		//
		inline static UINT get_cut_id()
		{
			// 「メニュー➡編集➡オブジェクトを切り取り」のIDです。
			static auto cut_id = 0;

			// 「メニュー➡編集➡オブジェクトを切り取り」のIDが無効の場合は
			if (!cut_id)
			{
				// ホストウィンドウのメニューを取得します。
				auto menu = ::GetMenu(hive.aviutl2_window);

				// 「編集」のインデックスを取得します。
				auto edit_index = find_menu_item(menu, L"編集");

				// 「編集」メニューを取得します。
				auto edit_menu = ::GetSubMenu(menu, edit_index);

				// 「オブジェクトを切り取り」のインデックスを取得します。
				auto cut_index = find_menu_item(edit_menu, L"オブジェクトを切り取り\tCtrl+X");

				// 「オブジェクトを切り取り」のIDを取得します。
				cut_id = ::GetMenuItemID(edit_menu, cut_index);
			}

			// 「メニュー➡編集➡オブジェクトを切り取り」のIDを返します。
			return cut_id;
		}

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

				// ホストウィンドウではない場合は何もしません。
				if (hwnd != hive.aviutl2_window)
					return orig_proc(menu, flags, x, y, reserved, hwnd, rc);

				// フラグがセットされていない場合は何もしません。
				if (!hive.flag_ignore_select_range)
					return orig_proc(menu, flags, x, y, reserved, hwnd, rc);

				// ポップアップメニューを表示して、結果を取得します。
				auto result = orig_proc(menu, flags, x, y, reserved, hwnd, rc);
				if (!result) return result;

				// メニュー項目のテキストを取得します。
				auto menu_item_text = my::get_menu_item_text(menu, result, MF_BYCOMMAND);

				// タイムラインで「切り取り」が実行された場合は
				if (menu_item_text == L"切り取り")
				{
					// 「メニュー➡編集➡オブジェクトを切り取り」のIDを取得できた場合は
					if (auto cut_id = get_cut_id())
					{
						// 「メニュー➡編集➡オブジェクトを切り取り」を実行します。
						::SendMessage(hwnd, WM_COMMAND, cut_id, 0);

						// コマンドを実行した後なので、
						// ホストアプリが何もしないように0を返します。
						return 0;
					}
				}

				// それ以外の場合は結果を返します。
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
					// 編集ハンドルが無効の場合は何もしません。
					if (!editor.handle) break;

					// フラグがセットされていない場合は何もしません。
					if (!hive.flag_ignore_select_range) break;

					// 「メニュー➡編集➡オブジェクトを切り取り」の場合は
					if (w_param && w_param == get_cut_id())
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
