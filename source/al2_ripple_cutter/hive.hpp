#pragma once

namespace apn::ripple_cutter
{
	//
	// このクラスは他クラスから共通して使用される変数を保持します。
	//
	inline struct hive_t : hive_base_t
	{
		//
		// TRUEの場合はカットしたフレームにマークを付けます。
		// テスト用です。現在は使用されていません。
		//
		BOOL flag_set_mark_frame = FALSE;
	} hive;
}
