#ifndef PIXELS_H_
#define PIXELS_H_

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

#define PIXELS_PI 3.141592653589793
#define PIXELS_TAU (2*PI)

#define PIXELS_CLAMP(v, min, max) ((v) < (min) ? (min) : ((v) > (max) ? (max) : (v)))
#define PIXELS_MIN(a, b) ((a) < (b) ? (a) : (b))
#define PIXELS_MAX(a, b) ((a) > (b) ? (a) : (b))

#ifndef PIXELS_MALLOC
#define PIXELS_MALLOC(x) malloc(x)
#endig

typedef struct {
  unsigned char red, green, blue, alpha;
} Pixels_Rgba;
#define Pixels_RGBa(r, g, b, a) ((Rgba) { .red = (r), .green = (g), .blue = (b), .alpha = a })
#define Pixels_RGB(r, g, b) RGBa(r, g, b, 255)
#define Pixels_RGB_Fmt "RGB(%d, %d, %d)"
#define Pixels_RGB_Arg(clr) (clr).red, (clr).green, (clr).blue
#define Pixels_RGBa_Fmt "RGBa(%d, %d, %d, %.2f)"
#define Pixels_RGBa_Arg(clr) (clr).red, (clr).green, (clr).blue, ((float)(clr).alpha)/255.0f
#define Pixels_RED Pixels_RGB(255, 0, 0)
#define Pixels_GREEN Pixels_RGB(0, 255, 0)
#define Pixels_BLUE Pixels_RGB(0, 0, 255)

typedef struct {
  float hue, saturation, lightness;
  float alpha;
} Pixels_Hsla;
#define Pixels_HSLa(h, s, l, a) ((Hsla) { .hue = (h), .saturation = (s), .lightness = (l), .alpha = (a) })
#define Pixels_HSL(h, s, l) HSLa(h, s, l, 1.0f)
#define Pixels_HSL_Fmt "HSL(%.2f, %.2f, %.2f)"
#define Pixels_HSL_Arg(clr) (clr).hue, (clr).saturation, (clr).lightness
#define Pixels_HSLa_Fmt "HSL(%.2f, %.2f, %.2f, %.2f)"
#define Pixels_HSLa_Arg(clr) (clr).hue, (clr).saturation, (clr).lightness, (clr).alpha

Pixels_Hsla pixels_rgb2hsl(Pixels_Rgba rgb);
Pixels_Rgba pixels_hsl2rgb(Pixels_Hsla hsl);

typedef struct {
  float x, y;
} Pixels_Vector2f;
#define Pixels_Vec2f(x, y) ((Pixels_Vector2f) { x, y })

typedef struct {
  int x, y;
} Pixels_Vector2i;
#define Pixels_Vec2i(x, y) ((Pixels_Vector2i) { x, y })


typedef struct {
  float x, y, z;
} Pixels_Vector3;
#define Pixels_Vec3(x, y, z) ((Pixels_Vector3) { x, y, z })

typedef struct {
  Pixels_Vector3 position;
  Pixels_Rgba color;
} Pixels_Vertice;

typedef struct {
  Pixels_Vertice a, b, c;
} Pixels_Triangle;

typedef struct {
  // Position of the camera in the world
  Pixels_Vector3 position;
  // Camera orientation is a set of Tait–Bryan angles
  Pixels_Vector3 orientation;
  // Display's position relative to the camera
  // Not sure about what's a better name so I made it a union
  union {
    Pixels_Vector3 screen;
    Pixels_Vector3 display;
  };
} Pixels_Camera;

Pixels_Camera pixels_default_camera(size_t width, size_t height);


// Formula from https://en.wikipedia.org/wiki/3D_projection#Mathematical_formula
Pixels_Vector2f pixels_calculate_perspective_projection(Pixels_Camera camera, Pixels_Vector3 point);


typedef struct {
  int width, height;
  size_t count;
  Pixels_Rgba *pixels;
} Pixels_Canvas;

Pixels_Canvas pixels_create_canvas(int width, int height);

