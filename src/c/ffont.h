
#pragma once
#include "fctx.h"

typedef struct __attribute__((__packed__)) FFont {
    fixed16_t units_per_em;
    fixed16_t ascent;
    fixed16_t descent;
    fixed16_t cap_height;
    uint16_t glyph_index_length;
    uint16_t glyph_table_length;
} FFont;

typedef struct __attribute__((__packed__)) FGlyphRange {
    uint16_t begin;
    uint16_t end;
} FGlyphRange;

typedef struct __attribute__((__packed__)) FGlyph {
    uint16_t path_data_offset;
    uint16_t path_data_length;
    fixed16_t horiz_adv_x;
} FGlyph;

FFont* ffont_create_from_resource(uint32_t resource_id);
void ffont_destroy(FFont* font);
#if 0
void ffont_debug_log(FFont* font, uint8_t log_level);
#endif
FGlyph* ffont_glyph_info(FFont* font, uint16_t unicode);
void* ffont_glyph_outline(FFont* font, FGlyph* glyph);

/**
 * Decode the next byte of a UTF-8 byte stream.
 * Initialize state to 0 the before calling this function for the first
 * time for a given stream.  If the returned value is 0, then cp has been
 * set to a valid code point.  Other return values indicate that a multi-
 * byte sequence is in progress, or there was a decoding error.
 *
 * @param byte the byte to decode.
 * @state the current state of the decoder.
 * @cp the decoded unitcode code point.
 * @return the state of the decode process after decoding the byte.
 */
uint16_t utf8_decode_byte(uint8_t byte, uint16_t* state, uint16_t* cp);
