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

#define WIDTH (400)
#define HEIGHT (WIDTH/4*3)
#define TRIS_COUNT 6

typedef struct {
  Vector2f position;
  struct {
    float top_left;
    float top_right;
    float bot_right;
    float bot_left;
  } depth;
  Vector2f size;
} Rectangle;

void render_rect(Canvas *cnv, Camera camera, Rectangle rect) {
  float x = rect.position.x, y = rect.position.y;
  float w = rect.size.x, h = rect.size.y;

  Triangle a = {
    { Vec3(x, y, rect.depth.top_left), RED },
    { Vec3(x + w, y, rect.depth.top_right), RED },
    { Vec3(x + w, y + h, rect.depth.bot_right), RED },
  };
  Triangle b = {
    { Vec3(x + w, y + h, rect.depth.bot_right), BLUE },
    { Vec3(x, y + h, rect.depth.bot_left), BLUE },
    { Vec3(x, y, rect.depth.top_left), BLUE },
  };

  render_triangle(cnv, camera, a);
  render_triangle(cnv, camera, b);
}

const float cube_unit_size = 200;
#define CUBE_SIZE_VEC Vec2f(cube_unit_size, cube_unit_size)

int main(void) {
  Canvas cnv = create_canvas(WIDTH, HEIGHT);
  Camera cam = default_camera(cnv.width, cnv.height);
  cam.position.y = -10.0f;
  cam.position.z = -1.0f;
  printf("Instanced canvas of size (%d, %d) with a pixel count of %zu\n", cnv.width, cnv.height, cnv.count);
  printf("Camera orientation is set to: (%.2f, %.2f, %.2f) \n", cam.orientation.x, cam.orientation.y, cam.orientation.z);

  float cube_y = -cube_unit_size/2.0;
  Triangle left_face_a = {
    { Vec3(-cube_unit_size, cube_y, 1), RED },
    { Vec3(0, cube_y, 0), RED },
    { Vec3(0, cube_y+cube_unit_size, 0), RED },
  };
  render_triangle(&cnv, cam, left_face_a);
  Triangle left_face_b = {
    { Vec3(0, cube_y+cube_unit_size, 0), BLUE },
    { Vec3(-cube_unit_size, cube_y+cube_unit_size, 1), BLUE },
    { Vec3(-cube_unit_size, cube_y, 1), BLUE },
  };
  render_triangle(&cnv, cam, left_face_b);

  Triangle right_face_a = {
    { Vec3(0, cube_y, 0), BLUE },
    { Vec3(0, cube_y+cube_unit_size, 0), BLUE },
    { Vec3(cube_unit_size, cube_y, 1), BLUE },
  };
  render_triangle(&cnv, cam, right_face_a);
  Triangle right_face_b = {
    { Vec3(cube_unit_size, cube_y, 1), RED },
    { Vec3(cube_unit_size, cube_y+cube_unit_size, 1), RED },
    { Vec3(0, cube_y+cube_unit_size, 0), RED },
  };
  render_triangle(&cnv, cam, right_face_b);

  Triangle top_face_a = {
    { Vec3(0, cube_y, 0), BLUE },
    { Vec3(-cube_unit_size, cube_y, 1), BLUE },
    { Vec3(0, cube_y, 2), BLUE },
  };
  Triangle top_face_b = {
    { Vec3(0, cube_y, 2), RED },
    { Vec3(cube_unit_size, cube_y, 1), RED },
    { Vec3(0, cube_y, 0), RED },
  };
  render_triangle(&cnv, cam, top_face_a);
  render_triangle(&cnv, cam, top_face_b);

  if (!render_frame_to_png("cube.png", &cnv)) return 1;

  return 0;
}