#define pixels_get_pixel(cnv, x, y) ((cnv)->pixels + ((x)+(y)*(cnv)->width))
#define pixels_set_pixel(cnv, x, y, clr) \
  do { \
    Pixels_Rgba *p = pixels_get_pixel(cnv, x, y); \
    p->red = (clr).red; \
    p->green = (clr).green; \
    p->blue = (clr).blue; \
    p->alpha = (clr).alpha; \
  } while (0)
#define pixels_foreach_pixel(cnv, it) for (Pixels_Rgba *it = (cnv)->pixels; it <= ((cnv)->pixels + ((cnv)->count-1)); ++it)


// Calculate the cross product of a Vec2
#define pixels_cross_2d(a, b) (a.x * b.y + a.y * b.x)


// Calculates the linearly interpolated value from start to end by given step
float pixels_lerpf(float start, float end, float step);
// Calculates the linearly interpolated value from start to end by given step and clamps it onto an unsigned char
#define pixels_uchar_lerpf(start, end, step) float_to_uchar_round_clamp(lerpf(start, end, step) - 0.5f)

// Computes a scalar cross product for the directed edge from a to b and point p
// If the returned value is positive, p is to the left of the directed edge of a and b
// If the returned value is negative, p is to the right of the directed edge of a and b
// If the returned value is zero, p is sitting on the directed edge of a and b
float pixels_tri_edge_function(const Pixels_Vector2f *a, const Pixels_Vector2f *b, const Pixels_Vector2f *p);

// Transform a float to an unsigned char rounded up if needed; clamping the value between 0-255
unsigned char pixels_float_to_uchar_round_clamp(float base_value);


// Get the eucledian distance between 2 Vector2f without the last step of the calculation which is the square root
float pixels_square_eucledian_dist_vec2f(Pixels_Vector2f a, Pixels_Vector2f b);
// Macro that fulfills the last step of the eucledian distance formula
#define pixels_full_eucledian_dist_vec2f(a, b) sqrtf(square_eucledian_dist_vec2f(a, b))


// Renders a filled in triangle onto the canvas after calculating the projected position with the camera
void pixels_render_triangle(Pixels_Canvas *cnv, Pixels_Camera camera, Pixels_Triangle tri);

#endif // PIXELS_H_




#ifdef PIXELS_IMPLEMENTATION

Pixels_Camera pixels_default_camera(size_t width, size_t height) {
  // Not really sure what's a good default for all of these but for now these seem ok
  Pixels_Camera camera = {
    // Camera is away from the center backwards?
    .position = { 0, 0, -1.0f },
    // Rotation is a Tait–Bryan angles, and I got no clue how to work with them directly
    .orientation = { 0, 0, 0 },
    // Screen display position relative to the camera
    .screen = { width / 2, height / 2, 1 },
  };
  return camera;
}

Pixels_Vector2f pixels_calculate_perspective_projection(Pixels_Camera camera, Pixels_Vector3 point) {
  float x = point.x - camera.position.x;
  float y = point.y - camera.position.y;
  float z = point.z - camera.position.z;

  double cos_x = cos(camera.orientation.x);
  double cos_y = cos(camera.orientation.y);
  double cos_z = cos(camera.orientation.z);

  double sin_x = sin(camera.orientation.x);
  double sin_y = sin(camera.orientation.y);
  double sin_z = sin(camera.orientation.z);

  double dx = cos_y * (sin_z * y + cos_z * x) - sin_y * z;
  double dy = sin_x * (cos_y * z + sin_y * (sin_z * y + cos_z * x)) + cos_x * (cos_z * y - sin_z * x);
  double dz = cos_x * (cos_y * z + sin_y * (sin_z * y + cos_z * x)) - sin_x * (cos_z * y - sin_z * x);

  double ratio = camera.screen.z / dz;
  float bx = ratio * dx + camera.screen.x;
  float by = ratio * dy + camera.screen.y;

  Pixels_Vector2f screen_pos = {
    .x = bx,
    .y = by,
  };
  return screen_pos;
}


