#pragma once

namespace apn::ripple_cutter
{
	template <typename T>
	struct reverse_iteration_helper_t
	{
		using collection_t = T;
		using iterator = collection_t::reverse_iterator;
		using const_iterator = collection_t::const_reverse_iterator;

		collection_t& collection;

		reverse_iteration_helper_t(collection_t& collection) : collection(collection) {}
		iterator begin(){ return collection.rbegin(); }
		const_iterator begin() const { return collection.rbegin(); }
		iterator end(){ return collection.rend(); }
		const_iterator end() const { return collection.rend(); }
	};

	template <typename T>
	inline auto reverse_iteration(T& collection) { return reverse_iteration_helper_t<T>(collection); }

	//
	// このクラスは範囲です。
	//
	struct range_t {
		int start, end;
		std::wstring memo;
	};

	//
	// 範囲のコレクションを正規化します。
	//
	inline static void normalize(std::vector<range_t>& ranges)
	{
		// 範囲のコレクションが空の場合は何もしません。
		if (ranges.empty()) return;

		// 範囲の開始位置が範囲の終了位置より左にくるようにします。
		for (auto& range : ranges)
		{
			if (range.start > range.end)
				std::swap(range.start, range.end);
		}

		// 範囲のコレクションをソートします。
		std::sort(ranges.begin(), ranges.end(),
			[](const range_t& lhs, const range_t& rhs)
		{
			return (lhs.start == rhs.start) ? (lhs.end < rhs.end) : (lhs.start < rhs.start);
		});

		// 範囲を逆順に走査します。
		for (auto i = ranges.size() - 1; i > 0; i--)
		{
			auto& current_range = ranges[i];
			auto& prev_range = ranges[i - 1];

			// 2つの範囲が重なっている場合は
			if (prev_range.end >= current_range.start)
			{
				// 範囲を1つにまとめます。
				prev_range.end = std::max(
					prev_range.end, current_range.end);

				// 範囲を削除します。
				ranges.erase(ranges.begin() + i);
			}
		}
	}

	//
	// このクラスはエディタです。
	//
	struct editor_t
	{
		//
		// 編集セクションです。
		//
		EDIT_SECTION* edit;

		//
		// 編集情報です。
		//
		EDIT_INFO edit_info;

		//
		// コンストラクタです。
		//
		editor_t(EDIT_SECTION* edit)
			: edit(edit)
		{
			// 編集情報をコピーしておきます。
			edit_info = *edit->info;
		}

		//
		// デストラクタです。
		//
		~editor_t()
		{
			// そのまま続けてアンドゥなどのキーボード操作ができるように、
			// aviutl2ウィンドウにフォーカスを移動します。
			::SetFocus(hive.aviutl2_window);
		}

		//
		// フレームマーク用のメモを返します。
		// テスト用です。現在は使用されていません。
		//
		LPCWSTR get_memo(OBJECT_HANDLE object)
		{
			if (auto name = edit->get_object_name(object))
				return name;
			else
				return L"カット位置";
		}

		//
		// フォーカスオブジェクトまたは最初の選択オブジェクトを返します。
		//
		OBJECT_HANDLE get_single_target_object()
		{
			MY_TRACE_FUNC("");

			// 選択オブジェクトの数を取得します。
			auto nb_selected_objects = edit->get_selected_object_num();

			// 選択オブジェクトが存在する場合は
			if (nb_selected_objects)
			{
				// 最初の選択オブジェクトを返します。
				return edit->get_selected_object(0);
			}
			// 選択オブジェクトが存在しない場合は
			else
			{
				// フォーカスオブジェクトを返します。
				return edit->get_focus_object();
			}
		}

