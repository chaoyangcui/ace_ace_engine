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
export interface UploadResponse {
  /**
   * Status code returned by the server.
   * @devices tv, phone, tablet, wearable
   */
  code: number;

  /**
   * Content returned by the server.
   * The value type is determined by the returned content.
   * @devices tv, phone, tablet, wearable
   */
  data: string;

  /**
   * Headers returned by the server.
   * @devices tv, phone, tablet, wearable
   */
  headers: Object;
}

/**
 * @devices tv, phone, tablet, wearable
 */
export interface DownloadResponse {
  /**
   * Download token, which is used to obtain the download status.
   * @devices tv, phone, tablet, wearable
   */
  token: string;
}

/**
 * @devices tv, phone, tablet, wearable
 */
export interface OnDownloadCompleteResponse {
  /**
   * URI of the download file.
   * @devices tv, phone, tablet, wearable
   */
  uri: string;
}

/**
 * @devices tv, phone, tablet, wearable
 */
export interface RequestFile {
  /**
   * File name in the header when multipart is used.
   * @devices tv, phone, tablet, wearable
   */
  filename?: string;

  /**
   * Name of a form item when multipart is used. The default value is file.
   * @devices tv, phone, tablet, wearable
   */
  name?: string;

  /**
   * Local storage directory of a file.
   * @devices tv, phone, tablet, wearable
   */
  uri: string;

  /**
   * Type of the file content.
   * By default, the type is obtained based on the suffix of the file name or URI.
   * @devices tv, phone, tablet, wearable
   */
  type?: string;
}

/**
 * @devices tv, phone, tablet, wearable
 */
export interface RequestData {
  /**
   * Name of the form element.
   * @devices tv, phone, tablet, wearable
   */
  name: string;

  /**
   * Value of the form element.
   * @devices tv, phone, tablet, wearable
   */
  value: string;
}

/**
 * @devices tv, phone, tablet, wearable
 */
export interface UploadRequestOptions {
  /**
   * Resource URL.
   * @devices tv, phone, tablet, wearable
   */
  url: string;

  /**
   * Form data in the request body.
   * @devices tv, phone, tablet, wearable
   */
  data?: Array<RequestData>;

  /**
   * List of files to upload, which is submitted through multipart/form-data.
   * @devices tv, phone, tablet, wearable
   */
  files: Array<RequestFile>;

  /**
   * Request header.
   * @devices tv, phone, tablet, wearable
   */
  header?: Object;

  /**
   * Request methods available: POST and PUT. The default value is POST.
   * @devices tv, phone, tablet, wearable
   */
  method?: string;

  /**
   * Called when the files are uploaded successfully.
   * @devices tv, phone, tablet, wearable
   */
  success?: (data: UploadResponse) => void;

  /**
   * Called when uploading fails.
   * @devices tv, phone, tablet, wearable
   */
  fail?: (data: any, code: number) => void;

  /**
   * Called when the execution is completed.
   * @devices tv, phone, tablet, wearable
   */
  complete?: () => void;
}

/**
 * @devices tv, phone, tablet, wearable
 */
export interface DownloadRequestOptions {
  /**
   * Resource URL.
   * @devices tv, phone, tablet, wearable
   */
  url: string;

  /**
   * Name of the file to downloaded.
   * The value is obtained from the current request or resource URL by default.
   * @devices tv, phone, tablet, wearable
   */
  filename?: string;

  /**
   * Request header.
   * @devices tv, phone, tablet, wearable
   */
  header?: string;

  /**
   * Download description.
   * The default value is the file name.
   * @devices tv, phone, tablet, wearable
   */
  description?: string;

  /**
   * Called when the files are successfully downloaded.
   * @devices tv, phone, tablet, wearable
   */
  success?: (data: DownloadResponse) => void;

  /**
   * Called when downloading fails.
   * @devices tv, phone, tablet, wearable
   */
  fail?: (data: any, code: number) => void;

  /**
   * Called when the execution is completed.
   * @devices tv, phone, tablet, wearable
   */
  complete?: () => void;
}

/**
 * @devices tv, phone, tablet, wearable
 */
export interface OnDownloadCompleteOptions {
  /**
   * Token of the result returned by the download function.
   * @devices tv, phone, tablet, wearable
   */
  token: string;

  /**
   * Called when the downloads are successfully obtained
   * @devices tv, phone, tablet, wearable
   */
  success?: (data: OnDownloadCompleteResponse) => void;

  /**
   * Called when the downloads fail to be obtained.
   * @devices tv, phone, tablet, wearable
   */
  fail?: (data: any, code: number) => void;

  /**
   * Called when the execution is completed.
   * @devices tv, phone, tablet, wearable
   */
  complete?: () => void;
}

/**
 * @devices tv, phone, tablet, wearable
 */
export default class Request {
  /**
   * Upload files.
   * @param options Options.
   * @devices tv, phone, tablet, wearable
   */
  static upload(options: UploadRequestOptions): void;

  /**
   * This API is used to download files.
   * @param options Options.
   * @devices tv, phone, tablet, wearable
   */
  static download(options: DownloadRequestOptions): void;

  /**
   * Listens to download task status.
   * @param options Options.
   * @devices tv, phone, tablet, wearable
   */
  static onDownloadComplete(options: OnDownloadCompleteOptions): void;
}