// Helper function for creating a new canvas
Pixels_Canvas pixels_create_canvas(int width, int height) {
  size_t count = (size_t) width * height;
  Pixels_Canvas cnv;

  cnv.width = width;
  cnv.height = height;
  cnv.count = count;
  cnv.pixels = PIXELS_MALLOC(sizeof(Pixels_Rgba)*count);
  pixels_foreach_pixel(&cnv, pixel) {
    pixel->red = pixel->green = pixel->blue = 0;
    pixel->alpha = 255;
  }
  return cnv;
}


// Calculate the linear interpolation between start and end by a given step
float pixels_lerpf(float start, float end, float step) {
  return (end - start) * step + start;
}


// Computes a scalar cross product for the directed edge from a to b and point p
// If the returned value is positive, p is to the left of the directed edge of a and b
// If the returned value is negative, p is to the right of the directed edge of a and b
// If the returned value is zero, p is sitting on the directed edge of a and b
float pixels_tri_edge_function(const Pixels_Vector2f *a, const Pixels_Vector2f *b, const Pixels_Vector2f *p) {
  return (p->x - a->x) * (b->y - a->y) - (p->y - a->y) * (b->x - a->x);
}


// Transform a float to an unsigned char rounded up if needed; clamping the value between 0-255
unsigned char pixels_float_to_uchar_round_clamp(float base_value) {
  if (base_value <= 0) return 0;
  if (base_value >= 254.5f) return 255;
  int v = floorf(base_value + 0.5f);
  return (unsigned char) v;
}


// Calculate the eucledian distance between 2 points represented by Vector2f structs but missing the last step of square rooting
float pixels_square_eucledian_dist_vec2f(Pixels_Vector2f a, Pixels_Vector2f b) {
  float x = a.x - b.x;
  float y = a.y - b.y;
  return (x * x) + (y * y);
}

// Find the average color based on a point within a triangle 
Pixels_Rgba pixels_average_vec2f_rgba(Pixels_Vector2f positions[3], Pixels_Rgba colors[3], Pixels_Vector2f point) {
  // If all colors are the same values then we don't need to compute anything else
  if (memcmp(colors, colors + 1 , sizeof(Pixels_Rgba)) == 0 && memcmp(colors + 1, colors + 2, sizeof(Pixels_Rgba)) == 0) {
    return colors[0];
  }

  Pixels_Rgba a_clr = colors[0];
  Pixels_Rgba b_clr = colors[1];
  Pixels_Rgba c_clr = colors[2];

  Pixels_Vector2f a_pos = positions[0];
  Pixels_Vector2f b_pos = positions[1];
  Pixels_Vector2f c_pos = positions[2];

  float dist_a = pixels_square_eucledian_dist_vec2f(point, a_pos);
  float dist_b = pixels_square_eucledian_dist_vec2f(point, b_pos);
  float dist_c = pixels_square_eucledian_dist_vec2f(point, c_pos);
  
  float max_dist = PIXELS_MAX(PIXELS_MAX(dist_a, dist_b), dist_c);

  float step_a = 1.0 -  dist_a / max_dist;
  float step_b = 1.0 - dist_b / max_dist;
  float step_c = 1.0 - dist_c / max_dist;

  unsigned char min_r = PIXELS_MIN(PIXELS_MIN(a_clr.red, b_clr.red), c_clr.red);
  float r_a = pixels_lerpf(min_r, a_clr.red, step_a);
  float r_b = pixels_lerpf(min_r, b_clr.red, step_b);
  float r_c = pixels_lerpf(min_r, c_clr.red, step_c);
  float avg_r = floorf((r_a + r_b + r_c) / 3.0f) + min_r;

  unsigned char min_g = PIXELS_MIN(PIXELS_MIN(a_clr.green, b_clr.green), c_clr.green);
  float g_a = pixels_lerpf(min_g, a_clr.green, step_a);
  float g_b = pixels_lerpf(min_g, b_clr.green, step_b);
  float g_c = pixels_lerpf(min_g, c_clr.green, step_c);
  float avg_g = floorf((g_a + g_b + g_c) / 3.0f) + min_g;

  unsigned char min_b = PIXELS_MIN(PIXELS_MIN(a_clr.blue, b_clr.blue), c_clr.blue);
  float b_a = pixels_lerpf(min_b, a_clr.blue, step_a);
  float b_b = pixels_lerpf(min_b, b_clr.blue, step_b);
  float b_c = pixels_lerpf(min_b, c_clr.blue, step_c);
  float avg_b = floorf((b_a + b_b + b_c) / 3.0f) + min_b;

  unsigned char min_a = PIXELS_MIN(PIXELS_MIN(a_clr.alpha, b_clr.alpha), c_clr.alpha);
  float a_a = pixels_lerpf(min_a, a_clr.alpha, step_a);
  float a_b = pixels_lerpf(min_a, b_clr.alpha, step_b);
  float a_c = pixels_lerpf(min_a, c_clr.alpha, step_c);
  float avg_a = floorf((a_a + a_b + a_c) / 3.0f) + min_a;

  Pixels_Rgba out;
  out.red = pixels_float_to_uchar_round_clamp(avg_r);
  out.green = pixels_float_to_uchar_round_clamp(avg_g);
  out.blue = pixels_float_to_uchar_round_clamp(avg_b);
  out.alpha = pixels_float_to_uchar_round_clamp(avg_a);

  return out;
}


