#include "engine/packets.h"
#include "build/packet.h"
#include "parse/packet.h"
#include <stdio.h>
#include <stdlib.h>
#include "h/fds.h"
#include "h/mem.h"
#include "h/packet.h"

#define MOTD "{\
    'version': {\
        'name': '1.21.8',\
        'protocol': 772\
    },\
    'players': {\
        'max': 20,\
        'online': 1,\
        'sample': [\
            {\
                'name': 'thinkofdeath',\
                'id': '4566e69f-c907-48ee-8d71-d7ba5aa00d20'\
			}\
        ]\
    },\
    'description': {\
        'text': 'Hello, world!'\
    },\
    'favicon': 'data:image/png;base64,<data>',\
    'enforcesSecureChat': false\
 }"

int process_packet(Packet* packet) {
    print_readable(packet->buf);
    
    int fd = packet->from;

    char *_ = fds_get(fd, "intent");
    int intent = -1;
    if (_) intent = *_;
    Buffer* buf = NULL;

    int error = 0;

    char *ptr;

    switch (intent) {
    case 1:
        buf = init_buffer();
        if (!parse_varint(packet->buf, &error)) {
            build_varint(buf, 0x00);
            build_string(buf, MOTD);            
        } else {
            build_varint(buf, 0x01);
            build_integer(buf, parse_integer(packet->buf, 8, 1, &error), 8, 1);
        }
        if (error) {
            free_buffer(buf);
            buf = NULL;
            close_connection(packet->from);
        }
        break;

    case 2:    
        buf = init_buffer();
        build_varint(buf, 0x00);
        build_string(buf, "{'text':'test'}");
        break;

    case 3:    
        buf = init_buffer();
        build_varint(buf, 0x00);
        build_string(buf, "{'text':'Transfers are not supported!!!','color':'red'}");
        break;
    
    default:
        if (parse_varint(packet->buf, &error) || error) return 1;
        parse_varint(packet->buf, &error);
        free(parse_string(packet->buf, 255, &error));
        parse_integer(packet->buf, 2, 0, &error);
        intent = parse_varint(packet->buf, &error);
        if (error || intent<1 || intent>3 ) return 2;

        ptr = malloc(sizeof(char));
        *ptr = intent;

        mem_add(fd, ptr);
        fds_set(fd, "intent", ptr);
        break;
    }

    if (buf) {
        packet_send(buf, fd);
        free_buffer(buf);
    }

    return 0;
}
