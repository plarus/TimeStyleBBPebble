#include "clock_digit.h"
#include "ffont.h"

/*
 * Array mapping numbers to resource ids
 */
uint32_t ClockDigit_FontIds[3][12] = {
  {RESOURCE_ID_CLOCK_DIGIT_0,
   RESOURCE_ID_CLOCK_DIGIT_1,
   RESOURCE_ID_CLOCK_DIGIT_2,
   RESOURCE_ID_CLOCK_DIGIT_3,
   RESOURCE_ID_CLOCK_DIGIT_4,
   RESOURCE_ID_CLOCK_DIGIT_5,
   RESOURCE_ID_CLOCK_DIGIT_6,
   RESOURCE_ID_CLOCK_DIGIT_7,
   RESOURCE_ID_CLOCK_DIGIT_8,
   RESOURCE_ID_CLOCK_DIGIT_9,
   RESOURCE_ID_CLOCK_DIGIT_COLON,
   RESOURCE_ID_CLOCK_DIGIT_SPACE},
  {RESOURCE_ID_CLOCK_DIGIT_LECO_0,
   RESOURCE_ID_CLOCK_DIGIT_LECO_1,
   RESOURCE_ID_CLOCK_DIGIT_LECO_2,
   RESOURCE_ID_CLOCK_DIGIT_LECO_3,
   RESOURCE_ID_CLOCK_DIGIT_LECO_4,
   RESOURCE_ID_CLOCK_DIGIT_LECO_5,
   RESOURCE_ID_CLOCK_DIGIT_LECO_6,
   RESOURCE_ID_CLOCK_DIGIT_LECO_7,
   RESOURCE_ID_CLOCK_DIGIT_LECO_8,
   RESOURCE_ID_CLOCK_DIGIT_LECO_9,
   RESOURCE_ID_CLOCK_DIGIT_LECO_COLON,
   RESOURCE_ID_CLOCK_DIGIT_LECO_SPACE},
  {RESOURCE_ID_CLOCK_DIGIT_BOLD_0,
   RESOURCE_ID_CLOCK_DIGIT_BOLD_1,
   RESOURCE_ID_CLOCK_DIGIT_BOLD_2,
   RESOURCE_ID_CLOCK_DIGIT_BOLD_3,
   RESOURCE_ID_CLOCK_DIGIT_BOLD_4,
   RESOURCE_ID_CLOCK_DIGIT_BOLD_5,
   RESOURCE_ID_CLOCK_DIGIT_BOLD_6,
   RESOURCE_ID_CLOCK_DIGIT_BOLD_7,
   RESOURCE_ID_CLOCK_DIGIT_BOLD_8,
   RESOURCE_ID_CLOCK_DIGIT_BOLD_9,
   RESOURCE_ID_CLOCK_DIGIT_BOLD_COLON,
   RESOURCE_ID_CLOCK_DIGIT_BOLD_SPACE}
};

void ClockDigit_draw_string(FContext* fctx, const char* text, uint8_t fontId, int16_t pixels, GTextAlignment alignment, FTextAnchor anchor) {
  FFont* font[5];
  int char_Count = 0;
  const char* p;
  int digit_Id;

  for (p = text; *p; ++p) {
    if (*p >= 48) {
      digit_Id = *p - 48;
    } else { // Space
      digit_Id = 11;
    }
    font[char_Count] = ffont_create_from_resource(ClockDigit_FontIds[fontId][digit_Id]);
    char_Count++;
  }
  fctx_set_text_em_height(fctx, font[0], pixels);
  fctx_draw_string(fctx, text, font, alignment, anchor);

  char_Count = 0;
  for (p = text; *p; ++p) {
    char_Count++;
    ffont_destroy(font[char_Count]);
  }
}
