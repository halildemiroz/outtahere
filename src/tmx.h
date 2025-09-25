#ifndef TMX_H
#define TMX_H

// TMX library stub to reproduce the compilation error
typedef struct tmx_map {
    // Empty structure for now
    int dummy;
} tmx_map;

// This function is missing from the library - this will cause the compilation error
// void tmx_set_error_handler(void (*handler)(const char *));

tmx_map* tmx_load(const char *filename);
void tmx_map_free(tmx_map *map);

#endif /* TMX_H */