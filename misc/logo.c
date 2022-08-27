#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double CHARACTER_RATIO = 2.2;

int cstarlogo(double x, double y) {
  double slope = 1.0 / sqrt(3);
  double width = 0.25;
  double x2 = x * x, y2 = y * y;

  int inOuter  = x2 + y2 < 1.0;
  int outInner = x2 + y2 > (1 - width) * (1 - width);
  int abovePos =  x < slope * (y + width);
  int belowPos =  x > slope * (y - width);
  int aboveNeg = -x < slope * (y + width);
  int belowNeg = -x > slope * (y - width);
  int aboveMid = y > width / -2.0;
  int belowMid = y < width /  2.0;

  return inOuter && ((outInner && (abovePos || belowNeg)) || ((abovePos && belowPos) || (aboveNeg && belowNeg) || (aboveMid && belowMid && !outInner)));
}

int main(int argc, char *argv[]) {
  double width, height, left, top;
  int    rows, cols = 101;

  if (argc > 1) {
    cols = atoi(argv[1]);
  }

  rows = (int)((double)cols / CHARACTER_RATIO);

  width  = 2.0 / (double)cols;
  height = 2.0 / (double)rows;
  left   = -(double)(cols / 2) * width;
  top    =  (double)(rows / 2) * height;

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      double x, y;
      // Find the point in the "picture"
      x = left + (double)j * width;
      y = top  - (double)i * height;
      // Determine inclusion
      if (cstarlogo(x, y)) {
        // Print according to parity
        if ((i * cols + j) % 2) printf("C");
        else                    printf("*");
      } else                    printf(" ");
    }
    printf("\n");
  }

  return 0;
}