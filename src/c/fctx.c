
#include "fctx.h"
#include "ffont.h"
#include <stdlib.h>


/*
 * Credit where credit is due:
 *
 * The functions fceil, floorDivMod, edge_init, and edge_step
 * are derived from Chris Hecker's "Perspective Texture Mapping"
 * series of articles in Game Developer Magazine (1995).  See
 * http://chrishecker.com/Miscellaneous_Technical_Articles
 *
 * The functions fpath_plot_edge_aa and fpath_end_fill_aa are derived
 * from:
 *   Scanline edge-flag algorithm for antialiasing
 *   Copyright (c) 2005-2007 Kiia Kallio <kkallio@uiah.fi>
 *   http://mlab.uiah.fi/~kkallio/antialiasing/
 *
 * The Edge Flag algorithm as used in both the black & white and
 * antialiased rendering functions here was presented by Bryan D. Ackland
 * and Neil H. Weste in "The Edge Flag Algorithm-A Fill Method for
 * Raster Scan Displays" (January 1981).
 *
 * The function countBits is Brian Kernighan's alorithm as presented
 * on Sean Eron Anderson's Bit Twiddling Hacks page at
 * http://graphics.stanford.edu/~seander/bithacks.html
 *
 * The bezier function is derived from Łukasz Zalewski's blog post
 * "Bezier Curves and GPaths on Pebble"
 * https://developer.getpebble.com//blog/2015/02/13/Bezier-Curves-And-GPaths/
 *
 */

#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}

#define MAX_ANGLE_TOLERANCE ((TRIG_MAX_ANGLE / 360) * 5)

bool checkObject(void* obj, const char* objname) {
    if (!obj) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "NULL %s", objname);
        return false;
    }
    return true;
}

// --------------------------------------------------------------------------
// Drawing support that is shared between BW and AA.
// --------------------------------------------------------------------------

void floorDivMod(int32_t numerator, int32_t denominator, int32_t* floor, int32_t* mod ) {
    Assert(denominator > 0); // we assume it's positive
    if (numerator >= 0) {
        // positive case, C is okay
        *floor = numerator / denominator;
        *mod = numerator % denominator;
    } else {
        // numerator is negative, do the right thing
        *floor = -((-numerator) / denominator);
        *mod = (-numerator) % denominator;
        if (*mod) {
            // there is a remainder
            --*floor;
            *mod = denominator - *mod;
        }
    }
}

typedef struct Edge {
    int32_t x;
    int32_t xStep;
    int32_t numerator;
    int32_t denominator;
    int32_t errorTerm; // DDA info for x
    int32_t y;         // current y
    int32_t height;    // vertical count
} Edge;

int32_t edge_step(Edge* e) {
    e->x += e->xStep;
    ++e->y;
    --e->height;

    e->errorTerm += e->numerator;
    if (e->errorTerm >= e->denominator) {
        ++e->x;
        e->errorTerm -= e->denominator;
    }
    return e->height;
}

void fctx_begin_fill(FContext* fctx) {

    GRect bounds = gbitmap_get_bounds(fctx->flag_buffer);
    fctx->extent_max.x = INT_TO_FIXED(bounds.origin.x);
    fctx->extent_max.y = INT_TO_FIXED(bounds.origin.y);
    fctx->extent_min.x = INT_TO_FIXED(bounds.origin.x + bounds.size.w);
    fctx->extent_min.y = INT_TO_FIXED(bounds.origin.y + bounds.size.h);

    fctx->path_init_point.x = 0;
    fctx->path_init_point.y = 0;
    fctx->path_cur_point.x = 0;
    fctx->path_cur_point.y = 0;
}

void fctx_deinit_context(FContext* fctx) {
    if (fctx->gctx) {
        gbitmap_destroy(fctx->flag_buffer);
        fctx->gctx = NULL;
    }
}

void fctx_set_fill_color(FContext* fctx, GColor c) {
    fctx->fill_color = c;
}

void fctx_set_offset(FContext* fctx, FPoint offset) {
    fctx->transform_offset = offset;
}

// --------------------------------------------------------------------------
// BW - black and white drawing with 1 bit-per-pixel flag buffer.
// --------------------------------------------------------------------------