// To be honest, I don't really understand all the math here...
Pixels_Rgba pixels_barycentric_trilerp(Pixels_Vector2f p[3], Pixels_Rgba color[3], Pixels_Vector2f pt) {
  // edge vectors
  float v0x = p[1].x - p[0].x;
  float v0y = p[1].y - p[0].y;
  float v1x = p[2].x - p[0].x;
  float v1y = p[2].y - p[0].y;
  float v2x = pt.x   - p[0].x;
  float v2y = pt.y   - p[0].y;

  // compute denominator (2 * area)
  float den = v0x * v1y - v1x * v0y;
  // if triangle is degenerate, just return nearest vertex color
  if (fabsf(den) < 1e-8f) {
    // choose closest vertex
    float da = (pt.x - p[0].x)*(pt.x - p[0].x) +
               (pt.y - p[0].y)*(pt.y - p[0].y);
    float db = (pt.x - p[1].x)*(pt.x - p[1].x) +
               (pt.y - p[1].y)*(pt.y - p[1].y);
    float dc = (pt.x - p[2].x)*(pt.x - p[2].x) +
               (pt.y - p[2].y)*(pt.y - p[2].y);
    int i = (da <= db && da <= dc) ? 0 : (db <= dc ? 1 : 2);
    return color[i];
  }

  float invDen = 1.0f / den;

  // barycentric coordinates (u, v, w) for pt relative to triangle (p0,p1,p2)
  float u = (v2x * v1y - v1x * v2y) * invDen; // weight for p1
  float v = (v0x * v2y - v2x * v0y) * invDen; // weight for p2
  float w = 1.0f - u - v;                      // weight for p0

  // linear blend channels (additive)
  float rf = u * color[1].red   + v * color[2].red   + w * color[0].red;
  float gf = u * color[1].green + v * color[2].green + w * color[0].green;
  float bf = u * color[1].blue  + v * color[2].blue  + w * color[0].blue;
  float af = u * color[1].alpha + v * color[2].alpha + w * color[0].alpha;

  Pixels_Rgba out;
  out.red   = pixels_float_to_uchar_round_clamp(rf);
  out.green = pixels_float_to_uchar_round_clamp(gf);
  out.blue  = pixels_float_to_uchar_round_clamp(bf);
  out.alpha = pixels_float_to_uchar_round_clamp(af);
  return out;
}


