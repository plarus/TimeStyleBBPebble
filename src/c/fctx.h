
#pragma once
#include "pebble.h"

// -----------------------------------------------------------------------------
// Fixed point graphics context.
// -----------------------------------------------------------------------------

#define CHECK(obj) checkObject(obj, #obj)
bool checkObject(void* obj, const char* objname);

typedef int32_t fixed_t;
struct FFont;
typedef struct FFont FFont;

// Defines the fixed point conversions
#define FIXED_POINT_SHIFT 4
#define FIXED_POINT_SCALE 16
#define INT_TO_FIXED(a) ((a) * FIXED_POINT_SCALE)
#define FIXED_TO_INT(a) ((a) / FIXED_POINT_SCALE)
#define FIXED_MULTIPLY(a, b) (((a) * (b)) / FIXED_POINT_SCALE)
#define FIX1 FIXED_POINT_SCALE

typedef struct FPoint {
    fixed_t x;
    fixed_t y;
} FPoint;
#define FPoint(x, y) ((FPoint){(x), (y)})
#define FPointI(x, y) ((FPoint){INT_TO_FIXED(x), INT_TO_FIXED(y)})
#define FPointZero FPoint(0, 0)
#define FPointOne FPoint(1, 1)

typedef struct FContext {
	GContext* gctx;
	GBitmap* flag_buffer;
    GRect flag_bounds;
	FPoint extent_min;
	FPoint extent_max;
    FPoint path_init_point;
    FPoint path_cur_point;
    FPoint transform_offset;
    FPoint transform_scale_from;
	FPoint transform_scale_to;
	fixed_t subpixel_adjust;

    GColor fill_color;
} FContext;

void fctx_set_fill_color(FContext* fctx, GColor c);
void fctx_set_offset(FContext* fctx, FPoint offset);

void fctx_transform_points(FContext* fctx, uint16_t pcount, FPoint* ppoints, FPoint* tpoints, FPoint advance);

typedef void (*fctx_init_context_func)(FContext* fctx, GContext* gctx);
typedef void (*fctx_plot_edge_func)(FContext* fctx, FPoint* a, FPoint* b);
typedef void (*fctx_end_fill_func)(FContext* fctx);

extern fctx_init_context_func fctx_init_context;
extern void fctx_begin_fill(FContext* fctx);
extern fctx_plot_edge_func fctx_plot_edge;
extern fctx_end_fill_func fctx_end_fill;
extern void fctx_deinit_context(FContext* fctx);

#ifdef PBL_COLOR
void fctx_enable_aa(bool enable);
bool fctx_is_aa_enabled();
#endif

// -----------------------------------------------------------------------------
// Compiled SVG path drawing.
// -----------------------------------------------------------------------------

typedef int16_t fixed16_t;
typedef struct __attribute__((__packed__)) FPathDrawCommand {
	uint16_t code;
	fixed16_t params[];
} FPathDrawCommand;

void fctx_draw_commands(FContext* fctx, FPoint advance, void* path_data, uint16_t length);

// -----------------------------------------------------------------------------
// Text drawing.
// -----------------------------------------------------------------------------

typedef enum {
	FTextAnchorBaseline = 0,
	FTextAnchorMiddle,
	FTextAnchorTop,
	FTextAnchorBottom
} FTextAnchor;

void fctx_set_text_em_height(FContext* fctx, FFont* font, int16_t pixels);
void fctx_draw_string(FContext* fctx, const char* text, FFont** font, GTextAlignment alignment, FTextAnchor anchor);
