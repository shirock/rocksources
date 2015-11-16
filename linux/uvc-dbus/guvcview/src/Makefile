OBJS = guvcview.o globals.o options.o \
  marker.o ms_time.o dct.o huffman.o \
  create_image.o jpgenc.o picture.o quant.o readYUV.o \
  string_utils.o utils.o \
  autofocus.o colorspaces.o \
  v4l2_controls.o v4l2_devices.o v4l2_dyna_ctrls.o v4l2_formats.o v4l2uvc.o \
  video.o video_filters.o \
  socket_cmd.o

build: libguvcview

libguvcview: $(OBJS)
	gcc -shared -O2 -o libguvcview $(OBJS) -lm \
  `pkg-config --libs glib-2.0 gthread-2.0 gtk+-2.0 sdl libpng12 libavcodec libavutil libv4l2`

.c.o:
	gcc -I. -I.. \
  `pkg-config --cflags glib-2.0 gthread-2.0 gtk+-2.0 sdl libpng12 libavcodec libavutil libv4l2` \
  -Wall -fPIC -O2 -c -o $@ $<

clean:
	rm -f *.o
	rm -f libguvcview-cli libguvcview