void pixels_render_triangle(Pixels_Canvas *cnv, Pixels_Camera camera, Pixels_Triangle tri) {
  Pixels_Vector2f a = pixels_calculate_perspective_projection(camera, tri.a.position);
  Pixels_Vector2f b = pixels_calculate_perspective_projection(camera, tri.b.position);
  Pixels_Vector2f c = pixels_calculate_perspective_projection(camera, tri.c.position);
  // printf("Rendering triangle at A(%.2f, %.2f) B(%.2f, %.2f) C(%.2f, %.2f)\n", a.x, a.y, b.x, b.y, c.x, c.y);

  // Compute bounding box
  float minx = fminf(fminf(a.x, b.x), c.x);
  float miny = fminf(fminf(a.y, b.y), c.y);
  float maxx = fmaxf(fmaxf(a.x, b.x), c.x);
  float maxy = fmaxf(fmaxf(a.y, b.y), c.y);

  if (isinf(minx)) minx = signbit(minx) == 0 ? FLT_MAX : -FLT_MAX;
  if (isinf(miny)) miny = signbit(miny) == 0 ? FLT_MAX : -FLT_MAX;
  if (isinf(maxx)) maxx = signbit(maxx) == 0 ? FLT_MAX : -FLT_MAX;
  if (isinf(maxy)) maxy = signbit(maxy) == 0 ? FLT_MAX : -FLT_MAX;

  // Ignore tri if bounding box does not overlap canvas
  if (maxx < 0.0f || minx >= (float)cnv->width ||
  maxy < 0.0f || miny >= (float)cnv->height) {
    // printf("Triangle is out of bounds so it was skipped");
    return;
  }


  int x0 = (int)floorf(minx);
  int y0 = (int)floorf(miny);
  int x1 = (int)ceilf(maxx);
  int y1 = (int)ceilf(maxy);

  if (x0 < 0) x0 = 0;
  if (y0 < 0) y0 = 0;
  if (x1 > cnv->width)  x1 = cnv->width;
  if (y1 > cnv->height) y1 = cnv->height;
  if (x0 >= x1 || y0 >= y1) return;

  // printf("BBox { %.2f, %.2f, %.2f, %.2f }\n", minx, miny, maxx, maxy);
  // printf("Points { (%d, %d), (%d, %d) }\n", x0, y0, x1, y1);

  float area = pixels_tri_edge_function(&a, &b, &c);
  float eps = 1e-32f;
  if (fabsf(area) < eps) {
    // printf("Triangle is a 'degenerate tri' so it was skipped");
    return; // Chat-GPT says this is called a "degenerate tri"
  }

  for (int y = y0; y < y1; ++y) {
    for (int x = x0; x < x1; ++x) {
      Pixels_Vector2f p = { x + 0.5f, y + 0.5f };
      float w0 = pixels_tri_edge_function(&b, &c, &p);
      float w1 = pixels_tri_edge_function(&c, &a, &p);
      float w2 = pixels_tri_edge_function(&a, &b, &p);

      // Point is inside if all edge functions have the same sign as area
      if ((w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f && area > 0.0f) ||
      (w0 <= 0.0f && w1 <= 0.0f && w2 <= 0.0f && area < 0.0f)) {
	Pixels_Vector2f pt = { x, y };
	Pixels_Vector2f ps[3] = { a, b, c };
	Pixels_Rgba cs[3] = { tri.a.color, tri.b.color, tri.c.color };
	Pixels_Rgba fill_color = pixels_barycentric_trilerp(ps, cs, pt);
	// printf("A"Pixels_RGB_Fmt" ~ "Pixels_RGB_Fmt" ~ "Pixels_RGB_Fmt" -> "Pixels_RGB_Fmt"\n", Pixels_RGB_Arg(tri.a.color), Pixels_RGB_Arg(tri.b.color), Pixels_RGB_Arg(tri.c.color), Pixels_RGB_Arg(fill_color));
        pixels_set_pixel(cnv, x, y, fill_color);
      }
    }
  }
}


// HSL 2 RGB & RGB 2 HSL transformations come from: https://gist.github.com/ciembor/1494530
Pixels_Hsla pixels_rgb2hsl(Pixels_Rgba rgb) {
  Pixels_Hsla out;

  float r = ((float)rgb.red) / 255.0f;
  float g = ((float)rgb.green) / 255.0f;
  float b = ((float)rgb.blue) / 255.0f;

  out.alpha = ((float)rgb.alpha) / 255.0f;

  float max_val = PIXELS_MAX(PIXELS_MAX(r, g), b);
  float min_val = PIXELS_MIN(PIXELS_MIN(r, g), b);

  out.hue = out.saturation = out.lightness = (max_val + min_val) / 2.0f;

  if (max_val == min_val) {
    out.hue = out.saturation = 0; // Achromatic
    return out;
  }

  float d = max_val - min_val;
  out.saturation = (out.lightness > 0.5) ? d / (2 - max_val - min_val) : d / (max_val + min_val);

  if (max_val == r) {
    out.hue = (g - b) / d + (g < b ? 6 : 0);
  } else if (max_val == g) {
    out.hue = (b - r) / d + 2;
  } else if (max_val == b) {
    out.hue = (r - g) / d + 4;
  }
  
  return out;
}

