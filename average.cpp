#include <QImage>

#include <assert.h>
#include <iostream>
#include <map>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

typedef pair<unsigned, unsigned> coordinate;
typedef map<coordinate, unsigned long> intImage;

static char *opts = "w:";

int main(int argc, char *argv[]) {
  short w = 5;

  char c;
  while ((c = getopt(argc, argv, opts)) != -1) {
    switch (c) {
      case 'w':
        w = atoi(optarg);
        break;
      default:
        break;
    }
  }

  // TODO(hermannloose): Let user choose which file to work on.
  QImage input("input.png");

  if (!input.isGrayscale()) {
    cerr << "Image isn't grayscale, aborting." << endl;
    exit(1);
  }

  int width = input.width();
  int height = input.height();

  cerr << "Got image [" << width << "x" << height <<"]." << endl;

  QImage output(input);
  intImage intSum;

  // TODO(hermannloose): Could be factored out into function.
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      coordinate current(x, y);
      unsigned long sum = 0;

      sum += qGray(input.pixel(x, y));

      if (x > 0) {
        intImage::iterator left = intSum.find(coordinate(x - 1, y));
        assert(left != intSum.end());
        sum += left->second;
      }

      if (y > 0) {
        intImage::iterator above = intSum.find(coordinate(x, y - 1));
        assert(above != intSum.end());
        sum += above->second;
      }

      if (x > 0 && y > 0) {
        intImage::iterator corner = intSum.find(coordinate(x - 1, y - 1));
        assert(corner != intSum.end());
        sum -= corner->second;
      }

      intSum[current] = sum;
    }
  }

  cerr << "Generated summed area table." << endl;

  int neighbours;
  if (w & 1) {
    neighbours = w * w;
  } else {
    // e.g. W = 4, W / 2 will actually go 2 pixels in each direction, giving
    // effectively W = 5 and would make pixels too bright if divided only by 16
    neighbours = (w + 1) * (w + 1);
  }

  // TODO(hermannloose): Could be factored out into function.
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      int left = max(0, x - (w / 2) - 1);
      int right = min(width - 1, x + (w / 2));
      int top = max(0, y - (w / 2) - 1);
      int bottom = min(height - 1, y + (w / 2));

      unsigned long gray = 0;

      intImage::iterator corner;

      corner = intSum.find(coordinate(right, bottom));
      assert(corner != intSum.end());
      gray += corner->second;

      corner = intSum.find(coordinate(right, top));
      assert(corner != intSum.end());
      gray -= corner->second;

      corner = intSum.find(coordinate(left, bottom));
      assert(corner != intSum.end());
      gray -= corner->second;

      corner = intSum.find(coordinate(left, top));
      assert(corner != intSum.end());
      gray += corner->second;

      gray /= neighbours;

      output.setPixel(x, y, gray);
    }
  }

  // TODO(hermannloose): Let user choose filename, maybe append kernel size.
  if (!output.save("output.png")) {
    cerr << "Couldn't save image!" << endl;
  }

  return 0;
}
