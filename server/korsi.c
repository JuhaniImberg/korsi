#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

#include <pulse/stream.h>
#include <pulse/sample.h>
#include <pulse/context.h>
#include <pulse/introspect.h>
#include <pulse/thread-mainloop.h>

const static char *SINK_NAME = "alsa_output.pci-0000_00_1b.0.analog-stereo";
const static int METER_RATE = 120;

int ardu;

void stream_read(pa_stream* stream, size_t  length, void *userdata) {
    const void *data;
    pa_stream_peek(stream, &data, &length);
    write(ardu, data, sizeof(char) * length);
    pa_stream_drop(stream);
}

void sink_info(pa_context *c, const pa_sink_info *sink,
               int eol, void *userdata) {
    if (sink == NULL) {
        return;
    }
    if (strcmp(sink->name, SINK_NAME) == 0) {
        printf("Sink found\n");
        pa_sample_spec sample_spec = {
            .channels = 1,
            .format = PA_SAMPLE_U8,
            .rate = METER_RATE
        };
        pa_stream* stream = pa_stream_new(c, "korsi",
                                          &sample_spec, NULL);
        pa_stream_set_read_callback(stream, stream_read, NULL);
        pa_stream_connect_record(stream, sink->monitor_source_name,
                                 NULL, PA_STREAM_PEAK_DETECT);
    }
}

void context_notify(pa_context *c, void *userdata) {
    pa_context_state_t state = pa_context_get_state(c);
    switch(state) {
    case PA_CONTEXT_READY:
        printf("Connected\n");
        pa_operation* o = pa_context_get_sink_info_list(c, sink_info, NULL);
        pa_operation_unref(o);
        break;
    case PA_CONTEXT_FAILED:
        fprintf(stderr, "Connection failed\n");
        break;
    case PA_CONTEXT_TERMINATED:
        fprintf(stderr, "Connection terminated\n");
        break;
    default:
        break;
    }

}

void init_ardu(const char *file) {
    struct termios termios_options;
    ardu = open(file, O_RDWR, O_NONBLOCK);
    if(ardu == -1) {
        fprintf(stderr, "Unable to open Arduino for writing\n");
        exit(EXIT_FAILURE);
    }
    if(tcgetattr(ardu, &termios_options) < 0) {
        fprintf(stderr, "Unable to open Arduino for writing\n");
        exit(EXIT_FAILURE);
    }
    cfsetispeed(&termios_options, B115200);
    cfsetospeed(&termios_options, B115200);

    termios_options.c_cflag &= ~PARENB;
    termios_options.c_cflag &= ~CSTOPB;
    termios_options.c_cflag &= ~CSIZE;
    termios_options.c_cflag |= CS8;
    termios_options.c_cflag &= ~020000000000;
    termios_options.c_cflag |= CREAD | CLOCAL;
    termios_options.c_cflag &= ~(IXON | IXOFF | IXANY);
    termios_options.c_cflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    termios_options.c_cflag &= ~OPOST;

    termios_options.c_cc[VMIN] = 0;
    termios_options.c_cc[VTIME] = 0;

    tcsetattr(ardu, TCSANOW, &termios_options);
    if(tcsetattr(ardu, TCSAFLUSH, &termios_options) < 0) {
        fprintf(stderr, "Unable to configure serial port\n");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv) {
    if(argc < 2) {
        fprintf(stderr, "You need to give me the serial tty!\n");
        return -1;
    }
    init_ardu(argv[1]);

    pa_threaded_mainloop *mainloop = pa_threaded_mainloop_new();
    pa_mainloop_api *api = pa_threaded_mainloop_get_api(mainloop);
    pa_context* context = pa_context_new(api, "korsi");
    pa_context_set_state_callback(context, context_notify, NULL);
    pa_context_connect(context, NULL, 0, NULL);

    pa_threaded_mainloop_start(mainloop);

    char c;
    scanf("%c", &c);

    pa_threaded_mainloop_stop(mainloop);
    pa_threaded_mainloop_free(mainloop);
}
