#pragma once
#include <furi.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/text_input.h>
#include <storage/storage.h>

void nfc_playlist_name_new_file_scene_on_enter(void* context);
bool nfc_playlist_name_new_file_scene_on_event(void* context, SceneManagerEvent event);
void nfc_playlist_name_new_file_scene_on_exit(void* context);