#pragma once

#include_next "config.h"

#ifndef PACKAGE_VERSION
#error "PACKAGE_VERSION not defined, check that config.h is used from vkd3d"
#endif

#define HAVE_SYNC_ADD_AND_FETCH 1
#define HAVE_SYNC_BOOL_COMPARE_AND_SWAP 1