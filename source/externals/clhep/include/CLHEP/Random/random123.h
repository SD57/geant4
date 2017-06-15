//    Copyright (c) 2017 Dmitry Savin <sd57@protonmail.ch>
//
//    Distributed under the GNU General Public License version 3.

/// Header to include Random123 files and suppress repetative warnings

#ifndef RANDOM123_H
#define RANDOM123_H

#ifdef __clang__
#warning "Disabling -Wexpansion-to-defined for clean compile output"
// TODO check portability
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#endif
#include "philox.h"
#include "threefry.h"
#ifdef __clang__
#pragma clang diagnostic pop
// FIXME looks like the diagnostic still remains suppressed
#endif

#endif // RANDOM123_H
