#ifndef INSTANT_OPERATOR_ELTWISE_HPP
#define INSTANT_OPERATOR_ELTWISE_HPP

#include <instant/operator/common.hpp>

namespace instant {

    inline auto make_relu_primitive(
      std::unordered_map<std::string, const mkldnn::memory> const&
      /*parameter_memory_table*/,
      std::unordered_map<std::string, std::tuple<const mkldnn::memory,
                                                 mkldnn::memory::format>> const&
        variable_memory_table,
      std::set<std::string> const& required_output_set,
      onnx::NodeProto const& node, mkldnn::engine const& engine) {
        auto negative_slope = 0.;
        auto const& input_memory_and_origin_format =
          find_value(variable_memory_table, node.input(0));
        auto const& input_memory = std::get<0>(input_memory_and_origin_format);
        auto input_origin_format = std::get<1>(input_memory_and_origin_format);
        auto input_output_dims = extract_dims(input_memory);

        auto const& output_name = node.output(0);

        auto op_desc = mkldnn::eltwise_forward::desc(
          mkldnn::prop_kind::forward_inference, mkldnn::algorithm::eltwise_relu,
          input_memory.get_primitive_desc().desc(), negative_slope);
        auto op_pd = mkldnn::eltwise_forward::primitive_desc(op_desc, engine);

        std::vector<mkldnn::primitive> net;
        std::vector<std::pair<
          std::string, std::tuple<mkldnn::memory, mkldnn::memory::format>>>
          variable_memory_list;
        std::vector<mkldnn::memory>
          temp_variable_memory_list; // for temporary memory's life
        std::vector<std::pair<std::string, array>> output_name_and_arr_list;

        manage_output_memory(
          required_output_set, output_name, dtype_t::float_, input_output_dims,
          input_origin_format, op_pd.dst_primitive_desc(), variable_memory_list,
          temp_variable_memory_list, output_name_and_arr_list, net, engine,
          [&input_memory, &node, &op_pd](auto& op_output_memory) {
              return mkldnn::eltwise_forward(op_pd, input_memory,
                                             op_output_memory);
          });

        return std::make_tuple(net, variable_memory_list,
                               temp_variable_memory_list,
                               output_name_and_arr_list);
    }

} // namespace instant

#endif // INSTANT_OPERATOR_ELTWISE_HPP
