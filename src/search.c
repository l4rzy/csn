/*
 * Copyright (C) 2019 l4rzy
 * MIT License
 */

#include "internal.h"

// pointer to result
static csn_result_t *temp;

static csn_result_t *parse_music_entry(jo_t *obj, int music_type) {
    csn_result_t *ret = csn_result_new(TYPE_MUSIC);
    ret->base->type = music_type;



}

static csn_result_t *parse_search_music(jo_t *obj, int options, int music_type) {
    csn_result_t *music_result = NULL; // a pointer that points to result

    switch (music_type) {
    case MUSIC_SONG:
        if (!(options & SEARCH_SONG)) {
            return NULL;
        }
        logs("Parsing song\n");
        break;
    case MUSIC_BEAT:
        if (!(options & SEARCH_BEAT)) {
            return NULL;
        }
        logs("Parsing beat\n");
        break;
    case MUSIC_VIDEO:
        if (!(options & SEARCH_VIDEO)) {
            return NULL;
        }
        logs("Parsing video\n");
        break;
    }

    // to the data field
    jo_t *data;
    int err = json_object_object_get_ex(obj, "data", &data);

    // data is now an array of song
    int len = json_object_array_length(data);
    for (int i = 0; i < len; ++i) {
        temp = parse_music_entry(json_object_array_get_idx(data, i), type);
        if (temp) {
            if (music_result == NULL) {
                music_result = temp;
            }
            else {
                music_result->next = temp;
            }
        }
        else {
            logs("Entry returns nothing\n");
        }
    }

    return music_result;
}

static csn_result_t *parse_search_artist(jo_t *obj, int options) {
    if (!(options & SEARCH_ARTIST)) {
        return NULL;
    }

    jo_t *data;

    // to the data field
    bool err = json_object_object_get_ex(obj, "data", &data);
    if (err) {
        logs("Could not get field `data`\n");
        return NULL;
    }

    // iter in field data
    jo_iter_t iter, iter_end;
    iter = json_object_iter_begin(data);
    iter_end = json_object_iter_end(data);

    while (!json_object_iter_equal(&iter, &iter_end)) {
        logf("%s\n", json_object_iter_peek_name(&iter));
        json_object_iter_next(&iter);
    }
}

static csn_result_t *parse_search_album(jo_t *obj, int options) {
    if (!(options & SEARCH_ARTIST)) {
        return NULL;
    }
}

csn_result_t *parse_search_result(buf_t *docbuf, int options) {
    csn_result_t *result = NULL; // points to head of result
    csn_result_t *ptr = NULL; // points to tail of result

    jt_t *jt = json_tokener_new();
    jo_t *jo = json_tokener_parse_ex(jt, docbuf->str, docbuf->len);
    jo_t *data;

    if (!jo) {
        logs("Could not parse returned json\n");
        return NULL;
    }
    logs("Parsed successfully\n");

    if (json_object_get_type(jo) != json_type_array) {
        goto _parse_err;
    }

    jo_t *jres = json_object_array_get_idx(jo, 0);
    if (json_object_get_type(jres) != json_type_object) {
        goto _parse_err;
    }

    result = NULL;

    // iter over categories and get data
    jo_iter_t iter, iter_end;
    iter = json_object_iter_begin(jres);
    iter_end = json_object_iter_end(jres);

#define IS(str) \
    !strcmp(str, json_object_iter_peek_name(&iter))

    while (!json_object_iter_equal(&iter, &iter_end)) {
        if (IS(CSN_S_SEARCH_CAT_SONG)) {
            data = json_object_iter_peek_value(&iter);
            parse_search_music(data, options, MUSIC_SONG);
        }
        if (IS(CSN_S_SEARCH_CAT_BEAT)) {
            data = json_object_iter_peek_value(&iter);
            parse_search_music(data, options, MUSIC_BEAT);
        }
        if (IS(CSN_S_SEARCH_CAT_VIDEO)) {
            data = json_object_iter_peek_value(&iter);
            parse_search_music(data, options, MUSIC_VIDEO);
        }
        if (IS(CSN_S_SEARCH_CAT_ARTIST)) {
            data = json_object_iter_peek_value(&iter);
        }
        if (IS(CSN_S_SEARCH_CAT_ALBUM)) {
            data = json_object_iter_peek_value(&iter);
        }
        json_object_iter_next(&iter);
    }

    return result;
#undef IS
    json_object_put(jo);

_parse_err:

    return NULL;
}