int32_t fceil(fixed_t value) {
    int32_t returnValue;
    int32_t numerator = value - 1 + FIXED_POINT_SCALE;
    if (numerator >= 0) {
        returnValue = numerator / FIXED_POINT_SCALE;
    } else {
        // deal with negative numerators correctly
        returnValue = -((-numerator) / FIXED_POINT_SCALE);
        returnValue -= ((-numerator) % FIXED_POINT_SCALE) ? 1 : 0;
    }
    return returnValue;
}

void edge_init(Edge* e, FPoint* top, FPoint* bottom) {

    e->y = fceil(top->y);
    int32_t yEnd = fceil(bottom->y);
    e->height = yEnd - e->y;
    if (e->height)    {
        int32_t dN = bottom->y - top->y;
        int32_t dM = bottom->x - top->x;
        int32_t initialNumerator = dM * 16 * e->y - dM * top->y +
        dN * top->x - 1 + dN * 16;
        floorDivMod(initialNumerator, dN*16, &e->x, &e->errorTerm);
        floorDivMod(dM*16, dN*16, &e->xStep, &e->numerator);
        e->denominator = dN*16;
    }
}

void fctx_init_context_bw(FContext* fctx, GContext* gctx) {

    GBitmap* frameBuffer = graphics_capture_frame_buffer(gctx);
    if (frameBuffer) {
        fctx->flag_bounds = gbitmap_get_bounds(frameBuffer);
        graphics_release_frame_buffer(gctx, frameBuffer);

        fctx->flag_buffer = gbitmap_create_blank(fctx->flag_bounds.size, GBitmapFormat1Bit);
        CHECK(fctx->flag_buffer);

        fctx->gctx = gctx;
        fctx->subpixel_adjust = -FIXED_POINT_SCALE / 2;
        fctx->transform_offset = FPointZero;
        fctx->transform_scale_from = FPointOne;
        fctx->transform_scale_to = FPointOne;
    }
}

void fctx_plot_edge_bw(FContext* fctx, FPoint* a, FPoint* b) {

    Edge edge;
    if (a->y > b->y) {
        edge_init(&edge, b, a);
    } else {
        edge_init(&edge, a, b);
    }

    uint8_t* data = gbitmap_get_data(fctx->flag_buffer);
    int16_t stride = gbitmap_get_bytes_per_row(fctx->flag_buffer);
    int16_t max_x = fctx->flag_bounds.size.w - 1;
    int16_t max_y = fctx->flag_bounds.size.h - 1;

    while (edge.height > 0 && edge.y < 0) {
        edge_step(&edge);
    }

    while (edge.height > 0 && edge.y <= max_y) {
        if (edge.x < 0) {
            uint8_t* p = data + edge.y * stride;
            *p ^= 1;
        } else if (edge.x <= max_x) {
            uint8_t* p = data + edge.y * stride + edge.x / 8;
            *p ^= (1 << (edge.x % 8));
        }
        edge_step(&edge);
    }
}

static inline void fctx_plot_point_bw(FContext* fctx, int16_t x, int16_t y) {
    int16_t max_y = fctx->flag_bounds.size.h - 1;
    if (y >= 0 && y < max_y) {
        uint8_t* data = gbitmap_get_data(fctx->flag_buffer);
        int16_t stride = gbitmap_get_bytes_per_row(fctx->flag_buffer);
        int16_t max_x = fctx->flag_bounds.size.w - 1;
        if (x < 0) {
            uint8_t* p = data + y * stride;
            *p ^= 1;
        } else if (x <= max_x) {
            uint8_t* p = data + y * stride + x / 8;
            *p ^= (1 << (x % 8));
        }
    }
}

