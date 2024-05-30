#include "../nfc_playlist.h"

int32_t nfc_playlist_playlist_rename_task(void* context) {
   NfcPlaylist* nfc_playlist = context;

   char const* old_file_path = furi_string_get_cstr(nfc_playlist->settings.playlist_path);

   FuriString* new_file_path = furi_string_alloc();
   path_extract_dirname(old_file_path, new_file_path);
   furi_string_cat_printf(new_file_path, "/%s.txt", nfc_playlist->text_input_output);
   char const* new_file_path_cstr = furi_string_get_cstr(new_file_path);

   Storage* storage = furi_record_open(RECORD_STORAGE);

   if (!storage_file_exists(storage, new_file_path_cstr)) {
      if (storage_common_rename(storage, old_file_path, new_file_path_cstr)) {
         furi_string_move(nfc_playlist->settings.playlist_path, new_file_path);
      }
   }

   furi_record_close(RECORD_STORAGE);

   return 0;
}

void nfc_playlist_playlist_rename_free(NfcPlaylist* nfc_playlist) {
   furi_assert(nfc_playlist);
   furi_thread_free(nfc_playlist->thread);
}

void nfc_playlist_playlist_rename_stop(NfcPlaylist* nfc_playlist) {
   furi_assert(nfc_playlist);
   furi_thread_join(nfc_playlist->thread);
}

void nfc_playlist_playlist_rename_thread_state_callback(FuriThreadState state, void* context) {
   if (state == FuriThreadStateStopped) {
      NfcPlaylist* nfc_playlist = context;
      scene_manager_handle_custom_event(nfc_playlist->scene_manager, 0);
   }
}

void nfc_playlist_playlist_rename_menu_callback(void* context) {
   NfcPlaylist* nfc_playlist = context;
   nfc_playlist->thread = furi_thread_alloc_ex("NfcPlaylistRenamer", 8192, nfc_playlist_playlist_rename_task, nfc_playlist);
   // DISABLED FOR NOW due to it causing a crash once finished renaming the file not triggering the scene switch nto sure why but looking into it
   // once fixed this will also be applied to new playlist creation to fix similar view port issues
   // furi_thread_set_state_context(nfc_playlist->thread, nfc_playlist);
   // furi_thread_set_state_callback(nfc_playlist->thread, nfc_playlist_playlist_rename_thread_state_callback);
   furi_thread_start(nfc_playlist->thread);
}

void nfc_playlist_playlist_rename_scene_on_enter(void* context) {
   NfcPlaylist* nfc_playlist = context;

   FuriString* tmp_str = furi_string_alloc();
   path_extract_filename(nfc_playlist->settings.playlist_path, tmp_str, true);

   nfc_playlist->text_input_output = malloc(MAX_PLAYLIST_NAME_LEN + 1);
   strcpy(nfc_playlist->text_input_output, furi_string_get_cstr(tmp_str));
   furi_string_free(tmp_str);

   text_input_set_header_text(nfc_playlist->text_input, "Enter new file name");
   text_input_set_minimum_length(nfc_playlist->text_input, 1);
   text_input_set_result_callback(nfc_playlist->text_input, nfc_playlist_playlist_rename_menu_callback, nfc_playlist, nfc_playlist->text_input_output, MAX_PLAYLIST_NAME_LEN, false);

   view_dispatcher_switch_to_view(nfc_playlist->view_dispatcher, NfcPlaylistView_TextInput);
}

bool nfc_playlist_playlist_rename_scene_on_event(void* context, SceneManagerEvent event) {
   NfcPlaylist* nfc_playlist = context;
   if(event.type == SceneManagerEventTypeCustom && event.event == 0) {
      nfc_playlist_playlist_rename_stop(nfc_playlist);
      nfc_playlist_playlist_rename_free(nfc_playlist);
      scene_manager_search_and_switch_to_previous_scene(nfc_playlist->scene_manager, NfcPlaylistScene_MainMenu);
      return true;
   }
   return false;
}

void nfc_playlist_playlist_rename_scene_on_exit(void* context) {
   NfcPlaylist* nfc_playlist = context;
   free(nfc_playlist->text_input_output);
   text_input_reset(nfc_playlist->text_input);
}