		//
		// フォーカスオブジェクトまたは選択オブジェクトを編集します。
		//
		void edit_focus_or_selected_objects(const auto& func)
		{
			// 選択オブジェクトの数を取得します。
			auto nb_selected_objects = edit->get_selected_object_num();

			// 選択オブジェクトが存在する場合は
			if (nb_selected_objects)
			{
				// 選択オブジェクトを走査します。
				for (auto i = 0; i < nb_selected_objects; i++)
				{
					// 選択オブジェクトを取得します。
					auto object = edit->get_selected_object(i);

					// 選択オブジェクトを編集します。
					func(object);
				}
			}
			// フォーカスオブジェクトが存在する場合は
			else if (auto focus_object = edit->get_focus_object())
			{
				// フォーカスオブジェクトを編集します。
				func(focus_object);
			}
		}

		//
		// 全てのオブジェクトまたは選択オブジェクトを編集します。
		//
		void edit_all_or_selected_objects(const auto& func, BOOL flag_use_selected_objects)
		{
			// 選択オブジェクトの数を取得します。
			auto nb_selected_objects = edit->get_selected_object_num();

			// 選択オブジェクトが存在する場合は
			if (nb_selected_objects && flag_use_selected_objects)
			{
				// 選択オブジェクトを走査します。
				for (auto i = 0; i < nb_selected_objects; i++)
				{
					// 選択オブジェクトを取得します。
					auto object = edit->get_selected_object(i);

					// 選択オブジェクトを編集します。
					func(object);
				}
			}
			// それ以外の場合は
			else
			{
				// 全てのレイヤーを走査します。
				for (auto layer = 0; layer <= edit_info.layer_max; layer++)
				{
					// レイヤーがロックされている場合は除外します。
					if (edit->get_layer_lock(layer)) continue;

					// 検索位置です。
					auto frame = 0;

					// オブジェクトが見つかった場合は
					while (auto object = edit->find_object(layer, frame))
					{
						// オブジェクトの位置を取得します。
						auto object_layer_frame = edit->get_object_layer_frame(object);

						// 検索位置を次に進めます。
						frame = object_layer_frame.end + 1;

						// オブジェクトを編集します。
						func(object);
					}
				}
			}
		}

		//
		// 指定された範囲で全てのオブジェクトまたは選択オブジェクトをカットします。
		//
		void cut_all_or_selected_objects(std::vector<range_t>& cut_ranges, BOOL flag_use_selected_objects)
		{
			MY_TRACE_FUNC("");

			// カットする範囲を正規化します。
			normalize(cut_ranges);

			// 編集対象のレイヤーです。
			std::unordered_set<int> target_layers;

			// 編集対象のオブジェクトです。
			std::unordered_set<OBJECT_HANDLE> target_objects;

			// オブジェクトを編集します。
			edit_all_or_selected_objects([&](OBJECT_HANDLE object)
			{
				// オブジェクトの位置を取得します。
				auto object_layer_frame = edit->get_object_layer_frame(object);

				// レイヤーがロックされている場合は除外します。
				if (edit->get_layer_lock(object_layer_frame.layer)) return;

				// 編集対象のレイヤーとして記憶しておきます。
				target_layers.emplace(object_layer_frame.layer);

				// 編集対象のオブジェクトとして記憶しておきます。
				target_objects.emplace(object);
			}, flag_use_selected_objects);

			// 編集したレイヤーを走査します。
			for (auto layer : target_layers)
			{
				// カットする範囲を逆順に走査します。
				for (const auto& cut_range : reverse_iteration(cut_ranges))
				{
					// フレームにマークを付ける場合は
					if (hive.flag_set_mark_frame)
					{
						// このままだとカットする範囲が複数ある場合、
						// 右にあるカットする範囲のマークがズレます。

						// フレームにマークを付けます。
						edit->set_mark_frame(cut_range.start, cut_range.memo.c_str());
					}

					// 移動量です。
					auto move_offset = cut_range.end - cut_range.start;

					// 検索位置です。
					auto frame = cut_range.start;

					// (カットする範囲の中にある)オブジェクトが見つかった場合は
					while (auto object = edit->find_object(layer, frame))
					{
						// オブジェクトの位置を取得します。
						auto object_layer_frame = edit->get_object_layer_frame(object);

						// オブジェクトがカットする範囲より右にある場合はループを終了します。
						if (object_layer_frame.start >= cut_range.end) break;

						// 検索位置を次に進めます。
						frame = object_layer_frame.end + 1;

						// カットする範囲でオブジェクトをカットします。
						cut_object(object, cut_range);
					}

					// (カットする範囲より右にある)オブジェクトが見つかった場合は
					while (auto object = edit->find_object(layer, frame))
					{
						// オブジェクトの位置を取得します。
						auto object_layer_frame = edit->get_object_layer_frame(object);

						// 検索位置を次に進めます。
						frame = object_layer_frame.end + 1;

						// (カットする範囲の分だけ)オブジェクトを左に移動します。
						auto result_move_object = edit->move_object(object,
							object_layer_frame.layer, object_layer_frame.start - move_offset);
						MY_TRACE_INT(result_move_object);
					}
				}
			}
		}

