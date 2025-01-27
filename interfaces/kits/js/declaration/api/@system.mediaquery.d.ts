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

/**
 * @devices tv, phone, tablet, wearable
 */
export interface MediaQueryEvent {
  /**
   * @devices tv, phone, tablet, wearable
   */
  matches: boolean;
}

/**
 * @devices tv, phone, tablet, wearable
 */
export interface MediaQueryList {
  /**
   * Serialized media query condition.
   * This parameter is read-only.
   * @devices tv, phone, tablet, wearable
   */
  media?: string;

  /**
   * Whether the query is successful. True if the query condition is matched successfully, false otherwise.
   * This parameter is read-only.
   * @devices tv, phone, tablet, wearable
   */
  matches?: boolean;

  /**
   * Called when the matches value changes.
   * @devices tv, phone, tablet, wearable
   */
  onchange?: (matches: boolean) => void;

  /**
   * Adds a listening function to MediaQueryList.
   * The listening function must be added before onShow is called, that is, added to the onInit or onReady function.
   * @devices tv, phone, tablet, wearable
   */
  addListener(callback: (event: MediaQueryEvent) => void): void;

  /**
   * Removes a listening function from MediaQueryList.
   * @devices tv, phone, tablet, wearable
   */
  removeListener(callback: (event: MediaQueryEvent) => void): void;
}

/**
 * @devices tv, phone, tablet, wearable
 */
export default class MediaQuery {
  /**
   * Queries a media item and returns a MediaQueryList object.
   * @devices tv, phone, tablet, wearable
   */
  static matchMedia(condition: string): MediaQueryList;
}
