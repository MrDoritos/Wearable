#pragma once

#include "config.h"
#include "ui.h"
#include "log.h"

namespace wbl {
namespace UI {

template<typename Buffer, typename DataLog = DataLogT<>, typename ElementT = ElementBaseT<Buffer>>
struct ElementLogT : public ElementT, public DataLog {
    using ElementT::ElementT;
    using ElementT::operator<<;

    constexpr ElementLogT(DataLog &log):DataLog(log){}
    constexpr ElementLogT(Buffer &buffer, DataLog &log):ElementT(buffer),DataLog(log){}
}

}
}
