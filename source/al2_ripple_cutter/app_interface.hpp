#pragma once

namespace apn::ripple_cutter
{
	//
	// このクラスはアプリケーションのインターフェイスです。
	//
	inline struct app_interface_t {
		//
		// コンストラクタです。
		//
		app_interface_t() { app = this; }

		virtual void cut_from_start_to_cursor() = 0;
		virtual void cut_from_cursor_to_end() = 0;
		virtual void cut_timeline_with_range() = 0;
		virtual void cut_timeline_with_object() = 0;
	} *app = nullptr;
}
