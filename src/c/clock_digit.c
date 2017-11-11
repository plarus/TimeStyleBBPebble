#include <pebble.h>
#include "clock_digit.h"

void adjustImagePalette(ClockDigit* this);

/*
 * Array mapping numbers to resource ids
 */
uint32_t ClockDigit_imageIds[3][10] = {
  {RESOURCE_ID_CLOCK_DIGIT_0,
    RESOURCE_ID_CLOCK_DIGIT_1,
    RESOURCE_ID_CLOCK_DIGIT_2,
    RESOURCE_ID_CLOCK_DIGIT_3,
    RESOURCE_ID_CLOCK_DIGIT_4,
    RESOURCE_ID_CLOCK_DIGIT_5,
    RESOURCE_ID_CLOCK_DIGIT_6,
    RESOURCE_ID_CLOCK_DIGIT_7,
    RESOURCE_ID_CLOCK_DIGIT_8,
    RESOURCE_ID_CLOCK_DIGIT_9},
  {RESOURCE_ID_CLOCK_DIGIT_LECO_0,
   RESOURCE_ID_CLOCK_DIGIT_LECO_1,
   RESOURCE_ID_CLOCK_DIGIT_LECO_2,
   RESOURCE_ID_CLOCK_DIGIT_LECO_3,
   RESOURCE_ID_CLOCK_DIGIT_LECO_4,
   RESOURCE_ID_CLOCK_DIGIT_LECO_5,
   RESOURCE_ID_CLOCK_DIGIT_LECO_6,
   RESOURCE_ID_CLOCK_DIGIT_LECO_7,
   RESOURCE_ID_CLOCK_DIGIT_LECO_8,
   RESOURCE_ID_CLOCK_DIGIT_LECO_9},
  {RESOURCE_ID_CLOCK_DIGIT_BOLD_0,
   RESOURCE_ID_CLOCK_DIGIT_BOLD_1,
   RESOURCE_ID_CLOCK_DIGIT_BOLD_2,
   RESOURCE_ID_CLOCK_DIGIT_BOLD_3,
   RESOURCE_ID_CLOCK_DIGIT_BOLD_4,
   RESOURCE_ID_CLOCK_DIGIT_BOLD_5,
   RESOURCE_ID_CLOCK_DIGIT_BOLD_6,
   RESOURCE_ID_CLOCK_DIGIT_BOLD_7,
   RESOURCE_ID_CLOCK_DIGIT_BOLD_8,
   RESOURCE_ID_CLOCK_DIGIT_BOLD_9}
};

void ClockDigit_setNumber(ClockDigit* this, int number, int fontId) {

  if(this->currentNum != number || this->currentFontId != fontId) {

    //deallocate the old bg image
    gbitmap_destroy(this->currentImage);

    //change over to the new digit image
    this->currentImageId = ClockDigit_imageIds[fontId][number];
    this->currentImage = gbitmap_create_with_resource(this->currentImageId);
    this->currentNum = number;
    this->currentFontId = fontId;

    //set the palette properly
    adjustImagePalette(this);

    //set the layer to the new image
    bitmap_layer_set_bitmap(this->imageLayer, this->currentImage);
  }

  // in case the layer was set to hidden, unhide
  layer_set_hidden((Layer *)this->imageLayer, false);
}

void ClockDigit_setBlank(ClockDigit* this) {
  layer_set_hidden((Layer *)this->imageLayer, true);
}

void ClockDigit_offsetPosition(ClockDigit* this, int posOffset) {
  layer_set_frame((Layer*)this->imageLayer,
                  GRect(this->position.x + posOffset, this->position.y, 48, 71));
}

void ClockDigit_setColor(ClockDigit* this, GColor fg, GColor bg) {
  // set the new colors
  this->fgColor = fg;
  this->bgColor = bg;

  adjustImagePalette(this);
}

void ClockDigit_construct(ClockDigit* this, GPoint pos) {
  this->currentNum = -1;
  this->bgColor = GColorWhite;
  this->fgColor = GColorBlack;
  this->position = pos;

  this->imageLayer = bitmap_layer_create(GRect(pos.x, pos.y, 48, 71));

  ClockDigit_setBlank(this);
  ClockDigit_setNumber(this, 1, 0);
  ClockDigit_setColor(this, GColorBlack, GColorWhite);
}

void ClockDigit_destruct(ClockDigit* this) {
  // destroy the background layer
  bitmap_layer_destroy(this->imageLayer);
}

void ClockDigit_destruct2(ClockDigit* this) {
  // deallocate the background image
  gbitmap_destroy(this->currentImage);
}

void adjustImagePalette(ClockDigit* this) {
  if(this->currentImage) {
    GColor* pal = gbitmap_get_palette(this->currentImage);

    pal[0] = this->fgColor;
    pal[1] = this->bgColor;
  }
}
