#ifndef __SRC_BASIC_LIBRARY_H__
#define __SRC_BASIC_LIBRARY_H__

#include <memory>
#include "module.h"

namespace webserver {

class Library {
public:
    static Module::ptr GetModule(const std::string& path);
};

}

#endif
