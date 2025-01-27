/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FOUNDATION_ACE_FRAMEWORKS_BASE_UTILS_MACROS_H
#define FOUNDATION_ACE_FRAMEWORKS_BASE_UTILS_MACROS_H

#ifndef ACE_EXPORT
#ifndef WEARABLE_PRODUCT
#define ACE_EXPORT __attribute__((visibility("default")))
#else
#define ACE_EXPORT
#endif
#endif

#ifdef ACE_DEBUG

#ifdef NDEBUG
#define CANCEL_NDEBUG
#undef NDEBUG
#endif // NDEBUG

#include <cassert>

#ifdef CANCEL_NDEBUG
#define NDEBUG
#undef CANCEL_NDEBUG
#endif // CANCEL_NDEBUG

#define ACE_DCHECK(expr) assert(expr)
#else
#define ACE_DCHECK(expr) ((void)0)

#endif // ACE_DEBUG

#endif // FOUNDATION_ACE_FRAMEWORKS_BASE_UTILS_MACROS_H
