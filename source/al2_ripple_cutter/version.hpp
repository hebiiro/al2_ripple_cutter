#pragma once

namespace apn::ripple_cutter
{
	//
	// このクラスはこのプログラムのバージョンを管理します。
	//
	inline struct version_t : version_base_t {
		//
		// コンストラクタです。
		//
		version_t() : version_base_t(L"リップルカット", L"🐍リップルカット🔖", L"r1") {}
	} version;
}
