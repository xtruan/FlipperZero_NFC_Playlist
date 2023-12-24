#include "nfc_playlist.h"
#include "scences/emulation.h"

bool cancel = false;

void nfc_playlist_emulation_scene_on_enter(void* context) {
    NfcPlaylist* nfc_playlist = context;
    nfc_playlist_emulation_setup(nfc_playlist);
    nfc_playlist_emulation_start(nfc_playlist);
}

bool nfc_playlist_emulation_scene_on_event(void* context, SceneManagerEvent event) {
    NfcPlaylist* nfc_playlist = context;
    UNUSED(nfc_playlist);
    FURI_LOG_RAW_I("nfc_playlist_emulation_scene_on_event: %ld", event.event);
    switch (event.event) {
        case 0:
            if (nfc_playlist_worker_is_emulating(nfc_playlist->nfc_playlist_worker) && cancel != true) {
                cancel = true;
                return true;
            }
        default:
            break;
    }

    return false;
}

void nfc_playlist_emulation_scene_on_exit(void* context) {
    NfcPlaylist* nfc_playlist = context;
    cancel = false;
    nfc_playlist_emulation_stop(nfc_playlist);
    nfc_playlist_emulation_free(nfc_playlist);
    popup_reset(nfc_playlist->popup);
}

void nfc_playlist_emulation_setup(void* context) {
    NfcPlaylist* nfc_playlist = context;

    nfc_playlist->thread = furi_thread_alloc_ex(
        "NfcPlaylistEmulationWorker", 8192, nfc_playlist_emulation_task, nfc_playlist);
    nfc_playlist->nfc_playlist_worker = nfc_playlist_worker_alloc();
}

void nfc_playlist_emulation_free(NfcPlaylist* nfc_playlist) {
    furi_assert(nfc_playlist);
    furi_thread_free(nfc_playlist->thread);
    nfc_playlist_worker_free(nfc_playlist->nfc_playlist_worker);
    nfc_playlist->thread = NULL;
    nfc_playlist->nfc_playlist_worker = NULL;
}

void nfc_playlist_emulation_start(NfcPlaylist* nfc_playlist) {
    furi_assert(nfc_playlist);
    furi_thread_start(nfc_playlist->thread);
}

void nfc_playlist_emulation_stop(NfcPlaylist* nfc_playlist) {
    furi_assert(nfc_playlist);
    furi_thread_join(nfc_playlist->thread);
}

