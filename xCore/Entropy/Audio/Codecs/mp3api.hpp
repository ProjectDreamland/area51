#ifndef MP3API_HPP
#define MP3API_HPP

#include "audio/codecs/mp3dec.h"

ASIRESULT AILEXPORT ASI_startup     (void);
ASIRESULT AILEXPORT ASI_shutdown    (void);

HASISTREAM   AILEXPORT ASI_stream_open (U32           user,  //)
                                        AILASIFETCHCB fetch_CB,  
                                        U32           total_size);

S32 AILEXPORT ASI_stream_process (HASISTREAM  stream, //)
                                  void        *buffer,
                                  S32         request_size);


ASIRESULT AILEXPORT ASI_stream_close(HASISTREAM stream);

#endif // MP3API_HPP