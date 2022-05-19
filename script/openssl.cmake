# Copyright 2021 Ingemar Hedvall
# SPDX-License-Identifier: MIT

if (NOT OPENSSL_FOUND)
    set(OPENSSL_USE_STATIC_LIBS ON)
    find_package(OpenSSL)

    message(STATUS "OPENSSL Found (Try 1): " ${OPENSSL_FOUND})
    message(STATUS "OPENSSL Version: " ${OPENSSL_VERSION})
    message(STATUS "OPENSSL Include Dir: " ${OPENSSL_INCLUDE_DIR})
    message(STATUS "OPENSSL Libraries: " ${OPENSSL_LIBRARIES})
    message(STATUS "OPENSSL Crypto Libraries: " ${OPENSSL_CRYPTO_LIBRARIES})
    message(STATUS "OPENSSL Crypto Library: " ${OPENSSL_CRYPTO_LIBRARY})
    message(STATUS "OPENSSL SSL Libraries: " ${OPENSSL_SSL_LIBRARIES})
    message(STATUS "OPENSSL SSL Library: " ${OPENSSL_SSL_LIBRARY})

endif()