void fctx_end_fill_bw(FContext* fctx) {

    uint8_t color;
#ifdef PBL_COLOR
    color = fctx->fill_color.argb;
#else
    uint8_t gray = 0;
    if (gcolor_equal(fctx->fill_color, GColorWhite)) {
        color = 0xff;
    } else if (gcolor_equal(fctx->fill_color, GColorBlack)) {
        color = 0x00;
    } else {
        gray = 0b01010101;
        color = gray;
    }
#endif

    int16_t rowMin = FIXED_TO_INT(fctx->extent_min.y);
    int16_t rowMax = FIXED_TO_INT(fctx->extent_max.y);
    int16_t colMin = FIXED_TO_INT(fctx->extent_min.x);
    int16_t colMax = FIXED_TO_INT(fctx->extent_max.x);

    if (rowMin < 0) rowMin = 0;
    if (rowMax >= fctx->flag_bounds.size.h) rowMax = fctx->flag_bounds.size.h - 1;

    GBitmap* fb = graphics_capture_frame_buffer(fctx->gctx);

    uint8_t* dest;
    uint8_t* src;
    uint8_t mask;
    int16_t col, row;

    for (row = rowMin; row <= rowMax; ++row) {
#ifdef PBL_BW
        if (gray) {
            if (row & 1) {
                color = gray;
            } else {
                color = ~gray;
            }
        }
#endif
        GBitmapDataRowInfo fbRowInfo = gbitmap_get_data_row_info(fb, row);
        GBitmapDataRowInfo flagRowInfo = gbitmap_get_data_row_info(fctx->flag_buffer, row);
        int16_t spanMin = (fbRowInfo.min_x > colMin) ? fbRowInfo.min_x : colMin;
        int16_t spanMax = (fbRowInfo.max_x < colMax) ? fbRowInfo.max_x : colMax;

        bool inside = false;
        for (col = spanMin; col <= spanMax; ++col) {

#ifdef PBL_COLOR
            dest = fbRowInfo.data + col;
#else
            dest = fbRowInfo.data + col / 8;
#endif
            src = flagRowInfo.data + col / 8;
            mask = 1 << (col % 8);
            if (*src & mask) {
                inside = !inside;
            }
            *src &= ~mask;
            if (inside) {
#ifdef PBL_COLOR
                *dest = color;
#else
                *dest = (color & mask) | (*dest & ~mask);
#endif
            }
        }
        if (col < flagRowInfo.max_x) {
            src = flagRowInfo.data + col / 8;
            mask = 1 << (col % 8);
            *src &= ~mask;
        }
    }

    graphics_release_frame_buffer(fctx->gctx, fb);

}

// --------------------------------------------------------------------------
// AA - anti-aliased drawing with 8 bit-per-pixel flag buffer.
// --------------------------------------------------------------------------

#ifdef PBL_COLOR

#define SUBPIXEL_COUNT 8
#define SUBPIXEL_SHIFT 3

#define FIXED_POINT_SHIFT_AA 1
#define FIXED_POINT_SCALE_AA 2
#define INT_TO_FIXED_AA(a) ((a) * FIXED_POINT_SCALE)
#define FIXED_TO_INT_AA(a) ((a) / FIXED_POINT_SCALE)
#define FIXED_MULTIPLY_AA(a, b) (((a) * (b)) / FIXED_POINT_SCALE_AA)

int32_t fceil_aa(fixed_t value) {
    int32_t returnValue;
    int32_t numerator = value - 1 + FIXED_POINT_SCALE_AA;
    if (numerator >= 0) {
        returnValue = numerator / FIXED_POINT_SCALE_AA;
    } else {
        // deal with negative numerators correctly
        returnValue = -((-numerator) / FIXED_POINT_SCALE_AA);
        returnValue -= ((-numerator) % FIXED_POINT_SCALE_AA) ? 1 : 0;
    }
    return returnValue;
}

/*
 * FPoint is at a scale factor of 16.  The anti-aliased scan conversion needs
 * to address 8x8 subpixels, so if we treat the FPoint coordinates as having
 * a scale factor of 2, then we should scan in sub-pixel coordinates, with
 * sub-sub-pixel correct endpoints!  Fukn shweet.
 */
void edge_init_aa(Edge* e, FPoint* top, FPoint* bottom) {
    static const int32_t F = 2;
    e->y = fceil_aa(top->y);
    int32_t yEnd = fceil_aa(bottom->y);
    e->height = yEnd - e->y;
    if (e->height)    {
        int32_t dN = bottom->y - top->y;
        int32_t dM = bottom->x - top->x;
        int32_t initialNumerator = dM * F * e->y - dM * top->y +
        dN * top->x - 1 + dN * F;
        floorDivMod(initialNumerator, dN*F, &e->x, &e->errorTerm);
        floorDivMod(dM*F, dN*F, &e->xStep, &e->numerator);
        e->denominator = dN*F;
    }
}

