#ifndef WRAPPER_H_
#define WRAPPER_H_


#ifdef __cplusplus
	extern "C" {
#endif

#define PMIDI_STATUS_END        0
#define PMIDI_STATUS_VERSION    1
#define PMIDI_STATUS_TEXT       2
#define PMIDI_STATUS_KEY        3
#define PMIDI_STATUS_TIMESIG    4
#define PMIDI_STATUS_TEMPO      5
#define PMIDI_STATUS_NOTE       6
#define PMIDI_STATUS_DUMMY      7
#define PMIDI_STATUS_ROOT       8
#define PMIDI_STATUS_CONTROL    9
#define PMIDI_STATUS_PROGRAM    10
#define PMIDI_STATUS_PITCH      11
#define PMIDI_STATUS_PRESSURE   12
#define PMIDI_STATUS_KEYTOUCH   13
#define PMIDI_STATUS_SYSEX      14
#define PMIDI_STATUS_SMPTEOFFS  15


extern int pmidi_wrapper_status;
extern int pmidi_open_midi_file( const char *fileName );
extern int pmidi_parse_midi_file( char** p, int *t, int *a, int *b, int *c, int *d);

#ifdef __cplusplus
	}
#endif

#endif /* WRAPPER_H_ */
