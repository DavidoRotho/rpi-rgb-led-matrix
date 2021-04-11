// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Small example how to use the library.
// For more examples, look at demo-main.cc
//
// This code is public domain
// (but note, that the led-matrix library this depends on is GPL v2)

#include "led-matrix.h"

#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <signal.h>

using rgb_matrix::RGBMatrix;
using rgb_matrix::Canvas;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
  interrupt_received = true;
}

void interDraw(int x, int y, int r, int g, int b, Canvas *can)
{
  int xWidth = 192;
  int yWidth = 192;

  // First 0 to 32 needs to go to y = 0 to y = 32
  // Second 0 to 32 needs to go to y = 0 to y = 32 but offset by x width
  if (y > 32 && y < 64)
  {
    y -= 32;
    x += xWidth;
  }
  
  // Third 64 to 96 needs to go to y = 32 to y = 64
  // Fourth 96 to 128 needs to go to y = 32 to y = 64 but offset by x width
  if (y > 64 && y < 96)
  {
    y -= 32;
  }
  if (y > 96 && y < 128)
  {
    y -= 64;
    x += xWidth;
  }

  // Fifth 128 to 160 needs to go to y = 64 to y = 96
  // Sixth 160 to 192 needs to go to y = 64 to y = 96 but offset by x width
  if (y > 128 && y < 160)
  {
    y -= 64;
  }
  if (y > 96 && y < 128)
  {
    y -= 96;
    x += xWidth;
  }


  can->SetPixel(x, y, r, g, b);
}

static void DrawOnCanvas(Canvas *canvas) {
  /*
   * Let's create a simple animation. We use the canvas to draw
   * pixels. We wait between each step to have a slower animation.
   */
  canvas->Fill(0, 0, 255);

  printf("\nCanvas Size: %d, %d\n\n", canvas->width(), canvas->height());
  for (float i = 0; i < 192; i++)
    for (float j = 0; j < 192; j++) 
    {
      if (interrupt_received)
        return;
      interDraw(i, j, 255, 0, 0, canvas);
      usleep(1 * 100);  // wait a little to slow down things.
    }
}

int main(int argc, char *argv[]) {
  RGBMatrix::Options defaults;
  defaults.hardware_mapping = "regular";  // or e.g. "adafruit-hat"
  defaults.rows = 32;
  defaults.chain_length = 1;
  defaults.parallel = 1;
  defaults.show_refresh_rate = true;
  Canvas *canvas = RGBMatrix::CreateFromFlags(&argc, &argv, &defaults);
  if (canvas == NULL)
    return 1;

  // It is always good to set up a signal handler to cleanly exit when we
  // receive a CTRL-C for instance. The DrawOnCanvas() routine is looking
  // for that.
  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  DrawOnCanvas(canvas);    // Using the canvas.

  // Animation finished. Shut down the RGB matrix.
  canvas->Clear();
  delete canvas;

  return 0;
}