void fctx_init_context_aa(FContext* fctx, GContext* gctx) {

    GBitmap* frameBuffer = graphics_capture_frame_buffer(gctx);
    if (frameBuffer) {
        GBitmapFormat format = gbitmap_get_format(frameBuffer);
        fctx->flag_bounds = gbitmap_get_bounds(frameBuffer);
        graphics_release_frame_buffer(gctx, frameBuffer);
        fctx->gctx = gctx;
        fctx->flag_buffer = gbitmap_create_blank(fctx->flag_bounds.size, format);
        fctx->fill_color = GColorWhite;
        fctx->subpixel_adjust = -1;
        fctx->transform_offset = FPointZero;
        fctx->transform_scale_from = FPointOne;
        fctx->transform_scale_to = FPointOne;
    }
}

static const int32_t k_sampling_offsets[SUBPIXEL_COUNT] = {
    2, 7, 4, 1, 6, 3, 0, 5 // 1/8ths
};

void fctx_plot_edge_aa(FContext* fctx, FPoint* a, FPoint* b) {

    Edge edge;
    if (a->y > b->y) {
        edge_init_aa(&edge, b, a);
    } else {
        edge_init_aa(&edge, a, b);
    }

    while (edge.height > 0 && edge.y < 0) {
        edge_step(&edge);
    }

    int32_t max_y = fctx->flag_bounds.size.h * SUBPIXEL_COUNT - 1;
    while (edge.height > 0 && edge.y <= max_y) {
        int32_t ySub = edge.y & (SUBPIXEL_COUNT - 1);
        uint8_t mask = 1 << ySub;
        int32_t pixelX = (edge.x + k_sampling_offsets[ySub]) / SUBPIXEL_COUNT;
        int32_t pixelY = edge.y / SUBPIXEL_COUNT;
        GBitmapDataRowInfo row = gbitmap_get_data_row_info(fctx->flag_buffer, pixelY);
        if (pixelX < row.min_x) {
            uint8_t* p = row.data + row.min_x;
            *p ^= mask;
        } else if (pixelX <= row.max_x) {
            uint8_t* p = row.data + pixelX;
            *p ^= mask;
        }
        edge_step(&edge);
    }
}

static inline void fctx_plot_point_aa(FContext* fctx, fixed_t x, fixed_t y) {
    int32_t ySub = y & (SUBPIXEL_COUNT - 1);
    uint8_t mask = 1 << ySub;
    int32_t pixelX = (x + k_sampling_offsets[ySub]) / SUBPIXEL_COUNT;
    int32_t pixelY = y / SUBPIXEL_COUNT;

    if (pixelY >= 0 && pixelY < fctx->flag_bounds.size.h) {
        GBitmapDataRowInfo row = gbitmap_get_data_row_info(fctx->flag_buffer, pixelY);
        if (pixelX < row.min_x) {
            uint8_t* p = row.data + row.min_x;
            *p ^= mask;
        } else if (pixelX <= row.max_x) {
            uint8_t* p = row.data + pixelX;
            *p ^= mask;
        }
    }
}

// count the number of bits set in v
uint8_t countBits(uint8_t v) {
    unsigned int c; // c accumulates the total bits set in v
    for (c = 0; v; c++)    {
        v &= v - 1; // clear the least significant bit set
    }
    return c;
}

static inline int8_t clamp8(int8_t val, int8_t min, int8_t max) {
    if (val <= min) return min;
    if (val >= max) return max;
    return val;
}

void fctx_end_fill_aa(FContext* fctx) {

    int16_t rowMin = FIXED_TO_INT(fctx->extent_min.y);
    int16_t rowMax = FIXED_TO_INT(fctx->extent_max.y);
    int16_t colMin = FIXED_TO_INT(fctx->extent_min.x);
    int16_t colMax = FIXED_TO_INT(fctx->extent_max.x);

    if (rowMin < 0) rowMin = 0;
    if (rowMax >= fctx->flag_bounds.size.h) rowMax = fctx->flag_bounds.size.h - 1;

    GBitmap* fb = graphics_capture_frame_buffer(fctx->gctx);

    int16_t col, row;

    GColor8 d;
    GColor8 s = fctx->fill_color;
    for (row = rowMin; row <= rowMax; ++row) {
        GBitmapDataRowInfo fbRowInfo = gbitmap_get_data_row_info(fb, row);
        GBitmapDataRowInfo flagRowInfo = gbitmap_get_data_row_info(fctx->flag_buffer, row);
        int16_t spanMin = (fbRowInfo.min_x > colMin) ? fbRowInfo.min_x : colMin;
        int16_t spanMax = (fbRowInfo.max_x < colMax) ? fbRowInfo.max_x : colMax;
        uint8_t* dest = fbRowInfo.data + spanMin;
        uint8_t* src = flagRowInfo.data + spanMin;

        uint8_t mask = 0;
        for (col = spanMin; col <= spanMax; ++col, ++dest, ++src) {

            mask ^= *src;
            *src = 0;
            uint8_t a = clamp8(countBits(mask), 0, 8);
            if (a) {
                d.argb = *dest;
                d.r = (s.r*a + d.r*(8 - a) + 4) / 8;
                d.g = (s.g*a + d.g*(8 - a) + 4) / 8;
                d.b = (s.b*a + d.b*(8 - a) + 4) / 8;
                *dest = d.argb;
            }
        }
        if (col < flagRowInfo.max_x) *src = 0;
    }

    graphics_release_frame_buffer(fctx->gctx, fb);

}

