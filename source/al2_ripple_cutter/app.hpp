#pragma once

namespace apn::ripple_cutter
{
	//
	// このクラスはアプリケーションです。
	//
	inline struct app_t : app_base_t, app_interface_t
	{
		//
		// プラグインの初期化処理を実行します。
		//
		virtual BOOL on_initialize_plugin(DWORD version) override
		{
			MY_TRACE_FUNC("");

			// 各種開始処理を実行します。
			debug.init();
			config_io.init();
			main_window.init(idd_main);

			// コンフィグをファイルから読み込みます。
			config_io.read();

			return TRUE;
		}

		//
		// プラグインの後始末処理を実行します。
		//
		virtual BOOL on_uninitialize_plugin() override
		{
			MY_TRACE_FUNC("");

			// 各種終了処理を実行します。
			main_window.exit();
			config_io.exit();
			debug.exit();

			return TRUE;
		}

		//
		// プラグインを登録します。
		//
		virtual BOOL on_register_plugin(HOST_APP_TABLE* host) override
		{
			MY_TRACE_FUNC("");

			// レイヤー・オブジェクト・編集メニューに登録します。
			{
				struct menu_item_t {
					LPCWSTR name; void (*func)(EDIT_SECTION* edit);
				} menu_items[] = {
					{ L"リップルカット\\オブジェクトの開始位置からカーソル位置までカット", [](EDIT_SECTION* edit) { app_impl.cut_from_start_to_cursor(edit); }, },
					{ L"リップルカット\\カーソル位置からオブジェクトの終了位置までカット", [](EDIT_SECTION* edit) { app_impl.cut_from_cursor_to_end(edit); }, },
					{ L"リップルカット\\TL全体を選択オブジェクト範囲でカット", [](EDIT_SECTION* edit) { app_impl.cut_timeline_with_range(edit); }, },
					{ L"リップルカット\\TL全体を選択オブジェクト毎にカット", [](EDIT_SECTION* edit) { app_impl.cut_timeline_with_object(edit); }, },
					{ L"リップルカット\\フレーム範囲選択を無視して切り取り", [](EDIT_SECTION* edit) { ::PostMessage(main_window, WM_COMMAND, idc_cut_without_select_range, 0); }, },
				};
				for (const auto menu_item : menu_items)
				{
					host->register_layer_menu(menu_item.name, menu_item.func);
					host->register_object_menu(menu_item.name, menu_item.func);
					host->register_edit_menu(menu_item.name, menu_item.func);

				}
			}

			// メインウィンドウを登録します。
			host->register_window_client(version_base->get_name().c_str(), main_window);

			return TRUE;
		}

		//
		// フォーカスオブジェクトの開始位置からカーソル位置までをカットします。
		//
		void cut_from_start_to_cursor(EDIT_SECTION* edit)
		{
			MY_TRACE_FUNC("");

			// エディタです。
			editor_t editor(edit);

			// カーソルのフレーム位置を取得します。
			auto cursor_frame = edit->info->frame;

			// 対象オブジェクトを取得します。
			auto object = editor.get_single_target_object();

			// 対象オブジェクトが無効の場合は何もしません。
			if (!object) return;

			// 対象オブジェクトの位置を取得します。
			auto object_layer_frame = edit->get_object_layer_frame(object);

			// カットする範囲のコレクションです。
			std::vector<range_t> cut_ranges;

			// カットする範囲を算出します。
			cut_ranges.emplace_back(object_layer_frame.start, cursor_frame, editor.get_memo(object));

			// 指定された範囲で全てのオブジェクトまたは選択オブジェクトをカットします。
			editor.cut_all_or_selected_objects(cut_ranges, TRUE);
		}

		//
		// カーソル位置からフォーカスオブジェクトの終了位置までをカットします。
		//
		void cut_from_cursor_to_end(EDIT_SECTION* edit)
		{
			MY_TRACE_FUNC("");

			// エディタです。
			editor_t editor(edit);

			// カーソルのフレーム位置を取得します。
			auto cursor_frame = edit->info->frame;

			// 対象オブジェクトを取得します。
			auto object = editor.get_single_target_object();

			// 対象オブジェクトが無効の場合は何もしません。
			if (!object) return;

			// 対象オブジェクトの位置を取得します。
			auto object_layer_frame = edit->get_object_layer_frame(object);

			// カットする範囲のコレクションです。
			std::vector<range_t> cut_ranges;

			// カットする範囲を算出します。
			cut_ranges.emplace_back(cursor_frame, object_layer_frame.end + 1, editor.get_memo(object));

			// 指定された範囲で全てのオブジェクトまたは選択オブジェクトをカットします。
			editor.cut_all_or_selected_objects(cut_ranges, TRUE);
		}

		//
		// 選択オブジェクト範囲でタイムライン全体をカットします。
		//
		void cut_timeline_with_range(EDIT_SECTION* edit)
		{
			MY_TRACE_FUNC("");

			// エディタです。
			editor_t editor(edit);

			// カットする範囲です。
			range_t cut_range = { INT_MAX };

			// フォーカスオブジェクトまたは選択オブジェクトを走査します。
			editor.edit_focus_or_selected_objects([&](OBJECT_HANDLE object)
			{
				// オブジェクトの位置を取得します。
				auto object_layer_frame = edit->get_object_layer_frame(object);

				// オブジェクトの位置でカットする範囲を拡大します。
				cut_range.start = std::min(cut_range.start, object_layer_frame.start);
				cut_range.end = std::max(cut_range.end, object_layer_frame.end + 1);
				cut_range.memo = editor.get_memo(object);
			});

			// カットする範囲が無効の場合は何もしません。
			if (cut_range.start < 0 || cut_range.start >= cut_range.end) return;

			// カットする範囲のコレクションです。
			std::vector<range_t> cut_ranges;

			// オブジェクトの位置をカットする範囲に追加します。
			cut_ranges.emplace_back(cut_range);

			// 指定された範囲で全てのオブジェクトをカットします。
			editor.cut_all_or_selected_objects(cut_ranges, FALSE);
		}

		//
		// 選択オブジェクトでタイムライン全体をカットします。
		//
		void cut_timeline_with_object(EDIT_SECTION* edit)
		{
			MY_TRACE_FUNC("");

			// エディタです。
			editor_t editor(edit);

			// カットする範囲のコレクションです。
			std::vector<range_t> cut_ranges;

			// フォーカスオブジェクトまたは選択オブジェクトを走査します。
			editor.edit_focus_or_selected_objects([&](OBJECT_HANDLE object)
			{
				// オブジェクトの位置を取得します。
				auto object_layer_frame = edit->get_object_layer_frame(object);

				// オブジェクトの位置をカットする範囲に追加します。
				cut_ranges.emplace_back(object_layer_frame.start, object_layer_frame.end + 1, editor.get_memo(object));
			});

			// 指定された範囲で全てのオブジェクトをカットします。
			editor.cut_all_or_selected_objects(cut_ranges, FALSE);
		}

		//
		// フォーカスオブジェクトの開始位置からカーソル位置までをカットします。
		//
		virtual void cut_from_start_to_cursor() override
		{
			MY_TRACE_FUNC("");

			// 編集セクションを開始します。
			editor.handle->call_edit_section([](EDIT_SECTION* edit) { app_impl.cut_from_start_to_cursor(edit); });
		}

		//
		// カーソル位置からフォーカスオブジェクトの終了位置までをカットします。
		//
		virtual void cut_from_cursor_to_end() override
		{
			MY_TRACE_FUNC("");

			// 編集セクションを開始します。
			editor.handle->call_edit_section([](EDIT_SECTION* edit) { app_impl.cut_from_cursor_to_end(edit); });
		}

		//
		// 選択オブジェクト範囲でタイムライン全体をカットします。
		//
		virtual void cut_timeline_with_range() override
		{
			MY_TRACE_FUNC("");

			// 編集セクションを開始します。
			editor.handle->call_edit_section([](EDIT_SECTION* edit) { app_impl.cut_timeline_with_range(edit); });
		}

		//
		// 選択オブジェクトでタイムライン全体をカットします。
		//
		virtual void cut_timeline_with_object() override
		{
			MY_TRACE_FUNC("");

			// 編集セクションを開始します。
			editor.handle->call_edit_section([](EDIT_SECTION* edit) { app_impl.cut_timeline_with_object(edit); });
		}

		//
		// フレーム範囲選択を無視して「メニュー➡編集➡オブジェクトを切り取り」を実行します。
		//
		virtual void cut_without_select_range() override
		{
			MY_TRACE_FUNC("");

			//
			// この関数は、指定されたテキストのメニュー項目のインデックスを返します。
			//
			constexpr auto find_menu_item = [](HMENU menu, const std::wstring& lhs) -> int
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
			};

			//
			// この関数は、「メニュー➡編集➡オブジェクトを切り取り」のIDを返します。
			//
			constexpr auto get_cut_id = [](HWND hwnd) -> UINT
			{
				// ホストウィンドウのメニューを取得します。
				auto menu = ::GetMenu(hwnd);

				// 「編集」のテキストです。
				auto edit_string = tr(L"Menu", L"編集");

				// 「編集」のインデックスを取得します。
				auto edit_index = find_menu_item(menu, edit_string);

				// 「編集」メニューを取得します。
				auto edit_menu = ::GetSubMenu(menu, edit_index);

				// 「オブジェクトを切り取り」のテキストです。
				auto cut_string = my::format(L"{/}{/}",
					tr(L"Menu", L"オブジェクトを切り取り"), L"\tCtrl+X");

				// 「オブジェクトを切り取り」のインデックスを取得します。
				auto cut_index = find_menu_item(edit_menu, cut_string);

				// 「オブジェクトを切り取り」のIDを取得して返します。
				return ::GetMenuItemID(edit_menu, cut_index);
			};

			// コンフィグハンドルが無効の場合は何もしません。
			if (!tr.config) return;

			// 編集ハンドルが無効の場合は何もしません。
			if (!editor.handle) return;

			// ホストウィンドウを取得します。
			auto hwnd = editor.handle->get_host_app_window();

			// 「メニュー➡編集➡オブジェクトを切り取り」のIDを取得します。
			auto cut_id = get_cut_id(hwnd);
			if (!cut_id) return;

			// 編集情報です。
			EDIT_INFO edit_info = {
				.select_range_start = -1,
				.select_range_end = -1,
			};

			// 編集セクションを開始します。
			editor.handle->call_edit_section_param(
				&edit_info, [](void* param, EDIT_SECTION* edit)
			{
				// 編集情報を取得しておきます。
				*(EDIT_INFO*)param = *edit->info;

				// 一時的にフレーム範囲選択を解除します。
				edit->set_select_range(-1, -1);
			});

			// 「メニュー➡編集➡オブジェクトを切り取り」を実行します。
			::SendMessage(hwnd, WM_COMMAND, cut_id, 0);

			// フレーム範囲選択が有効の場合は
			if (edit_info.select_range_start != -1 &&
				edit_info.select_range_end != -1)
			{
				// 編集セクションを開始します。
				editor.handle->call_edit_section_param(
					&edit_info, [](void* param, EDIT_SECTION* edit)
				{
					// 編集情報を取得します。
					auto& edit_info = *(EDIT_INFO*)param;

					// フレーム範囲選択を復元します。
					edit->set_select_range(
						edit_info.select_range_start,
						edit_info.select_range_end);
				});
			}

			// そのまま続けてアンドゥなどのキーボード操作ができるように、
			// ホストウィンドウにフォーカスを移動します。
			::SetFocus(hwnd);
		}
	} app_impl;
}
