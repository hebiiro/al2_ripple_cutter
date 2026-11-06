#pragma once

namespace apn::ripple_cutter
{
	//
	// このクラスはコンフィグの入出力を担当します。
	//
	inline struct config_io_t : config_io_base2_t
	{
		//
		// ノードからコンフィグを読み込みます。
		//
		virtual BOOL read_node(n_json& root)
		{
			MY_TRACE_FUNC("");

			return __super::read_node(root);
		}

		//
		// ノードにコンフィグを書き込みます。
		//
		virtual BOOL write_node(n_json& root)
		{
			MY_TRACE_FUNC("");

			return __super::write_node(root);
		}
	} config_io;
}