// Initialize for Anti-Aliased rendering.
fctx_init_context_func   fctx_init_context   = &fctx_init_context_aa;
fctx_plot_edge_func      fctx_plot_edge      = &fctx_plot_edge_aa;
fctx_end_fill_func       fctx_end_fill       = &fctx_end_fill_aa;

void fctx_enable_aa(bool enable) {
    if (enable) {
        fctx_init_context   = &fctx_init_context_aa;
        fctx_plot_edge      = &fctx_plot_edge_aa;
        fctx_end_fill       = &fctx_end_fill_aa;
    } else {
        fctx_init_context   = &fctx_init_context_bw;
        fctx_plot_edge      = &fctx_plot_edge_bw;
        fctx_end_fill       = &fctx_end_fill_bw;
    }
}

bool fctx_is_aa_enabled() {
    return fctx_init_context == &fctx_init_context_aa;
}

#else

// Initialize for Black & White rendering.
fctx_init_context_func   fctx_init_context   = &fctx_init_context_bw;
fctx_plot_edge_func      fctx_plot_edge      = &fctx_plot_edge_bw;
fctx_end_fill_func       fctx_end_fill       = &fctx_end_fill_bw;

#endif

// --------------------------------------------------------------------------
// Transformed Drawing
// --------------------------------------------------------------------------

static void bezier(FContext* fctx,
            fixed_t x1, fixed_t y1,
            fixed_t x2, fixed_t y2,
            fixed_t x3, fixed_t y3,
            fixed_t x4, fixed_t y4) {
    fixed_t x12, y12;
    fixed_t x34, y34;
    fixed_t x123, y123;
    fixed_t x234, y234;

    {
        fixed_t x23, y23;

        // Calculate all the mid-points of the line segments
        x12   = (x1 + x2) / 2;
        y12   = (y1 + y2) / 2;
        x23   = (x2 + x3) / 2;
        y23   = (y2 + y3) / 2;
        x34   = (x3 + x4) / 2;
        y34   = (y3 + y4) / 2;
        x123  = (x12 + x23) / 2;
        y123  = (y12 + y23) / 2;
        x234  = (x23 + x34) / 2;
        y234  = (y23 + y34) / 2;
    }

    FPoint a = {x1, y1};
    FPoint b = {x12, y12};
    fctx_plot_edge(fctx, &a, &b);
    a = (FPoint){x123, y123};
    fctx_plot_edge(fctx, &b, &a);
    b = (FPoint){x234, y234};
    fctx_plot_edge(fctx, &a, &b);
    a = (FPoint){x34, y34};
    fctx_plot_edge(fctx, &b, &a);
    b = (FPoint){x4, y4};
    fctx_plot_edge(fctx, &a, &b);
}

void fctx_move_to_func(FContext* fctx, FPoint* params) {
    fctx->path_init_point = params[0];
    fctx->path_cur_point = params[0];
}

void fctx_line_to_func(FContext* fctx, FPoint* params) {
    fctx_plot_edge(fctx, &fctx->path_cur_point, params + 0);
    fctx->path_cur_point = params[0];
}

void fctx_curve_to_func(FContext* fctx, FPoint* params) {
    bezier(fctx,
           fctx->path_cur_point.x, fctx->path_cur_point.y,
           params[0].x, params[0].y,
           params[1].x, params[1].y,
           params[2].x, params[2].y);
    fctx->path_cur_point = params[2];
}

