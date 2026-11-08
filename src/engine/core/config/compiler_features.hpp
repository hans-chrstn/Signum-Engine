#pragma once

#if defined(__cpp_impl_reflection) && __cpp_impl_reflection >= 202506L
    #define SNE_HAS_CPP26_REFLECTION 1
#else
    #define SNE_HAS_CPP26_REFLECTION 0
#endif
