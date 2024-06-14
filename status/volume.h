#ifndef VOLUME_H
#define VOLUME_H

#define VOLUME_STEP 0.01

void initialize_pulseaudio(void);
void cleanup_pulseaudio(void);

void adjust_volume(int);
float get_current_volume(void);

#endif

