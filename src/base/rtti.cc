#include "rtti.h"

TS_RTTI_TYPE(base::rtti::Base)

namespace base::rtti {
  bool TypeMetadata::match(const TypeMetadata* metadata) const
  {
    if (metadata == &InfoStructure<Base>::data) 
      return true;

    for (const auto* ti = this;
        ti != &InfoStructure<Base>::data;
        ti = ti->base)
      if (ti == metadata) 
        return true;

    return false;
  }
}