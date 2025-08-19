#include <stdio.h>
#include <stdbool.h>

#define PIXELS_IMPLEMENTATION
#define PIXELS_STRIP_PREFIX
#include "pixels.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

bool render_frame_to_png(const char *output_path, Canvas *cnv) {
  if (!stbi_write_png(output_path, cnv->width, cnv->height, 4, cnv->pixels, 0)) {
    fprintf(stderr, "[ERROR] Failed to generate png frame: '%s'\n", output_path);
    return false;
  }

  fprintf(stdout, "[INFO] Generated png frame: '%s'\n", output_path);
  return true;
}

int main(void) {
  Canvas cnv = create_canvas(400, 400);
  Camera camera = default_camera(cnv.width, cnv.height);

  const char *output_path = "./triangle.png";

  printf("Instanced canvas of size (%d, %d) with a pixel count of %zu\n", cnv.width, cnv.height, cnv.count);

  float tri_height = cnv.height * 0.95;
  Triangle tri = {
    // Left point
    .a = { .position = Vec3(-tri_height/2.0, tri_height / 2, 0), .color = BLUE },
    // Top point
    .b = { .position = Vec3(0, -tri_height / 2, 0), .color = RED },
    // Right point
    .c = { .position = Vec3(tri_height/2.0, tri_height / 2, 0), .color = GREEN },
  };
  render_triangle(&cnv, camera, tri);

  if (!render_frame_to_png(output_path, &cnv)) return 1;

  return 0;
}

