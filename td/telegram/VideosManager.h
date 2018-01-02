//
// Copyright Aliaksei Levin (levlam@telegram.org), Arseny Smirnov (arseny30@gmail.com) 2014-2018
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "td/telegram/td_api.h"
#include "td/telegram/telegram_api.h"

#include "td/telegram/files/FileId.h"
#include "td/telegram/Photo.h"
#include "td/telegram/SecretInputMedia.h"

#include "td/utils/buffer.h"
#include "td/utils/common.h"

#include <unordered_map>

namespace td {
class Td;
}  // namespace td

namespace td {

class VideosManager {
 public:
  explicit VideosManager(Td *td);

  int32 get_video_duration(FileId file_id);

  tl_object_ptr<td_api::video> get_video_object(FileId file_id);

  void create_video(FileId file_id, PhotoSize thumbnail, bool has_stickers, vector<FileId> &&sticker_file_ids,
                    string file_name, string mime_type, int32 duration, Dimensions dimensions, bool replace);

  tl_object_ptr<telegram_api::InputMedia> get_input_media(FileId file_id,
                                                          tl_object_ptr<telegram_api::InputFile> input_file,
                                                          tl_object_ptr<telegram_api::InputFile> input_thumbnail,
                                                          const string &caption, int32 ttl) const;

  SecretInputMedia get_secret_input_media(FileId video_file_id,
                                          tl_object_ptr<telegram_api::InputEncryptedFile> input_file,
                                          const string &caption, BufferSlice thumbnail) const;

  FileId get_video_thumbnail_file_id(FileId file_id) const;

  void delete_video_thumbnail(FileId file_id);

  FileId dup_video(FileId new_id, FileId old_id);

  bool merge_videos(FileId new_id, FileId old_id, bool can_delete_old);

  template <class T>
  void store_video(FileId file_id, T &storer) const;

  template <class T>
  FileId parse_video(T &parser);

  string get_video_search_text(FileId file_id) const;

 private:
  class Video {
   public:
    string file_name;
    string mime_type;
    int32 duration = 0;
    Dimensions dimensions;
    PhotoSize thumbnail;

    bool has_stickers = false;
    vector<FileId> sticker_file_ids;

    FileId file_id;

    bool is_changed = true;
  };

  const Video *get_video(FileId file_id) const;

  FileId on_get_video(std::unique_ptr<Video> new_video, bool replace);

  Td *td_;
  std::unordered_map<FileId, unique_ptr<Video>, FileIdHash> videos_;
};

}  // namespace td
