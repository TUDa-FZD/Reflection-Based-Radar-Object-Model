//
// Copyright Institute of Automotive Engineering
// of Technical University of Darmstadt 2020.
// Licensed under the EUPL-1.2-or-later
//
// This work covered by the EUPL can be used/merged and distributed
// in other works covered by GPL-2.0, GPL-3.0, LGPL, AGPL, CeCILL,
// OSL, EPL, MPL and other licences listed as compatible in the EUPL
// Appendix. This applies to the other (combined) work, while the
// original project stays covered by the EUPL without re-licensing.
//
// Alternatively, the contents of this file may be used under the
// terms of the Mozilla Public License, v. 2.0. If a copy of the MPL
// was not distributed with this file, you can obtain one at
// http://mozilla.org/MPL/2.0/.
//

#ifndef OSMPSENSORFRAMEWORK_SEQUENCE_HPP
#define OSMPSENSORFRAMEWORK_SEQUENCE_HPP

#include "strategy.hpp"
#include <vector>
#include <memory>

namespace model {

    class Sequence : public Strategy {
    public:
        Sequence(const Profile &profile, const Log &log, const Alert &alert);

        void apply(osi3::SensorData &data) override;

    private:
        std::vector<std::unique_ptr<Strategy>> strategies;
    };

}

#endif //OSMPSENSORFRAMEWORK_SEQUENCE_HPP