		//
		// オブジェクトを指定された範囲でカットします。
		//
		BOOL cut_object(OBJECT_HANDLE object, const range_t& range)
		{
			// オブジェクトの位置を取得します。
			auto object_layer_frame = edit->get_object_layer_frame(object);

			// オブジェクトの開始位置が範囲の終了位置より右にある場合は
			if (object_layer_frame.start >= range.end)
			{
				//       [object]
				// [range]

				// オブジェクトと範囲が交差しないのでFALSEを返します。
				return FALSE;
			}

			// オブジェクトの終了位置が範囲の開始位置より左にある場合は
			if (object_layer_frame.end <= range.start)
			{
				// [object]
				//        [range]

				// オブジェクトと範囲が交差しないのでFALSEを返します。
				return FALSE;
			}

			// この時点でオブジェクトと範囲が交差するのが確定しています。

			// オブジェクトの開始位置が範囲の開始位置より右にある場合は
			if (object_layer_frame.start >= range.start)
			{
				// オブジェクトの終了位置が範囲の終了位置より右にある場合は
				if (object_layer_frame.end >= range.end)
				{
					//    [object]
					// [range]

					// オブジェクトの開始位置が範囲の中にあるので
					// オブジェクトの開始位置をカットして左に詰めます。
					cut_object_start(object, range);
				}
				// それ以外の場合は
				else
				{
					//     [object]
					// [    range    ]

					// オブジェクトが完全に範囲の中にあるので
					// オブジェクト自体を削除します。
					edit->delete_object(object);
				}
			}
			// それ以外の場合は
			else
			{
				// オブジェクトの終了位置が範囲の終了位置より左にある場合は
				if (object_layer_frame.end <= range.end)
				{
					// [object]
					//     [range]

					// オブジェクトの終了位置が範囲の中にあるので
					// オブジェクトの終了位置をカットして左に詰めます。
					cut_object_end(object, range);
				}
				// それ以外の場合は
				else
				{
					// [     object    ]
					//      [range]

					// オブジェクトを範囲で分割して左に詰めます。
					cut_object_center(object, range);
				}
			}

			return TRUE;
		}

		//
		// オブジェクトの開始位置を指定された位置まで移動します。
		//
		void move_object_start(OBJECT_HANDLE object, int frame)
		{
			MY_TRACE_FUNC("{/}", frame);

			// まず、指定された位置より左にある中間点を削除します。

			// 中間点の数を取得します。
			auto nb_object_sections = edit->get_object_section_num(object);

			// 中間点を走査します。
			for (auto i = 1; i < nb_object_sections; i++)
			{
				// 最も左にある中間点から順番に削除していくので、
				// インデックスは1のまま参照していきます。

				// 最初の中間点のフレーム番号を取得します。
				auto section_frame = edit->get_object_section_frame(object, 1);

				// 最初の中間点のフレーム番号が指定された位置より左にある場合は
				if (section_frame <= frame)
				{
					// 最初の中間点を削除します。
					auto result_delete_object_section =
						edit->delete_object_section(object, 1);
					MY_TRACE_INT(result_delete_object_section);
				}
				// それ以外の場合は
				else
				{
					// ループを終了します。
					break;
				}
			}

			// オブジェクトの開始位置を指定された位置まで移動します。
			auto result_move_object_section =
				edit->move_object_section(object, 0, frame);
			MY_TRACE_INT(result_move_object_section);
		}

