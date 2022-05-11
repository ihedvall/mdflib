# Copyright 2021 Ingemar Hedvall
# SPDX-License-Identifier: MIT
#set(OPENSSL_ROOT "c:/msys64/mingw64" CACHE PATH "OpenSSL is required for calculating checksum (MD5)")

if (NOT OPENSSL_FOUND)
    set(OPENSSL_USE_STATIC_LIBS TRUE)
    find_package(OpenSSL REQUIRED)
endif()

message(STATUS "OPENSSL Found: " ${OPENSSL_FOUND})
message(STATUS "OPENSSL Version: " ${OPENSSL_VERSION})
message(STATUS "OPENSSL Include Dir: " ${OPENSSL_INCLUDE_DIR})
message(STATUS "OPENSSL Libraries: " ${OPENSSL_LIBRARIES})
message(STATUS "OPENSSL Crypto Libraries: " ${OPENSSL_CRYPTO_LIBRARIES})
message(STATUS "OPENSSL Crypto Library: " ${OPENSSL_CRYPTO_LIBRARY})
message(STATUS "OPENSSL SSL Libraries: " ${OPENSSL_SSL_LIBRARIES})
message(STATUS "OPENSSL SSL Library: " ${OPENSSL_SSL_LIBRARY})