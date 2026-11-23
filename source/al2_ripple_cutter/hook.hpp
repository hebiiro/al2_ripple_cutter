#pragma once

namespace my::hook
{
	template <typename T>
	void detach_import(T& hooker, HMODULE module, LPCSTR func_name)
	{
		hook_import(module, func_name, hooker.orig_proc);
	}
}

namespace apn::ripple_cutter
{
	//
	// このクラスはフック処理を担当します。
	//
	inline struct hook_t
	{
		//
		// フックの初期化処理を実行します。
		//
		BOOL init()
		{
			MY_TRACE_FUNC("");

			// APIフックを開始します。
			{
				auto host = ::GetModuleHandle(nullptr);

				my::hook::detours detours;
				my::hook::attach_import(TrackPopupMenu, host, "TrackPopupMenu");
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
				auto host = ::GetModuleHandle(nullptr);

				my::hook::detours detours;
				my::hook::detach_import(TrackPopupMenu, host, "TrackPopupMenu");
			}

			return TRUE;
		}

		//
		// このクラスは::TrackPopupMenu()をフックします。
		//
		struct {
			inline static BOOL WINAPI hook_proc(HMENU menu,
				UINT flags, int x, int y, int reserved, HWND hwnd, LPCRECT rc)
			{
				MY_TRACE_FUNC("{/hex}, {/hex}, {/}, {/}, {/hex}", menu, flags, x, y, hwnd);

				// コンフィグハンドルが無効の場合は何もしません。
				if (!tr.config)
					return orig_proc(menu, flags, x, y, reserved, hwnd, rc);

				// 編集ハンドルが無効の場合は何もしません。
				if (!editor.handle)
					return orig_proc(menu, flags, x, y, reserved, hwnd, rc);

				// ホストウィンドウではない場合は何もしません。
				if (hwnd != editor.handle->get_host_app_window())
					return orig_proc(menu, flags, x, y, reserved, hwnd, rc);

				// ポップアップメニューを表示して、結果を取得します。
				auto result = orig_proc(menu, flags, x, y, reserved, hwnd, rc);
				if (!result) return result;

				// メニュー項目のテキストを取得します。
				auto menu_item_text = my::get_menu_item_text(menu, result, MF_BYCOMMAND);

				// 「フレーム範囲選択を無視して切り取り」のテキストです。
				auto cut_string = tr(L"フレーム範囲選択を無視して切り取り");

				// 「フレーム範囲選択を無視して切り取り」ではない場合は何もしません。
				if (menu_item_text != cut_string) return result;

				// 「フレーム範囲選択を無視して切り取り」を実行します。
				app->cut_without_select_range();

				// コマンドを実行した後なので、
				// ホストアプリが何もしないように0を返します。
				return 0;
			}
			inline static decltype(&hook_proc) orig_proc = ::TrackPopupMenu;
		} TrackPopupMenu;
	} hook;
}