		//
		// オブジェクトの終了位置を指定された位置まで移動します。
		//
		void move_object_end(OBJECT_HANDLE object, int frame)
		{
			MY_TRACE_FUNC("{/}", frame);

			// まず、指定された位置より右にある中間点を削除します。

			// 中間点の数を取得します。
			auto nb_object_sections = edit->get_object_section_num(object);

			// 中間点を逆順に走査します。
			for (auto i = nb_object_sections - 1; i >= 0; i--)
			{
				// 中間点のフレーム番号を取得します。
				auto section_frame = edit->get_object_section_frame(object, i);

				// 中間点のフレーム番号が指定された位置より右にある場合は
				if (section_frame >= frame)
				{
					// 中間点を削除します。
					auto result_delete_object_section =
						edit->delete_object_section(object, i);
					MY_TRACE_INT(result_delete_object_section);
				}
				// それ以外の場合は
				else
				{
					// ループを終了します。
					break;
				}
			}

			// 中間点の数を再取得します。
			nb_object_sections = edit->get_object_section_num(object);

			// オブジェクトの終了位置を指定された位置まで移動します。
			auto result_move_object_section =
				edit->move_object_section(object, nb_object_sections, frame - 1);
			MY_TRACE_INT(result_move_object_section);
		}

		//
		// オブジェクトの開始位置を指定された範囲でカットします。
		//
		void cut_object_start(OBJECT_HANDLE object, const range_t& range)
		{
			MY_TRACE_FUNC("{/}, {/}", range.start, range.end);

			// オブジェクトの開始位置を範囲の終了位置まで移動します。
			move_object_start(object, range.end);

			// オブジェクトの位置を取得します。
			auto object_layer_frame = edit->get_object_layer_frame(object);

			// オブジェクトを範囲の開始位置まで移動します。
			auto result_move_object =
				edit->move_object(object, object_layer_frame.layer, range.start);
			MY_TRACE_INT(result_move_object);
		}

		//
		// オブジェクトの終了位置を指定された範囲でカットします。
		//
		void cut_object_end(OBJECT_HANDLE object, const range_t& range)
		{
			MY_TRACE_FUNC("{/}, {/}", range.start, range.end);

			// オブジェクトの終了位置を範囲の開始位置まで移動します。
			move_object_end(object, range.start);
		}

		//
		// 指定された範囲でオブジェクトを分割して左に詰めます。
		// オブジェクトが範囲を完全に内包していることを前提にしています。
		//
		void cut_object_center(OBJECT_HANDLE object, const range_t& range)
		{
			// オブジェクトの位置を取得します。
			auto object_layer_frame = edit->get_object_layer_frame(object);

			// オブジェクトのエイリアスを作成します。
			auto alias = (const char8_t*)edit->get_object_alias(object);

			// オブジェクトの終了位置を範囲の開始位置まで移動します。
			move_object_end(object, range.start);

			// オブジェクトのクローンを安全なレイヤーに作成します。
			auto clone_object = edit->create_object_from_alias(
				(LPCSTR)alias, edit_info.layer_max + 1, object_layer_frame.start,
				object_layer_frame.end + 1 - object_layer_frame.start);

			// クローンオブジェクトの開始位置を範囲の終了位置まで移動します。
			move_object_start(clone_object, range.end);

			// クローンオブジェクトを元のオブジェクトのすぐ右に移動します。
			auto result_move_object = edit->move_object(
				clone_object, object_layer_frame.layer, range.start);
			MY_TRACE_INT(result_move_object);
		}
	};
}
