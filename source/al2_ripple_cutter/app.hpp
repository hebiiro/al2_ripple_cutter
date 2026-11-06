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

			// レイヤーメニューとオブジェクトメニューを登録します。
			{
				struct menu_item_t {
					LPCWSTR name; void (*func)(EDIT_SECTION* edit);
				} menu_items[] = {
					{ L"リップルカット\\オブジェクトの開始位置からカーソル位置までカット", [](EDIT_SECTION* edit) { app_impl.cut_from_start_to_cursor(edit); }, },
					{ L"リップルカット\\カーソル位置からオブジェクトの終了位置までカット", [](EDIT_SECTION* edit) { app_impl.cut_from_cursor_to_end(edit); }, },
					{ L"リップルカット\\TL全体を選択オブジェクト範囲でカット", [](EDIT_SECTION* edit) { app_impl.cut_timeline_with_range(edit); }, },
					{ L"リップルカット\\TL全体を選択オブジェクト毎にカット", [](EDIT_SECTION* edit) { app_impl.cut_timeline_with_object(edit); }, },
				};
				for (const auto menu_item : menu_items)
				{
					host->register_layer_menu(menu_item.name, menu_item.func);
					host->register_object_menu(menu_item.name, menu_item.func);
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
	} app_impl;
}
