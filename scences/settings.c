#include "nfc_playlist.h"
#include "scences/settings.h"

void nfc_playlist_settings_menu_callback(void* context, uint32_t index) {
   NfcPlaylist* nfc_playlist = context;
   scene_manager_handle_custom_event(nfc_playlist->scene_manager, index);
}

void nfc_playlist_settings_options_change_callback(VariableItem* item) {
   NfcPlaylist* nfc_playlist = variable_item_get_context(item);

   uint8_t current_option = variable_item_list_get_selected_item_index(nfc_playlist->variable_item_list);
   uint8_t option_value_index = variable_item_get_current_value_index(item);
   FuriString* temp_str = furi_string_alloc();
   switch(current_option) {
      case NfcPlaylistSettings_Timeout:
         nfc_playlist->settings.emulate_timeout = option_value_index;
         furi_string_printf(temp_str, "%ds", options_emulate_timeout[nfc_playlist->settings.emulate_timeout]);
         variable_item_set_current_value_text(item, furi_string_get_cstr(temp_str));
         break;
      case NfcPlaylistSettings_Delay:
         nfc_playlist->settings.emulate_delay = option_value_index;
         furi_string_printf(temp_str, "%ds", options_emulate_delay[nfc_playlist->settings.emulate_delay]);
         variable_item_set_current_value_text(item, furi_string_get_cstr(temp_str));
         break;
      case NfcPlaylistSettings_LedIndicator:
         nfc_playlist->settings.emulate_led_indicator = option_value_index;
         variable_item_set_current_value_text(item, nfc_playlist->settings.emulate_led_indicator ? "ON" : "OFF");
         break;
      default:
         break;
   }
   furi_string_free(temp_str);
}

void nfc_playlist_settings_scene_on_enter(void* context) {
   NfcPlaylist* nfc_playlist = context;
   FuriString* temp_str = furi_string_alloc();

   variable_item_list_set_header(nfc_playlist->variable_item_list, "Settings");

   VariableItem* emulation_timeout_settings = variable_item_list_add(
      nfc_playlist->variable_item_list,
      "Emulate time",
      (sizeof(options_emulate_timeout)/sizeof(options_emulate_timeout[0])),
      nfc_playlist_settings_options_change_callback,
      nfc_playlist);
   variable_item_set_current_value_index(emulation_timeout_settings, nfc_playlist->settings.emulate_timeout);
   furi_string_printf(temp_str, "%ds", options_emulate_timeout[nfc_playlist->settings.emulate_timeout]);
   variable_item_set_current_value_text(emulation_timeout_settings, furi_string_get_cstr(temp_str));

   VariableItem* emulation_delay_settings = variable_item_list_add(
      nfc_playlist->variable_item_list,
      "Delay time",
      (sizeof(options_emulate_delay)/sizeof(options_emulate_delay[0])),
      nfc_playlist_settings_options_change_callback,
      nfc_playlist);
   variable_item_set_current_value_index(emulation_delay_settings, nfc_playlist->settings.emulate_delay);
   furi_string_printf(temp_str, "%ds", options_emulate_delay[nfc_playlist->settings.emulate_delay]);
   variable_item_set_current_value_text(emulation_delay_settings, furi_string_get_cstr(temp_str));

   VariableItem* emulation_led_indicator_settings = variable_item_list_add(
      nfc_playlist->variable_item_list,
      "LED Indicator",
      2,
      nfc_playlist_settings_options_change_callback,
      nfc_playlist);
   variable_item_set_current_value_index(emulation_led_indicator_settings, nfc_playlist->settings.emulate_led_indicator);
   variable_item_set_current_value_text(emulation_led_indicator_settings, nfc_playlist->settings.emulate_led_indicator ? "ON" : "OFF");

   variable_item_list_add(nfc_playlist->variable_item_list, "Reset settings", 0, NULL, NULL);

   VariableItem* credits = variable_item_list_add(nfc_playlist->variable_item_list, "acegoal07", 1, NULL, NULL);
   variable_item_set_current_value_text(credits, "Credits");

   variable_item_list_set_enter_callback(nfc_playlist->variable_item_list, nfc_playlist_settings_menu_callback, nfc_playlist);

   furi_string_free(temp_str);

   view_dispatcher_switch_to_view(nfc_playlist->view_dispatcher, NfcPlaylistView_Settings);
}

bool nfc_playlist_settings_scene_on_event(void* context, SceneManagerEvent event) {
   NfcPlaylist* nfc_playlist = context;
   bool consumed = false;
   if (event.type == SceneManagerEventTypeCustom) {
      switch(event.event) {
         case NfcPlaylistSettings_Reset:
            FuriString* temp_str = furi_string_alloc();

            nfc_playlist->settings.emulate_timeout = default_emulate_timeout;
            VariableItem* emulation_timeout_settings = variable_item_list_get(nfc_playlist->variable_item_list, NfcPlaylistSettings_Timeout);
            variable_item_set_current_value_index(emulation_timeout_settings, nfc_playlist->settings.emulate_timeout);
            furi_string_printf(temp_str, "%ds", options_emulate_timeout[nfc_playlist->settings.emulate_timeout]);
            variable_item_set_current_value_text(emulation_timeout_settings, furi_string_get_cstr(temp_str));

            nfc_playlist->settings.emulate_delay = default_emulate_delay;
            VariableItem* emulation_delay_settings = variable_item_list_get(nfc_playlist->variable_item_list, NfcPlaylistSettings_Delay);
            variable_item_set_current_value_index(emulation_delay_settings, nfc_playlist->settings.emulate_delay);
            furi_string_printf(temp_str, "%ds", options_emulate_delay[nfc_playlist->settings.emulate_delay]);
            variable_item_set_current_value_text(emulation_delay_settings, furi_string_get_cstr(temp_str));

            nfc_playlist->settings.emulate_led_indicator = default_emulate_led_indicator;
            VariableItem* emulation_led_indicator_settings = variable_item_list_get(nfc_playlist->variable_item_list, NfcPlaylistSettings_LedIndicator);
            variable_item_set_current_value_index(emulation_led_indicator_settings, nfc_playlist->settings.emulate_led_indicator);
            variable_item_set_current_value_text(emulation_led_indicator_settings, nfc_playlist->settings.emulate_led_indicator ? "ON" : "OFF");

            furi_string_free(temp_str);
            consumed = true;
            break;
         default:
            break;
      }
   }
   return consumed;
}

void nfc_playlist_settings_scene_on_exit(void* context) {
   NfcPlaylist* nfc_playlist = context;
   variable_item_list_reset(nfc_playlist->variable_item_list);
}