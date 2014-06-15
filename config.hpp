#ifndef __CPORT_CONFIG_HPP__
#define __CPORT_CONFIG_HPP__

#ifndef CPORT_DECL_TYPE
    #ifdef CPORT_HEADER_ONLY_LIB
        #define CPORT_DECL_TYPE inline
    #else
        #define CPORT_DECL_TYPE
    #endif // CPORT_HEADER_ONLY_LIB
#endif // CPORT_DECL_TYPE

#endif // __CPORT_CONFIG_HPP__