#include <pulse/pulseaudio.h>

#include "volume.h"

// PulseAudio main loop and context
static pa_threaded_mainloop *mainloop = NULL;
static pa_context *context = NULL;
void sink_info_callback(pa_context *context, const pa_sink_info *info, int eol, void *userdata);
// Function to retrieve the current volume
float
get_current_volume() {
    pa_operation *op;
    pa_cvolume volume;
    
    // Get the default sink
    op = pa_context_get_sink_info_by_name(context, NULL, sink_info_callback, &volume);
    pa_operation_unref(op);
    
    // Wait for the operation to complete
    while (pa_operation_get_state(op) != PA_OPERATION_DONE) {
        pa_threaded_mainloop_wait(mainloop);
    }
    
    // Return the average volume across channels
    return (float)pa_cvolume_avg(&volume) / PA_VOLUME_NORM;
}
// Function to set the volume
void set_volume(float volume) {
    pa_operation *op;
    pa_cvolume new_volume;
    
    // Get the default sink info
    op = pa_context_get_sink_info_by_name(context, NULL, sink_info_callback, &new_volume);
    
    if (op != NULL) {
        // Wait for the operation to complete
        while (pa_operation_get_state(op) == PA_OPERATION_RUNNING) {
            pa_threaded_mainloop_wait(mainloop);
        }
        
        pa_operation_unref(op);
        
        // Set the new volume on all channels
        pa_cvolume_set(&new_volume, new_volume.channels, (uint32_t)(volume * PA_VOLUME_NORM));
        
        // Get the default sink name
        const char *sink_name = NULL;
        if (pa_context_get_state(context) == PA_CONTEXT_READY) {
            pa_operation *op_info = pa_context_get_sink_info_list(context, sink_info_callback, &sink_name);
            if (op_info != NULL) {
                while (pa_operation_get_state(op_info) == PA_OPERATION_RUNNING) {
                    pa_threaded_mainloop_wait(mainloop);
                }
                pa_operation_unref(op_info);
            }
        }
        
        // Apply the new volume to the sink
        if (sink_name != NULL) {
            op = pa_context_set_sink_volume_by_name(context, sink_name, &new_volume, NULL, NULL);
            
            if (op != NULL) {
                // Wait for the operation to complete
                while (pa_operation_get_state(op) == PA_OPERATION_RUNNING) {
                    pa_threaded_mainloop_wait(mainloop);
                }
                
                pa_operation_unref(op);
            }
        }
    }
}
// Function to adjust the volume based on input
void 
adjust_volume(int input) {
    float current_volume = get_current_volume();
    
    if (input == -1) {
        // Decrease volume by 0.01
        set_volume(current_volume - 0.01);
    } else if (input == 1) {
        // Increase volume by 0.01
        set_volume(current_volume + 0.01);
    } else if (input == 0) {
        // Mute the volume
        set_volume(0.0);
    }
}

// Callback function for sink info
void 
sink_info_callback(pa_context *context, const pa_sink_info *info, int eol, void *userdata) {
    if (eol < 0) {
        // Failed to get sink info
        return;
    }
    
    if (info) {
        // Copy the volume information
        pa_cvolume *volume = (pa_cvolume *)userdata;
        *volume = info->volume;
    }
}

// Function to initialize PulseAudio
void 
initialize_pulseaudio() {
    // Create a new main loop
    mainloop = pa_threaded_mainloop_new();
    pa_threaded_mainloop_start(mainloop);
    
    // Create a new context
    context = pa_context_new(pa_threaded_mainloop_get_api(mainloop), "Volume Control");
    pa_context_connect(context, NULL, 0, NULL);
    
    // Wait for the context to be ready
    while (pa_context_get_state(context) != PA_CONTEXT_READY) {
        pa_threaded_mainloop_wait(mainloop);
    }
}

// Function to cleanup PulseAudio
void 
cleanup_pulseaudio() {
    // Disconnect the context
    pa_context_disconnect(context);
    pa_context_unref(context);
    
    // Stop and free the main loop
    pa_threaded_mainloop_stop(mainloop);
    pa_threaded_mainloop_free(mainloop);
}