float pixels_hue2rgb(float p, float q, float t) {
  if (t < 0) 
  t += 1;
  if (t > 1) 
  t -= 1;
  if (t < 1./6) 
  return p + (q - p) * 6 * t;
  if (t < 1./2) 
  return q;
  if (t < 2./3)   
  return p + (q - p) * (2./3 - t) * 6;
  
  return p;
}

Pixels_Rgba pixels_hsl2rgb(Pixels_Hsla hsl) {
  Pixels_Rgba out = { .alpha = (unsigned char)floorf(hsl.alpha * 255) };
  if (hsl.saturation == 0) {
    out.red = out.green = out.blue = hsl.lightness; // Achromatic
    return out;
  }

  float h = hsl.hue;
  float s = hsl.saturation;
  float l = hsl.lightness;

  float q = l < 0.5 ? l * (1 + s) : (l + s) - (l * s);
  float p = 2 * l - q;
  out.red = roundf(pixels_hue2rgb(p, q, h + 1.0/3) * 255);
  out.green = roundf(pixels_hue2rgb(p, q, h) * 255);
  out.red = roundf(pixels_hue2rgb(p, q, h - 1.0/3) * 255);

  return out;
}
#endif // PIXELS_IMPLEMENTATION

#ifndef PIXELS_STRIP_GUARD_H_
  #ifdef PIXELS_STRIP_PREFIX
    #define Rgba Pixels_Rgba
    #define RGBa Pixels_RGBa
    #define RGB Pixels_RGB
    #define RGB_Fmt Pixels_RGB_Fmt
    #define RGB_Arg Pixels_RGB_Arg
    #define RGBa_Fmt Pixels_RGBa_Fmt
    #define RGBa_Arg Pixels_RGBa_Arg
    #define RED Pixels_RED
    #define GREEN Pixels_GREEN
    #define BLUE Pixels_BLUE

    #define Hsla Pixels_Hsla
    #define HSLa Pixels_HSLa
    #define HSL Pixels_HSL
    #define HSL_Fmt Pixels_HSL_Fmt
    #define HSL_Arg Pixels_HSL_Arg
    #define HSLa_Fmt Pixels_HSLa_Fmt
    #define HSLa_Arg Pixels_HSLa_Arg
    #define rgb2hsl pixels_rgb2hsl
    #define hsl2rgb pixels_hsl2rgb

    #define Vector2f Pixels_Vector2f
    #define Vec2f Pixels_Vec2f
    #define Vector2i Pixels_Vector2i
    #define Vec2i Pixels_Vec2i
    #define Vector3 Pixels_Vector3
    #define Vec3 Pixels_Vec3
    #define Vertice Pixels_Vertice
    #define Triangle Pixels_Triangle

    #define Camera Pixels_Camera
    #define default_camera pixels_default_camera
    #define calculate_perspective_projection pixels_calculate_perspective_projection

    #define Canvas Pixels_Canvas
    #define create_canvas pixels_create_canvas

    #define get_pixel pixels_get_pixel
    #define set_pixel pixels_set_pixel
    #define foreach_pixel pixels_foreach_pixel

    #define cross_2d pixels_cross_2d
    #define lerpf pixels_lerpf
    #define uchar_lerpf pixels_uchar_lerpf

    #define float_to_uchar_round_clamp pixels_float_to_uchar_round_clamp
    #define square_eucledian_dist_vec2f pixels_square_eucledian_dist_vec2f
    #define full_eucledian_dist_vec2f pixels_full_eucledian_dist_vec2f

    #define render_triangle pixels_render_triangle
  #endif // PIXELS_STRIP_PREFIX
#endif // PIXELS_STRIP_GUARD_H_

