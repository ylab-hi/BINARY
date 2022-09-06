//
// Created by li002252 on 8/23/22.
//

#ifndef BUILDALL_STANDALONE_SV2NL_INFO_FILED_HPP_
#define BUILDALL_STANDALONE_SV2NL_INFO_FILED_HPP_
#include <binary/algorithm/interval_tree.hpp>
#include <binary/parser/vcf.hpp>

namespace sv2nl {
  namespace tree = binary::algorithm::tree;
  namespace vcf = binary::parser::vcf;

  struct Sv2nlInfoField : public vcf::BaseInfoField {
    std::string svtype{};
    vcf::pos_t svend{};

    std::string chr2{};
    bool strand1{true};  // true means positive
    bool strand2{true};

    constexpr Sv2nlInfoField() = default;
    Sv2nlInfoField(Sv2nlInfoField const&) = default;
    Sv2nlInfoField(Sv2nlInfoField&&) = default;
    Sv2nlInfoField& operator=(Sv2nlInfoField const&) = default;
    Sv2nlInfoField& operator=(Sv2nlInfoField&&) = default;
    ~Sv2nlInfoField() override = default;

    void update(std::shared_ptr<vcf::details::DataImpl> const& data) override;

    friend auto operator<<(std::ostream& os, Sv2nlInfoField const& info) -> std::ostream& {
      os << "svtype: " << info.svtype << " svend: " << info.svend;
      return os;
    }

    friend auto operator==(Sv2nlInfoField const& lhs, Sv2nlInfoField const& rhs) -> bool {
      return lhs.svtype == rhs.svtype && lhs.svend == rhs.svend;
    }
  };

  using Sv2nlVcfRecord = vcf::BaseVcfRecord<Sv2nlInfoField>;
  using Sv2nlVcfRanges = vcf::VcfRanges<Sv2nlVcfRecord>;
  using Sv2nlVcfInterval = vcf::BaseVcfInterval<Sv2nlVcfRecord>;
  using Sv2nlVcfIntervalNode = tree::IntervalNode<Sv2nlVcfInterval>;
  using Sv2nlVcfIntervalTree = tree::IntervalTree<Sv2nlVcfIntervalNode>;

}  // namespace sv2nl
#endif  // BUILDALL_STANDALONE_SV2NL_INFO_FILED_HPP_