void fctx_transform_points(FContext* fctx, uint16_t pcount, FPoint* ppoints, FPoint* tpoints, FPoint advance) {

    /* transform the parameters */
    FPoint* src = ppoints;
    FPoint* dst = tpoints;
    FPoint* end = dst + pcount;
    while (dst != end) {
        dst->x = (src->x + advance.x) * fctx->transform_scale_to.x / fctx->transform_scale_from.x;
        dst->y = (src->y + advance.y) * fctx->transform_scale_to.y / fctx->transform_scale_from.y;
        dst->x += fctx->transform_offset.x + fctx->subpixel_adjust;
        dst->y += fctx->transform_offset.y + fctx->subpixel_adjust;

        // grow a bounding box around the points visited.
        if (dst->x < fctx->extent_min.x) fctx->extent_min.x = dst->x;
        if (dst->y < fctx->extent_min.y) fctx->extent_min.y = dst->y;
        if (dst->x > fctx->extent_max.x) fctx->extent_max.x = dst->x;
        if (dst->y > fctx->extent_max.y) fctx->extent_max.y = dst->y;

        ++src;
        ++dst;
    }
}

typedef void (*fctx_draw_cmd_func)(FContext* fctx, FPoint* params);

void fctx_draw_commands(FContext* fctx, FPoint advance, void* path_data, uint16_t length) {

    fctx_draw_cmd_func func;
    FPoint initpt = FPointZero;
    FPoint curpt = FPointZero;
    FPoint ctrlpt = FPointZero;
    FPoint tpoints[3];

    void* path_data_end = path_data + length;
    while (path_data < path_data_end) {
        {
            /* choose the draw function and parameter count. */
            FPathDrawCommand* cmd = (FPathDrawCommand*)path_data;
            fixed16_t* param = (fixed16_t*)&cmd->params;
            FPoint ppoints[3];

            switch (cmd->code) {
                case 'M': // "moveto"
                    func = fctx_move_to_func;
                    ppoints[0].x = *param++;
                    ppoints[0].y = *param++;
                    curpt = ppoints[0];
                    initpt = curpt;
                    fctx_transform_points(fctx, 1, ppoints, tpoints, advance);
                    break;
                case 'Z': // "closepath"
                    func = fctx_line_to_func;
                    ppoints[0] = initpt;
                    curpt = ppoints[0];
                    fctx_transform_points(fctx, 1, ppoints, tpoints, advance);
                    break;
                case 'L': // "lineto"
                    func = fctx_line_to_func;
                    ppoints[0].x = *param++;
                    ppoints[0].y = *param++;
                    curpt = ppoints[0];
                    fctx_transform_points(fctx, 1, ppoints, tpoints, advance);
                    fctx_line_to_func(fctx, tpoints);
                    break;
                case 'H': // "horizontal lineto"
                    func = fctx_line_to_func;
                    ppoints[0].x = *param++;
                    ppoints[0].y = curpt.y;
                    curpt.x = ppoints[0].x;
                    fctx_transform_points(fctx, 1, ppoints, tpoints, advance);
                    break;
                case 'V': // "vertical lineto"
                    func = fctx_line_to_func;
                    ppoints[0].x = curpt.x;
                    ppoints[0].y = *param++;
                    curpt.y = ppoints[0].y;
                    fctx_transform_points(fctx, 1, ppoints, tpoints, advance);
                    fctx_line_to_func(fctx, tpoints);
                    break;
                case 'C': // "cubic bezier curveto"
                    func = fctx_curve_to_func;
                    ppoints[0].x = *param++;
                    ppoints[0].y = *param++;
                    ppoints[1].x = *param++;
                    ppoints[1].y = *param++;
                    ppoints[2].x = *param++;
                    ppoints[2].y = *param++;
                    ctrlpt = ppoints[1];
                    curpt = ppoints[2];
                    fctx_transform_points(fctx, 3, ppoints, tpoints, advance);
                    break;
                case 'S': // "smooth cubic bezier curveto"
                    func = fctx_curve_to_func;
                    ppoints[1].x = *param++;
                    ppoints[1].y = *param++;
                    ppoints[2].x = *param++;
                    ppoints[2].y = *param++;
                    ppoints[0].x = curpt.x - ctrlpt.x + curpt.x;
                    ppoints[0].y = curpt.y - ctrlpt.y + curpt.y;
                    ctrlpt = ppoints[1];
                    curpt = ppoints[2];
                    fctx_transform_points(fctx, 3, ppoints, tpoints, advance);
                    break;
                case 'Q': // "quadratic bezier curveto"
                    func = fctx_curve_to_func;
                    ctrlpt.x = *param++;
                    ctrlpt.y = *param++;
                    ppoints[2].x = *param++;
                    ppoints[2].y = *param++;
                    ppoints[0].x = (curpt.x      + 2 * ctrlpt.x) / 3;
                    ppoints[0].y = (curpt.y      + 2 * ctrlpt.y) / 3;
                    ppoints[1].x = (ppoints[2].x + 2 * ctrlpt.x) / 3;
                    ppoints[1].y = (ppoints[2].y + 2 * ctrlpt.y) / 3;
                    curpt = ppoints[2];
                    fctx_transform_points(fctx, 3, ppoints, tpoints, advance);
                    break;
                case 'T': // "smooth quadratic bezier curveto"
                    func = fctx_curve_to_func;
                    ctrlpt.x = curpt.x - ctrlpt.x + curpt.x;
                    ctrlpt.y = curpt.y - ctrlpt.y + curpt.y;
                    ppoints[2].x = *param++;
                    ppoints[2].y = *param++;
                    ppoints[0].x = (curpt.x      + 2 * ctrlpt.x) / 3;
                    ppoints[0].y = (curpt.y      + 2 * ctrlpt.y) / 3;
                    ppoints[1].x = (ppoints[2].x + 2 * ctrlpt.x) / 3;
                    ppoints[1].y = (ppoints[2].y + 2 * ctrlpt.y) / 3;
                    curpt = ppoints[2];
                    fctx_transform_points(fctx, 3, ppoints, tpoints, advance);
                    break;
                default:
                    APP_LOG(APP_LOG_LEVEL_ERROR, "invalid draw command %d", cmd->code);
                    return;
            }

            /* advance to next draw command */
            path_data = (void*)param;
        }

        if (func) {
            func(fctx, tpoints);
        }
    }
}

