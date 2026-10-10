// Miles Sound System (AIL) Complete Stubs for Android ARM64 and x86_64
#include <windows.h>
#include <mss.h>
#include <stdlib.h>
#include <string.h>

extern "C" {

S32 AILCALL AIL_startup(void) { return 1; }
void AILCALL AIL_shutdown(void) {}
char FAR* AILCALL AIL_set_redist_directory(char const FAR *dir) { return (char FAR*)dir; }
void FAR * AILCALL AIL_file_read(char const FAR *filename, void FAR *dest) { return NULL; }
S32 AILCALL AIL_file_type(void const FAR *data, U32 size) { return 0; }
S32 AILCALL AIL_WAV_info(void const FAR *data, AILSOUNDINFO FAR *info) { return 0; }
S32 AILCALL AIL_decompress_ADPCM(AILSOUNDINFO const FAR *info, void FAR * FAR *out_data, U32 FAR *out_size) { return 0; }
S32 AILCALL AIL_decompress_ASI(void const FAR *data, U32 in_size, char const FAR *filename_ext, void FAR * FAR *out_data, U32 FAR *out_size, AILLENGTHYCB callback) { return 0; }
void AILCALL AIL_mem_free_lock(void FAR *ptr) { if (ptr) free(ptr); }
void AILCALL AIL_set_file_callbacks(AIL_file_open_callback cb1, AIL_file_close_callback cb2, AIL_file_seek_callback cb3, AIL_file_read_callback cb4) {}

char FAR * AILCALL AIL_last_error(void) { static char s_err[] = ""; return s_err; }

HDIGDRIVER AILCALL AIL_open_digital_driver(U32 frequency, S32 bits, S32 channels, U32 flags) {
    return (HDIGDRIVER)1;
}
void AILCALL AIL_close_digital_driver(HDIGDRIVER dig) {}

HSAMPLE AILCALL AIL_allocate_sample_handle(HDIGDRIVER dig) {
    return (HSAMPLE)calloc(1, 64);
}
void AILCALL AIL_release_sample_handle(HSAMPLE S) {
    if (S) free(S);
}
void AILCALL AIL_init_sample(HSAMPLE S) {}
S32 AILCALL AIL_set_sample_file(HSAMPLE S, void const FAR *file_image, S32 block) { return 1; }
void AILCALL AIL_start_sample(HSAMPLE S) {}
void AILCALL AIL_stop_sample(HSAMPLE S) {}
void AILCALL AIL_resume_sample(HSAMPLE S) {}
void AILCALL AIL_end_sample(HSAMPLE S) {}
U32 AILCALL AIL_sample_status(HSAMPLE S) { return 0; }
void AILCALL AIL_set_sample_volume_pan(HSAMPLE S, F32 volume, F32 pan) {}
void AILCALL AIL_sample_volume_pan(HSAMPLE S, F32 FAR *volume, F32 FAR *pan) {
    if (volume) *volume = 1.0f;
    if (pan) *pan = 0.5f;
}
void AILCALL AIL_set_sample_loop_count(HSAMPLE S, S32 loop_count) {}

// 3D Provider & Listener
S32 AILCALL AIL_enumerate_3D_providers(HPROENUM FAR *next, HPROVIDER FAR *dest, C8 FAR * FAR *name) {
    return 0;
}
M3DRESULT AILCALL AIL_open_3D_provider(HPROVIDER lib) { return (M3DRESULT)0; }
void AILCALL AIL_close_3D_provider(HPROVIDER lib) {}
H3DPOBJECT AILCALL AIL_open_3D_listener(HPROVIDER lib) { return (H3DPOBJECT)1; }
void AILCALL AIL_close_3D_listener(H3DPOBJECT listener) {}

void AILCALL AIL_set_3D_position(H3DPOBJECT obj, F32 X, F32 Y, F32 Z) {}
void AILCALL AIL_set_3D_velocity(H3DPOBJECT obj, F32 dX_dt, F32 dY_dt, F32 dZ_dt, F32 magnitude) {}
void AILCALL AIL_set_3D_orientation(H3DPOBJECT obj, F32 X_face, F32 Y_face, F32 Z_face, F32 X_up, F32 Y_up, F32 Z_up) {}
void AILCALL AIL_update_3D_position(H3DPOBJECT obj, F32 dt) {}
void AILCALL AIL_auto_update_3D_position(H3DPOBJECT obj, S32 auto_update_status) {}

// 3D Samples
H3DSAMPLE AILCALL AIL_allocate_3D_sample_handle(HPROVIDER lib) {
    return (H3DSAMPLE)calloc(1, 64);
}
void AILCALL AIL_release_3D_sample_handle(H3DSAMPLE S) {
    if (S) free(S);
}
S32 AILCALL AIL_set_3D_sample_file(H3DSAMPLE S, void const FAR *file_image) { return 1; }
void AILCALL AIL_start_3D_sample(H3DSAMPLE S) {}
void AILCALL AIL_stop_3D_sample(H3DSAMPLE S) {}
void AILCALL AIL_resume_3D_sample(H3DSAMPLE S) {}
void AILCALL AIL_end_3D_sample(H3DSAMPLE S) {}
U32 AILCALL AIL_3D_sample_status(H3DSAMPLE S) { return 0; }
void AILCALL AIL_set_3D_sample_volume(H3DSAMPLE S, F32 volume) {}
F32 AILCALL AIL_3D_sample_volume(H3DSAMPLE S) { return 1.0f; }
void AILCALL AIL_set_3D_sample_loop_count(H3DSAMPLE S, U32 count) {}

// Streams
HSTREAM AILCALL AIL_open_stream(HDIGDRIVER dig, char const FAR *filename, S32 stream_mem) {
    return (HSTREAM)calloc(1, 64);
}
void AILCALL AIL_close_stream(HSTREAM stream) {
    if (stream) free(stream);
}
void AILCALL AIL_start_stream(HSTREAM stream) {}
void AILCALL AIL_pause_stream(HSTREAM stream, S32 onoff) {}
S32 AILCALL AIL_stream_status(HSTREAM stream) { return 0; }
void AILCALL AIL_set_stream_volume_levels(HSTREAM stream, F32 left_level, F32 right_level) {}
void AILCALL AIL_stream_volume_levels(HSTREAM stream, F32 FAR *left_level, F32 FAR *right_level) {
    if (left_level) *left_level = 1.0f;
    if (right_level) *right_level = 1.0f;
}
void AILCALL AIL_set_stream_loop_count(HSTREAM stream, S32 count) {}

} // extern "C"
