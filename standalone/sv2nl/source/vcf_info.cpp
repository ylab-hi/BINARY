//
// Created by li002252 on 8/23/22.
//

#include "vcf_info.hpp"

namespace sv2nl {

  void Sv2nlInfoField::update(const std::shared_ptr<vcf::details::DataImpl>& data,
                              std::string_view source) {
    svtype = vcf::get_info_field<char>("SVTYPE", data->header.get(), data->record.get());

    if (svtype == "TRA" || svtype == "BND") {
      chr2 = vcf::get_info_field<char>("CHR2", data->header.get(), data->record.get());
    }

    if (svtype == "INV") {
      // fetch strand info for inversion in non-linear result
      try {
        strand1
            = vcf::get_info_field<char>("STRAND1", data->header.get(), data->record.get()) == "+"
                  ? true
                  : false;

        strand2
            = vcf::get_info_field<char>("STRAND2", data->header.get(), data->record.get()) == "+"
                  ? true
                  : false;
      } catch (...) {
      }
    }

    if (svtype == "BND") {
      // delly tra result
      svend = vcf::get_info_field<vcf::pos_t>("POS2", data->header.get(), data->record.get());
    } else if (source == "nls") {
      // nls result
      svend = vcf::get_info_field<vcf::pos_t>("SVEND", data->header.get(), data->record.get());
    } else {
      // delly other results
      svend = vcf::get_info_field<vcf::pos_t>("END", data->header.get(), data->record.get());
    }
  }

}  // namespace sv2nl
