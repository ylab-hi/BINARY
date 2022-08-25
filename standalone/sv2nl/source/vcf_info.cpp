//
// Created by li002252 on 8/23/22.
//

#include "vcf_info.hpp"

void Sv2nlInfoField::update(const std::shared_ptr<vcf::details::DataImpl>& data) {
  svtype = vcf::details::get_info_field<char>("SVTYPE", data->header.get(), data->record.get());
  svend = vcf::details::get_info_field<vcf::pos_t>("SVEND", data->header.get(), data->record.get());
}