// --------------------------------------------------------------------------
// Text
// --------------------------------------------------------------------------

void fctx_set_text_em_height(FContext* fctx, FFont* font, int16_t pixels) {
    fctx->transform_scale_from.x = FIXED_TO_INT(font->units_per_em);
    fctx->transform_scale_from.y = -fctx->transform_scale_from.x;
    fctx->transform_scale_to.x = pixels;
    fctx->transform_scale_to.y = pixels;
}

void fctx_draw_string(FContext* fctx, const char* text, FFont* font, GTextAlignment alignment, FTextAnchor anchor) {

    FPoint advance = FPointZero;
    uint16_t code_point;
    uint16_t decode_state;
    const char* p;

    if (alignment != GTextAlignmentLeft) {
        fixed_t width = 0;
        decode_state = 0;
        for (p = text; *p; ++p) {
            if (0 == utf8_decode_byte(*p, &decode_state, &code_point)) {
                FGlyph* glyph = ffont_glyph_info(font, code_point);
                if (glyph) {
                    width += glyph->horiz_adv_x;
                }
            }
        }
        if (alignment == GTextAlignmentRight) {
            advance.x = -width;
        } else /* alignment == GTextAlignmentCenter */ {
            advance.x = -width / 2;
        }
    }

    if (anchor == FTextAnchorBottom) {
        advance.y = -font->descent;
    } else if (anchor == FTextAnchorMiddle) {
        advance.y = -font->ascent / 2;
    } else if (anchor == FTextAnchorTop) {
        advance.y = -font->ascent;
    } else /* anchor == FTextAnchorBaseline) */ {
        advance.y = 0;
    }

    decode_state = 0;
    for (p = text; *p; ++p) {
        if (0 == utf8_decode_byte(*p, &decode_state, &code_point)) {
            FGlyph* glyph = ffont_glyph_info(font, code_point);
            if (glyph) {
                void* path_data = ffont_glyph_outline(font, glyph);
                fctx_draw_commands(fctx, advance, path_data, glyph->path_data_length);
                advance.x += glyph->horiz_adv_x;
            }
        }
    }
}
