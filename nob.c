
#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#define cmd_rsr cmd_run_sync_and_reset

bool cmd_exec_vargs(const char *program, ...) {
  static Cmd cmd = {0};
  da_append(&cmd, program);
  va_list args;
  va_start(args, program);
  for (;;) {
    const char *program_arg = va_arg(args, const char*);
    if (program_arg == NULL) break;
    da_append(&cmd, program_arg);
  }
  va_end(args);
  return cmd_rsr(&cmd);
}
#define cmd_exec(...) cmd_exec_vargs(__VA_ARGS__, NULL)

#define streq(a, b) (strcmp(a, b) == 0)

#define BUILD_FOLDER "./build"

#define PIXELS_HEADER_NAME "pixels.h"
#define PIXELS_HEADER_PATH "./"PIXELS_HEADER_NAME

const char *simp_tri_input_paths[] = {
  "./simp-triangle.c",
  PIXELS_HEADER_PATH,
};
const char *simp_tri_output_name = "simp-tri";

const char *cube_input_paths[] = {
  "./cube.c",
  PIXELS_HEADER_PATH,
};
const char *cube_output_name = "cube";

typedef struct {
  const char *output_name;
  const char **input_paths;
  size_t inputs_count;
  bool forced, run;
} Build_Config;

#define simp_tri_config(...) ((Build_Config) { .output_name = simp_tri_output_name, .input_paths = simp_tri_input_paths, .inputs_count = NOB_ARRAY_LEN(simp_tri_input_paths), __VA_ARGS__ })

#define cube_config(...) ((Build_Config) { .output_name = cube_output_name, .input_paths = cube_input_paths, .inputs_count = NOB_ARRAY_LEN(cube_input_paths), __VA_ARGS__ })

bool check_build(Cmd *cmd, Build_Config *cfg) {
  bool built = false;
  bool result = true;

  const char *output_path = temp_sprintf(BUILD_FOLDER"/%s", cfg->output_name);
  
  if (needs_rebuild(output_path, cfg->input_paths, cfg->inputs_count)) {
    nob_log(INFO, "%s requires update", cfg->output_name);
    nob_cc(cmd);
    nob_cc_flags(cmd);
    cmd_append(cmd, "-lm");
    nob_cc_output(cmd, output_path);
    for (size_t i = 0; i < cfg->inputs_count; ++i) {
      nob_cc_inputs(cmd, cfg->input_paths[i]);
    }

    result = cmd_rsr(cmd);
    built = true;
  } else {
    nob_log(INFO, "%s is up to date", cfg->output_name);
    built = false;
  }

  if (!built && cfg->forced) {
    nob_log(INFO, "%s build demanded", cfg->output_name);
    nob_cc(cmd);
    nob_cc_flags(cmd);
    cmd_append(cmd, "-lm");
    nob_cc_output(cmd, output_path);
    nob_log(INFO, "  Inputs count: %zu", cfg->inputs_count);
    for (size_t i = 0; i < cfg->inputs_count; ++i) {
      nob_cc_inputs(cmd, cfg->input_paths[i]);
    }

    result = cmd_rsr(cmd);
  }

  if (result && cfg->run) {
    cmd_append(cmd, output_path);
    if (!cmd_rsr(cmd)) return false;
  }

  return result;
}


void usage(const char *program) {
  printf("%s [-run|-B] <tri|cube|all>\n", program);
  printf("  Flags:\n");
  printf("    -run    ---    Run program after building\n");
  printf("    -B      ---    Force rebuild of program\n");
  printf("  Targets:\n");
  printf("    tri     ---     Build example triangle program\n");
  printf("    cube    ---     Build example cube program\n");
  printf("    all     ---     Build all example programs\n");
}


int main(int argc, char **argv) {
  NOB_GO_REBUILD_URSELF(argc, argv);

  const char *program_name = shift(argv, argc);

  bool should_run = false;
  bool force_rebuild = false;
  const char *target = NULL;
  
  while (argc > 0) {
    const char *arg = shift(argv, argc);
    if (streq(arg, "-run")) {
      should_run = true;
      continue;
    }
    if (streq(arg, "-B")) {
      force_rebuild = true;
      continue;
    }
    if (target != NULL && arg[0] != '-') {
      nob_log(WARNING, "Only one target can be specified at a time, last one will be picked");
    }
    if (streq(arg, "all") || streq(arg, "tri") || streq(arg, "cube")) {
      target = arg;
      continue;
    }
    if (arg[0] == '-') {
      nob_log(ERROR, "Unknown flag: %s", arg);
    } else {
      nob_log(ERROR, "Unknown target: %s", arg);
    }
    return 1;
  }

  if (!target) {
    nob_log(ERROR, "No build target was provided");
    usage(program_name);
    return 1;
  }

  bool all_targets = streq(target, "all");

  if (!mkdir_if_not_exists("build")) return 1;

  Cmd cmd = {0};

  if (all_targets || streq(target, "tri")) {
    if (!check_build(&cmd, &simp_tri_config(.forced = force_rebuild, .run = should_run))) return 1;
  }

  if (all_targets || streq(target, "cube")) {
    if (!check_build(&cmd, &cube_config(.forced = force_rebuild, .run = should_run))) return 1;
  }


  return 0;
}