int32_t nfc_playlist_emulation_task(void* context) {
    NfcPlaylist* nfc_playlist = context;
    // open/alloc resources
    Storage* storage = furi_record_open(RECORD_STORAGE);
    Stream* stream = file_stream_alloc(storage);
    FuriString* line = furi_string_alloc();

    popup_reset(nfc_playlist->popup);
    popup_set_context(nfc_playlist->popup, nfc_playlist);
    view_dispatcher_switch_to_view(nfc_playlist->view_dispatcher, NfcPlaylistView_Popup);

    // Read file
    if(file_stream_open(stream, APP_DATA_PATH("playlist.txt"), FSAM_READ, FSOM_OPEN_EXISTING)) {

        int file_position = 0;
        // read the file line by line and print the text
        while(stream_read_line(stream, line) && cancel == false) {
            if (options_emulate_delay[nfc_playlist->emulate_delay] > 0) {
                if (file_position > 0) {
                    popup_set_header(nfc_playlist->popup, "Delaying", 64, 10, AlignCenter, AlignTop);
                    start_error_blink(nfc_playlist);
                    int time_counter_delay_ms = (options_emulate_delay[nfc_playlist->emulate_delay] * 1000);
                    do {
                        char display_text[10];
                        snprintf(display_text, 10, "%ds", (time_counter_delay_ms/1000));
                        popup_set_text(nfc_playlist->popup, display_text, 64, 50, AlignCenter, AlignTop);
                        furi_delay_ms(500);
                        time_counter_delay_ms -= 500;
                    } while(time_counter_delay_ms > 0 && cancel == false);
                } else {
                    file_position++;
                }
            }

            char* file_path = (char*)furi_string_get_cstr(line);
            char* file_name;
            if (strchr(file_path, '/') != NULL) {
                file_name = &strrchr(file_path, '/')[1];
            } else {
                file_name = file_path;
            }
            // char* file_ext = &strrchr(file_path, '.')[1];
            int time_counter_ms = (options_emulate_timeout[nfc_playlist->emulate_timeout] * 1000);

            if (storage_file_exists(storage, file_path) == false) {
                char popup_header_text[80];
                snprintf(popup_header_text, 80, "%s\n%s", "ERROR not found:", file_name);
                popup_set_header(nfc_playlist->popup, popup_header_text, 64, 10, AlignCenter, AlignTop);
                start_error_blink(nfc_playlist);
                do {
                    char popup_text[10];
                    snprintf(popup_text, 10, "%ds", (time_counter_ms/1000));
                    popup_set_text(nfc_playlist->popup, popup_text, 64, 50, AlignCenter, AlignTop);
                    furi_delay_ms(500);
                    time_counter_ms -= 500;
                } while(time_counter_ms > 0);
            } 

            // else if (strstr(file_ext, (char*)'nfc') == NULL) {
            //     char popup_header_text[80];
            //     snprintf(popup_header_text, 80, "%s\n%s", "ERROR invalid file:", file_name);
            //     popup_set_header(nfc_playlist->popup, popup_header_text, 64, 10, AlignCenter, AlignTop);
            //     start_error_blink(nfc_playlist);
            //     do {
            //         char popup_text[10];
            //         snprintf(popup_text, 10, "%ds", (time_counter_ms/1000));
            //         popup_set_text(nfc_playlist->popup, popup_text, 64, 50, AlignCenter, AlignTop);
            //         furi_delay_ms(500);
            //         time_counter_ms -= 500;
            //     } while(time_counter_ms > 0);
            // }
            
            else {
                char popup_header_text[80];
                snprintf(popup_header_text, 80, "%s\n%s", "Emulating:", file_name);
                popup_set_header(nfc_playlist->popup, popup_header_text, 64, 10, AlignCenter, AlignTop);
                nfc_playlist_worker_set_nfc_data(nfc_playlist->nfc_playlist_worker, file_path);
                nfc_playlist_worker_start(nfc_playlist->nfc_playlist_worker);
                start_normal_blink(nfc_playlist);
                do {
                    char popup_text[10];
                    snprintf(popup_text, 10, "%ds", (time_counter_ms/1000));
                    popup_set_text(nfc_playlist->popup, popup_text, 64, 50, AlignCenter, AlignTop);
                    furi_delay_ms(500);
                    time_counter_ms -= 500;
                } while(nfc_playlist_worker_is_emulating(nfc_playlist->nfc_playlist_worker) && time_counter_ms > 0 && cancel == false);
                nfc_playlist_worker_stop(nfc_playlist->nfc_playlist_worker);
            }
        }
        popup_reset(nfc_playlist->popup);
        popup_set_header(nfc_playlist->popup, "Emulation finished", 64, 10, AlignCenter, AlignTop);
        popup_set_text(nfc_playlist->popup, "Press back", 64, 25, AlignCenter, AlignTop);
        stop_blink(nfc_playlist);
    } else {
        popup_set_header(nfc_playlist->popup, "Error:", 64, 10, AlignCenter, AlignTop);
        popup_set_text(nfc_playlist->popup, "Failed to open file\n/ext/apps_data/nfc_playlist/playlist.txt", 64, 25, AlignCenter, AlignTop);
    }
    // Free/close resources
    furi_string_free(line);
    file_stream_close(stream);
    stream_free(stream);
    // Close storage    
    return 0;
}